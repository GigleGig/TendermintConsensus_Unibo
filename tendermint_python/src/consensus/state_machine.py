# src/consensus/state_machine.py

import asyncio
from consensus.messages import Message
from consensus.block import Block, BlockHeader, compute_block_hash
from consensus.vote import Vote
from util.logger import logger

class ConsensusState:
    def __init__(self, validator_id, validator_set, state_store, app, p2p_node, byzantine=False):
        """
        validator_id: current node ID
        validator_set: reference to all validator sets
        state_store: local block storage
        app: application layer (execute transactions)
        p2p_node: network communication object
        byzantine: whether the current node is malicious (sending double-spending votes)
        height / round / step: Tendermint triples, used to identify the current height, round, and stage (proposal / pre-vote / pre-commit, etc.)
        There are also some important local states, such as locked_block, proposal_block, prevotes, precommits, and the committed block height set committed_heights.
        """

        self.validator_id = validator_id
        self.validator_set = validator_set
        self.state_store = state_store
        self.app = app
        self.p2p_node = p2p_node
        self.byzantine = byzantine
        self.height = 1
        self.round = 0
        self.step = "propose"
        self.locked_block = None
        self.proposal_block = None
        self.prevotes = []
        self.precommits = []
        self.committed_heights = set()  # Used to track submitted heights

    async def run(self):
        # main loop
        while True:
            msg = await self.p2p_node.receive_message()
            await self.handle_message(msg)

    # Handle_message(self, msg): Differentiate calls based on msg.type
    async def handle_message(self, msg):
        if msg.type == "timeout":
            await self.handle_timeout(msg.data["event_type"])
        elif msg.type == "propose":
            await self.handle_propose(msg)
        elif msg.type == "prevote":
            await self.handle_prevote(msg)
        elif msg.type == "precommit":
            await self.handle_precommit(msg)
        elif msg.type == "commit":
            await self.handle_commit(msg)

    async def handle_propose(self, msg):
        """
        If a proposal message is received, check the block height. If it has not been submitted, save it as a local proposal_block and start prevote_timeout.
        In Tendermint, if you are the proposer, you will call broadcast_proposal to send the proposal to all nodes.
        """

        proposal = msg.data["block"]
        proposal_height = proposal["header"]["height"]
        proposal_round = proposal["header"]["round"]

        if proposal_height in self.committed_heights:
            logger.debug(f"{self.validator_id} already committed block at height {proposal_height}, ignoring proposal.")
            return

        self.proposal_block = proposal
        self.height = proposal_height
        self.round = proposal_round
        logger.debug(f"{self.validator_id} received proposal block at H:{proposal_height} R:{proposal_round}")
        # Record the start of the pre-voting timer after receiving the proposal
        asyncio.create_task(self.set_step_timeout("prevote_timeout", 0.5))

    async def broadcast_proposal(self):
        """
        Construct a block, calculate the block hash, and then package it into a Message (type="propose",...) and send it to all validators.
        """

        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, cannot propose.")
            return

        # Get transactions from mempool
        transactions = self.app.mempool.get_txs(num=5)
        hdr = BlockHeader(height=self.height, round=self.round, proposer_id=self.validator_id, block_hash="")
        block_hash = compute_block_hash(hdr, transactions)
        hdr.block_hash = block_hash
        block = Block(header=hdr, transactions=transactions)

        proposal = {
            "header": {
                "height": block.header.height,
                "round": block.header.round,
                "proposer_id": block.header.proposer_id,
                "block_hash": block.header.block_hash
            },
            "transactions": block.transactions
        }

        msg = Message(type="propose", sender=self.validator_id, data={"block": proposal})
        logger.debug(f"{self.validator_id} broadcasting proposal for block_hash={block_hash} at H:{self.height} R:{self.round}")
        for vid in self.validator_set.validators:
            await self.p2p_node.send_message(vid, msg)

    # Switch the current phase to prevote, set a timeout prevote_timeout, and then call broadcast_prevote.
    async def enter_prevote_step(self):

        self.step = "prevote"
        # Set timeout only if not submitted
        if self.height not in self.committed_heights:
            asyncio.create_task(self.set_step_timeout("prevote_timeout", 0.5))
        await self.broadcast_prevote()

    async def broadcast_prevote(self):
        """
        Normal nodes: send a vote for the current proposal block hash or "nil"
        Byzantine nodes: intentionally send different (wrong) block_hash to different nodes
        """

        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, cannot prevote.")
            return

        block_hash = self.proposal_block["header"]["block_hash"] if self.proposal_block else "nil"
        sig = self.validator_set.sign_vote(self.validator_id, block_hash)
        vote = Vote(
            validator_id=self.validator_id,
            block_hash=block_hash,
            height=self.height,
            round=self.round,
            signature=sig
        )
        msg = Message(type="prevote", sender=self.validator_id, data={"vote": vote.to_dict()})

        if self.byzantine:
            logger.warning(f"{self.validator_id} is byzantine and sends conflicting prevotes!")
            for i, vid in enumerate(self.validator_set.validators):
                # Generate conflicting prevotes with fake hashes
                fake_hash = block_hash if i == 0 else block_hash + f"_fake_{i}"
                fake_sig = self.validator_set.sign_vote(self.validator_id, fake_hash)
                fake_vote = Vote(
                    validator_id=self.validator_id,
                    block_hash=fake_hash,
                    height=self.height,
                    round=self.round,
                    signature=fake_sig
                )
                fake_msg = Message(type="prevote", sender=self.validator_id, data={"vote": fake_vote.to_dict()})
                await self.p2p_node.send_message(vid, fake_msg)
        else:
            # Normal broadcast for honest nodes
            for vid in self.validator_set.validators:
                await self.p2p_node.send_message(vid, msg)

    async def handle_prevote(self, msg):
        """
        After receiving the Prevote message from other nodes, first check whether the vote passes verify_vote. If valid, join prevotes.
        Call check_prevote_threshold(). If the number of prevotes reaches 2/3, proceed to the next step "Precommit".
        """

        vote_data = msg.data["vote"]
        vote = Vote.from_dict(vote_data)
        
        if vote.height != self.height:
            logger.debug(f"{self.validator_id} received prevote for height {vote.height}, current height is {self.height}. Ignoring.")
            return

        if self.validator_set.verify_vote(vote):
            self.prevotes.append(vote)
            logger.debug(f"{self.validator_id} received prevote from {vote.validator_id} for block_hash={vote.block_hash} at H:{vote.height}")
            await self.check_prevote_threshold()

    async def check_prevote_threshold(self):
        # Simple check: if 2/3 prevotes are collected, then enter precommit
        needed = (2 * len(self.validator_set.validators) // 3) + 1
        if len(self.prevotes) >= needed:
            logger.debug(f"{self.validator_id} got {needed} prevotes")
            await self.enter_precommit_step()

    # Switch to precommit, set the timeout precommit_timeout, and then broadcast_precommit.
    async def enter_precommit_step(self):
        self.step = "precommit"
        # Set timeout only if not submitted
        if self.height not in self.committed_heights:
            asyncio.create_task(self.set_step_timeout("precommit_timeout", 0.5))
        await self.broadcast_precommit()
    
    # Similar to prevote, issues a "pre-commit" for the current block.
    async def broadcast_precommit(self):
        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, cannot precommit.")
            return

        block_hash = self.proposal_block["header"]["block_hash"] if self.proposal_block else "nil"
        sig = self.validator_set.sign_vote(self.validator_id, block_hash)
        vote = Vote(
            validator_id=self.validator_id,
            block_hash=block_hash,
            height=self.height,
            round=self.round,
            signature=sig
        )
        msg = Message(type="precommit", sender=self.validator_id, data={"vote": vote.to_dict()})
        logger.debug(f"{self.validator_id} broadcasting precommit for block_hash={block_hash} at H:{self.height} R:{self.round}")
        for vid in self.validator_set.validators:
            await self.p2p_node.send_message(vid, msg)

    # The same check is done after receiving the pre-commit message, counting the vote into precommits and calling check_precommit_threshold.
    async def handle_precommit(self, msg):
        vote_data = msg.data["vote"]
        vote = Vote.from_dict(vote_data)
        
        if vote.height != self.height:
            logger.debug(f"{self.validator_id} received precommit for height {vote.height}, current height is {self.height}. Ignoring.")
            return

        if self.validator_set.verify_vote(vote):
            self.precommits.append(vote)
            logger.debug(f"{self.validator_id} received precommit from {vote.validator_id} for block_hash={vote.block_hash} at H:{vote.height}")
            await self.check_precommit_threshold()

    async def check_precommit_threshold(self):
        # Simple check: if 2/3 precommits are collected, commit the block
        needed = (2 * len(self.validator_set.validators) // 3) + 1
        if len(self.precommits) >= needed:
            if self.height in self.committed_heights:
                logger.debug(f"{self.validator_id} already committed block at height {self.height}, skipping commit via precommits.")
                return
            logger.debug(f"{self.validator_id} got {needed} precommits, commit block!")
            await self.commit_block()

    async def handle_commit(self, msg):
        block_data = msg.data["block"]
        block = Block.from_dict(block_data)
        block_height = block.header.height

        if block_height in self.committed_heights:
            logger.debug(f"{self.validator_id} already committed block at height {block_height}, ignoring commit message.")
            return

        # Submit block to state
        self.state_store.commit_block(block, self.app)
        self.committed_heights.add(block_height)
        logger.info(f"{self.validator_id} committed block at height {block_height} via commit message.")

        # Update node status
        if block_height >= self.height:
            self.height = block_height + 1
            self.round = 0
            self.step = "propose"
            self.prevotes.clear()
            self.precommits.clear()
            self.proposal_block = None
            # Start the next round of proposal timer
            asyncio.create_task(self.set_step_timeout("propose_timeout", 0.5))
            # If I am the proposer, I will send the next proposal
            proposer = self.validator_set.get_proposer(self.height, self.round)
            if proposer == self.validator_id:
                await self.broadcast_proposal()

    async def commit_block(self):
        """
        Store the block in state_store, call app.apply_block to update the application state, and then broadcast the block to all nodes.
        Finally, add 1 to the local height self.height, reset some local variables, and start the next height proposal timeout.
        """

        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, skipping commit.")
            return

        # Submit block to state
        block = self.build_block_from_proposal(self.proposal_block)
        self.state_store.commit_block(block, self.app)
        self.committed_heights.add(self.height)
        logger.info(f"{self.validator_id} committed block at height {self.height}")

        # Broadcast the committed block to all nodes so that they can update their StateStore
        commit_msg = Message(type="commit", sender=self.validator_id, data={"block": block.to_dict()})
        for vid in self.validator_set.validators:
            await self.p2p_node.send_message(vid, commit_msg)

        # Update node status
        self.height += 1
        self.round = 0
        self.step = "propose"
        self.prevotes.clear()
        self.precommits.clear()
        self.proposal_block = None
        # Start the next round of proposal timer
        asyncio.create_task(self.set_step_timeout("propose_timeout", 0.5))
        # If I am the proposer, I will send the next proposal
        proposer = self.validator_set.get_proposer(self.height, self.round)
        if proposer == self.validator_id:
            await self.broadcast_proposal()

    async def check_commit_or_next_round(self):
        # If it does not reach 2/3, proceed to the next round
        logger.debug(f"{self.validator_id} not enough precommits, start next round")
        self.round += 1
        self.step = "propose"
        self.prevotes.clear()
        self.precommits.clear()
        asyncio.create_task(self.set_step_timeout("propose_timeout", 0.5))
        proposer = self.validator_set.get_proposer(self.height, self.round)
        if proposer == self.validator_id:
            await self.broadcast_proposal()

    async def handle_timeout(self, event_type):
        """
        During the Tendermint consensus process, if the waiting timeout occurs at a certain stage (no proposal or sufficient votes received), the corresponding timeout event will be triggered and the next stage or round will be entered:
        propose_timeout timeout -> enter prevote
        prevote_timeout timeout -> enter precommit
        precommit_timeout timeout -> check whether there are enough precommits, if not, enter the next round
        """

        if event_type == "propose_timeout" and self.step == "propose":
            logger.debug(f"{self.validator_id} propose timeout, move to prevote step.")
            # If no proposal is received, start prevoting an empty block or enter the next round
            await self.enter_prevote_step()
        elif event_type == "prevote_timeout" and self.step == "prevote":
            logger.debug(f"{self.validator_id} prevote timeout, move to precommit step.")
            await self.enter_precommit_step()
        elif event_type == "precommit_timeout" and self.step == "precommit":
            logger.debug(f"{self.validator_id} precommit timeout, check if we have 2/3 precommit")
            await self.check_commit_or_next_round()
  
    # Convert proposal data into Block
    def build_block_from_proposal(self, proposal):
        hdr = BlockHeader(
            height=proposal["header"]["height"],
            round=proposal["header"]["round"],
            proposer_id=proposal["header"]["proposer_id"],
            block_hash=proposal["header"]["block_hash"]
        )
        return Block(header=hdr, transactions=proposal["transactions"])

    async def set_step_timeout(self, event_type, duration):
        await asyncio.sleep(duration)
        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, ignoring timeout event.")
            return
        timeout_msg = Message(type="timeout", sender="timer", data={"event_type": event_type})
        await self.p2p_node.send_message(self.validator_id, timeout_msg)

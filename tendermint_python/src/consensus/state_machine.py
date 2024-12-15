# src/consensus/state_machine.py

import asyncio
from consensus.messages import Message
from consensus.block import Block, BlockHeader, compute_block_hash
from consensus.vote import Vote
from util.logger import logger

class ConsensusState:
    def __init__(self, validator_id, validator_set, state_store, app, p2p_node, byzantine=False):
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
        self.committed_heights = set()  # 用于跟踪已提交的高度

    async def run(self):
        # 主循环
        while True:
            msg = await self.p2p_node.receive_message()
            await self.handle_message(msg)

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
        # 记录收到提议后启动预投票定时器
        asyncio.create_task(self.set_step_timeout("prevote_timeout", 0.5))

    async def handle_prevote(self, msg):
        vote_data = msg.data["vote"]
        vote = Vote.from_dict(vote_data)
        
        if vote.height != self.height:
            logger.debug(f"{self.validator_id} received prevote for height {vote.height}, current height is {self.height}. Ignoring.")
            return

        if self.validator_set.verify_vote(vote):
            self.prevotes.append(vote)
            logger.debug(f"{self.validator_id} received prevote from {vote.validator_id} for block_hash={vote.block_hash} at H:{vote.height}")
            await self.check_prevote_threshold()

    async def handle_commit(self, msg):
        block_data = msg.data["block"]
        block = Block.from_dict(block_data)
        block_height = block.header.height

        if block_height in self.committed_heights:
            logger.debug(f"{self.validator_id} already committed block at height {block_height}, ignoring commit message.")
            return

        # 提交区块到状态
        self.state_store.commit_block(block, self.app)
        self.committed_heights.add(block_height)
        logger.info(f"{self.validator_id} committed block at height {block_height} via commit message.")

        # 更新节点状态
        if block_height >= self.height:
            self.height = block_height + 1
            self.round = 0
            self.step = "propose"
            self.prevotes.clear()
            self.precommits.clear()
            self.proposal_block = None
            # 启动下一轮提议定时器
            asyncio.create_task(self.set_step_timeout("propose_timeout", 0.5))
            # 如果我是提议者就发送下一个 proposal
            proposer = self.validator_set.get_proposer(self.height, self.round)
            if proposer == self.validator_id:
                await self.broadcast_proposal()

    async def handle_timeout(self, event_type):
        if event_type == "propose_timeout" and self.step == "propose":
            logger.debug(f"{self.validator_id} propose timeout, move to prevote step.")
            # 若未收到 proposal 则开始 prevote 空块或进入下一轮
            await self.enter_prevote_step()
        elif event_type == "prevote_timeout" and self.step == "prevote":
            logger.debug(f"{self.validator_id} prevote timeout, move to precommit step.")
            await self.enter_precommit_step()
        elif event_type == "precommit_timeout" and self.step == "precommit":
            logger.debug(f"{self.validator_id} precommit timeout, check if we have 2/3 precommit")
            await self.check_commit_or_next_round()

    async def broadcast_proposal(self):
        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, cannot propose.")
            return

        # 从 mempool 获取交易
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

    async def broadcast_prevote(self):
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

    async def enter_prevote_step(self):
        self.step = "prevote"
        # 仅在未提交时设置超时
        if self.height not in self.committed_heights:
            asyncio.create_task(self.set_step_timeout("prevote_timeout", 0.5))
        await self.broadcast_prevote()

    async def enter_precommit_step(self):
        self.step = "precommit"
        # 仅在未提交时设置超时
        if self.height not in self.committed_heights:
            asyncio.create_task(self.set_step_timeout("precommit_timeout", 0.5))
        await self.broadcast_precommit()

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

    async def check_prevote_threshold(self):
        # 简单检查：如果收集到 2/3 prevote 则进入 precommit
        needed = (2 * len(self.validator_set.validators) // 3) + 1
        if len(self.prevotes) >= needed:
            logger.debug(f"{self.validator_id} got {needed} prevotes")
            await self.enter_precommit_step()

    async def check_precommit_threshold(self):
        # 简单检查：如果收集到 2/3 precommit 则提交区块
        needed = (2 * len(self.validator_set.validators) // 3) + 1
        if len(self.precommits) >= needed:
            if self.height in self.committed_heights:
                logger.debug(f"{self.validator_id} already committed block at height {self.height}, skipping commit via precommits.")
                return
            logger.debug(f"{self.validator_id} got {needed} precommits, commit block!")
            await self.commit_block()

    async def commit_block(self):
        if self.height in self.committed_heights:
            logger.debug(f"{self.validator_id} has already committed block at height {self.height}, skipping commit.")
            return

        # 提交区块到状态
        block = self.build_block_from_proposal(self.proposal_block)
        self.state_store.commit_block(block, self.app)
        self.committed_heights.add(self.height)
        logger.info(f"{self.validator_id} committed block at height {self.height}")

        # 广播已提交的区块给所有节点，以便他们更新自己的 StateStore
        commit_msg = Message(type="commit", sender=self.validator_id, data={"block": block.to_dict()})
        for vid in self.validator_set.validators:
            await self.p2p_node.send_message(vid, commit_msg)

        # 更新节点状态
        self.height += 1
        self.round = 0
        self.step = "propose"
        self.prevotes.clear()
        self.precommits.clear()
        self.proposal_block = None
        # 启动下一轮提议定时器
        asyncio.create_task(self.set_step_timeout("propose_timeout", 0.5))
        # 如果我是提议者就发送下一个 proposal
        proposer = self.validator_set.get_proposer(self.height, self.round)
        if proposer == self.validator_id:
            await self.broadcast_proposal()

    async def check_commit_or_next_round(self):
        # 若没有达到 2/3，则进行下一轮
        logger.debug(f"{self.validator_id} not enough precommits, start next round")
        self.round += 1
        self.step = "propose"
        self.prevotes.clear()
        self.precommits.clear()
        asyncio.create_task(self.set_step_timeout("propose_timeout", 0.5))
        proposer = self.validator_set.get_proposer(self.height, self.round)
        if proposer == self.validator_id:
            await self.broadcast_proposal()

    def build_block_from_proposal(self, proposal):
        # 将 proposal 的数据转化为 Block
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

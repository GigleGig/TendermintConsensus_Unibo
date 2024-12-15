# src/main.py

import asyncio
from config import NODE_IDS
from network.p2p import Network
from consensus.state_machine import ConsensusState
from consensus.validator import ValidatorSet
from state.store import StateStore
from state.app import SimpleApp
from state.mempool import Mempool
from consensus.messages import Message
import random


def assign_byzantine_nodes(consensus_states, num_byzantine):
    """Randomly select nodes to behave as Byzantine."""
    byzantine_nodes = random.sample(list(consensus_states.keys()), num_byzantine)
    for node_id in byzantine_nodes:
        consensus_states[node_id].byzantine = True
    return byzantine_nodes


async def main():
    network = Network(NODE_IDS, delay=0.01, drop_rate=0.0)
    validator_set = ValidatorSet(NODE_IDS)
    
    # Create state stores and applications for all nodes
    state_stores = {vid: StateStore() for vid in NODE_IDS}
    apps = {vid: SimpleApp(mempool=Mempool()) for vid in NODE_IDS}

    # Initialize the mempool for all nodes and add some transactions
    for vid in NODE_IDS:
        for i in range(20):
            tx = {"action": "set", "key": f"key{i}", "value": f"value{i}"}
            apps[vid].mempool.add_tx(tx)

    # Initialize consensus states for all nodes
    consensus_states = {}
    for vid in NODE_IDS:
        p2p_node = network.get_node(vid)
        cs = ConsensusState(vid, validator_set, state_stores[vid], apps[vid], p2p_node)
        consensus_states[vid] = cs

    # Start the consensus state machine for all nodes
    tasks = [asyncio.create_task(cs.run()) for cs in consensus_states.values()]

    # Assign Byzantine behavior to some nodes
    num_byzantine = len(NODE_IDS) // 3  # Up to 1/3 of nodes can be Byzantine
    byzantine_nodes = assign_byzantine_nodes(consensus_states, num_byzantine)
    print(f"Byzantine nodes: {byzantine_nodes}")

    # Find the current proposer and broadcast the first proposal
    proposer = validator_set.get_proposer(consensus_states["node1"].height, consensus_states["node1"].round)
    proposer_cs = consensus_states[proposer]
    await proposer_cs.broadcast_proposal()

    # Trigger propose timeout for all nodes to start the consensus process
    for vid in NODE_IDS:
        p2p_node = network.get_node(vid)
        timeout_msg = Message(type="timeout", sender="timer", data={"event_type": "propose_timeout"})
        await p2p_node.send_message(vid, timeout_msg)

    # Run the system for a short time for testing
    await asyncio.sleep(5)
    for t in tasks:
        t.cancel()

if __name__ == "__main__":
    asyncio.run(main())

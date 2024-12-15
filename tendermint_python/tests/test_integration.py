# tests/test_integration.py

import pytest
import asyncio
from config import NODE_IDS
from network.p2p import Network
from consensus.state_machine import ConsensusState
from consensus.validator import ValidatorSet
from state.store import StateStore
from state.app import SimpleApp
from state.mempool import Mempool
from consensus.messages import Message

@pytest.mark.asyncio
async def test_multiple_blocks_commit():
    network = Network(NODE_IDS)
    validator_set = ValidatorSet(NODE_IDS)
    state_store = StateStore()

    # 初始化 mempool 并添加一些交易
    mempool = Mempool()
    for i in range(20):
        tx = {"action": "set", "key": f"key{i}", "value": f"value{i}"}
        mempool.add_tx(tx)

    app = SimpleApp(mempool=mempool)

    consensus_states = {}
    for vid in NODE_IDS:
        p2p_node = network.get_node(vid)
        cs = ConsensusState(vid, validator_set, state_store, app, p2p_node)
        consensus_states[vid] = cs

    tasks = [asyncio.create_task(cs.run()) for cs in consensus_states.values()]

    # 模拟提交多个区块
    for height in range(1, 4):
        proposer = validator_set.get_proposer(height, 0)
        proposer_cs = consensus_states[proposer]
        await proposer_cs.broadcast_proposal()
        for vid in NODE_IDS:
            p2p_node = network.get_node(vid)
            timeout_msg = Message(type="timeout", sender="timer", data={"event_type": "propose_timeout"})
            await p2p_node.send_message(vid, timeout_msg)
        await asyncio.sleep(2)  # 等待区块提交

    # 验证是否提交多个区块
    assert len(state_store.blocks) >= 3, "Less than 3 blocks were committed."

    for t in tasks:
        t.cancel()

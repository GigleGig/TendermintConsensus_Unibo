# tests/test_consensus.py

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
from consensus.block import Block, BlockHeader

@pytest.mark.asyncio
async def test_basic_consensus():
    network = Network(NODE_IDS)
    validator_set = ValidatorSet(NODE_IDS)
    
    # 为每个节点创建独立的 StateStore 和 SimpleApp
    state_stores = {vid: StateStore() for vid in NODE_IDS}
    apps = {vid: SimpleApp(mempool=Mempool()) for vid in NODE_IDS}
    
    # 初始化 mempool 并添加一些交易到每个节点的 mempool
    for vid in NODE_IDS:
        for i in range(20):
            tx = {"action": "set", "key": f"key{i}", "value": f"value{i}"}
            apps[vid].mempool.add_tx(tx)
    
    consensus_states = {}
    for vid in NODE_IDS:
        p2p_node = network.get_node(vid)
        cs = ConsensusState(vid, validator_set, state_stores[vid], apps[vid], p2p_node)
        consensus_states[vid] = cs

    tasks = [asyncio.create_task(cs.run()) for cs in consensus_states.values()]

    # 找到当前轮次的提议者
    proposer = validator_set.get_proposer(consensus_states["node1"].height, consensus_states["node1"].round)
    proposer_cs = consensus_states[proposer]

    # 提议者立即广播 proposal 区块
    await proposer_cs.broadcast_proposal()

    # 对所有节点发送初始 propose_timeout 事件
    for vid in NODE_IDS:
        p2p_node = network.get_node(vid)
        timeout_msg = Message(type="timeout", sender="timer", data={"event_type": "propose_timeout"})
        await p2p_node.send_message(vid, timeout_msg)

    # 运行一段时间，让共识至少提交一个块
    await asyncio.sleep(5)

    # 验证每个节点只提交了一个区块
    committed_blocks = {vid: len(state_stores[vid].blocks) for vid in NODE_IDS}
    assert all(count == 1 for count in committed_blocks.values()), "Each node should have exactly 1 committed block."

    # 验证所有节点提交的区块高度一致
    heights = {vid: state_stores[vid].blocks[0].header.height for vid in NODE_IDS if state_stores[vid].blocks}
    assert len(set(heights.values())) == 1, "All nodes should have committed blocks at the same height."

    # 验证所有节点提交的区块哈希一致
    block_hashes = {vid: state_stores[vid].blocks[0].header.block_hash for vid in NODE_IDS if state_stores[vid].blocks}
    assert len(set(block_hashes.values())) == 1, "All nodes should have committed blocks with the same hash."

    for t in tasks:
        t.cancel()

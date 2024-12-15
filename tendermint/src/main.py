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

async def main():
    network = Network(NODE_IDS, delay=0.01, drop_rate=0.0)
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

    # 运行一段时间后退出（仅测试用）
    await asyncio.sleep(5)
    for t in tasks:
        t.cancel()

if __name__ == "__main__":
    asyncio.run(main())

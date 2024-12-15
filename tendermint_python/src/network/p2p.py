# src/network/p2p.py

import asyncio
import random

class P2PNode:
    def __init__(self, node_id, network):
        self.node_id = node_id
        self.network = network
        self.inbox = asyncio.Queue()

    async def receive_message(self):
        return await self.inbox.get()

    async def send_message(self, target_id, message):
        await self.network.send_message(target_id, message)

class Network:
    def __init__(self, node_ids, delay=0.01, drop_rate=0.0):
        self.nodes = {nid: P2PNode(nid, self) for nid in node_ids}
        self.delay = delay
        self.drop_rate = drop_rate
        self.partitioned = False  # 是否模拟网络分区
        self.partitions = {}  # 分区详情，暂未实现

    async def send_message(self, target_id, message):
        if self.partitioned and target_id in self.partitions.get("isolated", []):
            # 模拟网络分区，消息被隔离
            return
        # 故障注入：延迟发送
        await asyncio.sleep(self.delay)
        # 故障注入：概率丢包
        if random.random() > self.drop_rate:
            await self.nodes[target_id].inbox.put(message)

    def get_node(self, node_id):
        return self.nodes[node_id]
    
    def partition_network(self, isolated_nodes):
        """Isolate specific nodes in the network."""
        self.partitioned = True
        self.partitions = {"isolated": isolated_nodes}

    def heal_partition(self):
        """Restore normal network functionality."""
        self.partitioned = False
        self.partitions.clear()

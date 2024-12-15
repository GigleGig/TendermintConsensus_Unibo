# tests/test_network.py

import pytest
import asyncio
from network.p2p import Network

@pytest.mark.asyncio
async def test_message_delivery():
    node_ids = ["node1", "node2"]
    network = Network(node_ids, delay=0.1, drop_rate=0.0)
    node1 = network.get_node("node1")
    node2 = network.get_node("node2")
    
    msg = {"type": "test", "sender": "node1", "data": {"content": "hello"}}
    await node1.send_message("node2", msg)
    
    received_msg = await node2.receive_message()
    assert received_msg == msg, "Message was not delivered correctly."

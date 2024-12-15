import asyncio
from consensus.messages import Message

async def set_timeout(p2p_node, event_type, duration):
    await asyncio.sleep(duration)
    timeout_msg = Message(type="timeout", sender="timer", data={"event_type": event_type})
    # 假设p2p_node代表发起超时事件给自身
    await p2p_node.send_message(p2p_node.node_id, timeout_msg)

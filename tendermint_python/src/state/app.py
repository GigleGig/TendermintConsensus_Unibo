# src/state/app.py

import json

class SimpleApp:
    def __init__(self, mempool):
        self.mempool = mempool
    
    def apply_block(self, block, state_store):
        for tx_str in block.transactions:
            tx = json.loads(tx_str)
            if tx.get("action") == "set":
                state_store.app_state[tx["key"]] = tx["value"]
        state_store.blocks.append(block)

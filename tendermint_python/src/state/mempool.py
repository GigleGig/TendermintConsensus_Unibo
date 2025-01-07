# src/state/mempool.py

import json

class Mempool:
    """
    Responsible for managing the transaction list: add_tx, get_txs, size.
    When a node becomes a proposer, it will take out several transactions (5 by default) from the mempool to form a block.
    """
    
    def __init__(self):
        self.txs = []
    
    def add_tx(self, tx):
        # tx can be a dictionary, converted to json string storage
        if isinstance(tx, dict):
            tx_str = json.dumps(tx)
        else:
            tx_str = tx
        self.txs.append(tx_str)
    
    def get_txs(self, num=5):
        # Get at most num transactions from mempool, if not enough, return all
        # For simplicity, take from the front and delete after taking
        batch = self.txs[:num]
        self.txs = self.txs[num:]
        return batch
    
    def size(self):
        return len(self.txs)

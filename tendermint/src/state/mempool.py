# src/state/mempool.py

import json

class Mempool:
    def __init__(self):
        self.txs = []
    
    def add_tx(self, tx):
        # tx 可以是一个字典，转为json字符串存储
        if isinstance(tx, dict):
            tx_str = json.dumps(tx)
        else:
            tx_str = tx
        self.txs.append(tx_str)
    
    def get_txs(self, num=5):
        # 从mempool中获取最多num笔交易，如果不足则全部返回
        # 简单起见，从前面取，取完就删除
        batch = self.txs[:num]
        self.txs = self.txs[num:]
        return batch
    
    def size(self):
        return len(self.txs)

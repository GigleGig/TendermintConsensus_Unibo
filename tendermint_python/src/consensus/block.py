# src/consensus/block.py

import hashlib
import json

class BlockHeader:
    def __init__(self, height, round, proposer_id, block_hash):
        self.height = height
        self.round = round
        self.proposer_id = proposer_id
        self.block_hash = block_hash

    def to_dict(self):
        return {
            "height": self.height,
            "round": self.round,
            "proposer_id": self.proposer_id,
            "block_hash": self.block_hash
        }

    @staticmethod
    def from_dict(data):
        return BlockHeader(
            height=data["height"],
            round=data["round"],
            proposer_id=data["proposer_id"],
            block_hash=data["block_hash"]
        )

class Block:
    def __init__(self, header, transactions):
        self.header = header
        self.transactions = transactions

    def to_dict(self):
        return {
            "header": self.header.to_dict(),
            "transactions": self.transactions
        }

    @staticmethod
    def from_dict(data):
        header = BlockHeader.from_dict(data["header"])
        transactions = data["transactions"]
        return Block(header, transactions)

def compute_block_hash(header, transactions):
    """
    Calculates the block hash value based on the block header and transaction data.

    :param header: BlockHeader instance
    :param transactions: transaction list in the block
    :return: block hash string
    """
    # Serialize block headers and transaction data into JSON strings, ensuring the keys are in the same order
    block_content = {
        "header": header.to_dict(),
        "transactions": transactions
    }
    block_bytes = json.dumps(block_content, sort_keys=True).encode()
    # Calculate hash value using SHA-256
    return hashlib.sha256(block_bytes).hexdigest()

# consensus/vote.py

class Vote:
    def __init__(self, validator_id, block_hash, height, round, signature):
        self.validator_id = validator_id
        self.block_hash = block_hash
        self.height = height
        self.round = round
        self.signature = signature

    def to_dict(self):
        return {
            "validator_id": self.validator_id,
            "block_hash": self.block_hash,
            "height": self.height,
            "round": self.round,
            "signature": self.signature
        }

    @staticmethod
    def from_dict(data):
        return Vote(
            validator_id=data["validator_id"],
            block_hash=data["block_hash"],
            height=data["height"],
            round=data["round"],
            signature=data["signature"]
        )
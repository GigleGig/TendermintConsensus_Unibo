# src/consensus/validator.py

class ValidatorSet:
    def __init__(self, validators):
        # validators 是一个列表，如 ["node1", "node2", "node3", "node4"]
        self.validators = validators
    
    def get_proposer(self, height, round_):
        # 简化：依据 height + round 的模数选出提议者
        index = (height + round_) % len(self.validators)
        return self.validators[index]
    
    def is_validator(self, vid):
        return vid in self.validators
    
    def sign_vote(self, validator_id, block_hash):
        # 简单签名：f"{validator_id}:{block_hash}"
        return f"{validator_id}:{block_hash}"
    
    def verify_vote(self, vote):
        # 简单校验，检查签名格式和验证者身份
        if not self.is_validator(vote.validator_id):
            return False
        expected_sig = f"{vote.validator_id}:{vote.block_hash}"
        return vote.signature == expected_sig

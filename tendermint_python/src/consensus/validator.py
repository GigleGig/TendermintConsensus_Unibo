# src/consensus/validator.py

class ValidatorSet:
    def __init__(self, validators):
        # validators is a list, such as ["node1", "node2", "node3", "node4"]
        self.validators = validators
    
    def get_proposer(self, height, round_):
        # Simplified: select the proposer based on the modulus of height + round
        index = (height + round_) % len(self.validators)
        return self.validators[index]
    
    def is_validator(self, vid):
        return vid in self.validators
    
    def sign_vote(self, validator_id, block_hash):
        # Simple signature: f"{validator_id}:{block_hash}"
        return f"{validator_id}:{block_hash}"
    
    def verify_vote(self, vote):
        # Simple verification, check signature format and verifier identity
        if not self.is_validator(vote.validator_id):
            return False
        expected_sig = f"{vote.validator_id}:{vote.block_hash}"
        return vote.signature == expected_sig

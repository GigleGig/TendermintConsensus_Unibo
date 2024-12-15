# src/state/abci_app.py

from util.logger import logger
import json
import hashlib

class ABCIApplication:
    def __init__(self, state_store):
        self.state_store = state_store
        self.pending_validator_updates = []
        self.validators = []

    def init_chain(self, validators):
        # 初始化链时，记录验证者集合
        self.validators = validators.copy()

    def begin_block(self, height):
        # 区块开始逻辑，清除上轮的 pending_validator_updates
        self.pending_validator_updates.clear()

    def deliver_tx(self, tx_bytes):
        # 将交易字节转化为 JSON
        try:
            tx = json.loads(tx_bytes)
        except json.JSONDecodeError:
            logger.warning("Invalid transaction format.")
            return False

        action = tx.get("action")
        if action == "transfer":
            sender = tx.get("sender")
            receiver = tx.get("receiver")
            amount = tx.get("amount", 0)
            if (sender not in self.state_store.app_state) or (receiver not in self.state_store.app_state):
                logger.warning("Transfer failed: unknown accounts.")
                return False
            if self.state_store.app_state[sender] < amount:
                logger.warning("Transfer failed: insufficient funds.")
                return False
            self.state_store.app_state[sender] -= amount
            self.state_store.app_state[receiver] += amount
            logger.info(f"Transfer {amount} from {sender} to {receiver} applied.")
            return True

        elif action == "add_validator":
            # 特殊治理交易，用于添加新的验证者
            validator_id = tx.get("validator_id")
            if validator_id and validator_id not in self.validators:
                self.pending_validator_updates.append({"validator_id": validator_id, "power": 10})
                logger.info(f"Request to add validator {validator_id} received.")
                return True
            else:
                logger.warning("Add_validator failed: invalid or existing validator.")
                return False

        elif action == "set":
            # 简单的 KV set
            key = tx.get("key")
            value = tx.get("value")
            sender = tx.get("sender")
            if key and isinstance(value, str):
                self.state_store.app_state[key] = value
                logger.info(f"Set {key}={value} by {sender}.")
                return True
            else:
                logger.warning("Set transaction failed: invalid key or value.")
                return False

        # 未知交易类型
        logger.warning("Unknown transaction action.")
        return False

    def end_block(self, height):
        # 如果有 pending 的 validator_update，在 end_block 返回
        validator_updates = []
        for vu in self.pending_validator_updates:
            v_id = vu["validator_id"]
            power = vu.get("power", 10)
            # 在实际 Tendermint 中，ValidatorUpdate 包含更多信息，如 pubkey
            validator_updates.append({"validator_id": v_id, "power": power})
            self.validators.append(v_id)  # 更新本地验证者列表
        return validator_updates

    def commit(self):
        # 提交区块后返回 state 的哈希（简化）
        app_state_str = json.dumps(self.state_store.app_state, sort_keys=True)
        state_hash = hashlib.sha256(app_state_str.encode('utf-8')).hexdigest()
        logger.info(f"Committing block, state_hash={state_hash}.")
        self.state_store.height += 1
        return state_hash

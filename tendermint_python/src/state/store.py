# src/state/store.py

class StateStore:
    def __init__(self):
        self.blocks = []
        self.app_state = {}

    def commit_block(self, block, app):
        self.blocks.append(block)
        # 简化应用执行：假设每个交易是 {"action":"set","key":"k","value":"v"}
        app.apply_block(block, self)

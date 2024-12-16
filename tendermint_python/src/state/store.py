# src/state/store.py

class StateStore:
    def __init__(self):
        self.blocks = []
        self.app_state = {}

    def commit_block(self, block, app):
        self.blocks.append(block)
        # Simplified application execution: Assume that each transaction is {"action":"set","key":"k","value":"v"}
        app.apply_block(block, self)

# src/consensus/messages.py

from dataclasses import dataclass
from typing import Dict

@dataclass
class Message:
    type: str   # "propose", "prevote", "precommit", "timeout"
    sender: str
    data: Dict

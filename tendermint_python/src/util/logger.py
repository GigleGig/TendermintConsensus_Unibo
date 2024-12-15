# src/util/logger.py

import logging

logger = logging.getLogger("tendermint_sim")
logger.setLevel(logging.DEBUG)  # 可以根据需要调整为 INFO

ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)  # 可以根据需要调整为 INFO
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
logger.addHandler(ch)

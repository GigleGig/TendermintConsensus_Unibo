#include "Blockchain.h"

Blockchain::Blockchain() {
    // 创建创世区块
    Block genesisBlock(0, "0", {});
    chain.push_back(genesisBlock);
}

void Blockchain::addBlock(const Block& newBlock) {
    if (isValidNewBlock(newBlock, getLatestBlock())) {
        chain.push_back(newBlock);
    }
}

const Block& Blockchain::getLatestBlock() const {
    return chain.back();
}

bool Blockchain::isValidNewBlock(const Block& newBlock, const Block& previousBlock) const {
    if (previousBlock.getIndex() + 1 != newBlock.getIndex()) {
        return false;
    }
    if (previousBlock.getHash() != newBlock.getPreviousHash()) {
        return false;
    }
    return true;
}

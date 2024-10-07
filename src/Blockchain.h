#pragma once

#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "Block.h"
#include <vector>

class Blockchain {
public:
    Blockchain();

    void addBlock(const Block& newBlock);
    const Block& getLatestBlock() const;

private:
    std::vector<Block> chain;

    bool isValidNewBlock(const Block& newBlock, const Block& previousBlock) const;
};

#endif

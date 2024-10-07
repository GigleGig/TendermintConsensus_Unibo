#include "Block.h"
#include <sstream>
#include <openssl/sha.h>

Block::Block(int index, const std::string& previousHash, const std::vector<std::string>& transactions)
    : index(index), previousHash(previousHash), transactions(transactions) {
    hash = calculateHash();
}

std::string Block::calculateHash() const {
    std::stringstream ss;
    ss << index << previousHash;
    for (const auto& tx : transactions) {
        ss << tx;
    }

    std::string input = ss.str();
    unsigned char hashBytes[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input.c_str(), input.size(), hashBytes);

    std::stringstream hashString;
    for (unsigned char byte : hashBytes) {
        hashString << std::hex << (int)byte;
    }

    return hashString.str();
}

std::string Block::getHash() const {
    return hash;
}

int Block::getIndex() const {
    return index;
}

std::string Block::getPreviousHash() const {
    return previousHash;
}

std::vector<std::string> Block::getTransactions() const {
    return transactions;
}

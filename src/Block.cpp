#include "Block.h"
#include <sstream>
#include <openssl/sha.h>

Block::Block(int index, const std::string& previousHash, const std::vector<Transaction>& transactions)
    : index(index), previousHash(previousHash), transactions(transactions) {
    
    // Calculate hash
    hash = calculateHash();
}

std::string Block::calculateHash() const {
    std::stringstream ss;
    ss << index << previousHash;

    // Iterate over each transaction and add the string representation of the transaction to the hash input
    for (const auto& tx : transactions) {
        ss << tx.toString();
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

const std::vector<Transaction>& Block::getTransactions() const {
    return transactions;
}

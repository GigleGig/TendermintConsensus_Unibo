#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <vector>
#include "Transaction.h"

class Block {
public:
    Block(int index, const std::string& previousHash, const std::vector<Transaction>& transactions);

    std::string getHash() const;
    int getIndex() const;
    std::string getPreviousHash() const;

    // Return to transaction list
    const std::vector<Transaction>& getTransactions() const; 

private:
    int index;
    std::string previousHash;
    std::vector<Transaction> transactions;
    std::string hash;

    std::string calculateHash() const;
};

#endif

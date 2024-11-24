#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <vector>
#include "Transaction.h"  // 添加对 Transaction 类的包含

class Block {
public:
    Block(int index, const std::string& previousHash, const std::vector<Transaction>& transactions);

    std::string getHash() const;
    int getIndex() const;
    std::string getPreviousHash() const;
    const std::vector<Transaction>& getTransactions() const; // 返回交易列表

private:
    int index;
    std::string previousHash;
    std::vector<Transaction> transactions;
    std::string hash;

    std::string calculateHash() const;
};

#endif

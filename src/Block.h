#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <vector>

class Block {
public:
    Block(int index, const std::string& previousHash, const std::vector<std::string>& transactions);

    std::string getHash() const;
    int getIndex() const;
    std::string getPreviousHash() const;
    std::vector<std::string> getTransactions() const; // 添加 getTransactions() 方法

private:
    int index;
    std::string previousHash;
    std::vector<std::string> transactions;
    std::string hash;

    std::string calculateHash() const;
};

#endif

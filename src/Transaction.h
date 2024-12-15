#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <sstream>

class Transaction {
public:
    Transaction(int senderId, int receiverId, double amount);

    int getSenderId() const;
    int getReceiverId() const;
    double getAmount() const;

    std::string toString() const;

private:
    int senderId;
    int receiverId;
    double amount;
};

#endif

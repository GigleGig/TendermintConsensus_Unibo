#include "Transaction.h"
#include <sstream>

Transaction::Transaction(int senderId, int receiverId, double amount)
    : senderId(senderId), receiverId(receiverId), amount(amount) {}

int Transaction::getSenderId() const {
    return senderId;
}

int Transaction::getReceiverId() const {
    return receiverId;
}

double Transaction::getAmount() const {
    return amount;
}

std::string Transaction::toString() const {
    std::ostringstream ss;
    ss << "Transaction from Node " << senderId << " to Node " << receiverId << " of amount " << amount;
    return ss.str();
}

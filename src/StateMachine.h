#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Transaction.h"
#include <map>
#include <string>
#include <vector>

class StateMachine {
public:
    StateMachine();

    void applyTransactions(const std::vector<Transaction>& transactions);
    void createSnapshot();
    void rollback();
    double getBalance(int nodeId) const;
    void printState() const;

private:
    std::map<int, double> balances;
    std::vector<std::map<int, double>> snapshots; // 存储状态快照
};

#endif

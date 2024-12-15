#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "Transaction.h"
#include <unordered_map>
#include <vector>

class StateMachine {
public:
    StateMachine();

    void applyTransactions(const std::vector<Transaction>& transactions); // 直接应用交易（传统）
    void prepareState(const std::vector<Transaction>& transactions);      // 准备交易状态（ABCI 样式）
    void commitState();                                                   // 提交准备的状态
    void rollbackState();                                                 // 回滚到上一个状态
    double getBalance(int nodeId) const;                                  // 获取节点余额
    void createSnapshot();                                                // 创建快照
    void printState() const;                                              // 打印当前状态
    bool StateMachine::canProcessTransaction(const Transaction& tx) const;
    bool isCommitSuccessful() const; // New method to check commit success

private:
    std::unordered_map<int, double> balances;        // 节点账户余额
    std::unordered_map<int, double> pendingBalances; // 准备中的状态
    std::vector<std::unordered_map<int, double>> snapshots; // 快照历史
    
};

#endif

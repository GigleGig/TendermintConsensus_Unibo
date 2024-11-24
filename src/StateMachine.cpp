#include "StateMachine.h"
#include "Utils.h"
#include <iostream>

StateMachine::StateMachine() {
    // Initialize the account balance of all nodes, for example, the initial balance is 1000
    balances[1] = 1000.0;
    balances[2] = 1000.0;
    balances[3] = 1000.0;
    balances[4] = 1000.0;
}

void StateMachine::applyTransactions(const std::vector<Transaction>& transactions) {
    for (const auto& tx : transactions) {
        int sender = tx.getSenderId();
        int receiver = tx.getReceiverId();
        double amount = tx.getAmount();

        // Check if the balance is sufficient
        if (balances[sender] >= amount) {
            balances[sender] -= amount;
            balances[receiver] += amount;
            Utils::log("Transaction applied: " + tx.toString());
        } else {
            Utils::log("Transaction failed due to insufficient balance: " + tx.toString());
        }
    }
}

void StateMachine::createSnapshot() {
    snapshots.push_back(balances);
    Utils::log("State snapshot created.");
}

void StateMachine::rollback() {
    if (!snapshots.empty()) {
        balances = snapshots.back();
        snapshots.pop_back();
        Utils::log("State rolled back to previous snapshot.");
    }
}

double StateMachine::getBalance(int nodeId) const {
    auto it = balances.find(nodeId);
    if (it != balances.end()) {
        return it->second;
    }
    return 0.0;
}

void StateMachine::printState() const {
    std::cout << "Current State:" << std::endl;
    for (const auto& balance : balances) {
        std::cout << "Node " << balance.first << ": Balance = " << balance.second << std::endl;
    }
}

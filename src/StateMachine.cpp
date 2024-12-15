#include "StateMachine.h"
#include "Utils.h"
#include <iostream>

StateMachine::StateMachine() {
    // 初始化节点的账户余额
    balances[1] = 1000.0;
    balances[2] = 1000.0;
    balances[3] = 1000.0;
    balances[4] = 1000.0;
}

void StateMachine::applyTransactions(const std::vector<Transaction>& transactions) {
    try {
        for (const auto& tx : transactions) {
            if (balances[tx.getSenderId()] >= tx.getAmount()) {
                balances[tx.getSenderId()] -= tx.getAmount();
                balances[tx.getReceiverId()] += tx.getAmount();
            } else {
                Utils::log("Transaction failed: insufficient balance for sender " + std::to_string(tx.getSenderId()));
                throw std::runtime_error("Transaction failed: insufficient balance.");
            }
        }
    } catch (const std::exception& e) {
        Utils::log("Transaction processing error: " + std::string(e.what()));
        throw; // Ensure rollback is triggered
    }
}

void StateMachine::prepareState(const std::vector<Transaction>& transactions) {
    pendingBalances = balances; // Copy current state to pending state
    for (const auto& tx : transactions) {
        int sender = tx.getSenderId();
        int receiver = tx.getReceiverId();
        double amount = tx.getAmount();

        if (pendingBalances[sender] < amount) {
            Utils::log("Transaction preparation failed: insufficient balance for sender " + std::to_string(sender));
            return; // Log failure and exit the function without committing changes
        }
        pendingBalances[sender] -= amount;
        pendingBalances[receiver] += amount;
    }
    Utils::log("Transactions prepared successfully.");
}


void StateMachine::commitState() {
    try {
        balances = pendingBalances;
        pendingBalances.clear();
    } catch (const std::exception& e) {
        Utils::log("Error during state commit: " + std::string(e.what()));
    }
}

bool StateMachine::isCommitSuccessful() const {
    return pendingBalances.empty(); // Example logic
}


void StateMachine::rollbackState() {
    if (!snapshots.empty()) {
        balances = snapshots.back(); // Restore the last snapshot
        snapshots.pop_back();
        Utils::log("State rollback completed.");
    } else {
        Utils::log("No snapshots available to rollback.");
    }
}

double StateMachine::getBalance(int nodeId) const {
    auto it = balances.find(nodeId);
    if (it != balances.end()) {
        return it->second;
    }
    return 0.0; // Default to 0 if the node ID is not found
}

void StateMachine::createSnapshot() {
    snapshots.push_back(balances); // Save the current state as a snapshot
    Utils::log("State snapshot created.");
}

void StateMachine::printState() const {
    Utils::log("Current State:");
    for (const auto& [nodeId, balance] : balances) {
        std::cout << "  Node " << nodeId << ": Balance = " << balance << "\n";
    }

    // Check for newly added nodes without explicit balances
    for (int nodeId = 1; nodeId <= balances.size(); ++nodeId) {
        if (balances.find(nodeId) == balances.end()) {
            std::cout << "  Node " << nodeId << ": Balance = 0\n";
        }
    }
}

bool StateMachine::canProcessTransaction(const Transaction& tx) const {
    auto it = balances.find(tx.getSenderId());
    if (it != balances.end() && it->second >= tx.getAmount()) {
        return true; // Sufficient balance
    }
    return false; // Insufficient balance
}
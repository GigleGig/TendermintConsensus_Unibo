#include "Node.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Node::Node(int id, Network* network, StateMachine* stateMachine)
    : id(id), network(network), consensus(this, stateMachine), stateMachine(stateMachine) {
    // Consensus 初始化时传递 stateMachine
}

int Node::getId() const {
    return id;
}

void Node::receiveMessage(const Message& message) {
    // 处理接收到的消息逻辑
    Utils::log("Node " + std::to_string(id) + " received message: " + message.getContent());
}

void Node::proposeBlock() {
    // 节点发起区块提议的逻辑
    Utils::log("Node " + std::to_string(id) + " is proposing a new block.");
    consensus.startConsensus();  // 启动共识流程
}

void Node::sendMessageToAll(const Message& message) {
    if (network) {
        network->broadcastMessage(message);
    }
}

void Node::rollbackConsensus() {
    Utils::log("Node " + std::to_string(id) + " is rolling back consensus.");
    consensus.rollbackConsensus();  // 通过 Consensus 类调用回滚方法
}

void Node::printStatus(std::ostream& os) const {
    os << "Node ID: " << id << std::endl;
    os << "Blockchain length: " << blockchain.getChainLength() << std::endl;
    os << "Consensus stage: " << consensus.getCurrentStageAsString() << std::endl;

    if (stateMachine) {
        double balance = stateMachine->getBalance(id);
        os << "Current balance: " << balance << std::endl;  // 输出当前余额
    } else {
        os << "StateMachine not initialized." << std::endl;
    }

    os << "Pending transactions: " << std::endl;
    for (const auto& tx : pendingTransactions) {
        os << "  - " << tx.toString() << std::endl;
    }
}

void Node::createTransaction(int receiverId, double amount) {
    if (id == receiverId) {
        Utils::log("Transaction to self is not allowed.");
        return;
    }

    Transaction tx(id, receiverId, amount);
    pendingTransactions.push_back(tx);
    Utils::log("Transaction created: " + tx.toString());
}

const std::vector<Transaction>& Node::getPendingTransactions() const {
    return pendingTransactions;
}

void Node::clearPendingTransactions() {
    pendingTransactions.clear();
}

Blockchain& Node::getBlockchain() {
    return blockchain;
}
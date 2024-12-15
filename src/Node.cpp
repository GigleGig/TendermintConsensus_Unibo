#include "Node.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Node::Node(int id, Network* network, StateMachine* stateMachine)
    : id(id), network(network), stateMachine(stateMachine), consensus(this, stateMachine) {}

int Node::getId() const {
    return id;
}

void Node::receiveMessage(const Message& message) {
    Utils::log("Node " + std::to_string(id) + " received message: " + message.getContent());
    consensus.onReceiveMessage(message);
}

void Node::proposeBlock() {
    // Node initiating block proposal
    Utils::log("Node " + std::to_string(id) + " is proposing a new block.");
    consensus.startConsensus();  // Start the consensus process
}

void Node::sendMessageToAll(const Message& message) {
    if (network) {
        network->broadcastMessage(message);
    }
}

void Node::rollbackConsensus() {
    Utils::log("Node " + std::to_string(id) + " is rolling back consensus.");
    consensus.rollbackConsensus();  
}

void Node::printStatus(std::ostream& os) const {
    os << "Node ID: " << id << std::endl;
    os << "Blockchain length: " << blockchain.getChainLength() << std::endl;
    os << "Consensus stage: " << consensus.getCurrentStageAsString() << std::endl;

    if (stateMachine) {
        double balance = stateMachine->getBalance(id);
        os << "Current balance: " << balance << std::endl;  // Output current balance
    } else {
        os << "StateMachine not initialized." << std::endl;
    }

    os << "Pending transactions: " << std::endl;
    for (const auto& tx : pendingTransactions) {
        os << "  - " << tx.toString() << std::endl;
    }
}

void Node::createTransaction(int receiverId, double amount) {
    if (stateMachine->getBalance(this->id) < amount) {
        Utils::log("Transaction failed: insufficient balance.");
        return;
    }

    Transaction transaction(this->id, receiverId, amount);
    pendingTransactions.push_back(transaction);
    Utils::log("Transaction created: " + transaction.toString());

    // Directly add transaction to Consensus
    consensus.addPendingTransaction(transaction);
}



const std::vector<Transaction>& Node::getPendingTransactions() const {
    return pendingTransactions;
}

void Node::clearPendingTransactions() {
    if (consensus.getCurrentStageAsString() == "FINALIZED") {
        pendingTransactions.clear();
    } else {
        Utils::log("Transactions not cleared: Consensus is not finalized.");
    }
}

Blockchain& Node::getBlockchain() {
    return blockchain;
}

Network* Node::getNetwork() const {
    return network;
}
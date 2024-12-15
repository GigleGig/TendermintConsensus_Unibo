#include "Network.h"
#include "Node.h" // Include the full definition
#include "Utils.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

Network::Network() 
    : messageDropRate(0.0), maxDelayMs(0), randomGenerator(std::random_device{}()) {}

Network::Network(StateMachine* stateMachine)
    : messageDropRate(0.0), maxDelayMs(0), randomGenerator(std::random_device{}()), stateMachine(stateMachine) {}

void Network::registerNode(Node* node) {
    nodes.push_back(node);
    Utils::log("Node registered in the network.");
}

size_t Network::getTotalNodes() const {
    return nodes.size();
}

void Network::setMessageDropRate(double rate) {
    // if (rate < 0.0 || rate > 1.0) {
    //     Utils::log("Invalid message drop rate. Must be between 0.0 and 1.0.");
    //     return;
    // }
    // messageDropRate = rate;
    messageDropRate = 0.0; // Set to zero
}

void Network::setMaxDelayMs(int delayMs) {
    // if (delayMs < 0) {
    //     Utils::log("Invalid delay. Must be non-negative.");
    //     return;
    // }
    // maxDelayMs = delayMs;
    maxDelayMs = 0; // Set to zero
}

bool Network::shouldDropMessage() {
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(randomGenerator) < messageDropRate;
}

int Network::generateDelay() {
    std::uniform_int_distribution<int> distribution(0, maxDelayMs);
    return distribution(randomGenerator);
}

void Network::broadcastMessage(const Message& message) {
    for (Node* node : nodes) {
        if (node && node->getId() == message.getSenderId()) continue;

        if (!node) {
            Utils::log("[ERROR] Null node encountered during broadcast.");
            continue;
        }

        int attempts = 0;
        while (attempts < 3 && shouldDropMessage()) {
            attempts++;
            Utils::log("Message dropped: " + message.getContent() + " to Node " + std::to_string(node->getId()));
        }

        if (attempts >= 3) continue;

        int delay = generateDelay();
        Utils::log("Message delayed by " + std::to_string(delay) + " ms to Node " + std::to_string(node->getId()));
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        node->receiveMessage(message);
    }

    Utils::log("Network broadcast complete for message: " + message.getContent());
}

void Network::addNode(Node* node) {
    nodes.push_back(node);
    if (stateMachine) {
        stateMachine->prepareState({Transaction(0, node->getId(), 0)}); // Initialize the new node's balance
    }
    Utils::log("Node " + std::to_string(node->getId()) + " added to the network.");
}

void Network::addTransaction(const Transaction& transaction) {
    globalPendingTransactions.push_back(transaction);
}

bool Network::hasPendingTransactions() const {
    return !globalPendingTransactions.empty();
}
#ifndef NETWORK_H
#define NETWORK_H

#include "Message.h"
#include "Node.h"
#include "StateMachine.h"
#include <vector>
#include <memory>
#include <random>

class Node; // Forward declaration

class Network {
public:

    Network();
    Network(StateMachine* stateMachine); // Update constructor to take StateMachine

    void registerNode(Node* node); // Register a node in the network
    void broadcastMessage(const Message& message); // Broadcast a message to all nodes
    void addNode(Node* node); // Add a dynamically created node to the network

    void setMessageDropRate(double rate); // Set the message drop rate
    void setMaxDelayMs(int delayMs); // Set the maximum delay in ms
    size_t getTotalNodes() const; // Get the total number of nodes
    bool hasPendingTransactions() const;
    void addTransaction(const Transaction& transaction);

    const std::vector<Node*>& getNodes() const {
        return nodes;
    }

private:
    std::vector<Node*> nodes; // Nodes in the network
    double messageDropRate; // Message drop rate (0.0 to 1.0)
    int maxDelayMs; // Maximum network delay (in milliseconds)
    std::mt19937 randomGenerator; // Random number generator for network simulation
    StateMachine* stateMachine; // Pointer to the StateMachine
    std::vector<Transaction> globalPendingTransactions;

    bool shouldDropMessage(); // Decide whether to drop a message
    int generateDelay(); // Generate a random delay
};

#endif

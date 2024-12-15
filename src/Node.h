#ifndef NODE_H
#define NODE_H

#include "Blockchain.h"
#include "Network.h"
#include "Message.h"
#include "Consensus.h"
#include "StateMachine.h"
#include <string>
#include <iostream>
#include <vector>

class Network; // Forward declaration to avoid circular dependency

class Node {
public:
    Node(int id, Network* network, StateMachine* stateMachine);

    int getId() const;
    void receiveMessage(const Message& message);
    void proposeBlock();
    void handleConsensus();
    void sendMessageToAll(const Message& message);
    void rollbackConsensus();
    void printStatus(std::ostream& os = std::cout) const;

    void createTransaction(int receiverId, double amount);
    const std::vector<Transaction>& getPendingTransactions() const;
    void clearPendingTransactions();
    Blockchain& getBlockchain();
    Network* getNetwork() const;

private:
    int id;
    Blockchain blockchain;
    Network* network;
    Consensus consensus; // Ensure 'Consensus' is fully defined in the header
    
    StateMachine* stateMachine;
    std::vector<Transaction> pendingTransactions;

    void processProposal(const Message& message);
};

#endif // NODE_H

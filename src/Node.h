#ifndef NODE_H
#define NODE_H

#include "Blockchain.h"
#include "Network.h"
#include "Message.h"
#include "Consensus.h"
#include "StateMachine.h"
#include <string>
#include <iostream>

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

private:
    int id;
    Blockchain blockchain;
    Network* network;
    Consensus consensus;
    StateMachine* stateMachine;  // 添加状态机指针，用于获取节点余额
    std::vector<Transaction> pendingTransactions;

    void processProposal(const Message& message);
};

#endif

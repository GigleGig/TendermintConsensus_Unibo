#ifndef NODE_H
#define NODE_H

#include "Blockchain.h"
#include "Network.h"
#include "Message.h"
#include "Consensus.h"
#include <string>

class Node {
public:
    Node(int id, Network* network);

    int getId() const;
    void receiveMessage(const Message& message);
    void proposeBlock();
    void handleConsensus();
    void sendMessageToAll(const Message& message);

private:
    int id;
    Blockchain blockchain;
    Network* network;
    Consensus consensus;

    void processProposal(const Message& message);
};

#endif

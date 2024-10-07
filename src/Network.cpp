#include "Network.h"
#include "Node.h"
#include "Utils.h"
#include <iostream>

void Network::registerNode(Node* node) {
    nodes.push_back(node);
    Utils::log("Node registered in the network.");
}


size_t Network::getTotalNodes() const {
    return nodes.size();
}


void Network::broadcastMessage(const Message& message) {
    for (Node* node : nodes) {
        if (node && node->getId() != message.getSenderId()) {
            // Call receiveMessage to receive a message
            node->receiveMessage(message);  
        }
    }
    Utils::log("Network broadcast message from Node " + std::to_string(message.getSenderId()) + ": " + message.getContent());
}
#include "Network.h"
#include "Node.h"
#include "Utils.h"
#include <iostream>

void Network::registerNode(Node* node) {
    nodes.push_back(node);
    Utils::log("Node registered in the network.");
}


size_t Network::getTotalNodes() const {
    return nodes.size();  // 使用 size_t 作为返回类型
}


void Network::broadcastMessage(const Message& message) {
    for (Node* node : nodes) {
        if (node && node->getId() != message.getSenderId()) {
            node->receiveMessage(message);  // 这里调用 receiveMessage 以接收消息
        }
    }
    Utils::log("Network broadcast message from Node " + std::to_string(message.getSenderId()) + ": " + message.getContent());
}
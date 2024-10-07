#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include "Message.h"

class Node;

class Network {
public:
    void registerNode(Node* node);
    void broadcastMessage(const Message& message);
    size_t getTotalNodes() const;  // 将返回类型从 int 改为 size_t

private:
    std::vector<Node*> nodes;
};

#endif

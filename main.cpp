#include "Node.h"
#include "Network.h"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

int main() {
    Network network;

    // Creating multiple nodes
    std::vector<std::unique_ptr<Node>> nodes;
    int nodeCount = 4;
    for (int i = 0; i < nodeCount; ++i) {
        nodes.push_back(std::make_unique<Node>(i + 1, &network));
    }

    // Registering nodes to the network
    for (auto& node : nodes) {
        network.registerNode(node.get());
    }

    // Simulating multiple rounds of consensus process
    for (int round = 0; round < 5; ++round) {
        std::cout << "Starting consensus round " << round + 1 << std::endl;

        // Let each node take turns to be the proposer
        nodes[round % nodeCount]->proposeBlock();

        // Wait for each round of consensus to end
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // Analog Delay
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}

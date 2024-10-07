#include "Node.h"
#include "Network.h"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

int main() {
    Network network;

    // 创建多个节点
    std::vector<std::unique_ptr<Node>> nodes;
    int nodeCount = 4;
    for (int i = 0; i < nodeCount; ++i) {
        nodes.push_back(std::make_unique<Node>(i + 1, &network));
    }

    // 注册节点到网络中
    for (auto& node : nodes) {
        network.registerNode(node.get());
    }

    // 模拟多轮共识过程
    for (int round = 0; round < 5; ++round) {
        std::cout << "Starting consensus round " << round + 1 << std::endl;

        // 让每个节点轮流成为提议者
        nodes[round % nodeCount]->proposeBlock();

        // 等待每轮共识结束
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // 模拟延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}

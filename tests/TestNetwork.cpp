#include <gtest/gtest.h>
#include "Network.h"
#include "Node.h"

TEST(NetworkTest, NetworkRegisterNode) {
    Network network;
    StateMachine stateMachine;  // 创建状态机实例

    // 使用三个参数创建节点实例
    Node node1(1, &network, &stateMachine);

    // 没有具体返回值，通过日志检查
    network.registerNode(&node1);
}

TEST(NetworkTest, NetworkBroadcastMessage) {
    Network network;
    StateMachine stateMachine;  // 创建状态机实例

    // 使用三个参数创建节点实例
    Node node1(1, &network, &stateMachine);
    Node node2(2, &network, &stateMachine);

    // 注册节点到网络中
    network.registerNode(&node1);
    network.registerNode(&node2);

    // 通过日志或节点状态进行验证
    Message message(PROPOSAL, 1, "TestBroadcast");
    network.broadcastMessage(message);
}

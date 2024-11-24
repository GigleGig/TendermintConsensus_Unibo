#include <gtest/gtest.h>
#include "Node.h"
#include "Network.h"

TEST(NodeTest, NodeInitialization) {
    Network network;
    StateMachine stateMachine;  // 创建状态机实例

    // 使用三个参数创建节点
    Node node(1, &network, &stateMachine);
    EXPECT_EQ(node.getId(), 1);
}

TEST(NodeTest, NodeReceiveMessage) {
    Network network;
    StateMachine stateMachine;  // 创建状态机实例

    // 使用三个参数创建节点
    Node node(1, &network, &stateMachine);

    // 通过观察日志或其他方式验证节点接收消息的逻辑
    Message message(PROPOSAL, 2, "TestProposal");
    node.receiveMessage(message);
}

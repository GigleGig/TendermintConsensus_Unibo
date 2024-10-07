#include <gtest/gtest.h>
#include "Node.h"
#include "Network.h"

TEST(NodeTest, NodeInitialization) {
    Network network;
    Node node(1, &network);
    EXPECT_EQ(node.getId(), 1);
}

TEST(NodeTest, NodeReceiveMessage) {
    Network network;
    Node node(1, &network);
    Message message(PROPOSAL, 2, "TestProposal");
    node.receiveMessage(message);
    // 可以通过观察日志或增加函数返回值来验证节点处理逻辑
}

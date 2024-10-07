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

    // Verifying the node processing logic by observing the log or increasing the function return value
    Message message(PROPOSAL, 2, "TestProposal");
    node.receiveMessage(message);
}

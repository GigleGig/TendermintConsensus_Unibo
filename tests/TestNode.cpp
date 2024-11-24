#include <gtest/gtest.h>
#include "Node.h"
#include "Network.h"

TEST(NodeTest, NodeInitialization) {
    Network network;
    StateMachine stateMachine;

    Node node(1, &network, &stateMachine);
    EXPECT_EQ(node.getId(), 1);
}

TEST(NodeTest, NodeReceiveMessage) {
    Network network;
    StateMachine stateMachine;

    Node node(1, &network, &stateMachine);

    Message message(PROPOSAL, 2, "TestProposal");
    node.receiveMessage(message);
}

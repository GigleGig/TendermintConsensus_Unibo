#include <gtest/gtest.h>
#include "Network.h"
#include "Node.h"

TEST(NetworkTest, NetworkRegisterNode) {
    Network network;
    StateMachine stateMachine;

    Node node1(1, &network, &stateMachine);

    network.registerNode(&node1);
}

TEST(NetworkTest, NetworkBroadcastMessage) {
    Network network;
    StateMachine stateMachine;

    Node node1(1, &network, &stateMachine);
    Node node2(2, &network, &stateMachine);

    network.registerNode(&node1);
    network.registerNode(&node2);

    Message message(PROPOSAL, 1, "TestBroadcast");
    network.broadcastMessage(message);
}

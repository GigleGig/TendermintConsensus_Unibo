#include <gtest/gtest.h>
#include "Network.h"
#include "Node.h"

TEST(NetworkTest, NetworkRegisterNode) {
    Network network;
    Node node1(1, &network);

    // No specific return value, check it through the log
    network.registerNode(&node1);
}

TEST(NetworkTest, NetworkBroadcastMessage) {
    Network network;
    Node node1(1, &network);
    Node node2(2, &network);
    network.registerNode(&node1);
    network.registerNode(&node2);

    // Verify through logs or node status
    Message message(PROPOSAL, 1, "TestBroadcast");
    network.broadcastMessage(message);
}

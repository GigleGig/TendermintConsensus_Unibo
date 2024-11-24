#include <gtest/gtest.h>
#include "Consensus.h"
#include "Node.h"
#include "StateMachine.h"
#include "Network.h"

TEST(ConsensusTest, ConsensusProposal) {
    Network network;
    StateMachine stateMachine;

    // 创建 Node 实例时传入 stateMachine 指针
    Node node(1, &network, &stateMachine);
    Consensus consensus(&node, &stateMachine);

    // Verify by checking if the proposal message was broadcast correctly
    consensus.startConsensus();
    
}

TEST(ConsensusTest, ConsensusHandleMessages) {
    Network network;
    StateMachine stateMachine;

    // 创建 Node 实例时传入 stateMachine 指针
    Node node(1, &network, &stateMachine);
    Consensus consensus(&node, &stateMachine);

    // Verify correct processing through logging
    Message proposalMessage(PROPOSAL, 2, "Block_1");
    consensus.onReceiveMessage(proposalMessage);
}

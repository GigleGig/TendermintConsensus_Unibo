#ifndef CONSENSUS_H
#define CONSENSUS_H

#include "Message.h"
#include "StateMachine.h"
#include <vector>
#include <string>

class Node;

class Consensus {
public:
    Consensus(Node* node, StateMachine* stateMachine);

    void startConsensus();
    void onReceiveMessage(const Message& message);
    void rollbackConsensus();
    std::string getCurrentStageAsString() const; // 声明 getCurrentStageAsString 方法

private:
    Node* node;
    StateMachine* stateMachine;

    enum class ConsensusStage {
        PROPOSAL,
        PREVOTE,
        PRECOMMIT,
        FINALIZED
    };

    ConsensusStage currentStage;
    int proposalBlockIndex;
    std::string proposalHash;
    std::vector<int> prevotesReceived;
    std::vector<int> precommitsReceived;

    void handleProposal(const Message& message);
    void handlePrevote(const Message& message);
    void handlePrecommit(const Message& message);
    void initiateProposal();
    void broadcastMessage(MessageType type, const std::string& content);
    void checkForTimeout();
    void finalizeConsensus();
};

#endif

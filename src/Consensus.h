#ifndef CONSENSUS_H
#define CONSENSUS_H

#include "Message.h"
#include "StateMachine.h"
#include <unordered_set>
#include <vector>
#include <string>

class Node; // Forward declaration to avoid circular dependency
class Network;

enum class ConsensusStage {
    PROPOSAL,
    PREVOTE,
    PRECOMMIT,
    FINALIZED
};

class Consensus {
public:
    Consensus(Node* node, StateMachine* stateMachine);

    void startConsensus();
    void onReceiveMessage(const Message& message);
    std::string getCurrentStageAsString() const;
    void rollbackConsensus();
    void Consensus::addPendingTransaction(const Transaction& transaction);

private:
    Node* node;                    // Pointer to the node
    StateMachine* stateMachine;    // Pointer to the state machine
    ConsensusStage currentStage;   // Current stage of the consensus
    std::string proposalHash;      // Hash of the proposal
    size_t proposalBlockIndex;     // Index of the proposal block
    size_t currentLeaderId;        // Current leader ID
    size_t retryCount;             // Retry count for consensus
    size_t threshold;              // Dynamic threshold for consensus
    std::unordered_set<size_t> prevotesReceived;
    std::unordered_set<size_t> precommitsReceived;
    std::unordered_set<size_t> byzantineNodes;
    std::vector<Transaction> pendingTransactions;

    void Consensus::waitForNewTransactions();
    void initiateProposal();
    void broadcastMessage(MessageType type, const std::string& content);
    void handleProposal(const Message& message);
    void handlePrevote(const Message& message);
    void handlePrecommit(const Message& message);
    void checkForTimeout();
    void finalizeConsensus();
    void electNewLeader();
    bool isQuorumReached(const std::unordered_set<size_t>& votes, size_t quorumThreshold);
};

#endif // CONSENSUS_H

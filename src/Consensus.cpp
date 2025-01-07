#include "Consensus.h"
#include "Node.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>

// Define MAX_RETRIES if not already defined
#ifndef MAX_RETRIES
#define MAX_RETRIES 5
#endif

Consensus::Consensus(Node* node, StateMachine* stateMachine)
    : node(node),
      stateMachine(stateMachine),
      currentStage(ConsensusStage::PROPOSAL),
      proposalBlockIndex(0),
      currentLeaderId(-1),
      retryCount(0),
      threshold(0) { // Default threshold is 0, dynamically calculated
}

void Consensus::startConsensus() {
    try {
        size_t totalNodes = node->getNetwork()->getTotalNodes();
        threshold = (2 * totalNodes) / 3 + 1; // Calculate 2/3 majority dynamically
        Utils::log("Threshold for consensus set to " + std::to_string(threshold) + " out of " + std::to_string(totalNodes) + " nodes.");

        // Always re-fetch transactions from Node
        pendingTransactions = node->getPendingTransactions();

        if (!pendingTransactions.empty()) {
            initiateProposal();
        } else {
            Utils::log("No transactions available for consensus. Waiting...");
        }
    } catch (const std::exception& e) {
        Utils::log("Exception in startConsensus: " + std::string(e.what()));
    }
}

void Consensus::onReceiveMessage(const Message& message) {
    switch (message.getType()) {
        case PROPOSAL:
            handleProposal(message);
            break;
        case PREVOTE:
            handlePrevote(message);
            break;
        case PRECOMMIT:
            handlePrecommit(message);
            break;
        default:
            Utils::log("Unknown message type received by Consensus");
    }
}

void Consensus::initiateProposal() {
    electNewLeader();
    if (node->getId() == currentLeaderId) {
        Utils::log("Node " + std::to_string(node->getId()) + " is the leader. Proposing a new block.");
        Utils::log("Transactions in Node (before proposal): " + std::to_string(node->getPendingTransactions().size()));

        proposalHash = "Block_" + std::to_string(proposalBlockIndex++);
        pendingTransactions = node->getPendingTransactions(); // Transfer transactions
        Utils::log("Transactions being proposed (Consensus): " + std::to_string(pendingTransactions.size()));

        if (stateMachine) {
            stateMachine->createSnapshot();
        }

        broadcastMessage(MessageType::PROPOSAL, proposalHash);
        currentStage = ConsensusStage::PREVOTE;
    } else {
        Utils::log("Node " + std::to_string(node->getId()) + " is waiting for proposal from leader.");
    }
}

void Consensus::broadcastMessage(MessageType type, const std::string& content) {
    if (node) {
        Message message(type, node->getId(), content);
        node->sendMessageToAll(message);
    }
}

void Consensus::handleProposal(const Message& message) {
    Utils::log("Node " + std::to_string(node->getId()) + " received proposal from Node " + std::to_string(message.getSenderId()) + ": " + message.getContent());

    currentStage = ConsensusStage::PREVOTE;
    proposalHash = message.getContent();

    broadcastMessage(MessageType::PREVOTE, proposalHash);
}

void Consensus::handlePrevote(const Message& message) {
    Utils::log("Node " + std::to_string(node->getId()) + " received prevote from Node " + std::to_string(message.getSenderId()));

    if (byzantineNodes.count(message.getSenderId())) {
        Utils::log("Prevote from Byzantine Node " + std::to_string(message.getSenderId()) + " ignored.");
        return;
    }

    prevotesReceived.insert(message.getSenderId());

    if (isQuorumReached(prevotesReceived, threshold)) {
        Utils::log("Quorum reached for PREVOTE. Broadcasting PRECOMMIT.");
        currentStage = ConsensusStage::PRECOMMIT;
        broadcastMessage(MessageType::PRECOMMIT, proposalHash);
    } else {
        checkForTimeout();
    }
}

void Consensus::handlePrecommit(const Message& message) {
    Utils::log("Node " + std::to_string(node->getId()) + " received precommit from Node " + std::to_string(message.getSenderId()));

    if (byzantineNodes.count(message.getSenderId())) {
        Utils::log("Precommit from Byzantine Node " + std::to_string(message.getSenderId()) + " ignored.");
        return;
    }

    precommitsReceived.insert(message.getSenderId());

    if (isQuorumReached(precommitsReceived, threshold)) {
        Utils::log("Quorum reached for PRECOMMIT. Finalizing consensus.");
        finalizeConsensus();
    } else {
        checkForTimeout();
    }
}

void Consensus::checkForTimeout() {
    if (retryCount < MAX_RETRIES) {
        retryCount++;
        Utils::log("Retrying consensus, attempt " + std::to_string(retryCount));

        // Retain pending transactions
        if (pendingTransactions.empty() && !node->getPendingTransactions().empty()) {
            pendingTransactions = node->getPendingTransactions();
            Utils::log("Re-synchronized transactions during retry. Size: " + std::to_string(pendingTransactions.size()));
        }

        startConsensus();
    } else {
        Utils::log("Consensus failed after maximum retries. Exiting consensus loop.");
        retryCount = 0;
        rollbackConsensus();
    }
}

void Consensus::waitForNewTransactions() {
    while (node->getPendingTransactions().empty()) {
        Utils::log("No new transactions. Waiting...");
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Check every 500ms
    }

    Utils::log("New transactions detected. Resuming consensus.");
    pendingTransactions = node->getPendingTransactions(); // Retrieve transactions
    startConsensus(); // Restart consensus with new transactions
}

void Consensus::finalizeConsensus() {
    Utils::log("Consensus finalized for block: " + proposalHash);

    if (!pendingTransactions.empty()) {
        Utils::log("Transactions before processing (Consensus): " + std::to_string(pendingTransactions.size()));
        stateMachine->prepareState(pendingTransactions);
        stateMachine->commitState();
        stateMachine->printState();

        pendingTransactions.clear();
        node->clearPendingTransactions();
    } else {
        Utils::log("No transactions to process.");
    }

    Utils::log("Ready for the next round of consensus.");
    startConsensus();
}


void Consensus::rollbackConsensus() {
    Utils::log("Consensus failed and state has been rolled back.");

    if (stateMachine) {
        stateMachine->rollbackState();
    }

    // Rebroadcast pending transactions
    for (const auto& transaction : pendingTransactions) {
        broadcastMessage(MessageType::PROPOSAL, transaction.toString());
    }

    Utils::log("Restarting consensus after rollback...");
    startConsensus();
}

std::string Consensus::getCurrentStageAsString() const {
    switch (currentStage) {
        case ConsensusStage::PROPOSAL:
            return "PROPOSAL";
        case ConsensusStage::PREVOTE:
            return "PREVOTE";
        case ConsensusStage::PRECOMMIT:
            return "PRECOMMIT";
        case ConsensusStage::FINALIZED:
            return "FINALIZED";
        default:
            return "UNKNOWN";
    }
}

bool Consensus::isQuorumReached(const std::unordered_set<size_t>& votes, size_t quorumThreshold) {
    return votes.size() >= quorumThreshold;
}

void Consensus::electNewLeader() {
    currentLeaderId = (currentLeaderId + 1) % node->getNetwork()->getTotalNodes();
    Utils::log("New leader elected: Node " + std::to_string(currentLeaderId));
}

void Consensus::addPendingTransaction(const Transaction& transaction) {
    pendingTransactions.push_back(transaction);
    Utils::log("Transaction added. Current pending size: " + std::to_string(pendingTransactions.size()));
}

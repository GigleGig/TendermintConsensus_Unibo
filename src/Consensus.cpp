#include "Consensus.h"
#include "Node.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>

Consensus::Consensus(Node* node, StateMachine* stateMachine)
    : node(node), stateMachine(stateMachine), currentStage(ConsensusStage::PROPOSAL), proposalBlockIndex(-1) {}

void Consensus::startConsensus() {
    initiateProposal();
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
    Utils::log("Node starting consensus - initiating proposal");

    // Assume that the proposer is this node itself
    currentStage = ConsensusStage::PROPOSAL;
    proposalBlockIndex++;

    // Simple hash simulation
    proposalHash = "Block_" + std::to_string(proposalBlockIndex); 

    // Create a state snapshot
    if (stateMachine) {
        stateMachine->createSnapshot();
    }

    // Send proposal message
    broadcastMessage(MessageType::PROPOSAL, proposalHash);
}

void Consensus::broadcastMessage(MessageType type, const std::string& content) {
    if (node) {
        Message message(type, node->getId(), content);
        node->sendMessageToAll(message);
    }
}

void Consensus::handleProposal(const Message& message) {
    Utils::log("Node received proposal from Node " + std::to_string(message.getSenderId()) + ": " + message.getContent());

    currentStage = ConsensusStage::PREVOTE;
    proposalHash = message.getContent();

    //Send pre-voting message
    broadcastMessage(MessageType::PREVOTE, proposalHash);
}

void Consensus::handlePrevote(const Message& message) {
    Utils::log("Node received prevote from Node " + std::to_string(message.getSenderId()));
    prevotesReceived.push_back(message.getSenderId());

    // If a majority of prevotes are collected, enter the precommit phase
    if (prevotesReceived.size() >= 2) { // Simple majority judgment
        currentStage = ConsensusStage::PRECOMMIT;
        broadcastMessage(MessageType::PRECOMMIT, proposalHash);
    } else {
        // Check if timeout processing is required
        checkForTimeout(); 
    }
}

void Consensus::handlePrecommit(const Message& message) {
    Utils::log("Node received precommit from Node " + std::to_string(message.getSenderId()));
    precommitsReceived.push_back(message.getSenderId());

    // If most precommits are collected, enter the finalized phase
    if (precommitsReceived.size() >= 2) {
        finalizeConsensus();
    } else {
        checkForTimeout();
    }
}

void Consensus::checkForTimeout() {
    // In a project, a timeout may be set. Here I pass it.
    Utils::log("Consensus stage timed out, attempting to retry or rollback.");

    // If timeout, rollback is required
    rollbackConsensus();
}

void Consensus::finalizeConsensus() {
    Utils::log("Consensus finalized for block: " + proposalHash);
    currentStage = ConsensusStage::FINALIZED;

    // Generate a set of dummy transactions for testing 
    // Transactions can be obtained from the proposed block
    std::vector<Transaction> transactions = {
        // Send from current node to next node
        Transaction(node->getId(), (node->getId() % 4) + 1, 100.0) 
    };

    // Apply the transaction to the state machine
    if (stateMachine) {
        stateMachine->applyTransactions(transactions);
    }

    // Print current state
    stateMachine->printState();

    // Start consensus for the next block
    startConsensus();
}

void Consensus::rollbackConsensus() {

    // Record the number of failures
    // static int failureCount = 0; 

    if (stateMachine) {
        stateMachine->rollback();
    }
    Utils::log("Consensus failed and state has been rolled back.");
    
    // TODO: Can choose to restart consensus or perform other processing
    // If restart, then
    // Utils::log("Attempting to restart consensus after rollback.");
    // startConsensus();

    // else can send a rollback message to other nodes in the network
    // broadcastMessage(MessageType::ROLLBACK, proposalHash);

    // Enter the observation state to avoid frequent retries
    // Utils::log("Node entering observation mode after failed consensus.");

    // Wait for a while and try to start consensus again
    // std::this_thread::sleep_for(std::chrono::seconds(5)); 
    // startConsensus();

    // failureCount++;

    // If the number of consecutive failures reaches the threshold, an alarm is triggered
    // if (failureCount >= 3) {
    //     Utils::log("WARNING: Consensus failed multiple times, manual intervention may be needed.");
    //     failureCount = 0; // Reset Counter
    // } else {
    //     Utils::log("Retrying consensus...");
    //     startConsensus(); // Restart consensus
    // }
}

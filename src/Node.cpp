#include "Node.h"
#include "Utils.h"
#include "StateMachine.h"
#include <iostream>

Node::Node(int id, Network* network)
    : id(id), network(network), consensus(this, new StateMachine()) {}

int Node::getId() const {
    return id;
}

void Node::receiveMessage(const Message& message) {
    Utils::log("Node " + std::to_string(id) + " received message from Node " + std::to_string(message.getSenderId()) + ": " + message.getContent());
    consensus.onReceiveMessage(message);
}

void Node::proposeBlock() {
    consensus.startConsensus();
}

void Node::handleConsensus() {
    Utils::log("Node " + std::to_string(id) + " is handling consensus.");
    consensus.startConsensus();
}

void Node::sendMessageToAll(const Message& message) {
    if (network) {
        network->broadcastMessage(message);
    }
}

void Node::processProposal(const Message& message) {
    Utils::log("Node " + std::to_string(id) + " processing proposal: " + message.getContent());
}

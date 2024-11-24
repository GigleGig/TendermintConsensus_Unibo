#include "Node.h"
#include "Network.h"
#include "StateMachine.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

int main() {
    Network network;

    // Create multiple nodes, each node shares a state machine
    StateMachine stateMachine;
    std::vector<std::unique_ptr<Node>> nodes;

    int nodeCount = 4;
    for (int i = 0; i < nodeCount; ++i) {
        nodes.push_back(std::make_unique<Node>(i + 1, &network, &stateMachine));
    }

    // Registering nodes to the network
    for (auto& node : nodes) {
        network.registerNode(node.get());
    }

    // Start the interactive client
    std::string command;
    while (true) {
        std::cout << "Enter command (help for a list of commands): ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        } else if (command == "help") {
            std::cout << "Commands:\n";
            std::cout << "  start <node_id>  - Start consensus from a specific node\n";
            std::cout << "  rollback <node_id> - Rollback the consensus state for a node\n";
            std::cout << "  status <node_id> - Show the current state of a node\n";
            std::cout << "  create_transaction <sender_id> <receiver_id> <amount> - Create a transaction from one node to another\n";
            std::cout << "  exit - Exit the program\n";
        } else if (command.find("start") == 0) {
            int nodeId = std::stoi(command.substr(6));
            if (nodeId > 0 && nodeId <= nodeCount) {
                nodes[nodeId - 1]->proposeBlock();
            } else {
                std::cout << "Invalid node ID.\n";
            }
        } else if (command.find("rollback") == 0) {
            int nodeId = std::stoi(command.substr(9));
            if (nodeId > 0 && nodeId <= nodeCount) {
                nodes[nodeId - 1]->rollbackConsensus();
            } else {
                std::cout << "Invalid node ID.\n";
            }
        } else if (command.find("status") == 0) {
            int nodeId = std::stoi(command.substr(7));
            if (nodeId > 0 && nodeId <= nodeCount) {
                nodes[nodeId - 1]->printStatus();
            } else {
                std::cout << "Invalid node ID.\n";
            }
        } else if (command.find("create_transaction") == 0) {
            std::istringstream ss(command);
            std::string token;
            int senderId, receiverId;
            double amount;

            ss >> token >> senderId >> receiverId >> amount;

            if (senderId > 0 && senderId <= nodeCount && receiverId > 0 && receiverId <= nodeCount) {
                nodes[senderId - 1]->createTransaction(receiverId, amount);
            } else {
                std::cout << "Invalid node ID or transaction parameters.\n";
            }
        } else {
            std::cout << "Unknown command. Type 'help' for a list of commands.\n";
        }
    }

    return 0;
}

#include "Node.h"
#include "Network.h"
#include "StateMachine.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip> // For formatting output
#include "Utils.h"

int main() {
    // Shared state machine and initial nodes
    StateMachine stateMachine;
    std::vector<std::unique_ptr<Node>> nodes;

    Network network(&stateMachine); // Pass StateMachine to the Network

    // Configure network parameters
    network.setMessageDropRate(0.01);  // 10% message drop rate
    network.setMaxDelayMs(200);      // Max delay of 200 milliseconds

    // Initialize 4 nodes
    int initialNodeCount = 4;
    for (int i = 0; i < initialNodeCount; ++i) {
        nodes.push_back(std::make_unique<Node>(i + 1, &network, &stateMachine));
    }

    // Register initial nodes in the network
    for (auto& node : nodes) {
        network.registerNode(node.get());
    }

    // Interactive client
    std::string command;
    while (true) {
        std::cout << "Enter command (help for a list of commands): ";
        std::getline(std::cin, command);

        try {
            if (command == "exit") {
                break;
            } else if (command == "help") {
                std::cout << "Commands:\n";
                std::cout << "  start <node_id>  - Start consensus from a specific node\n";
                std::cout << "  rollback <node_id> - Rollback the consensus state for a node\n";
                std::cout << "  status <node_id> - Show the current state of a node\n";
                std::cout << "  create_transaction <sender_id> <receiver_id> <amount> - Create a transaction\n";
                std::cout << "  add_node - Add a new node dynamically to the network\n";
                std::cout << "  exit - Exit the program\n";
            } else if (command.find("start") == 0) {
                int nodeId = std::stoi(command.substr(6));
                if (nodeId > 0 && nodeId <= static_cast<int>(nodes.size())) {
                    try {
                        nodes[nodeId - 1]->proposeBlock();
                    } catch (const std::exception& e) {
                        std::cerr << "[ERROR] Exception occurred during consensus: " << e.what() << std::endl;
                    }
                } else {
                    std::cout << "Invalid node ID. Please enter a value between 1 and " << nodes.size() << ".\n";
                }
            } else if (command.find("rollback") == 0) {
                int nodeId = std::stoi(command.substr(9));
                if (nodeId > 0 && nodeId <= static_cast<int>(nodes.size())) {
                    nodes[nodeId - 1]->rollbackConsensus();
                } else {
                    std::cout << "Invalid node ID. Please enter a value between 1 and " << nodes.size() << ".\n";
                }
            } else if (command == "status_all") {
                std::cout << "Current status of all nodes:\n";
                for (const auto& node : nodes) {
                    std::ostringstream status;
                    node->printStatus(status);
                    std::cout << status.str() << std::endl;
                }
            } else if (command.find("status") == 0) {
                int nodeId = std::stoi(command.substr(7));
                if (nodeId > 0 && nodeId <= static_cast<int>(nodes.size())) {
                    std::ostringstream status;
                    nodes[nodeId - 1]->printStatus(status);
                    std::cout << status.str() << std::endl;
                } else {
                    std::cout << "Invalid node ID. Please enter a value between 1 and " << nodes.size() << ".\n";
                }
            } else if (command.find("create_transaction") == 0) {
                try {
                    int senderId, receiverId;
                    double amount;
                    std::istringstream ss(command);
                    std::string token;
                    ss >> token >> senderId >> receiverId >> amount;

                    if (senderId > 0 && senderId <= static_cast<int>(nodes.size()) && receiverId > 0 && receiverId <= static_cast<int>(nodes.size())) {
                        nodes[senderId - 1]->createTransaction(receiverId, amount);
                    } else {
                        std::cout << "Invalid node ID or transaction parameters. Please check your input.\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "Error processing transaction: " << e.what() << "\n";
                }
            } else if (command == "add_node") {
                int newId = static_cast<int>(network.getTotalNodes() + 1); // Dynamically assign an ID
                auto newNode = std::make_unique<Node>(newId, &network, &stateMachine);
                network.registerNode(newNode.get());
                stateMachine.prepareState({Transaction(0, newId, 0)}); // Start with a default balance
                nodes.push_back(std::move(newNode));
                std::cout << "Node " << newId << " added to the network.\n";
            } else {
                std::cout << "Unknown command. Type 'help' for a list of commands.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error processing command: " << e.what() << "\n";
        }
    }

    return 0;
}

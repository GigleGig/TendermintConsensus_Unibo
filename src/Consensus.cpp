#include "Consensus.h"
#include "Node.h"
#include "Utils.h"
#include <iostream>

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

    // 假设提议者是这个节点自己
    currentStage = ConsensusStage::PROPOSAL;
    proposalBlockIndex++;
    proposalHash = "Block_" + std::to_string(proposalBlockIndex); // 简单哈希模拟

    // 创建状态快照
    if (stateMachine) {
        stateMachine->createSnapshot();
    }

    // 发送提案消息
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

    // 发送预投票消息
    broadcastMessage(MessageType::PREVOTE, proposalHash);
}

void Consensus::handlePrevote(const Message& message) {
    Utils::log("Node received prevote from Node " + std::to_string(message.getSenderId()));
    prevotesReceived.push_back(message.getSenderId());

    // 如果收集到多数 prevotes，则进入 precommit 阶段
    if (prevotesReceived.size() >= 2) { // 简单多数判断
        currentStage = ConsensusStage::PRECOMMIT;
        broadcastMessage(MessageType::PRECOMMIT, proposalHash);
    } else {
        checkForTimeout(); // 检查是否需要超时处理
    }
}

void Consensus::handlePrecommit(const Message& message) {
    Utils::log("Node received precommit from Node " + std::to_string(message.getSenderId()));
    precommitsReceived.push_back(message.getSenderId());

    // 如果收集到多数 precommits，则进入 finalized 阶段
    if (precommitsReceived.size() >= 2) { // 简单多数判断
        finalizeConsensus();
    } else {
        checkForTimeout(); // 检查是否需要超时处理
    }
}

void Consensus::checkForTimeout() {
    // 在真实实现中，可能会设定一个超时时间
    Utils::log("Consensus stage timed out, attempting to retry or rollback.");

    // 如果超时，需要回滚
    rollbackConsensus();
}

void Consensus::finalizeConsensus() {
    Utils::log("Consensus finalized for block: " + proposalHash);
    currentStage = ConsensusStage::FINALIZED;

    // 生成一组虚拟交易进行测试（可以从提议的区块中获取交易）
    std::vector<Transaction> transactions = {
        Transaction(node->getId(), (node->getId() % 4) + 1, 100.0) // 从当前节点发送到下一个节点
    };

    // 将交易应用到状态机
    if (stateMachine) {
        stateMachine->applyTransactions(transactions);
    }

    // 打印当前状态
    stateMachine->printState();

    // 开始下一个区块的共识
    startConsensus();
}

void Consensus::rollbackConsensus() {
    if (stateMachine) {
        stateMachine->rollback();
    }
    Utils::log("Consensus failed and state has been rolled back.");
    // 你可以选择重新启动共识或者进行其他处理
}

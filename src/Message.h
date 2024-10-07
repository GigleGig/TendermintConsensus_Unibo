#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

enum MessageType {
    PROPOSAL,
    PREVOTE,
    PRECOMMIT
};

class Message {
public:
    Message(MessageType type, int senderId, const std::string& content);

    MessageType getType() const;
    int getSenderId() const;
    std::string getContent() const;

private:
    MessageType type;
    int senderId;
    std::string content;
};

#endif

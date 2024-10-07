#include "Message.h"

Message::Message(MessageType type, int senderId, const std::string& content)
    : type(type), senderId(senderId), content(content) {}

MessageType Message::getType() const {
    return type;
}

int Message::getSenderId() const {
    return senderId;
}

std::string Message::getContent() const {
    return content;
}

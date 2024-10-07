#include "Utils.h"
#include <iostream>
#include <openssl/sha.h>
#include <sstream>

std::string Utils::calculateHash(const std::string& input) {
    unsigned char hashBytes[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input.c_str(), input.size(), hashBytes);

    std::stringstream hashString;
    for (unsigned char byte : hashBytes) {
        hashString << std::hex << (int)byte;
    }

    return hashString.str();
}

void Utils::log(const std::string& message) {
    std::cout << "[LOG] " << message << std::endl;
}

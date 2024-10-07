#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <string>

class Utils {
public:
    static std::string calculateHash(const std::string& input);
    static void log(const std::string& message);
};

#endif

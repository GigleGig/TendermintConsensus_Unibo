#pragma once

#ifndef CONFIG_H
#define CONFIG_H

class Config {
public:
    static int getNodeCount();
    static int getTimeout();

private:
    static const int NODE_COUNT = 4;
    static const int TIMEOUT = 1000; // 毫秒
};

#endif

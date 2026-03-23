#ifndef PERIPHERALOPERATOR_H
#define PERIPHERALOPERATOR_H

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include "define.h"


class peripheralOperator {
public:
    peripheralOperator() {}

    int init(int port);
    int fileOperate(const std::string attr, const std::string val);

private:
    std::string optDevice, optFile, expDevice;
};

// dummy operator for debugging
class dummyOperator {
public:
    dummyOperator() {}
    int init(int) { return 0; }
    int fileOperate(const std::string, const std::string) { return 0; }
};


// ========== 平台宏切换逻辑 ==========
// 1. Windows平台：映射到dummyOperator
#if defined(_WIN32) || defined(_WIN64)
#define PlatformOperator dummyOperator

// 2. Linux平台：映射到peripheralOperator
#elif defined(__linux__)
#define PlatformOperator peripheralOperator
#endif


#endif // PERIPHERALOPERATOR_H

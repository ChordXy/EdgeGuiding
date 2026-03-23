#ifndef DYNAMICLIBLOADER_H
#define DYNAMICLIBLOADER_H


#include <string>
#include "alg_detection.h"

class DynamicLibLoader {
public:
    DynamicLibLoader();
    ~DynamicLibLoader();

    bool loadLibrary(const std::string& libPath);   // 加载动态库（参数：库路径，如 "algorithm1.dll" 或 "libalgorithm1.so"）
    void unloadLibrary();                           // 卸载动态库
    Detection* createAlgorithmInstance();           // 创建算法实例（调用动态库的CreateAlgorithm函数）

    std::string getLastError() const { return m_lastError; }    // 获取最后一次错误信息

private:
    typedef void* LibHandle;
    static constexpr LibHandle INVALID_HANDLE = nullptr;
    LibHandle m_libHandle = INVALID_HANDLE;         // 动态库句柄
    CreateAlgorithmFunc m_createFunc = nullptr;     // 创建算法的函数指针
    std::string m_lastError;                        // 错误信息
};


#endif // DYNAMICLIBLOADER_H

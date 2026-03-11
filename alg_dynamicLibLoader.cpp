#include "alg_dynamicLibLoader.h"

#ifdef _WIN32
    #include <windows.h>
    #define GET_LAST_ERROR() std::to_string(GetLastError())
#else
    #include <dlfcn.h>
    #define GET_LAST_ERROR() dlerror()
#endif

DynamicLibLoader::DynamicLibLoader() {}

DynamicLibLoader::~DynamicLibLoader() {
    unloadLibrary();
}

bool DynamicLibLoader::loadLibrary(const std::string& libPath) {
    if (m_libHandle != INVALID_HANDLE) {
        unloadLibrary();
    }

    // 1. 加载动态库
#ifdef _WIN32
    m_libHandle = LoadLibraryA(libPath.c_str());
#else
    // RTLD_NOW：立即解析所有符号；RTLD_GLOBAL：符号可被其他库使用
    m_libHandle = dlopen(libPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif

    if (m_libHandle == INVALID_HANDLE) {
        m_lastError = "加载库失败：" + libPath + "，错误：" + GET_LAST_ERROR();
        return false;
    }

    // 2. 获取动态库中导出的CreateAlgorithm函数
#ifdef _WIN32
    m_createFunc = (CreateAlgorithmFunc)GetProcAddress((HMODULE)m_libHandle, "CreateAlgorithm");
#else
    m_createFunc = (CreateAlgorithmFunc)dlsym(m_libHandle, "CreateAlgorithm");
#endif

    if (!m_createFunc) {
        m_lastError = "获取CreateAlgorithm函数失败，错误：" + std::string(GET_LAST_ERROR());
        unloadLibrary(); // 加载函数失败，卸载库
        return false;
    }

    m_lastError = "";
    return true;
}

void DynamicLibLoader::unloadLibrary() {
    if (m_libHandle == INVALID_HANDLE) {
        return;
    }

    // 卸载动态库
#ifdef _WIN32
    FreeLibrary((HMODULE)m_libHandle);
#else
    dlclose(m_libHandle);
#endif

    m_libHandle = INVALID_HANDLE;
    m_createFunc = nullptr;
}

Detection* DynamicLibLoader::createAlgorithmInstance() {
    if (!m_createFunc) {
        m_lastError = "未加载有效的算法库";
        return nullptr;
    }

    return m_createFunc();      // 调用动态库的CreateAlgorithm函数，创建算法实例
}

#include <string>

#include "Detection.h"
#include "opencv2/opencv.hpp"

#define version "Track Block v1.0"

// 加法算法实现
class AlgorithmDemo : public Detection {
public:
    int calculate(const cv::Mat & img) override {

        return 50; 
    }

    const char* getAlgorithmName() const override {
        return m_name.c_str();
    }

private:
    std::string m_name = version;
};




// ========== 导出创建实例的函数（必须命名为CreateAlgorithm） ==========
extern "C" {
#ifdef _WIN32
    __declspec(dllexport)
#endif
    Detection* CreateAlgorithm() {
        return new AlgorithmDemo();
    }
}
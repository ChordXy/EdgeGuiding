#ifndef DETECTION_H
#define DETECTION_H

#include "opencv2/opencv.hpp"
#include <vector>

#define PIXEL_TO_MM         100
#define MM_TO_MILLIAMP      1

class Detection {
public:
    virtual ~Detection() = default;

    // ret: -1 fail;   else: success
    virtual int calculate(const cv::Mat & img) = 0;

    void setBoundary(const int l, const int r) { lBoundary = l; rBoundary = r; }
    void setMiddle(const int m) { middle = m; }
    void setPosition(const int p) { lmr = p; }

    double calCurrent(int cx) {
        return (cx - lBoundary) * PIXEL_TO_MM * MM_TO_MILLIAMP;
    }

    virtual const char* getAlgorithmName() const = 0;

private:
    int lBoundary = 0;
    int rBoundary = 320;
    int middle = 160;
    int lmr;

    int selectHueBayes(const cv::Mat& hsv, int targetHue, int lastHue){
        cv::Mat hist = cv::Mat::zeros(1, 180, CV_32F);

        for (int y = 0; y < hsv.rows; ++y) {
            const cv::Vec3b* p = hsv.ptr<cv::Vec3b>(y);
            for (int x = 0; x < hsv.cols; ++x) {
                int h = p[x][0];
                hist.at<float>(0, h) += 1.f;
            }
        }

        auto gaussProb = [&](int h, int mean) {
            if (mean < 0) return 1.0f; // 第一帧没有先验
            int d = std::abs(h - mean);
            d = std::min(d, 180 - d);
            const float sigma = 8.f;
            return expf(-(d * d) / (2.0f * sigma * sigma));
        };

        int search = 20;
        int bestH = -1;
        float bestScore = -1.f;

        for (int h = 0; h < 180; ++h) {
            int d = std::abs(h - targetHue);
            d = std::min(d, 180 - d);
            if (d > search) continue;

            float likelihood = hist.at<float>(0, h);
            float prior = gaussProb(h, lastHue);
            float score = likelihood * prior;

            if (score > bestScore) {
                bestScore = score;
                bestH = h;
            }
        }

        if (bestScore < 20.f)  // 阈值：太弱则认为没找到
            return -1;
        return bestH;
    }
};


// ========== 动态库导出函数的统一签名 ==========
    // 所有算法动态库必须导出该函数，用于创建算法实例
    // 注意：extern "C" 避免C++名称修饰，保证动态库中函数名唯一

extern "C" {
    typedef Detection* (*CreateAlgorithmFunc)(); // 创建实例的函数指针类型
}


#endif // DETECTION_H

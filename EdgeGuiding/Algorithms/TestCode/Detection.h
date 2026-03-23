#ifndef DETECTION_H
#define DETECTION_H

#include "opencv2/opencv.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

#define PIXEL_TO_MM         100
#define MM_TO_MILLIAMP      1

class Detection {
public:
    virtual ~Detection() = default;

    // 返回 X 坐标，-1 表示失败
    virtual int calculate(const cv::Mat& img) = 0;

    virtual void init(const std::string& name) = 0;

    // 参数设置
    void setBoundary(const int l, const int r) { lBoundary = l; rBoundary = r; }
    void setMiddle(const int m) { middle = m; }
    void setPosition(const int p) { lmr = p; } // 0:Left, 1:Mid, 2:Right

    // 辅助计算
    double calCurrent(int cx) {
        return (cx - lBoundary) * PIXEL_TO_MM * MM_TO_MILLIAMP;
    }

    virtual const char* getAlgorithmName() const = 0;

protected:
    int lBoundary = 0;
    int rBoundary = 320;
    int middle = 160;
    int lmr = 0;        // 0:Left, 1:Mid, 2:Right
    int targetHue = 0;

    // 通用贝叶斯 Hue 选择逻辑
    int selectHueBayes(const cv::Mat& hsv, int targetHue, int lastHue) {
        cv::Mat hist = cv::Mat::zeros(1, 180, CV_32F);
        for (int y = 0; y < hsv.rows; ++y) {
            const cv::Vec3b* p = hsv.ptr<cv::Vec3b>(y);
            for (int x = 0; x < hsv.cols; ++x) {
                hist.at<float>(0, p[x][0]) += 1.f;
            }
        }

        auto gaussProb = [&](int h, int mean) {
            if (mean < 0) return 1.0f;
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
        if (bestScore < 20.f) return -1;
        return bestH;
    }

    // 内部标定辅助函数
    int internalCalibrate(const cv::Mat& img, int pickX) {
        if (img.empty()) return -1;
        int x0 = std::max(0, pickX - 2);
        int x1 = std::min(img.cols - 1, pickX + 2);
        int centerY = img.rows / 2;
        int halfH = 20; // 标定取色不需要太高，上下20像素即可

        int y0 = std::max(0, centerY - halfH);
        int y1 = std::min(img.rows, centerY + halfH);
        if (y0 >= img.rows) y0 = img.rows / 2;
        if (y1 > img.rows) y1 = img.rows;

        cv::Rect roiRect(x0, y0, x1 - x0 + 1, y1 - y0);
        if (roiRect.width <= 0 || roiRect.height <= 0) return -1;

        cv::Mat roi = img(roiRect);
        cv::Mat hsv;
        cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
        cv::Mat mask;
        cv::inRange(hsv, cv::Scalar(0, 50, 50), cv::Scalar(179, 255, 255), mask);

        if (cv::countNonZero(mask) == 0) return -1;
        cv::Scalar meanHue = cv::mean(hsv, mask);
        return static_cast<int>(meanHue[0]);
    }
};

extern "C" {
    typedef Detection* (*CreateAlgorithmFunc)();
}

#endif // DETECTION_H

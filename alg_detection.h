#ifndef DETECTION_H
#define DETECTION_H

#include "opencv2/opencv.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <define.h>

#define PIXEL_TO_MM         100
#define MM_TO_MILLIAMP      1


class DllConfig {
public:
    int targetHue = 0;
    int lockMid=0;
    int lockHue=0;
    int lastBayesHue=0;
    int lastHue = 0;
    int lastResultX = 0;
};

class Detection {
public:
    virtual ~Detection() = default;

    // 返回 X 坐标，-1 表示失败
    virtual int calculate(const cv::Mat& img) = 0;

    virtual void reset() = 0;

    // 参数设置
    void setBoundary(const int l, const int r) { lBoundary = l; rBoundary = r; }
    void setMiddle(const int m) { reset(); middle = m; }
    void setPosition(const int p) { lmr = p; } // 0:Left, 1:Mid, 2:Right

    // 辅助计算
    double calCurrent(int cx) {
        return (cx - lBoundary) * PIXEL_TO_MM * MM_TO_MILLIAMP / (rBoundary - lBoundary);
    }

    virtual const char* getAlgorithmName() const = 0;

protected:
    int lBoundary = 0;
    int rBoundary = 320;
    int middle = 160;
    int lmr = 0;        // 0:Left, 1:Mid, 2:Right
    int targetHue = 0;
    //保存本次跟踪数据
    void saveDllConfig(DllConfig* config) {

        std::ifstream in(INI_FILE_PATH);
        if (!in.is_open()) return;

        std::stringstream buffer;
        std::string line;
        bool foundTargetHue=false;
        bool foundLastResultX=false;
        bool foundLastHue=false;
        bool inUserSection = false;
        while (std::getline(in, line)) {
            std::string originalLine = line;

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // 判断 section
            if (line.find("[User]") != std::string::npos) {
                inUserSection = true;
            }
            else if (!line.empty() && line[0] == '[') {
                inUserSection = false;
            }

            if (inUserSection) {
                if (line.find("TargetHue=") == 0) {
                    buffer << "TargetHue=" << config->targetHue << "\n";
                    foundTargetHue=true;
                    continue;
                }
                if (line.find("LastHue=") == 0) {
                    buffer << "LastHue=" << config->lastHue << "\n";
                    foundLastHue=true;
                    continue;
                }
                if (line.find("LastResultX=") == 0) {
                    buffer << "LastResultX=" << config->lastResultX << "\n";
                    foundLastResultX=true;
                    continue;
                }
            }

            buffer << originalLine << "\n";
        }
        if(!foundTargetHue)buffer <<"TargetHue=" << config->targetHue << "\n";
        if(!foundLastHue)buffer <<"LastHue=" << config->lastHue << "\n";
        if(!foundLastResultX)buffer << "LastResultX=" << config->lastResultX << "\n";

        in.close();

        std::ofstream out(INI_FILE_PATH);
        if (!out.is_open()) return;

        out << buffer.str();
        return;
    }

    //从配置文件中恢复上次跟踪的数据
    DllConfig* loadConfig() {
        std::ifstream file(INI_FILE_PATH);
        if (!file.is_open()) return nullptr;
        DllConfig* config = new DllConfig();

        std::string line;
        bool inUserSection = false;

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // 判断 section
            if (line.find("[User]") != std::string::npos) {
                inUserSection = true;
                continue;
            } else if (!line.empty() && line[0] == '[') {
                inUserSection = false;
            }
            //读取数据
            if (inUserSection) {
                if (line.find("TargetHue=") == 0)config->targetHue = std::stoi(line.substr(10));

                if (line.find("LastHue=") == 0)config->lastResultX = std::stoi(line.substr(8));

                if (line.find("LastResultX=") == 0)config->lastResultX = std::stoi(line.substr(12));

                if (line.find("LockMid=") == 0)config->lastResultX = std::stoi(line.substr(8));

                if (line.find("LockHue=") == 0)config->lastResultX = std::stoi(line.substr(8));

                if (line.find("LastBayesHue=") == 0)config->lastResultX = std::stoi(line.substr(13));

            }
        }

        return config;
    }


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

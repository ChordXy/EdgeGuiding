#include "Detection.h"

class ColorBlockTracker : public Detection {
public:
    ColorBlockTracker() {
        lastHue = -1;
        lastResultX = -1;
        isCalibrated = false;
    }

    const char* getAlgorithmName() const override {
        return "ColorBlock_v1.0";
    }

    void reset() override {
        lastHue = -1;
        lastResultX = -1;
        isCalibrated = false;
    }

    int calculate(const cv::Mat &img) override {
        if (img.empty()) return -1;

        // --- 阶段1: 自动标定逻辑 ---
        if (!isCalibrated) {
            int hue = internalCalibrate(img, this->middle);
            if (hue >= 0) {
                targetHue = hue;
                lastResultX = this->middle; // 初始位置
                lastHue = -1;
                isCalibrated = true;
                return lastResultX;
            } else {
                return -1;
            }
        }

        // --- 阶段2: 正常跟踪逻辑 ---
        int extend = 5;
        cv::Rect roiRect(std::max(0, lBoundary - extend), 100,
                         std::min(img.cols, rBoundary - lBoundary + 2 * extend), 40);

        if (roiRect.x < 0 || roiRect.width <= 0) return -1;

        cv::Mat roi = img(roiRect);
        cv::Mat hsv;
        cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
        cv::GaussianBlur(hsv, hsv, cv::Size(3, 3), 0);

        int bestH = selectHueBayes(hsv, targetHue, lastHue);
        if (bestH < 0) return -1;
        lastHue = bestH;

        int tol = 8;
        cv::Scalar meanVal = cv::mean(hsv);
        int Smin = std::max(50, int(meanVal[1] * 0.5));
        int Vmin = std::max(50, int(meanVal[2] * 0.5));

        int Hlow = (bestH - tol + 180) % 180;
        int Hhigh = (bestH + tol) % 180;
        cv::Mat colorMask;

        if (Hlow <= Hhigh) {
            cv::inRange(hsv, cv::Scalar(Hlow, Smin, Vmin), cv::Scalar(Hhigh, 255, 255), colorMask);
        } else {
            cv::Mat m1, m2;
            cv::inRange(hsv, cv::Scalar(0, Smin, Vmin), cv::Scalar(Hhigh, 255, 255), m1);
            cv::inRange(hsv, cv::Scalar(Hlow, Smin, Vmin), cv::Scalar(179, 255, 255), m2);
            cv::bitwise_or(m1, m2, colorMask);
        }

        cv::Mat k = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1));
        cv::dilate(colorMask, colorMask, k);

        cv::Mat labels, stats, centroids;
        int nLabels = cv::connectedComponentsWithStats(colorMask, labels, stats, centroids, 8, CV_32S);

        int mainLabel = -1;
        int maxArea = 0;
        int bestDist = 2147483647;

        for (int i = 1; i < nLabels; ++i) {
            int area = stats.at<int>(i, cv::CC_STAT_AREA);
            if (area < 80) continue;

            int left = stats.at<int>(i, cv::CC_STAT_LEFT) + roiRect.x;
            int width = stats.at<int>(i, cv::CC_STAT_WIDTH);
            int mid = left + width / 2;

            if (lastResultX >= 0) {
                int dist = std::abs(mid - lastResultX);
                if (dist < bestDist) {
                    bestDist = dist;
                    mainLabel = i;
                }
            } else {
                if (area > maxArea) {
                    maxArea = area;
                    mainLabel = i;
                }
            }
        }

        int finalX = -1;
        if (mainLabel > 0) {
            int left = stats.at<int>(mainLabel, cv::CC_STAT_LEFT) + roiRect.x;
            int width = stats.at<int>(mainLabel, cv::CC_STAT_WIDTH);
            int right = left + width;
            int mid = left + width / 2;

            if (lmr == 0)      finalX = left;
            else if (lmr == 1) finalX = mid;
            else if (lmr == 2) finalX = right;
            else               finalX = mid;

            lastResultX = finalX;
        }

        return finalX;
    }

private:
    int lastHue;
    int lastResultX;
    bool isCalibrated;
};

extern "C" {
#ifdef _WIN32
__declspec(dllexport)
#endif
Detection* CreateAlgorithm() {
    return new ColorBlockTracker();
}
}

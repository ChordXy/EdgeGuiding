#include "Detection.h"

class LineTracker : public Detection {
public:
    LineTracker() {
        // 构造时初始化
        lockMid = -1;
        lockHue = -1;
        lostFrames = 0;
        lastBayesHue = -1;
        isCalibrated = false;
    }

    const char* getAlgorithmName() const override {
        return "LineTracker_v1.0";
    }

    void reset() override {
        lockMid = -1;
        lockHue = -1;
        lostFrames = 0;     
        lastBayesHue = -1;
        isCalibrated = false;
    }

    int calculate(const cv::Mat &img) override {
        if (img.empty()) return -1;

        if (!isCalibrated) {
            int hue = internalCalibrate(img, this->middle);
            if (hue >= 0) {
                targetHue = hue;
                lockMid = this->middle; // 强制锁定标定点
                lockHue = -1;
                lostFrames = 0;
                lastBayesHue = -1;
                isCalibrated = true; // 标记标定完成
                return lockMid;
            } else {
                return -1; // 标定失败
            }
        }

        if (lockMid < 0) return -1;

        int extend = 5;
        cv::Rect roiRect(std::max(0, lBoundary - extend), 60,
                         std::min(img.cols, rBoundary - lBoundary + 2 * extend), 120);

        if (roiRect.x < 0 || roiRect.width <= 0 || roiRect.x + roiRect.width > img.cols) return -1;

        cv::Mat roi = img(roiRect);
        cv::Mat hsv;
        cv::cvtColor(roi, hsv, cv::COLOR_BGR2HSV);
        cv::GaussianBlur(hsv, hsv, cv::Size(3, 3), 0);

        int bestH = selectHueBayes(hsv, targetHue, lastBayesHue);
        if (bestH >= 0) lastBayesHue = bestH;

        int maskHue = (lockHue >= 0 ? lockHue : bestH);
        if (maskHue < 0) maskHue = targetHue;

        int tol = 8;
        cv::Scalar meanVal = cv::mean(hsv);
        int Smin = std::max(50, int(meanVal[1] * 0.6));
        int Vmin = std::max(50, int(meanVal[2] * 0.6));

        int Hlow = (maskHue - tol + 180) % 180;
        int Hhigh = (maskHue + tol) % 180;
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

        int selectedMid = -1;
        int selectedHue = -1;
        int bestDist = 2147483647;

        for (int i = 1; i < nLabels; ++i) {
            int area = stats.at<int>(i, cv::CC_STAT_AREA);
            if (area < minArea) continue;

            int left = stats.at<int>(i, cv::CC_STAT_LEFT) + roiRect.x;
            int mid = left + stats.at<int>(i, cv::CC_STAT_WIDTH) / 2;

            int d = std::abs(mid - lockMid);
            if (d < bestDist) {
                bestDist = d;
                selectedMid = mid;
                cv::Mat m = (labels == i);
                cv::Scalar mh = cv::mean(hsv, m);
                selectedHue = static_cast<int>(mh[0]) % 180;
            }
        }

        bool found = false;
        if (selectedMid >= 0) {
            const int maxJump = std::max(10, (rBoundary - lBoundary) / 3);
            if (bestDist <= maxJump) {
                int dHue = std::abs(selectedHue - (lockHue >= 0 ? lockHue : selectedHue));
                dHue = std::min(dHue, 180 - dHue);

                if (lockHue < 0 || dHue <= hueTol) {
                    lockMid = selectedMid;
                    lockHue = selectedHue;
                    lostFrames = 0;
                    found = true;
                }
            }
        }

        if (!found) {
            lostFrames++;
            if (lostFrames > maxLostFrames) {
                lockMid = -1; // 彻底丢失
                return -1;
            }
        }

        return lockMid;
    }

private:
    int lockMid;
    int lockHue;
    int lostFrames;
    int lastBayesHue;
    bool isCalibrated;

    const int maxLostFrames = 300;
    const int hueTol = 12;
    const int minArea = 300;
};

extern "C" {
#ifdef _WIN32
__declspec(dllexport)
#endif
Detection* CreateAlgorithm() {
    return new LineTracker();
}
}

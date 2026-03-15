#include <iostream>
#include <opencv2/opencv.hpp>
#include "Detection.h"

extern "C" Detection * CreateAlgorithm();

// 全局变量
bool g_triggerCalibration = false;
int g_clickX = -1;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        g_clickX = x;
        g_triggerCalibration = true;
        std::cout << "[User] Clicked at X: " << x << std::endl;
    }
}

/* 测试 LineTracker */
int main() {
    int cameraIndex = 0;
    cv::VideoCapture cap(cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open USB Camera (Index: " << cameraIndex << ")." << std::endl;
        return -1;
    }

    // [强烈建议] 手动设置摄像头的分辨率，保持和之前测试视频一样的比例，防止 ROI 变形
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    /*std::string videoPath = "test.mp4";
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open video." << std::endl;
        return -1;
    }*/

    Detection* tracker = CreateAlgorithm();
    if (!tracker) return -1;

    // --- 1) 设置边界参数 ---
    int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // 假设边界是 80 和 240 (根据你的mainwindow代码习惯)
    int boundL = 80;
    int boundR = width - 80;
    tracker->setBoundary(boundL, boundR);

    cv::namedWindow("Algorithm Test", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Algorithm Test", onMouse, nullptr);

    cv::Mat frame;
    bool paused = false;

    while (true) {
        if (!paused) {
            cap >> frame;
            if (frame.empty()) {
                //cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                //continue;
                std::cerr << "Error: Camera disconnected or failed to grab frame!" << std::endl;
                break; // 摄像头没有帧代表掉线或故障，直接退出
            }
        }

        cv::Mat displayImg = frame.clone();

        // 处理点击
        if (g_triggerCalibration) {
            tracker->init("reset");
            tracker->setMiddle(g_clickX);
            g_triggerCalibration = false;
        }

        // 执行算法
        int resultX = tracker->calculate(frame);

        // --- 2) 绘制辅助线 (边界 & ROI) ---
        // 绘制左右边界 (黄色虚线)
        cv::line(displayImg, cv::Point(boundL, 0), cv::Point(boundL, height), cv::Scalar(0, 255, 255), 2);
        cv::line(displayImg, cv::Point(boundR, 0), cv::Point(boundR, height), cv::Scalar(0, 255, 255), 2);

        // 绘制 ROI 高度范围 (算法里写死是 60~180 或 100~140，这里画个大概示意)
        //cv::rectangle(displayImg, cv::Point(0, 60), cv::Point(width, 180), cv::Scalar(255, 0, 0), 1);
        //::putText(displayImg, "ROI Area", cv::Point(10, 55), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
        int roiH = 120;
        int roiY = (frame.rows / 2) - (roiH / 2);

        // 画出新的 ROI 框 (蓝色)
        cv::rectangle(displayImg,
            cv::Point(0, roiY),
            cv::Point(width, roiY + roiH),
            cv::Scalar(255, 0, 0), 1);

        // --- 3) 绘制结果 ---
        std::string statusText;
        cv::Scalar statusColor;

        if (resultX >= 0) {
            // 判断是否在边界内
            if (resultX < boundL || resultX > boundR) {
                statusText = "Status: OUT OF BOUNDS (Holding)";
                statusColor = cv::Scalar(0, 165, 255); // 橙色
            }
            else {
                statusText = "Status: LOCKED";
                statusColor = cv::Scalar(0, 255, 0);   // 绿色
            }

            // 画中心线
            cv::line(displayImg, cv::Point(resultX, 0), cv::Point(resultX, height), statusColor, 2);
            // 画坐标文字
            cv::putText(displayImg, "X:" + std::to_string(resultX), cv::Point(resultX + 5, 100),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, statusColor, 2);
        }
        else {
            statusText = "Status: LOST / SEARCHING";
            statusColor = cv::Scalar(0, 0, 255);   // 红色
            cv::putText(displayImg, "Click line to start!", cv::Point(width / 2 - 100, height / 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, statusColor, 2);
        }

        // 顶部状态栏
        cv::rectangle(displayImg, cv::Point(0, 0), cv::Point(300, 30), cv::Scalar(0, 0, 0), -1);
        cv::putText(displayImg, statusText, cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);

        cv::imshow("Algorithm Test", displayImg);

        char key = (char)cv::waitKey(30);
        if (key == 27) break;
        if (key == 32) paused = !paused;
    }

    delete tracker;
    return 0;
}


/* 测试 ColorBlockTracker */
//int main() {
//    std::string videoPath = "test2.mp4";
//    cv::VideoCapture cap(videoPath);
//    if (!cap.isOpened()) {
//        std::cerr << "Error: Cannot open video." << std::endl;
//        return -1;
//    }
//
//    // 创建算法实例 (此时链接的是 ColorBlockTracker)
//    Detection* tracker = CreateAlgorithm();
//    if (!tracker) {
//        std::cerr << "Error: Failed to create algorithm." << std::endl;
//        return -1;
//    }
//
//    int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
//    int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
//
//    // 设置左右边界
//    int boundL = 80;
//    int boundR = width - 80;
//    tracker->setBoundary(boundL, boundR);
//
//    // 默认位置模式: 1 = Middle
//    int currentPosMode = 1;
//    tracker->setPosition(currentPosMode);
//
//    cv::namedWindow("ColorBlock Test", cv::WINDOW_AUTOSIZE);
//    cv::setMouseCallback("ColorBlock Test", onMouse, nullptr);
//
//    cv::Mat frame;
//    bool paused = false;
//
//    std::cout << "=== Color Block Tracker Test ===" << std::endl;
//    std::cout << "Operations:" << std::endl;
//    std::cout << "  [Click]: Calibrate color" << std::endl;
//    std::cout << "  [A]: Track LEFT" << std::endl;
//    std::cout << "  [S]: Track MIDDLE" << std::endl;
//    std::cout << "  [D]: Track RIGHT" << std::endl;
//    std::cout << "  [Space]: Pause" << std::endl;
//
//    while (true) {
//        if (!paused) {
//            cap >> frame;
//            if (frame.empty()) {
//                cap.set(cv::CAP_PROP_POS_FRAMES, 0);
//                continue;
//            }
//        }
//
//        cv::Mat displayImg = frame.clone();
//
//        // 1. 处理标定
//        if (g_triggerCalibration) {
//            tracker->init("reset");
//            tracker->setMiddle(g_clickX);
//            // 这里会触发内部的 calibrate 逻辑，锁定颜色
//            g_triggerCalibration = false;
//        }
//
//        // 2. 执行算法
//        int resultX = tracker->calculate(frame);
//
//        // 3. 绘制辅助信息
//
//        // (A) 绘制 ROI 区域 (高度40，居中)
//        int roiH = 40;
//        int roiY = (height / 2) - (roiH / 2);
//        cv::rectangle(displayImg, cv::Point(0, roiY), cv::Point(width, roiY + roiH), cv::Scalar(255, 0, 0), 1);
//
//        // (B) 绘制左右边界
//        cv::line(displayImg, cv::Point(boundL, 0), cv::Point(boundL, height), cv::Scalar(0, 255, 255), 2);
//        cv::line(displayImg, cv::Point(boundR, 0), cv::Point(boundR, height), cv::Scalar(0, 255, 255), 2);
//
//        // (C) 绘制结果
//        if (resultX >= 0) {
//            // 画出识别到的中心线 (绿色)
//            cv::line(displayImg, cv::Point(resultX, 0), cv::Point(resultX, height), cv::Scalar(0, 255, 0), 2);
//
//            // 在目标点画个圆，表示这是色块中心
//            cv::circle(displayImg, cv::Point(resultX, height / 2), 5, cv::Scalar(0, 0, 255), -1);
//
//            cv::putText(displayImg, "X:" + std::to_string(resultX), cv::Point(resultX + 10, roiY - 10),
//                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
//        }
//        else {
//            cv::putText(displayImg, "LOST / Click to Start", cv::Point(width / 2 - 100, height / 2),
//                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
//        }
//
//        // (D) 显示当前模式 (左/中/右)
//        std::string modeStr = "Mode: ";
//        if (currentPosMode == 0) modeStr += "LEFT";
//        else if (currentPosMode == 1) modeStr += "MIDDLE";
//        else if (currentPosMode == 2) modeStr += "RIGHT";
//
//        cv::rectangle(displayImg, cv::Point(0, 0), cv::Point(250, 40), cv::Scalar(0, 0, 0), -1);
//        cv::putText(displayImg, modeStr, cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
//
//        cv::imshow("ColorBlock Test", displayImg);
//
//        // 4. 键盘控制
//        char key = (char)cv::waitKey(30);
//        if (key == 27) break; // ESC
//        if (key == 32) paused = !paused; // Space
//
//        // 切换模式 L/M/R
//        if (key == 'a' || key == 'A') {
//            currentPosMode = 0;
//            tracker->setPosition(0);
//            std::cout << "Switched to LEFT" << std::endl;
//        }
//        if (key == 's' || key == 'S') {
//            currentPosMode = 1;
//            tracker->setPosition(1);
//            std::cout << "Switched to MIDDLE" << std::endl;
//        }
//        if (key == 'd' || key == 'D') {
//            currentPosMode = 2;
//            tracker->setPosition(2);
//            std::cout << "Switched to RIGHT" << std::endl;
//        }
//    }
//
//    delete tracker;
//    return 0;
//}
#include <iostream>
#include <opencv2/opencv.hpp>
#include "Detection.h"

extern "C" Detection * CreateAlgorithm();

// ȫ�ֱ���
bool g_triggerCalibration = false;
int g_clickX = -1;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        g_clickX = x;
        g_triggerCalibration = true;
        std::cout << "[User] Clicked at X: " << x << std::endl;
    }
}

/* ���� LineTracker */
int main() {
    int cameraIndex = 0;
    cv::VideoCapture cap(cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open USB Camera (Index: " << cameraIndex << ")." << std::endl;
        return -1;
    }

    // [ǿ�ҽ���] �ֶ���������ͷ�ķֱ��ʣ����ֺ�֮ǰ������Ƶһ���ı�������ֹ ROI ����
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

    // --- 1) ���ñ߽���� ---
    int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // ����߽��� 80 �� 240 (�������mainwindow����ϰ��)
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
                break; // ����ͷû��֡������߻���ϣ�ֱ���˳�
            }
        }

        cv::Mat displayImg = frame.clone();

        // ������
        if (g_triggerCalibration) {
            tracker->init("reset");
            tracker->setMiddle(g_clickX);
            g_triggerCalibration = false;
        }

        // ִ���㷨
        int resultX = tracker->calculate(frame);

        // --- 2) ���Ƹ����� (�߽� & ROI) ---
        // �������ұ߽� (��ɫ����)
        cv::line(displayImg, cv::Point(boundL, 0), cv::Point(boundL, height), cv::Scalar(0, 255, 255), 2);
        cv::line(displayImg, cv::Point(boundR, 0), cv::Point(boundR, height), cv::Scalar(0, 255, 255), 2);

        // ���� ROI �߶ȷ�Χ (�㷨��д���� 60~180 �� 100~140�����ﻭ�����ʾ��)
        //cv::rectangle(displayImg, cv::Point(0, 60), cv::Point(width, 180), cv::Scalar(255, 0, 0), 1);
        //::putText(displayImg, "ROI Area", cv::Point(10, 55), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
        int roiH = 120;
        int roiY = (frame.rows / 2) - (roiH / 2);

        // �����µ� ROI �� (��ɫ)
        cv::rectangle(displayImg,
            cv::Point(0, roiY),
            cv::Point(width, roiY + roiH),
            cv::Scalar(255, 0, 0), 1);

        // --- 3) ���ƽ�� ---
        std::string statusText;
        cv::Scalar statusColor;

        if (resultX >= 0) {
            // �ж��Ƿ��ڱ߽���
            if (resultX < boundL || resultX > boundR) {
                statusText = "Status: OUT OF BOUNDS (Holding)";
                statusColor = cv::Scalar(0, 165, 255); // ��ɫ
            }
            else {
                statusText = "Status: LOCKED";
                statusColor = cv::Scalar(0, 255, 0);   // ��ɫ
            }

            // ��������
            cv::line(displayImg, cv::Point(resultX, 0), cv::Point(resultX, height), statusColor, 2);
            // ����������
            cv::putText(displayImg, "X:" + std::to_string(resultX), cv::Point(resultX + 5, 100),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, statusColor, 2);
        }
        else {
            statusText = "Status: LOST / SEARCHING";
            statusColor = cv::Scalar(0, 0, 255);   // ��ɫ
            cv::putText(displayImg, "Click line to start!", cv::Point(width / 2 - 100, height / 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, statusColor, 2);
        }

        // ����״̬��
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


/* ���� ColorBlockTracker */
//int main() {
//    std::string videoPath = "test2.mp4";
//    cv::VideoCapture cap(videoPath);
//    if (!cap.isOpened()) {
//        std::cerr << "Error: Cannot open video." << std::endl;
//        return -1;
//    }
//
//    // �����㷨ʵ�� (��ʱ���ӵ��� ColorBlockTracker)
//    Detection* tracker = CreateAlgorithm();
//    if (!tracker) {
//        std::cerr << "Error: Failed to create algorithm." << std::endl;
//        return -1;
//    }
//
//    int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
//    int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
//
//    // �������ұ߽�
//    int boundL = 80;
//    int boundR = width - 80;
//    tracker->setBoundary(boundL, boundR);
//
//    // Ĭ��λ��ģʽ: 1 = Middle
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
//        // 1. ����궨
//        if (g_triggerCalibration) {
//            tracker->init("reset");
//            tracker->setMiddle(g_clickX);
//            // ����ᴥ���ڲ��� calibrate �߼���������ɫ
//            g_triggerCalibration = false;
//        }
//
//        // 2. ִ���㷨
//        int resultX = tracker->calculate(frame);
//
//        // 3. ���Ƹ�����Ϣ
//
//        // (A) ���� ROI ���� (�߶�40������)
//        int roiH = 40;
//        int roiY = (height / 2) - (roiH / 2);
//        cv::rectangle(displayImg, cv::Point(0, roiY), cv::Point(width, roiY + roiH), cv::Scalar(255, 0, 0), 1);
//
//        // (B) �������ұ߽�
//        cv::line(displayImg, cv::Point(boundL, 0), cv::Point(boundL, height), cv::Scalar(0, 255, 255), 2);
//        cv::line(displayImg, cv::Point(boundR, 0), cv::Point(boundR, height), cv::Scalar(0, 255, 255), 2);
//
//        // (C) ���ƽ��
//        if (resultX >= 0) {
//            // ����ʶ�𵽵������� (��ɫ)
//            cv::line(displayImg, cv::Point(resultX, 0), cv::Point(resultX, height), cv::Scalar(0, 255, 0), 2);
//
//            // ��Ŀ��㻭��Բ����ʾ����ɫ������
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
//        // (D) ��ʾ��ǰģʽ (��/��/��)
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
//        // 4. ���̿���
//        char key = (char)cv::waitKey(30);
//        if (key == 27) break; // ESC
//        if (key == 32) paused = !paused; // Space
//
//        // �л�ģʽ L/M/R
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
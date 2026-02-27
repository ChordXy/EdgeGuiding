#include "uvcapture.h"

ImageCapturer::ImageCapturer(QObject *parent)
    : QObject(parent) {
    m_captureTimer = new QTimer(this);
    m_captureTimer->setInterval(CAPTURE_INTERVAL);
    connect(m_captureTimer, &QTimer::timeout, this, &ImageCapturer::captureFrame);
    m_captureTimer->setSingleShot(false);
}

ImageCapturer::~ImageCapturer() {
    stopCapture();
    if (m_cap.isOpened()) {
        m_cap.release();
    }
}

void ImageCapturer::initCamera() {
    qDebug() << "开始初始化相机，当前线程ID：" << QThread::currentThreadId();

    m_cap.open(CAMERA_PATH);
    if (!m_cap.isOpened()) {
        emit captureError(QString("相机%1初始化失败").arg(CAMERA_PATH));
        return;
    }

    m_cap.set(cv::CAP_PROP_FRAME_WIDTH,     CAMERA_WIDTH);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT,    CAMERA_HEIGHT);
    m_cap.set(cv::CAP_PROP_FPS,             CAMERA_FPS);

    m_isCameraInited = true;
    emit cameraInitSuccess();
}

void ImageCapturer::startCapture() {
    if (!m_isCameraInited) {
        emit captureError("相机未初始化，正在初始化...");
        initCamera();
    }

    if (m_isCapturing) return;
    m_isCapturing = true;
    m_captureTimer->start();
//    emit captureStatusChanged(true);
}

void ImageCapturer::stopCapture() {
    if (!m_isCameraInited) {
        emit captureError("相机未初始化，跳过采集");
        return;
    }

    if (!m_isCapturing) return;

    m_isCapturing = false;
    m_captureTimer->stop();
//    emit captureStatusChanged(false);
}

void ImageCapturer::captureFrame() {
    if (!m_cap.isOpened()) {
        stopCapture();
        emit captureError("摄像头已断开");
        return;
    }

    m_cap >> frame;
    if (frame.empty()) {
        stopCapture();
        emit captureError("读取图像帧失败");
        return;
    }

    emit imageCaptured(frame);
}

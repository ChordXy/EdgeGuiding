#include "m_uvcCapture.h"

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
        logging("相机释放", "UVC Camera -> ~ImageCapturer");
    }
}

void ImageCapturer::initCamera() {
    logging("开始初始化相机，当前线程ID：" + QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16).toUpper(), "UVC Camera -> initCamera");
    m_isCameraInited = false;

    m_cap.open(CAMERA_PATH);
    if (!m_cap.isOpened()) {
        emit captureError(_CAMERA_INIT_FAIL, QString("相机%1初始化失败").arg(CAMERA_PATH));
        logging(QString("相机%1初始化失败").arg(CAMERA_PATH), "UVC Camera -> initCamera");
        return;
    }

    m_cap.set(cv::CAP_PROP_FRAME_WIDTH,     CAMERA_WIDTH);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT,    CAMERA_HEIGHT);
    m_cap.set(cv::CAP_PROP_FPS,             CAMERA_FPS);

    m_isCameraInited = true;
    m_tryReconnectCount = 0;
    emit cameraInitSuccess();
}

void ImageCapturer::startCapture() {
    if (!m_isCameraInited) {
        logging("相机未初始化", "UVC Camera -> startCapture");
        emit captureError(_CAMERA_NOT_INIT, "相机未初始化，正在初始化...");
        initCamera();
    }

    if (m_isCapturing) return;
    m_isCapturing = true;
    m_captureTimer->start();
    logging("相机采集定时器 On", "UVC Camera -> startCapture");
//    emit captureStatusChanged(true);
}

void ImageCapturer::stopCapture() {
    if (!m_isCameraInited) {
        logging("相机未初始化，跳过采集", "UVC Camera -> stopCapture");
        emit captureError(_CAMERA_NOT_INIT, "相机未初始化，跳过采集");
        return;
    }

    if (!m_isCapturing) return;

    m_isCapturing = false;
    m_captureTimer->stop();
    logging("相机采集定时器 Off", "UVC Camera -> stopCapture");
//    emit captureStatusChanged(false);
}

void ImageCapturer::captureFrame() {
    if (!m_isCameraInited) {
        if (m_tryReconnectCount++ >= TRY_RECONNECT_CAMERA) {
            logging(QString("正在等待相机重连...%1").arg(m_tryReconnectCount), "UVC Camera -> captureFrame");
            m_tryReconnectCount = 0;
            tryReconnectCamera();
        }
        return;
    }


    if (!m_cap.isOpened()) {
        logging("摄像头断开", "UVC Camera -> captureFrame");
        emit captureError(_CAMERA_DISCONNECTED, "摄像头已断开");
        tryReconnectCamera();
        return;
    }

    m_cap >> frame;

    if (frame.empty()) {
        logging("读取图像帧失败，尝试重连相机", "UVC Camera -> captureFrame");
        emit captureError(_CAMERA_FRAME_READ_FAIL, "读取图像帧失败，尝试重连相机");
        tryReconnectCamera();
        return;
    }

    emit imageCaptured(frame);
}

void ImageCapturer::tryReconnectCamera() {
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    initCamera();
    logging("读取图像帧失败，尝试重连相机", "UVC Camera -> captureFrame");
}

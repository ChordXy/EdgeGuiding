#ifndef UVCAPTURE_H
#define UVCAPTURE_H


#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QPixmap>

#include "opencv2/opencv.hpp"
#include "m_loggings.h"
#include "define.h"


class ImageCapturer : public QObject
{
    Q_OBJECT
public:
    explicit ImageCapturer(QObject *parent = nullptr);
    ~ImageCapturer() override;

    void startCapture();        // 开定时器
    void stopCapture();         // 关定时器

signals:
    void cameraInitSuccess();               // 初始化成功

    void imageCaptured(const cv::Mat &mat);
    void captureError(int errCode, const QString &msg);
//    void captureStatusChanged(bool isRunning);


private slots:
    void captureFrame();            // 定时器调用，采集图像

private:
    QTimer *m_captureTimer;         // 采集定时器
    cv::VideoCapture m_cap;         // 摄像头对象（子线程内创建）
    bool m_isCameraInited = false;  // 相机是否初始化成功
    bool m_isCapturing = false;     // 采集状态
    int m_tryReconnectCount = 0;    // 重试相机连接


    cv::Mat frame;


    void initCamera();
    void tryReconnectCamera();
};

#endif // UVCAPTURE_H

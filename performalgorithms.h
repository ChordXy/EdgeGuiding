#ifndef PERFORMALGORITHMS_H
#define PERFORMALGORITHMS_H

#include <QDebug>
#include <QMutex>


#include "DynamicLibLoader.h"
#include "define.h"


class performAlgorithms : public QObject
{
    Q_OBJECT
public:
    explicit performAlgorithms(QObject *parent = nullptr);
    ~performAlgorithms() override;

    void setAlgorithm(int i) { curAlg = i; }
    void setBoundary(int l, int r) { Algs[curAlg]->setBoundary(l, r); }
    void setMiddle(int m) { Algs[curAlg]->setMiddle(m); }
    void setTrack(int lmr) { Algs[curAlg]->setPosition(lmr); }
    void startDetect();
    void stopDetect();

public slots:
    void recvImage(const cv::Mat &mat);         // 处理图像
    void recvErrors(const QString &msg);        // 槽函数

signals:
    void algorithmInitSuccess();                // 初始化成功
    void startCaptureSignal();
    void stopCaptureSignal();

    void calculatedResults(const FrameData &fd);
    void processErrors(const QString &msg);


private:
    Detection* Algs[2] = { nullptr, nullptr };

    bool m_isAlgorithmInited = false;    // 算法是否初始化成功
    bool m_isDetecting = false;          // 算法是否正在运行
    cv::Mat frame;
    int curAlg = 1;
    int cnt = 0;
    QMutex m_mutex;
    int calcenter;

    void initAlgorithm();
    QPixmap cvMatToPixmap(const cv::Mat &mat);
};


#endif // PERFORMALGORITHMS_H

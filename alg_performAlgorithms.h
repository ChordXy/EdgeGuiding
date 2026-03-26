#ifndef PERFORMALGORITHMS_H
#define PERFORMALGORITHMS_H

#include <QDebug>
#include <QMutex>
#include <QString>

#include "alg_dynamicLibLoader.h"
#include "define.h"
#include "m_loggings.h"
#include "m_peripheralOperator.h"


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
    void initParameters(int t_alg, int t_lb, int t_rb, int t_mid, int t_lmr) { m_alg = t_alg; m_lb = t_lb; m_rb = t_rb; m_mid = t_mid; m_lmr = t_lmr; }
    void startDetect();
    void stopDetect();
    void loadSettings(const std::map<std::string, int>& m){Algs[curAlg]->fromMap(m);}
    void saveSettings(){saveDynamicLibSettings(Algs[curAlg]->toMap());}
    double getRatioMid(){return ratioMid;}
public slots:
    void recvImage(const cv::Mat &mat);                 // 处理图像
    void recvErrors(int errCode, const QString &msg);   // 槽函数

signals:
    void algorithmInitSuccess();                // 初始化成功
    void startCaptureSignal();
    void stopCaptureSignal();

    void calculatedResults(const FrameData &fd);
    void processErrors(int errCode, const QString &msg);


private:
    Detection* Algs[2] = { nullptr, nullptr };

    bool m_isAlgorithmInited = false;    // 算法是否初始化成功
    bool m_isDetecting = false;          // 算法是否正在运行
    cv::Mat frame;
    int curAlg = 1;
    int cnt = 0;
    QMutex m_mutex;
    int calcenter;

    PlatformOperator pwmOpt;
    int duty_pwm;
    char duty_pwm_str[10];
    double ratioMid;

    int m_alg, m_lb, m_rb, m_mid, m_lmr;        // 仅用于初始化

    void initAlgorithm();
    QPixmap cvMatToPixmap(const cv::Mat &mat);
};


#endif // PERFORMALGORITHMS_H

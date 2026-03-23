#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QStyle>
#include <QImage>
#include <QDebug>

#include "opencv2/opencv.hpp"

#include "dlg_calibration.h"
#include "autolockwindow.h"
#include "dlg_settings.h"
#include "dlg_unlock.h"
#include "m_uvcCapture.h"
#include "define.h"
#include "alg_performAlgorithms.h"
#include "m_loggings.h"


const QString str_lock_state[] = {
    "解锁", "锁定"
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public autoLockWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void startDetect();
    void stopDetect();
    void resultToDialog(const FrameData &fd);

private slots:
    void on_pbn_settings_clicked();
    void on_pbn_calibration_clicked();
    void on_pbn_lock_clicked();

    void onProcessResults(const FrameData &fd);             // 接收采集到的图像
    void handlerErrors(int errCode, const QString &msg);    // 接收错误信息

private:
    Ui::MainWindow *ui;

    AppConfigs ac;                  // 本地配置参数

    QThread *m_captureThread;       // 采集线程
    QThread *m_algorithmThread;     // 算法线程
    ImageCapturer *m_capturer;      // 采集器对象
    performAlgorithms *m_algorithm; // 算法处理器对象

    QPen dashPenR, linePenR;        // 中心位置线的画笔
    QPixmap tmp;                    // 绘图临时变量
    QFont font;                     // 字体

    PlatformOperator *ledOpt = new PlatformOperator;       // LED控制


    int cur_lock_state = _STATE_UNLOCK_SCREEN;         // 0: Locked   1:Unlocked.
    bool tSignalToDialog = false;   // 是否转发信号给Dialog

    void initCameraAndAlgorithm();
    void lockScreen();
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : autoLockWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ac = loadSettings();
    this->installEventFilterToAllChildren(this);
    this->set_timer_count(_lock_screen_time[ac.locktime]);

    connect(this, &MainWindow::locked, this, &MainWindow::lockScreen);

    initCameraAndAlgorithm();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initCameraAndAlgorithm() {
    // 1. 创建采集器和子线程
    m_capturer = new ImageCapturer();
    m_algorithm = new performAlgorithms();
    m_captureThread = new QThread(this);
    m_algorithmThread = new QThread(this);

    // 2. 将采集器移到子线程（关键：避免主线程阻塞）
    m_capturer->moveToThread(m_captureThread);
    m_algorithm->moveToThread(m_algorithmThread);

    // 3. 建立信号槽连接（线程安全）
    // 3.1 主窗口信号→算法处理器（确保在子线程执行）
    connect(this, &MainWindow::startDetect, m_algorithm, &performAlgorithms::startDetect);
    connect(this, &MainWindow::stopDetect, m_algorithm, &performAlgorithms::stopDetect);
    connect(this, &MainWindow::destroyed, m_algorithm, &performAlgorithms::stopDetect);
    // 3.2 算法器信号→主线程UI更新
    connect(m_algorithm, &performAlgorithms::calculatedResults, this, &MainWindow::onProcessResults);
    connect(m_algorithm, &performAlgorithms::processErrors, this, &MainWindow::handlerErrors);
    // 3.3 算法器信号→图像采集器
    connect(m_capturer, &ImageCapturer::imageCaptured, m_algorithm, &performAlgorithms::recvImage);
    connect(m_capturer, &ImageCapturer::captureError, m_algorithm, &performAlgorithms::recvErrors);
    connect(m_algorithm, &performAlgorithms::startCaptureSignal, m_capturer, &ImageCapturer::startCapture);
    connect(m_algorithm, &performAlgorithms::stopCaptureSignal, m_capturer, &ImageCapturer::stopCapture);

    // 4. 线程退出→释放采集器
    // 4.1 线程结束 → 释放工作对象（避免内存泄漏）
    connect(m_algorithmThread, &QThread::finished, m_capturer, &QObject::deleteLater);
    connect(m_captureThread, &QThread::finished, m_capturer, &QObject::deleteLater);
    // 4.2 窗口关闭 → 先停止相机，再退出子线程
    connect(this, &QWidget::destroyed, [&]() {
            m_capturer->stopCapture();          // 停止采集
            m_captureThread->quit();            // 退出线程事件循环
            m_captureThread->wait(1000);        // 等待线程退出（最多1秒）
        });

    // 启动子线程（仅启动事件循环，不立即采集）
    m_captureThread->start();
    m_algorithmThread->start();

    emit startDetect();
}

/*🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨*/
void MainWindow::on_pbn_settings_clicked() {
    emit stopDetect();
    settingsDialog sD(ac, this);
    this->installEventFilterToAllChildren(&sD);
    sD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    sD.setWindowModality(Qt::WindowModal);
    sD.move(0, 0);
    sD.exec();
    ac = sD.get_setting_results();
    emit startDetect();
}

/*🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫*/
void MainWindow::on_pbn_lock_clicked() {
    if (cur_lock_state == 0) {
        this->stopTimer();
        emit stopDetect();
        unlockDialog uD(1, ac.password, ac.autotime, this);
        uD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        uD.setWindowModality(Qt::WindowModal);
        uD.move(0, 0);
        uD.exec();
        this->startTimer();
        emit startDetect();
        if (uD.get_operation_result() == false) return;
        ui->pbn_settings->setEnabled(true);
        ui->pbn_calibration->setEnabled(true);
    }
    else {
        lockScreen();
    }
}

void MainWindow::lockScreen() {
    cur_lock_state = 0;
    ui->pbn_settings->setEnabled(false);
    ui->pbn_calibration->setEnabled(false);
    ui->pbn_lock->setText(QString(str_lock_state[cur_lock_state]));
}


/*🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦*/
void MainWindow::on_pbn_calibration_clicked() {
    tSignalToDialog = true;
    calibrationDialog calD(ac, m_algorithm, this);
    this->installEventFilterToAllChildren(&calD);
    connect(this, &MainWindow::resultToDialog, &calD, &calibrationDialog::onReceiveImage);
    calD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    calD.setWindowModality(Qt::WindowModal);
    calD.move(0, 0);
    calD.exec();
    tSignalToDialog = false;
}

void MainWindow::onProcessResults(const FrameData &fd) {
    if (!tSignalToDialog){
        QPixmap tmp = fd.pixm.scaled(ui->label->size(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
        ui->label->setPixmap(tmp);
        qDebug() << QString("Image %1 : %2").arg(fd.frameId).arg(fd.center);
    } else {
        emit resultToDialog(fd);
    }
}

void MainWindow::handlerErrors(const QString &msg) {
    qDebug() << msg;
}

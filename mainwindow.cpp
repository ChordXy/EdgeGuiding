#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : autoLockWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 1. 加载设置
    ac = loadSettings();
    // 2. 窗体添加自动锁定事件（只要任一子控件有触摸等动作，则锁定重新计时
    this->installEventFilterToAllChildren(this);
    // 3. 设置窗体自动锁定时间
    this->set_timer_count(_lock_screen_time[ac.locktime]);

    // 4. 将窗体自动锁定的 Signal::locked 绑定到锁屏槽函数
    connect(this, &MainWindow::locked, this, &MainWindow::lockScreen);

    // 5. 初始化相机和算法
    initCameraAndAlgorithm();

    // 6. 切换锁定状态为 "解锁" 状态
    ui->pbn_lock->setProperty("status", "active");  // 激活状态（改背景色）
    ui->pbn_lock->style()->unpolish(ui->pbn_lock);  // 刷新样式
    ui->pbn_lock->style()->polish(ui->pbn_lock);

    ui->pbn_settings->setProperty("status", "active");      // 激活状态（改背景色）
    ui->pbn_settings->style()->unpolish(ui->pbn_settings);  // 刷新样式
    ui->pbn_settings->style()->polish(ui->pbn_settings);

    ui->pbn_calibration->setProperty("status", "active");           // 激活状态（改背景色）
    ui->pbn_calibration->style()->unpolish(ui->pbn_calibration);    // 刷新样式
    ui->pbn_calibration->style()->polish(ui->pbn_calibration);

    // 7. 字体、画笔颜色初始化
    font.setFamily(FONT_FAMILY);
    font.setPointSize(FONT_SIZE);

    dashPenR.setColor(Qt::red);          // 虚线颜色
    dashPenR.setWidth(2);                // 虚线宽度
    dashPenR.setStyle(Qt::DotLine);      // 预设虚线样式（Qt::DashLine/Qt::DotLine等）

    linePenR.setColor(Qt::red);          // 实线颜色
    linePenR.setWidth(2);                // 实线宽度
    linePenR.setStyle(Qt::SolidLine);    // 预设实线样式

    // 8. LED初始化
    ledOpt->init(4);
    char duty_light[10];
    std::sprintf(duty_light, "%d", int(10000 * (10 - ac.light)));
    ledOpt->fileOperate("period", "100000");
    ledOpt->fileOperate("duty_cycle", duty_light);
    ledOpt->fileOperate("enable", "1");
}

MainWindow::~MainWindow() {
    ledOpt->fileOperate("enable", "0");
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

    // 5. 启动子线程（仅启动事件循环，不立即采集）
    m_captureThread->start();
    m_algorithmThread->start();

    // 6. 初始化算法参数，以便在检测前初始化算法时，能够按照保存的参数进行初始化
    m_algorithm->initParameters(ac.mode, ac.lboundry, ac.rboundry, ac.mid, ac.track);

    // 7. 开始采集图像
    emit startDetect();
}

/*🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨*/
void MainWindow::on_pbn_settings_clicked() {
    // 1. 暂停自动锁屏定时器、暂停采集图像
    this->stopTimer();
    emit stopDetect();

    // 2.新建设置界面
    //   - 模态化窗口、无边框
    //   - 解锁结果通过窗体对象的 get_operation_result() 获取
    //   - 设置项更新内容
    //     - 用户态： 锁屏时间（locktime）、语言（language）
    //     - 设备态： 模式、对线中心、亮度、中心位置、左边界、右边界
    ac = loadSettings();
    settingsDialog sD(ac, this);
    sD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    sD.setWindowModality(Qt::WindowModal);
    sD.move(0, 0);
    sD.exec();

    ac = sD.get_setting_results();

    // 3. 恢复窗体自动锁屏、图像采集
    this->startTimer();
    emit startDetect();

    // 4. 更新 MainWindow 自动锁屏时间
    this->set_timer_count(_lock_screen_time[ac.locktime]);
}


/*🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫*/
void MainWindow::on_pbn_lock_clicked() {
    if (cur_lock_state == _STATE_LOCK_SCREEN) { // 如果当前是锁定状态，则需要进行解锁
        // 1. 暂停自动锁屏定时器、暂停采集图像
        this->stopTimer();
        emit stopDetect();

        // 2. 新建解锁界面，传入密码、自动关闭时间
        //   - 模态化窗口、无边框
        //   - 解锁结果通过窗体对象的 get_operation_result() 获取
        unlockDialog uD(1, ac.password, ac.autotime, this);
        uD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        uD.setWindowModality(Qt::WindowModal);
        uD.move(0, 0);
        uD.exec();

        // 3. 恢复窗体自动锁屏、图像采集
        this->startTimer();
        emit startDetect();

        // 4. 如果解锁失败，则直接返回
        if (uD.get_operation_result() == false) return;

        // 5. 解锁成功，将 设置、标定 按钮恢复可用，"锁定/解锁" 按钮文本切换
        cur_lock_state = 1;
        ui->pbn_settings->setEnabled(true);
        ui->pbn_calibration->setEnabled(true);

        // 6. 切换锁定状态为 "解锁" 状态
        ui->pbn_lock->setProperty("status", "active");  // 激活状态（改背景色）
        ui->pbn_lock->style()->unpolish(ui->pbn_lock);  // 刷新样式
        ui->pbn_lock->style()->polish(ui->pbn_lock);

        ui->pbn_settings->setProperty("status", "active");      // 激活状态（改背景色）
        ui->pbn_settings->style()->unpolish(ui->pbn_settings);  // 刷新样式
        ui->pbn_settings->style()->polish(ui->pbn_settings);

        ui->pbn_calibration->setProperty("status", "active");           // 激活状态（改背景色）
        ui->pbn_calibration->style()->unpolish(ui->pbn_calibration);    // 刷新样式
        ui->pbn_calibration->style()->polish(ui->pbn_calibration);
    }
    else { // 如果当前是解锁状态，则锁定
        lockScreen();
    }
}

void MainWindow::lockScreen() {
    // 1. 切换状态为锁定，并关闭 设置、标定 按钮不可用，"锁定/解锁" 按钮文本切换
    cur_lock_state = 0;
    ui->pbn_settings->setEnabled(false);
    ui->pbn_calibration->setEnabled(false);

    // 2. 切换锁定状态为 "解锁" 状态
    ui->pbn_lock->setProperty("status", "disabled");    // 激活状态（改背景色）
    ui->pbn_lock->style()->unpolish(ui->pbn_lock);      // 刷新样式
    ui->pbn_lock->style()->polish(ui->pbn_lock);

    ui->pbn_settings->setProperty("status", "disabled");    // 激活状态（改背景色）
    ui->pbn_settings->style()->unpolish(ui->pbn_settings);  // 刷新样式
    ui->pbn_settings->style()->polish(ui->pbn_settings);

    ui->pbn_calibration->setProperty("status", "disabled");           // 激活状态（改背景色）
    ui->pbn_calibration->style()->unpolish(ui->pbn_calibration);    // 刷新样式
    ui->pbn_calibration->style()->polish(ui->pbn_calibration);
}


/*🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦*/
void MainWindow::on_pbn_calibration_clicked() {
    // 1. 暂停自动锁屏定时器
    this->stopTimer();

    // 2.新建标定界面
    //   - ! tSignalToDialog 将图像采集信号转发至标定窗体 | 如果不转发，则直接显示
    //   - 模态化窗口、无边框
    //   - 传入 "参数对象ac" 和 "算法句柄"
    //   - 解锁结果通过窗体对象的 get_operation_result() 获取
    //   - 设置项更新内容
    //     - 用户态： 锁屏时间（locktime）、语言（language）
    //     - 设备态： 模式、对线中心、亮度、中心位置、左边界、右边界
    tSignalToDialog = true;
    ac = loadSettings();
    calibrationDialog calD(ac, m_algorithm, ledOpt, this);
    connect(this, &MainWindow::resultToDialog, &calD, &calibrationDialog::onReceiveImage);
    calD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    calD.setWindowModality(Qt::WindowModal);
    calD.move(0, 0);
    calD.exec();
    tSignalToDialog = false;

    // 3. 恢复窗体自动锁屏、图像采集
    this->startTimer();
}

void MainWindow::onProcessResults(const FrameData &fd) {
    if (!tSignalToDialog){
        tmp = fd.pixm;
        QPainter painter(&tmp);
        painter.setRenderHint(QPainter::Antialiasing);

        if (fd.center != -1) {
            // 4 绘制检测中心
            // 4.1 检测中心（红 细）
            painter.setPen(dashPenR);
            painter.drawLine(fd.center, 0, fd.center, CAMERA_HEIGHT);
            // 4.2 绘制 ⟭|⟬
            painter.setPen(linePenR);
            // 4.2.1 左侧
            painter.drawLine(QPoint(fd.center - DRAW_VLINE_I, DRAW_VLINE_S), QPoint(fd.center - DRAW_VLINE_I, DRAW_VLINE_E));
            painter.drawLine(QPoint(fd.center - DRAW_RLINE_I, DRAW_RLINE_S), QPoint(fd.center - DRAW_VLINE_I, DRAW_VLINE_S));
            painter.drawLine(QPoint(fd.center - DRAW_RLINE_I, DRAW_RLINE_E), QPoint(fd.center - DRAW_VLINE_I, DRAW_VLINE_E));
            // 4.2.2 右侧
            painter.drawLine(QPoint(fd.center + DRAW_VLINE_I, DRAW_VLINE_S), QPoint(fd.center + DRAW_VLINE_I, DRAW_VLINE_E));
            painter.drawLine(QPoint(fd.center + DRAW_RLINE_I, DRAW_RLINE_S), QPoint(fd.center + DRAW_VLINE_I, DRAW_VLINE_S));
            painter.drawLine(QPoint(fd.center + DRAW_RLINE_I, DRAW_RLINE_E), QPoint(fd.center + DRAW_VLINE_I, DRAW_VLINE_E));
            // 4.3 检测中心和偏移量（文本，黄底红字）
            painter.setFont(font);
            QRect textRectM(fd.center - 30, 5, 60, 25);
            painter.fillRect(textRectM, Qt::red);
            painter.setPen(Qt::white);
            painter.drawText(textRectM, Qt::AlignCenter, "中心 "+QString::number(m_algorithm->getRatioMid()));


        }

        ui->label->setPixmap(tmp.scaled(ui->label->size(),
                                            Qt::IgnoreAspectRatio,
                                            Qt::SmoothTransformation));
        logging(QString("Image %1 : %2").arg(fd.frameId).arg(fd.center), "MainWindow -> onProcessResults");
    } else {
        emit resultToDialog(fd);
        logging(QString("Transmit %1").arg(fd.frameId), "MainWindow -> onProcessResults");
    }
}

void MainWindow::handlerErrors(int errCode, const QString &msg) {
//    qDebug() << "\t @@@ " << errCode << "\t" << msg;
    if (errCode == _CAMERA_DISCONNECTED || errCode == _CAMERA_FRAME_READ_FAIL || \
        errCode == _CAMERA_INIT_FAIL    || errCode == _CAMERA_NOT_INIT) {
        ui->label->clear();
        ui->label->setText("正在连接相机...");
    }

}

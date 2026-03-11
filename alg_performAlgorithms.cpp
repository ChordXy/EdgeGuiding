#include "alg_performAlgorithms.h"


/*🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦*/
performAlgorithms::performAlgorithms(QObject *parent)
    : QObject(parent) {

    // 开启PWM调控
    pwmOpt.init(5);
    pwmOpt.fileOperate("period", "100000");
    logging("PWM Operator init", "Algorithm -> init");
}

performAlgorithms::~performAlgorithms() {
    // 如果加载模型，则进行释放
    if (Algs[0] != nullptr) {
        delete Algs[0];
        Algs[0] = nullptr;
    }
    if (Algs[1] != nullptr) {
        delete Algs[1];
        Algs[1] = nullptr;
    }

    // 关闭PWM输出
    pwmOpt.fileOperate("enable", "0");
}

void performAlgorithms::initAlgorithm() {
    /// 1. Line Algorithm
    DynamicLibLoader* linelibLoader = new DynamicLibLoader();
    if (linelibLoader->loadLibrary(LINE_ALGORITHM)) {
        Algs[0] = linelibLoader->createAlgorithmInstance();
        if (Algs[0]) {
            logging(QString("成功加载 Line 算法"), "Algorithm -> init");
        }
    } else {
        logging(QString(" ERR 加载 Line 算法失败, ") + QString::fromStdString(linelibLoader->getLastError().c_str()), "Algorithm -> init");
        m_isAlgorithmInited = false;
        emit processErrors(_NO_ALGORITHM_LINE_LOADED, QString("Error in initialize Line Algorithm"));
        return;
    }

    /// 2. Block Algorithm
    DynamicLibLoader* blocklibLoader = new DynamicLibLoader();
    if (blocklibLoader->loadLibrary(BLOCK_ALGORITHM)) {
        Algs[1] = blocklibLoader->createAlgorithmInstance();
        if (Algs[1]) {
            logging(QString("成功加载 Block 算法"), "Algorithm -> init");
        }
    } else {
        logging(QString(" ERR 加载 Block 算法失败, ") + QString::fromStdString(linelibLoader->getLastError().c_str()), "Algorithm -> init");
        m_isAlgorithmInited = false;
        emit processErrors(_NO_ALGORITHM_BLOCK_LOADED, QString("Error in initialize Block Algorithm"));
        return;
    }

    setAlgorithm(m_alg);
    setBoundary(m_lb, m_rb);
    setMiddle(m_mid);
    setTrack(m_lmr);
    logging(QString(" >>> 初始化\n\t > Alg=%1\n\t > L=%2 R=%3\n\t > Mid=%4\n\t > LMR=%5").arg(m_alg).arg(m_lb).arg(m_rb).arg(m_mid).arg(m_lmr), "Algorithm -> init");

    m_isAlgorithmInited = true;
}

void performAlgorithms::startDetect() {
    if (!m_isAlgorithmInited) {
        logging(QString("初始化算法..."), "Algorithm -> startDetect");
        emit processErrors(_NO_ALGORITHM_INITED, "算法未初始化，正在尝试初始化...");
        initAlgorithm();
    }

    if (m_isDetecting) return;

    logging(QString("开始检测..."), "Algorithm -> startDetect");
    m_isDetecting = true;
    emit startCaptureSignal();
}

void performAlgorithms::stopDetect() {
    if (!m_isAlgorithmInited) {
        logging(QString("算法未初始化！"), "Algorithm -> stopDetect");
        emit processErrors(_NO_ALGORITHM_INITED, "算法未初始化，请先初始化...");
        return;
    }

    if (!m_isDetecting) return;

    logging(QString("停止检测..."), "Algorithm -> startDetect");
    m_isDetecting = false;
    emit stopCaptureSignal();
}

void performAlgorithms::recvImage(const cv::Mat &mat) {
    // 1. 锁定
    QMutexLocker locker(&m_mutex);

    // 2. 算法解算
    calcenter = Algs[curAlg]->calculate(mat);

    // 3. 绘制中心
//    cv::circle(mat, cv::Point(calcenter, CAMERA_HEIGHT / 2), 2, cv::Scalar(255, 0, 255), -1);

    // 4. 构建返回数据
    FrameData tmp(cnt++, cvMatToPixmap(mat));
    tmp.center = calcenter;

    logging(QString(" >>> Cal %1").arg(calcenter), "Algorithm -> recvImage");

    // 5. 发送结果
    emit calculatedResults(tmp);

    // 6. PWM调节
    ratioMid = Algs[curAlg]->calCurrent(calcenter);
    duty_pwm = 1000 * (100.00 - ratioMid);
    std::sprintf(duty_pwm_str, "%d", duty_pwm);
    if (pwmOpt.fileOperate("duty_cycle", duty_pwm_str)) {
        logging(QString("纠偏PWM设置失败"), "Algorithm -> recvImage");
        return;
    }

    if (pwmOpt.fileOperate("enable", "1")) {
        logging(QString("纠偏PWM开启失败"), "Algorithm -> recvImage");
        return;
    }
    logging(QString("当前计算偏移量 %1，PWM = %2 ").arg(ratioMid).arg(duty_pwm / 1000), "Algorithm -> recvImage");
}

void performAlgorithms::recvErrors(int errCode, const QString &msg) {
    emit processErrors(errCode, msg);
}

QPixmap performAlgorithms::cvMatToPixmap(const cv::Mat &mat) {
    cv::Mat rgbMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
    } else {
        rgbMat = mat.clone();
    }

    return QPixmap::fromImage(QImage(rgbMat.data, rgbMat.cols, rgbMat.rows,
                                     rgbMat.step, QImage::Format_RGB888));
}

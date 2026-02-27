#include "performalgorithms.h"


/*🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦*/
performAlgorithms::performAlgorithms(QObject *parent)
    : QObject(parent) {

}

performAlgorithms::~performAlgorithms() {
    if (Algs[0] != nullptr) {
        delete Algs[0];
        Algs[0] = nullptr;
    }
    if (Algs[1] != nullptr) {
        delete Algs[1];
        Algs[1] = nullptr;
    }
}

void performAlgorithms::initAlgorithm() {
    /// 1. Line Algorithm
    DynamicLibLoader* linelibLoader = new DynamicLibLoader();
    if (linelibLoader->loadLibrary(LINE_ALGORITHM)) {
        Algs[0] = linelibLoader->createAlgorithmInstance();
        if (Algs[0]) {
            qDebug() << "Succeeded loading line algorithm";
        }
    } else {
        qDebug() << "Failed to load line algorithm" << linelibLoader->getLastError().c_str();
        m_isAlgorithmInited = false;
        emit processErrors(QString("Error in initialize Line Algorithm"));
        return;
    }

    /// 2. Block Algorithm
    DynamicLibLoader* blocklibLoader = new DynamicLibLoader();
    if (blocklibLoader->loadLibrary(BLOCK_ALGORITHM)) {
        Algs[1] = blocklibLoader->createAlgorithmInstance();
        if (Algs[1]) {
            qDebug() << "Succeeded loading Block algorithm";
        }
    } else {
        qDebug() << "Failed to load Block Algorithm" << blocklibLoader->getLastError().c_str();
        m_isAlgorithmInited = false;
        emit processErrors(QString("Error in initialize Block Algorithm"));
        return;
    }

    m_isAlgorithmInited = true;
}

void performAlgorithms::startDetect() {
    if (!m_isAlgorithmInited) {
        emit processErrors("算法未初始化，正在尝试初始化...");
        initAlgorithm();
    }

    if (m_isDetecting) return;

    m_isDetecting = true;
    emit startCaptureSignal();
}

void performAlgorithms::stopDetect() {
    if (!m_isAlgorithmInited) {
        emit processErrors("算法未初始化，请先初始化...");
        return;
    }

    if (!m_isDetecting) return;

    m_isDetecting = false;
    emit stopCaptureSignal();
}

void performAlgorithms::recvImage(const cv::Mat &mat) {
    QMutexLocker locker(&m_mutex);

    calcenter = Algs[curAlg]->calculate(mat);
    cv::circle(mat, cv::Point(calcenter, CAMERA_HEIGHT / 2), 2, cv::Scalar(255, 0, 255), -1);

    FrameData tmp(cnt++, cvMatToPixmap(mat));
    tmp.center = calcenter;

    qDebug() << " @@@ Current Algorithm is" << Algs[curAlg]->getAlgorithmName();

    emit calculatedResults(tmp);
}

void performAlgorithms::recvErrors(const QString &msg) {
    emit processErrors(msg);
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

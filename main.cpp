#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>

#include "m_encrypt.h"
#include "define.h"




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    if (!checkID()) {
//        shutdownSystem();
//        return 0;
//    }

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<FrameData>("FrameData");

    MainWindow w;
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    w.move(0, 0);
    w.show();
    return a.exec();
}

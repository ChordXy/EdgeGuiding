#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>

#include "encrypt.h"
#include "define.h"




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    encrypt epy;
    if (!epy.checkID()) {
        epy.shutdownSystem();
        return 0;
    }

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<FrameData>("FrameData");


//    int dragDist = 30;
//    a.setStartDragDistance(dragDist);

    MainWindow w;
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    w.move(0, 0);
    w.show();
    return a.exec();
}

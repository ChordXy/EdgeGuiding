QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    alg_dynamicLibLoader.cpp \
    alg_performAlgorithms.cpp \
    dlg_calibration.cpp \
    dlg_settings.cpp \
    dlg_unlock.cpp \
    longPressButton.cpp \
    m_encrypt.cpp \
    m_peripheralOperator.cpp \
    m_uvcCapture.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    alg_detection.h \
    alg_dynamicLibLoader.h \
    alg_performAlgorithms.h \
    autoclosedialog.h \
    autolockwindow.h \
    define.h \
    dlg_calibration.h \
    dlg_settings.h \
    dlg_unlock.h \
    longPressButton.h \
    m_encrypt.h \
    m_loggings.h \
    m_peripheralOperator.h \
    m_uvcCapture.h \
    mainwindow.h

FORMS += \
    dlg_calibration.ui \
    dlg_settings.ui \
    dlg_unlock.ui \
    mainwindow.ui


INCLUDEPATH += \
    E:\Libs\opencv_MinGW\include
    E:\Libs\opencv_MinGW\include\opencv2


LIBS += \
    E:\Libs\opencv_MinGW\bin\libopencv*.dll


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resource.qrc

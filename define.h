#ifndef DEFINE_H
#define DEFINE_H

#include <QString>
#include <QSettings>
#include <QtGlobal>
#include <QMetaType>
#include <QPixmap>

#include <vector>
#include <iostream>

#include "opencv2/opencv.hpp"

#ifdef Q_OS_WIN
    #define INI_FILE_PATH       "D:/config.ini"
    #define SYS_INFO_PATH       "D:/sys_info"
    #define ENCRYPT_CODE_PATH   "D:/code"
    #define PWM_BASE            ""
    #define CAMERA_PATH         0
    class dummyOperator;
    typedef dummyOperator PlatformOperator;

#elif defined(Q_OS_LINUX)
    #define INI_FILE_PATH       "/opt/config.ini"
    #define SYS_INFO_PATH       "/sys/class/sunxi_info/sys_info"
    #define ENCRYPT_CODE_PATH   "/lib/code"
    #define PWM_BASE            "/sys/devices/platform/soc@3000000/2000c00.pwm/pwm/pwmchip0"
    #define CAMERA_PATH         0
    class peripheralOperator;
    typedef peripheralOperator PlatformOperator;

#endif

#define PRESET_ID "26-SEU-GangQun-T113-Device"
#define SUNXI_SERIAL_HEAD "sunxi_serial"

#define LINE_ALGORITHM "E:/Projects/Qt/myDemo/Algorithms/AlgorithmLine.dll"
#define BLOCK_ALGORITHM "E:/Projects/Qt/myDemo/Algorithms/AlgorithmBlock.dll"

#define CAPTURE_INTERVAL    30
#define CAMERA_WIDTH       320
#define CAMERA_HEIGHT      240
#define CAMERA_FPS          60
#define ADJUST_INTERVAL      2
#define MIN_BOUNDARY_INTER  10

const std::vector<int> _lock_screen_time = { 5, 15, 30, 60 };
const std::vector<QString> _languages = { "中文" };

struct FrameData {
    int frameId;
    QPixmap pixm;
    int center;

    FrameData() {}
    FrameData(int id, const QPixmap &p) : frameId(id), pixm(p) {}

    FrameData(const FrameData& other) {
        frameId = other.frameId;
        pixm = other.pixm;
        center = other.center;
    }

    FrameData& operator=(const FrameData& other) {
        if (this != &other) {
            frameId = other.frameId;
            pixm = other.pixm;
            center = other.center;
        }
        return *this;
    }
};

Q_DECLARE_METATYPE(FrameData)

typedef struct {
    QString password = "1234";
    int locktime = 0;
    int language = 0;
    int autotime = 20;         // 长时间未操作，Dialog自动关闭时间

    int mode = 0;               // 模式 - 0:线段  1:色块
    int track = 0;              // 色块模式-对线中心  0:左  1:中  2:右
    int light = 5;              // 亮度： 0-10
    int mid = 160;              // 中心位置
    int lboundry = 10;          // 左边界
    int rboundry = 310;         // 右边界
} AppConfigs;


inline AppConfigs loadSettings(void) {
    AppConfigs ac;
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);

    ac.autotime = settings.value("General/AutoTime", 20).toInt();
    ac.locktime = settings.value("General/LockTime", 0).toInt();
    ac.language = settings.value("General/Language", 0).toInt();
    ac.password = settings.value("General/Password", "1234").toString();

    ac.mode = settings.value("User/TrackMode", 1).toInt();
    ac.track = settings.value("User/TrackSide", 0).toInt();
    ac.light = settings.value("User/Light", 5).toInt();
    ac.mid = settings.value("User/Middle", 160).toInt();
    ac.lboundry = settings.value("User/LeftBoundry", 10).toInt();
    ac.rboundry = settings.value("User/RightBoundry", 310).toInt();

    return ac;
}

inline void saveSettings(AppConfigs ac) {
    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);

    settings.setValue("General/AutoTime", ac.autotime);
    settings.setValue("General/LockTime", ac.locktime);
    settings.setValue("General/Language", ac.language);
    settings.setValue("General/Password", ac.password);

    settings.setValue("User/TrackMode", ac.mode);
    settings.setValue("User/TrackSide", ac.track);
    settings.setValue("User/Light", ac.light);
    settings.setValue("User/Middle", ac.mid);
    settings.setValue("User/LeftBoundry", ac.lboundry);
    settings.setValue("User/RightBoundry", ac.rboundry);

    settings.sync();
}

#endif // DEFINE_H

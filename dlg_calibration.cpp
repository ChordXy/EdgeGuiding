#include "dlg_calibration.h"
#include "ui_dlg_calibration.h"


calibrationDialog::calibrationDialog(AppConfigs ac, performAlgorithms *tAlgorithm, PlatformOperator *lOpt, QWidget *parent) :
    autoCloseDialog(parent),
    ui(new Ui::calibrationDialog)
{
    ui->setupUi(this);
    // 1. 安装自动关闭事件，并设置自动关闭时间
    this->installEventFilterToAllChildren(this);
    this->set_timer_count(ac.autotime);

    // 2. 读取用户态设置、对线算法
    curAC = ac;
    curAlgorithm = tAlgorithm;
    font.setFamily(FONT_FAMILY);
    font.setPointSize(FONT_SIZE);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // 3. 模式选择窗体隐藏
    ui->frame_mode->hide();

    // 4. 连接信号与槽、初始化环境参数
    connectSignalsSlots();
    initEnvoriments();

    // 5. 初始化LED操作
    ledOpt = lOpt;
}

calibrationDialog::~calibrationDialog() {
    delete ui;
}

void calibrationDialog::on_pbn_return_clicked() {
    // 退出时，如果有按钮锁定，保存参数后退出
    if (onSetting != -1) {
        (this->*toggledFuncs[onSetting])(false);
    }

    saveSettings(curAC);
    this->close();
}

void calibrationDialog::onReceiveImage(const FrameData &fd) {
    QPixmap tmp = fd.pixm;
    QPainter painter(&tmp);
    painter.setRenderHint(QPainter::Antialiasing);

    if (onSetting == _SETTINGS_LBOUNDRY) {  // 设置左边界
        // 1 绘制边界
        // 1.1 左边界（黄 粗）
        painter.setPen(dashPenY);
        painter.drawLine(curAC.lboundry, 0, curAC.lboundry, CAMERA_HEIGHT);
        // 1.2 右边界（白 细）
        painter.setPen(dashPenW);
        painter.drawLine(curAC.rboundry, 0, curAC.rboundry, CAMERA_HEIGHT);
        // 1.3 左边界（文本，黄底红字）
        painter.setFont(font);
        QRect textRectL(curAC.lboundry - 25, 30, 55, 25);
        painter.fillRect(textRectL, Qt::yellow);
        painter.setPen(Qt::red);
        painter.drawText(textRectL, Qt::AlignCenter, "左边界");
        // 1.4 右边界（文本，白底黑字）
        QRect textRectR(curAC.rboundry - 25, 30, 55, 25);
        painter.fillRect(textRectR, Qt::white);
        painter.setPen(Qt::black);
        painter.drawText(textRectR, Qt::AlignCenter, "右边界");

    } else if (onSetting == _SETTINGS_RBOUNDRY) {
        // 2 绘制边界
        // 2.1 左边界（白 细）
        painter.setPen(dashPenW);
        painter.drawLine(curAC.lboundry, 0, curAC.lboundry, CAMERA_HEIGHT);
        // 2.2 右边界（黄 粗）
        painter.setPen(dashPenY);
        painter.drawLine(curAC.rboundry, 0, curAC.rboundry, CAMERA_HEIGHT);
        // 2.3 左边界（文本，白底黑字）
        painter.setFont(font);
        QRect textRectL(curAC.lboundry - 25, 30, 55, 25);
        painter.fillRect(textRectL, Qt::white);
        painter.setPen(Qt::black);
        painter.drawText(textRectL, Qt::AlignCenter, "左边界");
        // 2.4 右边界（文本，黄底红字）
        QRect textRectR(curAC.rboundry - 25, 30, 55, 25);
        painter.fillRect(textRectR, Qt::yellow);
        painter.setPen(Qt::red);
        painter.drawText(textRectR, Qt::AlignCenter, "右边界");

    } else if (onSetting == _SETTINGS_TARGET) {
        // 3 绘制 边界 和 标定中心
        // 3.1 左右边界（白 细）
        painter.setPen(dashPenW);
        painter.drawLine(curAC.lboundry, 0, curAC.lboundry, CAMERA_HEIGHT);
        painter.drawLine(curAC.rboundry, 0, curAC.rboundry, CAMERA_HEIGHT);
        // 3.2 标定中心（黄 粗）
        painter.setPen(dashPenY);
        painter.drawLine(curAC.mid, 0, curAC.mid, CAMERA_HEIGHT);
        // 3.3 标定中心（文本，黄底红字）
        painter.setFont(font);
        QRect textRectM(curAC.mid - 15, 30, 40, 25);
        painter.fillRect(textRectM, Qt::yellow);
        painter.setPen(Qt::red);
        painter.drawText(textRectM, Qt::AlignCenter, "标定");
        // 3.4 左边界（文本，白底黑字）
        QRect textRectL(curAC.lboundry - 25, 30, 55, 25);
        painter.fillRect(textRectL, Qt::white);
        painter.setPen(Qt::black);
        painter.drawText(textRectL, Qt::AlignCenter, "左边界");
        // 3.5 右边界（文本，白底黑字）
        QRect textRectR(curAC.rboundry - 25, 30, 55, 25);
        painter.fillRect(textRectR, Qt::white);
        painter.setPen(Qt::black);
        painter.drawText(textRectR, Qt::AlignCenter, "右边界"); 
    }

    // 如果计算结果中，center不等于-1，即表示检测到边缘，需要显示
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
        // 4.3 检测中心（文本，黄底红字）
        painter.setFont(font);
        QRect textRectM(fd.center - 15, 5, 40, 25);
        painter.fillRect(textRectM, Qt::red);
        painter.setPen(Qt::white);
        painter.drawText(textRectM, Qt::AlignCenter, "中心");

    }

    ui->label_disp->setPixmap(tmp.scaled(ui->label_disp->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
}

/*🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚*/
void calibrationDialog::connectSignalsSlots() {
    // 1. 面板按钮 -> 按钮互斥
    //    A按下，B弹起 ： B保存并释放，并触发A
    //    A按下，无弹起： 仅触发A
    toggledFuncs[0] = &calibrationDialog::pbn_mode_toggled;
    toggledFuncs[1] = &calibrationDialog::pbn_lBoundry_toggled;
    toggledFuncs[2] = &calibrationDialog::pbn_rBoundry_toggled;
    toggledFuncs[3] = &calibrationDialog::pbn_light_toggled;
    toggledFuncs[4] = &calibrationDialog::pbn_center_toggled;

    btnSettingsGroup->setExclusive(false);
    btnSettingsGroup->addButton(ui->pbn_mode, 0);
    btnSettingsGroup->addButton(ui->pbn_lBoundry, 1);
    btnSettingsGroup->addButton(ui->pbn_rBoundry, 2);
    btnSettingsGroup->addButton(ui->pbn_light, 3);
    btnSettingsGroup->addButton(ui->pbn_center, 4);
    foreach (QAbstractButton *btn, btnSettingsGroup->buttons()) {
        connect(btn, &QAbstractButton::clicked, this, [=](bool checked) {
            int btnId = btnSettingsGroup->id(btn);
            handler_btnGroup_settings(btn, btnId, checked);
        });
    }

    // 2. 模式选择
    //    按钮互斥，选择不同的功能
    btnModeGroup->setExclusive(true);
    btnModeGroup->addButton(ui->pbn_mode_line, 0);
    btnModeGroup->addButton(ui->pbn_mode_block, 1);
    connect(btnModeGroup,
            static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this,
            &calibrationDialog::handler_btnGroup_mode);

    // 3. 色块跟踪方式（对中，对左，对右）
    btnTrackGroup->setExclusive(true);
    btnTrackGroup->addButton(ui->pbn_center_left, 0);
    btnTrackGroup->addButton(ui->pbn_center_mid, 1);
    btnTrackGroup->addButton(ui->pbn_center_right, 2);
    foreach (QAbstractButton *btn, btnTrackGroup->buttons())
        btn->setEnabled(curAC.mode == 1);
    connect(btnTrackGroup,
            static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this,
            &calibrationDialog::handler_btnGroup_track);
}

void calibrationDialog::initEnvoriments() {
    btnModeGroup->button(curAC.mode)->setChecked(true);
    btnTrackGroup->button(curAC.track)->setChecked(true);

    dashPenW.setColor(Qt::white);        // 虚线颜色
    dashPenW.setWidth(2);                // 虚线宽度
    dashPenW.setStyle(Qt::DashLine);     // 预设虚线样式（Qt::DashLine/Qt::DotLine等）

    dashPenY.setColor(Qt::yellow);       // 虚线颜色
    dashPenY.setWidth(3);                // 虚线宽度
    dashPenY.setStyle(Qt::DashLine);     // 预设虚线样式（Qt::DashLine/Qt::DotLine等）

    dashPenR.setColor(Qt::red);          // 虚线颜色
    dashPenR.setWidth(2);                // 虚线宽度
    dashPenR.setStyle(Qt::DotLine);      // 预设虚线样式（Qt::DashLine/Qt::DotLine等）

    linePenR.setColor(Qt::red);          // 实线颜色
    linePenR.setWidth(2);                // 实线宽度
    linePenR.setStyle(Qt::SolidLine);    // 预设实线样式


}


/*🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫🟫*/
/*****************************************************************************************
 *                                     左侧设置面板区域                                     *
 *****************************************************************************************/
void calibrationDialog::handler_btnGroup_settings(QAbstractButton *btn, int btnId, bool isChecked){
    if (!isChecked) {           // 按钮释放
        // qDebug() << QString(" <<< End %1").arg(btn->text());
        onSetting = -1;
        offSetting = btnId;
    }

    if (isChecked) {
        if (onSetting == -1) {  // 新的按钮按下，且此前并无按钮按下
            // qDebug() << QString(" >>> Clicked %1").arg(btn->text());
            onSetting = btnId;
            offSetting = -1;
        } else {                // 已有按钮按下，切换其他按钮
            // qDebug() << QString("From %1 to %2").arg(btnSettingsGroup->button(onSetting)->text()).arg(btn->text());
            offSetting = onSetting;
            onSetting = btnId;
        }

        // 按钮互斥
        foreach (QAbstractButton *b, btnSettingsGroup->buttons()) {
            if (b != btn) {
                b->setChecked(false); // 其他按钮取消选中
            }
        }
    }

    if (offSetting >= 0) {  // 如果存在前一个按钮，则关闭
        (this->*toggledFuncs[offSetting])(false);
    }

    if (onSetting >= 0) {   // 如果有新的按钮，则打开
        (this->*toggledFuncs[onSetting])(true);
    }
}

/*-----------------------------------------------------------
 * @@@ Settings btns - 1 : Mode selection
 *-----------------------------------------------------------*/
void calibrationDialog::pbn_mode_toggled(bool state) {
    if (state) {
        ui->frame_disp->hide();
        ui->frame_mode->show();

        curAlgorithm->stopDetect();
        logging(QString("模式选择界面 On"), "Calibration -> pbn_mode_toggled");

    } else {
        curAlgorithm->setAlgorithm(curAC.mode);
        logging(QString("模式选择完毕，当前模式 %1").arg(curAC.mode), "Calibration -> pbn_mode_toggled");
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        logging(QString("左右边界 L=%1 R=%2").arg(curAC.lboundry).arg(curAC.rboundry), "Calibration -> pbn_mode_toggled");
        curAlgorithm->setMiddle(curAC.mid);
        logging(QString("中心位置 %1").arg(curAC.mid), "Calibration -> pbn_mode_toggled");
        curAlgorithm->setTrack(curAC.track);
        logging(QString("跟踪对象 %1").arg(curAC.track), "Calibration -> pbn_mode_toggled");
        curAlgorithm->startDetect();
        logging(QString("模式选择界面 Off"), "Calibration -> pbn_mode_toggled");
        ui->frame_disp->show();
        ui->frame_mode->hide();
        saveSettings(curAC);
    }

    ui->pbn_up->setEnabled(false);
    ui->pbn_down->setEnabled(false);
}

/*-----------------------------------------------------------
 * @@@ Settings btns - 2 : Left Boundry selection
 *-----------------------------------------------------------*/
void calibrationDialog::pbn_lBoundry_toggled(bool state) {
    if (state) {
        // Switch on Left Boundry
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Left Boundry
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(false);
        ui->pbn_down->setEnabled(false);
        saveSettings(curAC);
    }
}

/*-----------------------------------------------------------
 * @@@ Settings btns - 3 : Right Boundry selection
 *-----------------------------------------------------------*/
void calibrationDialog::pbn_rBoundry_toggled(bool state) {
    if (state) {
        // Switch on Right Boundry
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Right Boundry
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(false);
        ui->pbn_down->setEnabled(false);
        saveSettings(curAC);
    }
}

/*-----------------------------------------------------------
 * @@@ Settings btns - 4 : Light selection
 *-----------------------------------------------------------*/
void calibrationDialog::pbn_light_toggled(bool state) {
    if (state) {
        // Switch on Light
        ui->pbn_up->setText(QString("亮 %1").arg(curAC.light));
        ui->pbn_down->setText(QString("暗 %1").arg(curAC.light));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Light
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(false);
        ui->pbn_down->setEnabled(false);
        saveSettings(curAC);
    }
}

/*-----------------------------------------------------------
 * @@@ Settings btns - 5 : Center selection
 *-----------------------------------------------------------*/
void calibrationDialog::pbn_center_toggled(bool state) {
    if (state) {
        // Switch on Middle
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Middle
        ui->pbn_up->setText(QString("-->"));
        ui->pbn_down->setText(QString("<--"));
        ui->pbn_up->setEnabled(false);
        ui->pbn_down->setEnabled(false);
        saveSettings(curAC);
    }
}



/*🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨*/
/*****************************************************************************************
 *                                     模式选择面板区域                                     *
 *****************************************************************************************/
void calibrationDialog::handler_btnGroup_mode(int id) {
    if (id == curAC.mode) return;

    curAC.mode = id;
    logging(QString("Mode = %1").arg(curAC.mode), "Calibration -> handler_btnGroup");
    saveSettings(curAC);
    // 色块模式时，需要开放对线位置按钮
    foreach (QAbstractButton *btn, btnTrackGroup->buttons())
        btn->setEnabled(curAC.mode == 1);

}

void calibrationDialog::handler_btnGroup_track(int id) {
    // 色块模式下，对线跟踪 左 中 右     色块模式-对线中心  0:左  1:中  2:右
    curAC.track = id;
    curAlgorithm->setTrack(curAC.track);
    saveSettings(curAC);

}


/*🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦*/
void calibrationDialog::on_pbn_up_clicked() {

    switch(onSetting) {
    case(_SETTINGS_MODE) :
        return;

    case(_SETTINGS_LBOUNDRY) :
        curAC.lboundry = qMin(curAC.rboundry - MIN_BOUNDARY_INTER, curAC.lboundry + ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        saveSettings(curAC);
        return;

    case(_SETTINGS_RBOUNDRY) :
        curAC.rboundry = qMin(CAMERA_WIDTH, curAC.rboundry + ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        saveSettings(curAC);
        return;

    case(_SETTINGS_LIGHT) :
        curAC.light = qMin(curAC.light + 1, 10);
        ui->pbn_up->setText(QString("亮 %1").arg(curAC.light));
        ui->pbn_down->setText(QString("暗 %1").arg(curAC.light));
        std::sprintf(duty_light, "%d", int(10000 * (10 - curAC.light)));
        ledOpt->fileOperate("duty_cycle", duty_light);
        logging(QString("灯光亮度 %1").arg(curAC.light), "Calibration -> on_pbn_up");
        saveSettings(curAC);
        return;

    case(_SETTINGS_TARGET) :
        curAC.mid = qMin(curAC.rboundry, curAC.mid + ADJUST_INTERVAL);
        curAC.mid = qMin(curAC.mid, curAC.rboundry);
        curAC.mid = qMax(curAC.mid, curAC.lboundry);
        curAlgorithm->setMiddle(curAC.mid);
        logging(QString("Cur Middle %1").arg(curAC.mid), "Calibration -> on_pbn_up");
        saveSettings(curAC);
        return;
    }  
}

void calibrationDialog::on_pbn_down_clicked() {
    switch(onSetting) {
    case(_SETTINGS_MODE) :
        return;

    case(_SETTINGS_LBOUNDRY) :
        curAC.lboundry = qMax(0, curAC.lboundry - ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        return;

    case(_SETTINGS_RBOUNDRY) :
        curAC.rboundry = qMax(curAC.lboundry + MIN_BOUNDARY_INTER, curAC.rboundry - ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        return;

    case(_SETTINGS_LIGHT) :
        curAC.light = qMax(curAC.light - 1, 0);
        ui->pbn_up->setText(QString("亮 %1").arg(curAC.light));
        ui->pbn_down->setText(QString("暗 %1").arg(curAC.light));
        std::sprintf(duty_light, "%d", int(10000 * (10 - curAC.light)));
        ledOpt->fileOperate("duty_cycle", duty_light);
        logging(QString("灯光亮度 %1").arg(curAC.light), "Calibration -> on_pbn_down");
        return;

    case(_SETTINGS_TARGET) :
        curAC.mid = qMax(curAC.lboundry, curAC.mid - ADJUST_INTERVAL);
        curAC.mid = qMin(curAC.mid, curAC.rboundry);
        curAC.mid = qMax(curAC.mid, curAC.lboundry);
        curAlgorithm->setMiddle(curAC.mid);
        logging(QString("Cur Middle %1").arg(curAC.mid), "Calibration -> on_pbn_down");
        return;
    }
}

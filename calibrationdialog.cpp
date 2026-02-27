#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"


calibrationDialog::calibrationDialog(AppConfigs ac, performAlgorithms *tAlgorithm, QWidget *parent) :
    autoCloseDialog(parent),
    ui(new Ui::calibrationDialog)
{
    ui->setupUi(this);
    this->installEventFilterToAllChildren(this);
    this->set_timer_count(ac.autotime);
    curAC = ac;
    curAlgorithm = tAlgorithm;
    ui->frame_mode->hide();

    connectSignalsSlots();
    initEnvoriments();
}

calibrationDialog::~calibrationDialog() {
    delete ui;
}

void calibrationDialog::on_pbn_return_clicked() {
    saveSettings(curAC);
    this->close();
}

void calibrationDialog::onReceiveImage(const FrameData &fd) {
    QPixmap tmp = fd.pixm;
    if (onSetting == 1) {
        QPainter painter(&tmp);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(dashPenY);
        painter.drawLine(curAC.lboundry, 0, curAC.lboundry, CAMERA_HEIGHT);
        painter.setPen(dashPenW);
        painter.drawLine(curAC.rboundry, 0, curAC.rboundry, CAMERA_HEIGHT);
    } else if (onSetting == 2) {
        QPainter painter(&tmp);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(dashPenW);
        painter.drawLine(curAC.lboundry, 0, curAC.lboundry, CAMERA_HEIGHT);
        painter.setPen(dashPenY);
        painter.drawLine(curAC.rboundry, 0, curAC.rboundry, CAMERA_HEIGHT);
    } else if (onSetting == 4) {
        QPainter painter(&tmp);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(dashPenW);
        painter.drawLine(curAC.lboundry, 0, curAC.lboundry, CAMERA_HEIGHT);
        painter.drawLine(curAC.rboundry, 0, curAC.rboundry, CAMERA_HEIGHT);
        painter.setPen(dashPenY);
        painter.drawLine(curAC.mid, 0, curAC.mid, CAMERA_HEIGHT);
    }

    ui->label_disp->setPixmap(tmp.scaled(ui->label_disp->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
}

/*🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚🍚*/
void calibrationDialog::connectSignalsSlots() {
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

    btnModeGroup->setExclusive(true);
    btnModeGroup->addButton(ui->pbn_mode_line, 0);
    btnModeGroup->addButton(ui->pbn_mode_block, 1);
    connect(btnModeGroup,
            static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this,
            &calibrationDialog::handler_btnGroup_mode);

    btnTrackGroup->setExclusive(true);
    btnTrackGroup->addButton(ui->pbn_center_left, 0);
    btnTrackGroup->addButton(ui->pbn_center_mid, 1);
    btnTrackGroup->addButton(ui->pbn_center_right, 2);
    connect(btnTrackGroup,
            static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this,
            &calibrationDialog::handler_btnGroup_track);

    toggledFuncs[0] = &calibrationDialog::pbn_mode_toggled;
    toggledFuncs[1] = &calibrationDialog::pbn_lBoundry_toggled;
    toggledFuncs[2] = &calibrationDialog::pbn_rBoundry_toggled;
    toggledFuncs[3] = &calibrationDialog::pbn_light_toggled;
    toggledFuncs[4] = &calibrationDialog::pbn_center_toggled;
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
        qDebug() << " $$$ >> 1";

    } else {
        curAlgorithm->setAlgorithm(curAC.mode);
        qDebug() << " $$$ >> 2";
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        qDebug() << " $$$ >> 3";
        curAlgorithm->setMiddle(curAC.mid);
        qDebug() << " $$$ >> 4";
        curAlgorithm->setTrack(curAC.track);
        qDebug() << " $$$ >> 5";
        curAlgorithm->startDetect();
        qDebug() << " $$$ >> 6";
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
        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Left Boundry
        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
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
        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Right Boundry
        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
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
        ui->pbn_up->setText(QString("💡%1↑").arg(curAC.light));
        ui->pbn_down->setText(QString("💡%1↓").arg(curAC.light));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Light
        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
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
        curAC.mid = qMin(curAC.mid, curAC.rboundry);
        curAC.mid = qMax(curAC.mid, curAC.lboundry);

        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
        ui->pbn_up->setEnabled(true);
        ui->pbn_down->setEnabled(true);
    }
    else {
        // Switch off Middle
        ui->pbn_up->setText(QString("👉"));
        ui->pbn_down->setText(QString("👈"));
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
    qDebug() << id << '\t' << curAC.mode;

    foreach (QAbstractButton *btn, btnTrackGroup->buttons())
        btn->setEnabled(curAC.mode == 1);
}

void calibrationDialog::handler_btnGroup_track(int id) {
    curAC.track = id;
    curAlgorithm->setTrack(curAC.track);
}


/*🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦*/
void calibrationDialog::on_pbn_up_clicked() {
    switch(onSetting) {
    case(0) :
        return;
    case(1) :
        curAC.lboundry = qMin(curAC.rboundry - MIN_BOUNDARY_INTER, curAC.lboundry + ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        return;
    case(2) :
        curAC.rboundry = qMin(CAMERA_WIDTH, curAC.rboundry + ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        return;
    case(3) :
        curAC.light = qMin(curAC.light + 1, 10);
        ui->pbn_up->setText(QString("💡%1↑").arg(curAC.light));
        ui->pbn_down->setText(QString("💡%1↓").arg(curAC.light));
        return;
    case(4) :
        curAC.mid = qMin(curAC.rboundry, curAC.mid + ADJUST_INTERVAL);
        curAlgorithm->setMiddle(curAC.mid);
        return;
    }
}

void calibrationDialog::on_pbn_down_clicked() {
    switch(onSetting) {
    case(0) :
        return;
    case(1) :
        curAC.lboundry = qMax(0, curAC.lboundry - ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        return;
    case(2) :
        curAC.rboundry = qMax(curAC.lboundry + MIN_BOUNDARY_INTER, curAC.rboundry - ADJUST_INTERVAL);
        curAlgorithm->setBoundary(curAC.lboundry, curAC.rboundry);
        return;
    case(3) :
        curAC.light = qMax(curAC.light - 1, 0);
        ui->pbn_up->setText(QString("💡%1↑").arg(curAC.light));
        ui->pbn_down->setText(QString("💡%1↓").arg(curAC.light));
        return;
    case(4) :
        curAC.mid = qMax(curAC.lboundry, curAC.mid - ADJUST_INTERVAL);
        curAlgorithm->setMiddle(curAC.mid);
        return;
    }
}

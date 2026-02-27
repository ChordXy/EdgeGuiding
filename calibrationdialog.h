#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>
#include <QString>
#include <QPainter>

#include "autoclosedialog.h"
#include "performalgorithms.h"
#include "define.h"


namespace Ui {
class calibrationDialog;
}

class calibrationDialog;
using ToggledFuncPtr = void (calibrationDialog::*)(bool);

class calibrationDialog : public autoCloseDialog
{
    Q_OBJECT

public:
    explicit calibrationDialog(AppConfigs ac, performAlgorithms *tAlgorithm, QWidget *parent = nullptr);
    ~calibrationDialog();

public slots:
    void onReceiveImage(const FrameData &fd);


private slots:
    void on_pbn_return_clicked();

    void on_pbn_up_clicked();
    void on_pbn_down_clicked();

    void handler_btnGroup_settings(QAbstractButton *btn, int btnId, bool isChecked);
    void handler_btnGroup_mode(int id);
    void handler_btnGroup_track(int id);


private:
    Ui::calibrationDialog *ui;

    AppConfigs curAC;
    performAlgorithms *curAlgorithm;

    QButtonGroup *btnSettingsGroup = new QButtonGroup(this);
    QButtonGroup *btnModeGroup = new QButtonGroup(this);
    QButtonGroup *btnTrackGroup = new QButtonGroup(this);

    int onSetting = -1;
    int offSetting = -1;

    QPixmap curPix;

    QPen dashPenY, dashPenW;

    ToggledFuncPtr toggledFuncs[5] = {nullptr};

    // @ Btns in settings.
    void pbn_mode_toggled(bool state);
    void pbn_lBoundry_toggled(bool state);
    void pbn_rBoundry_toggled(bool state);
    void pbn_light_toggled(bool state);
    void pbn_center_toggled(bool state);

    // $ Btns in Mode selection.
    void pbn_mode_line_toggled();
    void pbn_mode_block_toggled();

    void connectSignalsSlots();
    void initEnvoriments();


};

#endif // CALIBRATIONDIALOG_H

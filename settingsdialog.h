#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDebug>

#include "autoclosedialog.h"
#include "unlockdialog.h"
#include "define.h"


namespace Ui {
class settingsDialog;
}

class settingsDialog : public autoCloseDialog
{
    Q_OBJECT

public:
    explicit settingsDialog(AppConfigs ac, QWidget *parent = nullptr);

    ~settingsDialog();

    AppConfigs get_setting_results();

private slots:
    void on_pbn_language_clicked();
    void on_pbn_lockTime_clicked();
    void on_pbn_setCode_clicked();
    void on_pbn_return_clicked();

private:
    Ui::settingsDialog *ui;

    AppConfigs curAC;
    void refreshUI(int nBtn);
};

#endif // SETTINGSDIALOG_H

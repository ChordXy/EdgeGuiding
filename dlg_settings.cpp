#include "dlg_settings.h"
#include "ui_dlg_settings.h"

settingsDialog::settingsDialog(AppConfigs ac, QWidget *parent) :
    autoCloseDialog(parent),
    ui(new Ui::settingsDialog)
{
    ui->setupUi(this);
    // 1. 安装自动关闭事件，并设置自动关闭时间
    this->installEventFilterToAllChildren(this);
    this->set_timer_count(ac.autotime);

    curAC = ac;

    // 2. 分别更新 语言、锁屏时间 内容
    refreshUI(0);
    refreshUI(1);
}

settingsDialog::~settingsDialog()
{
    delete ui;
}

void settingsDialog::refreshUI(int nBtn) {
    /// nBtn :
    ///         0  : language
    ///         1  : lock_time
    ///         2  : reserved
    saveSettings(curAC);
    switch (nBtn) {
    case(0)   :     ui->pbn_language->setText(QString("语言\n-------\n%1").arg(_languages[curAC.language])); return;
    case(1)   :     ui->pbn_lockTime->setText(QString("锁屏时间\n-------\n%1秒").arg(_lock_screen_time[curAC.locktime])); return;
    default   :     return;
    }
}

void settingsDialog::on_pbn_language_clicked()
{
    if (_languages.size() == 1) return;
    curAC.language = (curAC.language + 1) % _languages.size();
    refreshUI(0);
}

void settingsDialog::on_pbn_lockTime_clicked()
{
    if (_lock_screen_time.size() == 1) return;
    curAC.locktime = (curAC.locktime + 1) % _lock_screen_time.size();
    refreshUI(1);
}

void settingsDialog::on_pbn_setCode_clicked()
{
    // 1.暂停当前窗体的自动关闭
    this->stopTimer();

    // 2. 新建解锁对话框
    unlockDialog uD(0, curAC.password, curAC.autotime, this);
    uD.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    uD.setWindowModality(Qt::WindowModal);
    uD.move(0, 0);
    uD.exec();

    // 3. 开启当前窗体自动关闭
    this->startTimer();

    // 4. 获取修改密码结果，如果修改成果，则更新密码 | 否则直接跳过
    if (uD.get_operation_result()) curAC.password = uD.get_new_password();
}

void settingsDialog::on_pbn_return_clicked() {
    // 退出时，保存当前设置
    saveSettings(curAC);
    this->close();
}

AppConfigs settingsDialog::get_setting_results() {
    // 返回当前设置
    return curAC;
}

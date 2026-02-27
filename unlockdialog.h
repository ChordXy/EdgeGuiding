#ifndef UNLOCKDIALOG_H
#define UNLOCKDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QTimer>
#include <QString>

#include "autoclosedialog.h"


QT_BEGIN_NAMESPACE
namespace Ui { class unlockDialog; }
QT_END_NAMESPACE

class unlockDialog : public autoCloseDialog
{
    Q_OBJECT

public:
    explicit unlockDialog(int mode, QString password_original, int n_auto_close_count, QWidget *parent = nullptr);
    ~unlockDialog();

    bool get_operation_result();
    QString get_new_password();

private slots:
    void on_pbn_see_clicked();
    void on_pbn_delete_clicked();
    void on_pbn_confirm_clicked();
    void on_pbn_return_clicked();

private:
    Ui::unlockDialog *ui;
    int m_mode;                 // 0 代表密码设置模式，1代表密码校验模式
    int m_display;              // 0 代表明码，1代表暗码
    int m_passwordLength = 4;   // 密码长度为4位
    bool success_flag = false;  // true：修改/解锁成功，关闭dialog

    QString m_inputPassword;    // 保存输入的密码
    QString m_newPassword;      // 新密码
    QString m_oldPassword;      // 原始密码

    QString codeImage[2] = {
        ":/image/open.png",
        ":/image/close.png",
    };

    void display_pad(int display);
    void update_ConfirmBtn_State();
    void on_pbn_pad_clicked();              // 替换原有10个数字按键槽函数，新增通用数字按键槽函数
};
#endif // UNLOCKDIALOG_H

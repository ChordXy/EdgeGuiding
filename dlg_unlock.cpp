#include "dlg_unlock.h"
#include "ui_dlg_unlock.h"

unlockDialog::unlockDialog(int mode, QString password_original, int n_auto_close_count, QWidget *parent)
    : autoCloseDialog(parent)
    , ui(new Ui::unlockDialog)
{
    ui->setupUi(this);
    // 1. 安装自动关闭事件，并设置自动关闭时间
    this->installEventFilterToAllChildren(this);
    this->set_timer_count(n_auto_close_count);

    // 2. （锁定/解锁）模式、密码等初始化
    m_mode = mode;
    m_display= mode;
    ui->pbn_see->setIcon(QIcon(codeImage[m_display]));
    m_oldPassword = password_original;

    // 3. 界面初始化
    m_inputPassword = "";
    ui->lbl_password->setText("- - - -");
    update_ConfirmBtn_State();

    // 4. 核心：绑定所有数字按键的clicked信号到通用槽函数
    connect(ui->pbn_pad0, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad1, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad2, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad3, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad4, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad5, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad6, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad7, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad8, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
    connect(ui->pbn_pad9, &QPushButton::clicked, this, &unlockDialog::on_pbn_pad_clicked);
}

unlockDialog::~unlockDialog() {
    delete ui;
}

void unlockDialog::display_pad(int display) {
    QString displayStr = "- - - -";

    for (int i = 0; i < m_inputPassword.length(); i++) {
        if (display == 0) {
            displayStr[2 * i] = m_inputPassword[i];
        } else {
            displayStr[2 * i] = '*';
        }
    }

    //更新显示
    ui->lbl_password->setText(displayStr);
}

void unlockDialog::on_pbn_pad_clicked() {
    // 1. 获取点击的按钮对象
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return; // 非按钮触发则直接返回

    // 2. 获取按钮上的数字（按钮text需为"0"-"9"，若UI里按钮text不是数字，可改用objectName判断）
    QString input_number = btn->text();

    // 3. 复用原有逻辑
    if (m_inputPassword.length() < m_passwordLength) {
        m_inputPassword += input_number;
    }

    // 4. 输入栏、按钮状态的更新
    display_pad(m_display);
    update_ConfirmBtn_State();
}

void unlockDialog::on_pbn_see_clicked() {
    m_display = 1 - m_display;
    ui->pbn_see->setIcon(QIcon(codeImage[m_display]));
    display_pad(m_display);
}

void unlockDialog::on_pbn_delete_clicked() {
    if (m_inputPassword.length() == 0)   return;
    m_inputPassword.chop(1);
    display_pad(m_display);
    update_ConfirmBtn_State();
}

void unlockDialog::update_ConfirmBtn_State() {
    ui->pbn_confirm->setEnabled(m_inputPassword.length() >= m_passwordLength);
}

void unlockDialog::on_pbn_confirm_clicked() {
    // 1. 如果是解锁状态，密码错误时
    if (m_mode == 1 && m_inputPassword != m_oldPassword) {
        ui->lbl_password->setText("密码错误！");
        m_inputPassword = "";
        success_flag = false;
    }

    // 2. 如果是密码设置状态，更新密码
    if (m_mode == 0) {
        m_newPassword = m_inputPassword;
        ui->lbl_password->setText("修改成功！");
        success_flag = true;
    }

    // 3. 如果是解锁状态，且密码正确
    if (m_mode == 1 && m_inputPassword == m_oldPassword) {
        ui->lbl_password->setText("解锁成功！");
        success_flag = true;
    }

    // 4. 如果 解锁/密码设置 成功，则禁用除返回外全部按钮，延时一秒后关闭窗体
    if(success_flag) {
        foreach (QPushButton* btn, this->findChildren<QPushButton*>()) {
            if (!btn->objectName().endsWith("return")) { // 匹配以pbn开头的按钮
                btn->setEnabled(false); // 禁用按钮
                // 可选：设置禁用样式（比如灰显）
                btn->setStyleSheet("QPushButton:disabled { background-color: #cccccc; color: #666666; }");
            }
        }

        // 4.1 延时关闭
        QTimer::singleShot(1000, this, [=]() {this->close();});
    }
}

void unlockDialog::on_pbn_return_clicked()
{
    this->close();
}

bool unlockDialog::get_operation_result()
{
    return success_flag;
}

QString unlockDialog::get_new_password()
{
    return m_newPassword;
}

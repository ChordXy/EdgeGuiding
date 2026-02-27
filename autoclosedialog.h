#ifndef AUTOCLOSEDIALOG_H
#define AUTOCLOSEDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEvent>
#include <QDebug>


class autoCloseDialog : public QDialog {
    Q_OBJECT
public:
    autoCloseDialog(QWidget *parent = nullptr) : QDialog(parent) {
        // ========== 1. 初始化空闲定时器 ==========
        m_idleTimer = new QTimer(this);
        m_idleTimer->setSingleShot(true);   // 单次触发：超时后只关闭一次
        m_idleTimer->setInterval(1000 * _n_close_timer_count);     // 5秒无操作自动关闭
        connect(m_idleTimer, &QTimer::timeout, this, &autoCloseDialog::onIdleTimeout);

        // ========== 2. 安装事件过滤器（监控自身+所有子控件） ==========
        this->installEventFilter(this);

        // ========== 3. 启动定时器 ==========
        m_idleTimer->start();
    }

    void set_timer_count(int n){
        _n_close_timer_count = n;
        m_idleTimer->setInterval(1000 * _n_close_timer_count);     // 5秒无操作自动关闭
        if (m_idleTimer->isActive()) {
            m_idleTimer->stop();
        }
        m_idleTimer->start();
    }

    void stopTimer() {
        if (m_idleTimer->isActive()) {
            m_idleTimer->stop();
        }
    }

    void startTimer() {
        m_idleTimer->start();
    }

    void installEventFilterToAllChildren(QWidget *parentWidget) {
        if (!parentWidget) return;

        // 1. 获取父控件下的所有直接子控件（递归获取所有层级）
        QList<QWidget *> allChildren = parentWidget->findChildren<QWidget *>();

        // 2. 遍历所有子控件，安装事件过滤器
        foreach (QWidget *child, allChildren) {
            child->installEventFilter(this);
//            qDebug() << "为子控件安装过滤器：" << child->metaObject()->className() << "(" << child->objectName() << ")";
        }
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (this->isVisible()) {
            switch (event->type()) {
            // 鼠标事件（点击/移动/滚轮）
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:       // 可选：鼠标移动算操作（注释则仅点击算）
            case QEvent::Wheel:
            // 键盘事件
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
            // 触摸事件（触摸屏适配）
            case QEvent::TouchBegin:
            case QEvent::TouchUpdate:
            case QEvent::TouchEnd:
                // 重置定时器：停止当前计时，重新开始
                if (m_idleTimer->isActive()) {
                    m_idleTimer->stop();
                }
                m_idleTimer->start();
                break;
            default:
                break;
            }
        }
        // 事件透传：不影响控件原有逻辑（如按钮点击、输入框输入）
        return QDialog::eventFilter(watched, event);
    }

    void showEvent(QShowEvent *event) override
    {
        m_idleTimer->start(); // 显示时重启定时器
        QDialog::showEvent(event);
    }

private slots:
    // 定时器超时：关闭Dialog
    void onIdleTimeout() {
        this->close();
    }

private:
    QTimer *m_idleTimer;    // 空闲自动关闭定时器

    int _n_close_timer_count = 3;
};


#endif // AUTOCLOSEDIALOG_H

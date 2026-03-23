#ifndef AUTOLOCKWINDOW_H
#define AUTOLOCKWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QEvent>
#include <QDebug>


class autoLockWindow : public QWidget {
    Q_OBJECT
public:
    autoLockWindow(QWidget *parent = nullptr) : QWidget(parent) {
        // ========== 1. 初始化空闲定时器 ==========
        m_idleTimer = new QTimer(this);
        m_idleTimer->setSingleShot(true);
        m_idleTimer->setInterval(1000 * _n_lcok_timer_count);
        connect(m_idleTimer, &QTimer::timeout, this, &autoLockWindow::onIdleTimeout);

        // ========== 2. 安装事件过滤器（监控自身+所有子控件） ==========
        this->installEventFilter(this);

        // ========== 3. 启动定时器 ==========
        m_idleTimer->start();
    }

    void set_timer_count(int n){
        _n_lcok_timer_count = n;
        m_idleTimer->setInterval(1000 * _n_lcok_timer_count);
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
        return QWidget::eventFilter(watched, event);
    }

    void showEvent(QShowEvent *event) override
    {
        m_idleTimer->start(); // 显示时重启定时器
        QWidget::showEvent(event);
    }
signals:
    void locked();

private slots:
    // 定时器超时：发送锁屏信号
    void onIdleTimeout() {
        emit locked();
    }

private:
    QTimer *m_idleTimer;    // 空闲自动锁屏定时器
    int _n_lcok_timer_count = 2;
};



#endif // AUTOLOCKWINDOW_H

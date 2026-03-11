#include "longPressButton.h"
#include <QMouseEvent>

longPressButton::longPressButton(QWidget *parent)
    : QPushButton(parent)
    , m_pressDelay(500)    // 默认长按500ms后开始连击
    , m_clickInterval(100) // 默认每秒10次连击
{
    m_delayTimer = new QTimer(this);        // 初始化延迟定时器（仅触发一次）
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer, &QTimer::timeout, this, &longPressButton::onDelayTimeout);

    m_clickTimer = new QTimer(this);        // 初始化连击定时器（循环触发）
    connect(m_clickTimer, &QTimer::timeout, this, &longPressButton::onClickTimeout);
}

void longPressButton::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        m_delayTimer->start(m_pressDelay);  // 按下左键：启动延迟定时器
    }
    QPushButton::mousePressEvent(e);        // 保留原有的鼠标按下逻辑（比如按钮按下样式）
}

void longPressButton::mouseReleaseEvent(QMouseEvent *e) {
    m_delayTimer->stop();               // 释放鼠标：停止所有定时器
    m_clickTimer->stop();               // 释放鼠标：停止所有定时器
    QPushButton::mouseReleaseEvent(e);  // 保留原有的鼠标释放逻辑
}

void longPressButton::leaveEvent(QEvent *e) {
    m_delayTimer->stop();               // 鼠标离开按钮时停止所有定时器，防止长按移出后仍连击
    m_clickTimer->stop();               // 鼠标离开按钮时停止所有定时器，防止长按移出后仍连击
    QPushButton::leaveEvent(e);     // 调用父类的 leaveEvent
}

void longPressButton::onDelayTimeout() {
    m_clickTimer->start(m_clickInterval);   // 延迟结束：启动连击定时器，且立即触发一次点击
    onClickTimeout();
}

void longPressButton::onClickTimeout() {
    emit clicked();                         // 发送点击信号（和普通点击效果一致）
    // 若需要区分“长按连击”和“普通点击”，可自定义信号：
    // emit longPressClicked();
}

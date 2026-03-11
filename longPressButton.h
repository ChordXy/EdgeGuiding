#ifndef longPressButton_H
#define longPressButton_H

#include <QPushButton>
#include <QTimer>

class longPressButton : public QPushButton
{
    Q_OBJECT

public:
    explicit longPressButton(QWidget *parent = nullptr);

    int pressDelay() const { return m_pressDelay; }             // 获取长按延迟（默认500ms：按下后多久开始连击）
    void setPressDelay(int ms) {m_pressDelay = qMax(0, ms); }   // 设置长按延迟（默认500ms：按下后多久开始连击）

    int clickInterval() const { return m_clickInterval; }       // 获取连击间隔（默认100ms：连击的频率）
    void setClickInterval(int ms) {                             // 设置连击间隔（默认100ms：连击的频率）
        m_clickInterval = qMax(ms, 10);
        m_clickTimer->setInterval(m_clickInterval);
    }

protected:  // 重写鼠标事件
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private slots:
    void onDelayTimeout();                  // 延迟定时器触发：开始连击
    void onClickTimeout();                  // 连击定时器触发：发送点击信号

private:
    QTimer *m_delayTimer;    // 长按延迟定时器
    QTimer *m_clickTimer;    // 连击定时器
    int m_pressDelay;        // 长按延迟（ms）
    int m_clickInterval;     // 连击间隔（ms）
};

#endif // longPressButton_H

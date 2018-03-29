#ifndef ANIMATIONPROGRESSBAR_H
#define ANIMATIONPROGRESSBAR_H

#include <QWidget>
#include <QVariant>
#include <QProgressBar>
#include <QPropertyAnimation>

class AnimationProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    AnimationProgressBar();

protected:
    bool start;
    bool end;

    //滚动条增加动画
    QPropertyAnimation *proAnimation;

    qreal m_value;

private slots:
    void m_valueChanged(QVariant index);
    void setProValue(QVariant value);
signals:
    void StartProValue(QVariant value);
};

#endif // ANIMATIONPROGRESSBAR_H

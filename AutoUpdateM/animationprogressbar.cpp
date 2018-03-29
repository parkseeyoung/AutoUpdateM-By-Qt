#include "animationprogressbar.h"

AnimationProgressBar::AnimationProgressBar()
{
    m_value = 0;
    proAnimation = new QPropertyAnimation(this,"");
    proAnimation->setStartValue(0);
    proAnimation->setEndValue(0);
    connect(proAnimation,SIGNAL(valueChanged(QVariant)),this,SLOT(m_valueChanged(QVariant)));
    connect(this,SIGNAL(StartProValue(QVariant)),this,SLOT(setProValue(QVariant)));
}
void AnimationProgressBar::m_valueChanged(QVariant index)
{
    this->setValue(index.toInt());
}
void AnimationProgressBar::setProValue(QVariant value)
{
    proAnimation->setStartValue(proAnimation->endValue());
    proAnimation->setEndValue(value);
    proAnimation->setDuration(2000);
    proAnimation->start();
}

#include "bardelegate.h"
#include <QPainter>
#include <QProgressBar>
#include "animationprogressbar.h"
#include <QApplication>
BarDelegate::BarDelegate(QObject *parent)
{

}
void BarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant value = index.data();
    AnimationProgressBar * apb = new AnimationProgressBar();
    apb->StartProValue(value);
    //绘制进度条
}

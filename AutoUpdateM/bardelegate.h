#ifndef BARDELEGATE_H
#define BARDELEGATE_H
#include <QItemDelegate>

class BarDelegate : public QItemDelegate
{
public:
    BarDelegate(QObject *parent = 0 );
private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // BARDELEGATE_H

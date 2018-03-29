#include "uploaddelegate.h"
#include <QPainter>
#include <QDebug>
#include <QComboBox>
UploadDelegate::UploadDelegate(ComboBoxType m_type, QWidget *parent)
    :QItemDelegate(parent),type(m_type)
{

}
QWidget * UploadDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    if(type == FileType)
    {
        editor->addItem(QString::fromLocal8Bit("更新网站"));
        editor->addItem(QString::fromLocal8Bit("更新软件"));
    }
    if(type == IfCover)
    {
        editor->addItem(QString::fromLocal8Bit("更新覆盖"));
        editor->addItem(QString::fromLocal8Bit("更新启动"));
    }
    return editor;
}
void UploadDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    QString str =index.model()->data(index).toString();

    QComboBox *box = static_cast<QComboBox*>(editor);
    int i=box->findText(str);
    box->setCurrentIndex(i);
}
void UploadDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *box = static_cast<QComboBox*>(editor);
    QString str = box->currentText();
    model->setData(index,str);
}

void UploadDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

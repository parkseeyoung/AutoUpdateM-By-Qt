#ifndef UPLOADDELEGATE_H
#define UPLOADDELEGATE_H

#include <QItemDelegate>

class UploadDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    enum ComboBoxType{FileType,IfCover};
    UploadDelegate(ComboBoxType m_type, QWidget *parent = 0);
private:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private slots:
   // void commitAndCloseEditor();
private:
    ComboBoxType type;
};

#endif // UPLOADDELEGATE_H

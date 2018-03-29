#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QtSql/QtSql>
#include <QStandardItemModel>
#include "mydatabase.h"
#include <QFile>
#include "uploaddelegate.h"
#include "uploadfile.h"
#include "xmlconfig.h"
#include <QThreadPool>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "sendmesgtosevr.h"

// gSoap lib import
#include "./include/soapH.h"
#include "./include/soapStub.h"

// Qt lib import
#include <QtCore>

//全局原子对象
extern std::atomic<int> threadCount;
//全局互斥锁
extern std::mutex mtx;
//全局条件变量
extern std::condition_variable cv;


class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    bool TestWebService();
    QString queryData_WS();

private slots:
    void treeClick(const QModelIndex &index);
    void treeItemChanged(QStandardItem * item);

    void updateCheckTree(const QModelIndex &index);
    void saveTable(bool);
    void uploadingTable();
    void submitFile();
    void showToolTip(const QModelIndex &index);
    //右键菜单
    void TreeViewcontextMenu(const QPoint &pos);
    //ut表右键操作
    void utContextMenu(const QPoint &pos);

    //设置这个服务器
    void setServer();

    //删除这个文件
    void deleteFile();
    //清空列表
    void clearTable();
    //单个文件上传成功
    void fileUploadFinished(const int row, const qreal progress);

    //接收到服务器消息
    void getServMesg(int res);

    //更新进度条
    void updateProcBar(QVariant value);

    //更新view
    void updateInfoInquire();

    //uptree被点击
    void upRecordTreeClick(const QModelIndex &index);
//Action
public:
    QAction *exitAction;
    QAction *saveAction;
    QStatusBar *statusbar;

    //当前选中的treeindex
    QModelIndex selectedIndex;
    //当前选中的utindex
    QModelIndex selectedUtIndex;
    //当前服务器
    QModelIndex currentIndex;
private:
    QTreeView *view;
    MyDataBase *m_pDB;
    QTableView * tview;
    QStandardItemModel *tmodel;
    QSqlRelationalTableModel *qtm;
    //上传的table
    QTableView *ut;
    QStandardItemModel *utModel;


    //checktree and Configuration file
    QSplitter *config;

    //上传情况查询
    QSplitter *upInfo;
    QTreeView *checkedTree;
    QStandardItemModel* ctmodel;
    //上传情况查询表
    QTreeView * upRecordTreeView;
    QStandardItemModel *upInfoTreeMd;

    QTableView *upRecordTabelView;
    QStandardItemModel *upInfoTableMd;


    //菜单
    QMenuBar *menubar;

    //文件名称列表
    QStringList * list_file;

    //QTreeView的右键菜单
    QMenu * m_treeMenu;
    //ut的右键菜单
    QMenu * m_utMenu;

    //成功接到消息
    bool m_GetMesg;

    //线程
    QThread *thread;
    //动画
    QPropertyAnimation *UploadProc;
    //当前行
    int currRow;
    //softname
    QComboBox *SoftListCmb;
    //单位的服务器ip列表
    QStringList m_addr;
    //当前服务器序号
    int m_addrIndex;
private:
    void iniAction();
    void iniConnect();
    void iniTable();
    void iniTree();
    //初始化动画
    void iniAnimation();
    //通过数据库新建节点
    void addCheckedItem();
    //上传文件表
    void iniUploadTable();
    //绑定数据
    void addUploadFile();

    void updateTree();
    void updateTable();
    void updateUT();
    //上传情况表
    void iniUpRecordTable();

    //加载样式表
    void initStyle();

    bool ConnectDB(QString strPath);


    //获取文件图片
    QIcon fileIcon(const QString &extension) const;

    //设置主服务器后的配置
    void setUnEdit(QStandardItem *fwq);
        //还原
    void setCanEdit();
        //设置childUnEdit
    void setChildUnEdit(QStandardItem *child);

    //子节点递归全选
    void treeItem_checkAllChild(QStandardItem * item, bool check);
    void treeItem_checkAllChild_recursion(QStandardItem * item,bool check);
    void treeItem_CheckChildChanged(QStandardItem * item);
    Qt::CheckState checkSibling(QStandardItem *item);


    /*---简单的发消息---*/
    void sendToSvrMessage(QStringList &_filenames);

private:
    void Highlights();

    void closeEvent(QCloseEvent *event);
    void readSettings();

    //uptable刷新
    void upRecordTabelRefresh(QString);

};

#endif // WIDGET_H

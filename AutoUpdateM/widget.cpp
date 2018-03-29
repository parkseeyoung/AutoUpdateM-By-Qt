#define TIME_OUT 1
#include "widget.h"

/*webservice采用gsoap生成*/
#include <sapi.h>
#include <sphelper.h>
#include "./include/updataServiceSoap.nsmap"

#pragma comment(lib,"sapi.lib")
#define BACKLOG (100)    /* Max. request backlog */

std::atomic<int> threadCount = 0;
std::mutex mtx;
std::condition_variable cv;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QString wtitle = QString::fromLocal8Bit("软件自动更新系统文件上传");
    setWindowTitle(wtitle);

    m_pDB = MyDataBase::GetInstance();
    //从配置文件中读取数据库
    QString iniPath = ".\\DataPath.ini";
    QSettings *dataPathConfigIni = new QSettings(iniPath,QSettings::IniFormat);
    QString dataPath = dataPathConfigIni->value("DATAPATH").toString();
    ConnectDB(dataPath);

    m_addrIndex = -1;
    initStyle();
    iniAnimation();
    iniTable();
    iniTree();
    iniConnect();
    this->setMinimumSize(800,500);
    connect(view,SIGNAL(clicked(QModelIndex)),this,SLOT(treeClick(const QModelIndex)));

    updateTree();
    updateTable();
    //showFullScreen();

//    QComboBox *d = new QComboBox(this);
//    QListView *l = new QListView(this);
//    l->addAction(exitAction);
//    d->setView(l);
}

Widget::~Widget()
{

}
//退出时保存最后一次的几何形状和位置
void Widget::closeEvent(QCloseEvent *event)
{
    QSettings settings("ZBXH","AUTOUPDATE");
    settings.setValue("geometry",saveGeometry());
    //settings.setValue("windowState",saveState());
    QWidget::closeEvent(event);
}

void Widget::treeClick(const QModelIndex & index)
{
    qtm->setFilter(QString::fromLocal8Bit("名称='%1'").arg(index.data().toString()));
    updateTable();
    //qtm->setFilter("");
}

void Widget::iniAction()
{
    menubar = new QMenuBar(this);

    QMenu *menu = new QMenu(QString::fromLocal8Bit("文件"));
    saveAction = menu->addAction(QString::fromLocal8Bit("保存"));
    exitAction = menu->addAction(QString::fromLocal8Bit("退出"));


    statusbar = new QStatusBar();
    menubar->addMenu(menu);

    //建立QTreeView的右键菜单
    QAction *setThisSer = new QAction(QString::fromLocal8Bit("设置当前服务器"));
    m_treeMenu = new QMenu(this);
    m_treeMenu->addAction(setThisSer);
    connect(setThisSer,SIGNAL(triggered(bool)),this,SLOT(setServer()));


    //建立ut的右键菜单
    QAction *deleteThisFile = new QAction(QString::fromLocal8Bit("移除文件"));
    m_utMenu = new QMenu(this);
    m_utMenu->addAction(deleteThisFile);
    connect(deleteThisFile,SIGNAL(triggered(bool)),this,SLOT(deleteFile()));

    QAction *deleteAllFile = new QAction(QString::fromLocal8Bit("清空列表"));
    m_utMenu->addAction(deleteAllFile);
    connect(deleteAllFile,SIGNAL(triggered(bool)),this,SLOT(clearTable()));

}
//初始化动画
void Widget::iniAnimation()
{
    UploadProc = new QPropertyAnimation();
    UploadProc->setStartValue(0);
    UploadProc->setEndValue(0);
    connect(UploadProc,SIGNAL(valueChanged(QVariant)),this,SLOT(updateProcBar(QVariant)));
}

//删除列表
void Widget::clearTable()
{
    utModel->removeRows(0,utModel->rowCount());
    list_file->clear();
    //清空文件列表
}

//删除选中行文件
void Widget::deleteFile()
{
    if(!selectedUtIndex.isValid())
        return;
    int selectedRow = selectedUtIndex.row();
    utModel->removeRow(selectedRow);
    list_file->removeAt(selectedRow);
}

bool Widget::ConnectDB(QString strPath)
{
    if(NULL == m_pDB)
    {
        return false;
    }
    if(!m_pDB->ConnectAccessDB(strPath,"",""))
    {
        return false;
    }
    return true;
}

void Widget::iniConnect()
{
/*********************布局*************************/
    QLayout *exist_layout = this->layout();
    if(exist_layout)
    {
        delete exist_layout;
    }
    QSplitter *layout = new QSplitter;
    layout->addWidget(view);
    layout->addWidget(tview);
    layout->setContentsMargins(0,0,0,0);
    QGridLayout *sLayout = new QGridLayout;

    //tab
    QTabWidget *tab = new QTabWidget(this);
    tab->setTabPosition(QTabWidget::North);

    tab->addTab(layout,QString::fromLocal8Bit("设置关联服务器"));


    config = new QSplitter();
    tab->addTab(config,QString::fromLocal8Bit("上传文件"));
    upInfo = new QSplitter();
    tab->addTab(upInfo,QString::fromLocal8Bit("上传情况查询"));

        //添加一个勾选的tree
    //iniCheckedTree();
    addCheckedItem();
    //menu
    iniAction();

    //上传文件
    iniUploadTable();

    //上传情况表
    iniUpRecordTable();


    sLayout->addWidget(menubar,0,0);
    sLayout->setMargin(0);
    sLayout->addWidget(tab,1,0);
    sLayout->addWidget(statusbar,2,0);
    this->setLayout(sLayout);

/*********************connect*************************/
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(saveAction, SIGNAL(triggered(bool)),this,SLOT(saveTable(bool)));
}
//上传情况
void Widget::iniUpRecordTable()
{
    //tree
    upRecordTreeView = new QTreeView();
    upInfoTreeMd = new QStandardItemModel(upRecordTreeView);
    upInfoTreeMd->setHorizontalHeaderLabels(QStringList()<<QString::fromLocal8Bit("单位名称"));

    upRecordTreeView->setModel(upInfoTreeMd);
    upInfo->addWidget(upRecordTreeView);

    //双击变单击
    connect(upRecordTreeView,SIGNAL(clicked(QModelIndex)),upRecordTreeView,SLOT(edit(QModelIndex)));
    //建立单击的connect
    connect(upRecordTreeView,SIGNAL(clicked(QModelIndex)),this,SLOT(upRecordTreeClick(QModelIndex)));
    //table
    upRecordTabelView = new QTableView();
    upInfoTableMd = new QStandardItemModel(upRecordTabelView);
    upInfoTableMd->setHorizontalHeaderLabels(QStringList()<<QString::fromLocal8Bit("软件名称")<<QString::fromLocal8Bit("更新时间")
                                             <<QString::fromLocal8Bit("更新结果")
                                             <<QString::fromLocal8Bit("更新服务器")<<QString::fromLocal8Bit("更新服务器ip"));
    upRecordTabelView->setModel(upInfoTableMd);
    upRecordTabelView->horizontalHeader()->setStretchLastSection(true);

    upInfo->addWidget(upRecordTabelView);

}
void Widget::upRecordTreeClick(const QModelIndex &index)
{
    QString dwname = index.data().toString();
    upRecordTabelRefresh(dwname);
    for (int column = 0; column < upInfoTreeMd->columnCount(); ++column)
    {
        upRecordTreeView->resizeColumnToContents(column);
    }
}
void Widget::upRecordTabelRefresh(QString dwname)
{
    //只调用了一次webservice，然后将结果保存到本地，现在只要用xml里面的东西就行
    XmlConfig::refreshUpRecordTable(upInfoTableMd,dwname);
}

//上传文件表
void Widget::iniUploadTable()
{
    ut = new QTableView();
    ut->horizontalHeader()->setStretchLastSection(true);
    utModel = new QStandardItemModel();
    utModel->setHorizontalHeaderLabels(QStringList()<<QString::fromLocal8Bit("文件名")
                                       <<QString::fromLocal8Bit("文件操作")
                                       <<QString::fromLocal8Bit("文件相对路径"));
    ut->setModel(utModel);
    QPushButton *uploadBtn = new QPushButton(QString::fromLocal8Bit("添加上传文件"));
    connect(uploadBtn,SIGNAL(clicked(bool)),this,SLOT(uploadingTable()));
    QPushButton *submitBtn = new QPushButton(QString::fromLocal8Bit("开始上传"));
    connect(submitBtn,SIGNAL(clicked(bool)),this,SLOT(submitFile()));
    QLabel *selectSoftLab = new QLabel(QString::fromLocal8Bit("选择上传的更新软件名称:"));
    SoftListCmb = new QComboBox();
    QStringList softList;
    softList<<"soft1"<<"soft2";
    SoftListCmb->addItems(softList);
    QGridLayout *selectSoftLayout = new QGridLayout();
    selectSoftLayout->addWidget(selectSoftLab,0,0);
    selectSoftLayout->addWidget(SoftListCmb,0,1);

    QGridLayout * qgl = new QGridLayout();
    qgl->addWidget(ut,0,0);
    qgl->addLayout(selectSoftLayout,1,0);
    qgl->addWidget(uploadBtn,2,0);
    qgl->addWidget(submitBtn,3,0);

    QGroupBox *opbtns = new QGroupBox();
    opbtns->setLayout(qgl);
    config->addWidget(opbtns);

    //ut->setItemDelegateForColumn(1,new UploadDelegate(UploadDelegate::FileType,this));
    ut->setItemDelegateForColumn(1,new UploadDelegate(UploadDelegate::IfCover,this));

    //初始化文件名列表
    list_file = new QStringList;
    //设置鼠标跟随
    ut->setMouseTracking(true);
    connect(ut,SIGNAL(entered(QModelIndex)),this,SLOT(showToolTip(QModelIndex)));
    ut->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ut,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(utContextMenu(QPoint)));
}
//ut表右键操作
void Widget::utContextMenu(const QPoint &pos)
{
    selectedUtIndex = ut->indexAt(pos);
    m_utMenu->exec(QCursor::pos());
}

//设置hint
void Widget::showToolTip(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    int row = index.row();
    QString file_name = list_file->at(row);

    if(file_name.isEmpty())
        return;
    QToolTip::showText(QCursor::pos(),file_name);
}

/*------------------------------------------------------------------------------*/
void Widget::addCheckedItem()
{
    checkedTree = new QTreeView();
    ctmodel = new QStandardItemModel(checkedTree);
    ctmodel->setHorizontalHeaderLabels(QStringList()<<QString::fromLocal8Bit("单位名称")<<QString::fromLocal8Bit("更新终端软件"));

    QString rootName;
    QString coding;

    QSqlQuery query(QSqlDatabase::database("MyAccessDB"));
    QString sql = QString::fromLocal8Bit("select 名称,编码 from BD_AU_单位字典表 where 序号=0");
    query.exec(sql);
    query.next();
    rootName = query.value(QString::fromLocal8Bit("名称")).toString();
    coding = query.value(QString::fromLocal8Bit("编码")).toString();
    //加了才知道row
    QStandardItem* rootItem = new QStandardItem(QIcon(":/train.png"),rootName);
    rootItem->setCheckable(true);
    rootItem->setEditable(false);
    rootItem->setTristate(true);
    QStandardItem* rootzdItem = new QStandardItem("");
    rootzdItem->setEditable(false);
    rootzdItem->setCheckable(true);
    ctmodel->appendRow(rootItem);
    ctmodel->setItem(ctmodel->indexFromItem(rootItem).row(),1,rootzdItem);


    //路局
    sql = QString::fromLocal8Bit("select 名称,编码 from BD_AU_单位字典表 where 上级编码='%1' and 类码<>'W'").arg(coding);
    query.exec(sql);
    qDebug()<<query.lastError()<<"\n"<<sql;
    while(query.next())
    {
        QString ljName = query.value(QString::fromLocal8Bit("名称")).toString();
        QString bmName = query.value(QString::fromLocal8Bit("编码")).toString();

        QStandardItem * ljItem = new QStandardItem(QIcon(":/train.png"),ljName);
        ljItem->setCheckable(true);
        ljItem->setEditable(false);
        ljItem->setTristate(true);
        rootItem->appendRow(ljItem);
        //路局终端
        QStandardItem * ljZdItem = new QStandardItem("");
        ljZdItem->setEditable(false);
        ljZdItem->setCheckable(true);
        rootItem->setChild(ljItem->index().row(),1,ljZdItem);

        QSqlQuery queryDwd(QSqlDatabase::database("MyAccessDB"));
        sql = QString::fromLocal8Bit("select 名称,编码 from BD_AU_单位字典表 where 上级编码='%1' and 类码<>'W'").arg(bmName);
        queryDwd.exec(sql);
        while(queryDwd.next())
        {
            QString dwdName = queryDwd.value(QString::fromLocal8Bit("名称")).toString();
            QString dwdBmName = queryDwd.value(QString::fromLocal8Bit("编码")).toString();

            QStandardItem * dwdItem = new QStandardItem(QIcon(":/train.png"),dwdName);
            dwdItem->setCheckable(true);
            dwdItem->setEditable(false);

            //电务段终端
            QStandardItem * dwdZdItem = new QStandardItem("");
            dwdZdItem->setEditable(false);
            dwdZdItem->setCheckable(true);
            ljItem->appendRow(dwdItem);
            ljItem->setChild(dwdItem->index().row(),1,dwdZdItem);
        }
    }

    //
    checkedTree->setModel(ctmodel);
    config->addWidget(checkedTree);
    //connect(ctmodel,&QStandardItemModel::itemChanged,this,&Widget::treeItemChanged);
    connect(checkedTree,SIGNAL(expanded(QModelIndex)),this,SLOT(updateCheckTree(QModelIndex)));
    checkedTree->expand(ctmodel->index(0,0));
}

void Widget::treeItemChanged(QStandardItem *item)
{
    if ( item == nullptr )
        return ;
        if ( item->isCheckable ())
        {
            //如果条目是存在复选框的，那么就进行下面的操作
            Qt::CheckState state = item->checkState (); //获取当前的选择状态
            if ( item->isTristate ())
            {
                 //如果条目是三态的，说明可以对子目录进行全选和全不选的设置
                if ( state!= Qt::PartiallyChecked )
                {
                    //当前是选中状态，需要对其子项目进行全选
                    treeItem_checkAllChild( item , state == Qt::Checked ? true : false );
                }
            }
            else
            {
                //说明是两态的，两态会对父级的三态有影响
                //判断兄弟节点的情况

                treeItem_CheckChildChanged ( item );
                return;
            }
        }
}
void Widget::treeItem_checkAllChild(QStandardItem *item, bool check)
{
    if(item == nullptr)
        return;
    int rowCount = item->rowCount();
    for(int i=0;i<rowCount;++i)
    {
        QStandardItem* childItems = item->child(i);
        treeItem_checkAllChild_recursion(childItems,check);
    }
    if(item->isCheckable())
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
}

void Widget::treeItem_checkAllChild_recursion(QStandardItem *item, bool check)
{
    if(item == nullptr)
        return;
    int rowCount = item->rowCount();
    for(int i = 0;i<rowCount;++i)
    {
        QStandardItem* childItems = item->child(i);
        treeItem_checkAllChild_recursion(childItems,check);
    }
    if(item->isCheckable())
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
}
//用来处理子节点对父节点的影响
void Widget::treeItem_CheckChildChanged(QStandardItem *item)
{
    if(nullptr == item)
        return;
    Qt::CheckState siblingState = checkSibling(item);
    QStandardItem * parentItem = item->parent();
    if(nullptr == parentItem)
        return;
    if(Qt::PartiallyChecked == siblingState)
    {
        if(parentItem->isCheckable() && parentItem->isTristate())
            parentItem->setCheckState(Qt::PartiallyChecked);
    }
    else if(Qt::Checked == siblingState)
    {
        if(parentItem->isCheckable())
            parentItem->setCheckState(Qt::Checked);
    }
    else
    {
        if(parentItem->isCheckable())
            parentItem->setCheckState(Qt::Unchecked);
    }
    treeItem_CheckChildChanged(parentItem);
}
Qt::CheckState Widget::checkSibling(QStandardItem * item)
{
    //先通过父节点获取兄弟节点
    QStandardItem * parent = item->parent();
    if(nullptr == parent)
        return item->checkState();
    int brotherCount = parent->rowCount();
    int checkedCount(0),unCheckedCount(0);
    Qt::CheckState state;
    for(int i=0;i<brotherCount;++i)
    {
        QStandardItem* siblingItem = parent->child(i);
        state = siblingItem->checkState();
        if(Qt::PartiallyChecked == state)
            return Qt::PartiallyChecked;
        else if(Qt::Unchecked == state)
            ++unCheckedCount;
        else
            ++checkedCount;
        if(checkedCount>0 && unCheckedCount>0)
            return Qt::PartiallyChecked;
    }
    if(unCheckedCount>0)
        return Qt::Unchecked;
    return Qt::Checked;
}
/*------------------------------------------------------------------------------*/
void Widget::iniTable()
{
    qtm = new QSqlRelationalTableModel(this,QSqlDatabase::database("MyAccessDB"));
    qtm->setEditStrategy(QSqlTableModel::OnManualSubmit);
    qtm->setTable(QString::fromLocal8Bit("BD_AU_单位字典表"));
    qtm->select();
    tview = new QTableView;
    tview->setSelectionBehavior(QAbstractItemView::SelectRows);
    tview->horizontalHeader()->setStretchLastSection(true);

    tview->setModel(qtm);

    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("ID")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("分类编号")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("序号")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("单位编号")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("上级编码")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("管辖电务段数量")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("电务职工总数")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("换算道岔组数")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("管辖里程")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("所属铁路局")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("站场数量")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("车间数量")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("生产班组数量")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("在岗人数")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("固定资产")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("类码")),true);
    tview->setColumnHidden(qtm->fieldIndex(QString::fromLocal8Bit("编码")),true);
}

void Widget::iniTree()
{
    QStringList headers;
    headers << QString::fromLocal8Bit("单位名称");


    tmodel = new QStandardItemModel();

    tmodel->setHorizontalHeaderLabels(headers);
    QString rootName;
    QString coding;

    QSqlQuery query(QSqlDatabase::database("MyAccessDB"));
    QString sql = QString::fromLocal8Bit("select 名称,编码 from BD_AU_单位字典表 where 序号=0");
    query.exec(sql);
    query.next();
    rootName = query.value(QString::fromLocal8Bit("名称")).toString();
    coding = query.value(QString::fromLocal8Bit("编码")).toString();

    QStandardItem* rootItem = new QStandardItem(QIcon(":/train.png"),rootName);

    rootItem->setEditable(false);

    tmodel->appendRow(rootItem);

    //路局
    sql = QString::fromLocal8Bit("select 名称,编码 from BD_AU_单位字典表 where 上级编码='%1' and 类码<>'W'").arg(coding);
    query.exec(sql);
    qDebug()<<query.lastError()<<"\n"<<sql;
    while(query.next())
    {
        QString ljName = query.value(QString::fromLocal8Bit("名称")).toString();
        QString bmName = query.value(QString::fromLocal8Bit("编码")).toString();

        QStandardItem * ljItem = new QStandardItem(QIcon(":/train.png"),ljName);

        ljItem->setEditable(false);
        ljItem->setTristate(true);
        rootItem->appendRow(ljItem);
        //路局终端
        QStandardItem * ljZdItem = new QStandardItem("");
        ljZdItem->setEditable(false);

        rootItem->setChild(ljItem->index().row(),1,ljZdItem);

        QSqlQuery queryDwd(QSqlDatabase::database("MyAccessDB"));
        sql = QString::fromLocal8Bit("select 名称,编码 from BD_AU_单位字典表 where 上级编码='%1' and 类码<>'W'").arg(bmName);
        queryDwd.exec(sql);
        while(queryDwd.next())
        {
            QString dwdName = queryDwd.value(QString::fromLocal8Bit("名称")).toString();
            QString dwdBmName = queryDwd.value(QString::fromLocal8Bit("编码")).toString();

            QStandardItem * dwdItem = new QStandardItem(QIcon(":/train.png"),dwdName);

            dwdItem->setEditable(false);

            //电务段终端
            QStandardItem * dwdZdItem = new QStandardItem("");
            dwdZdItem->setEditable(false);

            ljItem->appendRow(dwdItem);
            ljItem->setChild(dwdItem->index().row(),1,dwdZdItem);

        }
    }

    view = new QTreeView;
    view->setModel(tmodel);
    view->expand(tmodel->index(0,0));
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(TreeViewcontextMenu(QPoint)));
    connect(view,SIGNAL(expanded(QModelIndex)),this,SLOT(updateCheckTree(QModelIndex)));


}
//更新tree列宽
void Widget::updateTree()
{
    for (int column = 0; column < tmodel->columnCount(); ++column)
        view->resizeColumnToContents(column);
}
//更新table列宽
void Widget::updateTable()
{
    for (int column = 0; column < qtm->columnCount(); ++column)
    {
        tview->resizeColumnToContents(column);
    }
    //充满

}
//更新checkedTree
void Widget::updateCheckTree(const QModelIndex &index)
{
    for (int column = 0; column < qtm->columnCount(); ++column)
    {
        checkedTree->resizeColumnToContents(column);
    }
}

//加载样式表
void Widget::initStyle()
{
    //加载样式表
    QFile file(":/qss/psblack.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }
}

void Widget::saveTable(bool)
{
    qDebug()<<qtm->submitAll();
    qDebug()<<qtm->lastError();

    if(m_addrIndex==-1)
        return;
    TestWebService();
    queryData_WS();
}
//上传文件
QFileInfoList GetFileList(QString path)
{
    QDir dir(path);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for(int i = 0; i != folder_list.size(); i++)
    {
         QString name = folder_list.at(i).absoluteFilePath();
         QFileInfoList child_file_list = GetFileList(name);
         file_list.append(child_file_list);
    }

    return file_list;
}
void Widget::uploadingTable()
{
    //选择文件夹
    QFileDialog qd;
    QString path = qd.getExistingDirectory();
    if(path=="")
        return;

    QFileInfoList fileInfoList =GetFileList(path);
    QDir relative(path);
    for(int i =0 ;i<fileInfoList.size();i++)
    {
        QString filename = fileInfoList.at(i).filePath();
        if(list_file)
            if(list_file->contains(filename))
                continue;
        list_file->append(filename);
        //加入表中

        QString repath = relative.relativeFilePath(fileInfoList.at(i).absoluteFilePath());
        QString re_path="";
        if(repath.lastIndexOf("/")!=-1)
            re_path = repath.left(repath.lastIndexOf("/"));

        QList<QStandardItem *>items;
        QStandardItem *nameitem = new QStandardItem(fileInfoList.at(i).fileName());
        //nameitem->setTextAlignment(Qt::AlignCenter);
        QStandardItem *opitem = new QStandardItem(QString::fromLocal8Bit("更新覆盖"));
        QStandardItem *reitem = new QStandardItem(re_path);
        items<<nameitem<<opitem<<reitem;
        nameitem->setIcon(fileIcon(filename));
        utModel->appendRow(items);
    }
    updateUT();
}
void Widget::updateUT()
{
    for (int column = 0; column < utModel->columnCount(); ++column)
    {
        ut->resizeColumnToContents(column);
    }
}
QIcon Widget::fileIcon(const QString &extension) const
{
    QFileInfo info;
    info.setFile(extension);
    //获取图标、类型
    QFileIconProvider provider;
    QIcon icon = provider.icon(info);
    QString strType = provider.type(info);
    return icon;
}
//上传文件
void Widget::submitFile()
{

    //如果没有设置主服务器
    if(m_addrIndex == -1)
    {
        QMessageBox alertBox(this);
        alertBox.setText(QString::fromLocal8Bit("未设定当前服务器或服务器无法连接"));
        alertBox.exec();
        return;
    }
    UploadFile *uf;
    XmlConfig::createXml(ctmodel,utModel,SoftListCmb->currentText());
    QVector<theFile*>files;
    QStringList _filenames;

    //还要加上xml文件
    QString xmlFilePath = QDir::currentPath()+"/UpdaterList.xml";
    theFile *file = new theFile("UpdaterList.xml",xmlFilePath,-1);
    files.append(file);
    _filenames.append(QString::fromLocal8Bit("UpdaterList.xml"));

    if(utModel->rowCount() == 0)
        return;
    for (int row = 0; row < utModel->rowCount(); ++row)
    {
        QString filename = utModel->index(row,0).data().toString();
        QString softname = QString::fromLocal8Bit("铁路app");
        QString ver = "1.0.0.1";
        QString dwname = QString::fromLocal8Bit("武汉电务段");
        QString filepath = list_file->at(row);
        theFile *file = new theFile(filename,filepath,row);
        qDebug()<<filename<<"****"<<filepath<<"****";
        files.append(file);
        _filenames.append(filename);
        qDebug()<<filename;
    }

    sendToSvrMessage(_filenames);

    uf = new UploadFile(files,m_addr[m_addrIndex]);
    thread = new QThread();
    uf->moveToThread(thread);
    connect(thread,SIGNAL(started()),uf,SLOT(start()));
    //退出
    connect(uf,SIGNAL(finished()),thread,SLOT(quit()));
    connect(uf,SIGNAL(finished()),uf,SLOT(deleteLater()));
    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
    connect(uf,SIGNAL(replyFinished(int,qreal)),this,SLOT(fileUploadFinished(int,qreal)));
}
//上传完成槽
void Widget::fileUploadFinished(const int row, const qreal progress)
{
    if(row<0)
        return;
    currRow = row;
    if(progress<0)
    {
        utModel->setData(utModel->index(row,0),QVariant(QColor("#FF0000")),Qt::BackgroundColorRole);
    }
    else
    {
        UploadProc->setStartValue(UploadProc->endValue());
        UploadProc->setEndValue(progress);
    }
}
//更新进度条
void Widget::updateProcBar(QVariant value)
{
    //获取第一列的长度
    qreal col_width = ut->columnWidth(0);
    qreal progress = value.toReal();
    QLinearGradient linearGrad(QPointF(progress, progress), QPointF(col_width,col_width));
    linearGrad.setColorAt(0, QColor("#00CD00"));
    linearGrad.setColorAt(1, Qt::black);
    QBrush brush(linearGrad);
    utModel->setData(utModel->index(currRow,0),QVariant(brush),Qt::BackgroundColorRole);
    if(value.toInt()==100)
    {
        UploadProc->setStartValue(0);
        UploadProc->setEndValue(0);
    }
}

//右键菜单
void Widget::TreeViewcontextMenu(const QPoint &pos)
{
    selectedIndex = view->indexAt(pos);
    m_treeMenu->exec(QCursor::pos());
}
//设置当前服务器
void Widget::setServer()
{
    if(!selectedIndex.isValid())
        return;
    tmodel->setData(selectedIndex,QVariant(QIcon(":/checkedTrain.png")),Qt::DecorationRole);
    if(currentIndex.isValid())
        tmodel->setData(currentIndex,QVariant(QIcon(":/train.png")),Qt::DecorationRole);
    currentIndex = selectedIndex;

    QStandardItem * root = ctmodel->item(0,0);
    for(int row = 0 ; row < root->rowCount();row++)
    {
        if(root->child(row,0)->data(Qt::DisplayRole).toString() == currentIndex.data().toString())
        {
            setCanEdit();
            setUnEdit(root->child(row,0));
            break;
        }
        for(int _row = 0 ; _row <root->child(row,0)->rowCount() ; _row++)
        {
            if(root->child(row,0)->child(_row,0)->data(Qt::DisplayRole).toString() == currentIndex.data().toString())
            {
                setCanEdit();
                setUnEdit(root->child(row,0)->child(_row,0));
                break;
            }
        }
    }
    //算出当前的ip地址
    QString p_sql = QString::fromLocal8Bit("select * from BD_AU_单位字典表 where 名称='%1'").arg(currentIndex.data().toString());
    QSqlQuery p_query(QSqlDatabase::database("MyAccessDB"));
    p_query.exec(p_sql);
    qDebug()<<p_sql<<""<<p_query.size();
    p_query.next();
    m_addrIndex = -1;
    if(!m_addr.isEmpty())
    {
        m_addr.clear();
    }
    m_addr = p_query.value(QString::fromLocal8Bit("IP")).toString().split(",");
    //查看ip是否在线
    for(int i = 0 ; i<m_addr.count() ; i++)
    {
        QProcess *cmd = new QProcess;

        QString strArg = "ping "+m_addr[i]+" -n 1 -w "+ QString::number(TIME_OUT);
        cmd->start(strArg);
        cmd->waitForReadyRead();
        cmd->waitForFinished();
        QString retStr = QString::fromLocal8Bit(cmd->readAll());
        qDebug()<<retStr;
        if(retStr.indexOf("TTL") != -1)
        {
            m_addrIndex = i;
            break;
        }
        retStr.clear();
    }
    //m_addrIndex = 0;
    //更新上传情况查询表
    updateInfoInquire();
}
void Widget::setUnEdit(QStandardItem * fwq)
{
    if(nullptr == fwq)
        return;
    //兄弟节点全un
    QStandardItem *parent = fwq->parent();
    if(nullptr != parent)
    {
        parent->setCheckState(Qt::Unchecked);
        parent->setEnabled(false);
        setUnEdit(parent);
        //
        QStandardItem * parent2 = ctmodel->item(parent->row(),1);
        parent2->setCheckState(Qt::Unchecked);
        parent2->setEnabled(false);
        setUnEdit(parent2);
        for(int i = 0;i<parent->rowCount();i++)
        {
            QStandardItem * siblingfwq = parent->child(i,0);
            if(siblingfwq!=fwq)
            {
                siblingfwq->setCheckState(Qt::Unchecked);
                siblingfwq->setEnabled(false);
                setChildUnEdit(siblingfwq);

                QStandardItem * siblingfwq2 = parent->child(i,1);
                siblingfwq2->setCheckState(Qt::Unchecked);
                siblingfwq2->setEnabled(false);
                setChildUnEdit(siblingfwq2);
            }
            //兄弟节点下面的都不能选
        }
    }
}
void Widget::setCanEdit()
{
    QStandardItem * root = ctmodel->item(0,0);
    root->setCheckState(Qt::Unchecked);
    root->setEnabled(true);
    QStandardItem * root1 = ctmodel->item(0,1);
    root1->setCheckState(Qt::Unchecked);
    root1->setEnabled(true);
    //直接在创建他的时候看

    return;
    for(int row1 = 0 ;row1 <root->rowCount();row1++)
    {
        QStandardItem *item1 = root->child(row1,0);
        qDebug()<<item1->data(Qt::DisplayRole).toString();
        item1->setCheckState(Qt::Unchecked);
        item1->setEnabled(true);
        //
//        QStandardItem *item11 = root->child(row1,1);
//        item11->setCheckState(Qt::Unchecked);
//        item11->setEnabled(true);
//        qDebug()<<item11->rowCount();

        for(int row2 = 0 ; row2 <item1->rowCount();row2++)
        {
            QStandardItem *item2 = item1->child(row2,0);
            item2->setCheckState(Qt::Unchecked);
            item2->setEnabled(true);
            //
//            QStandardItem *item22 = item11->child(row2,0);
//            item22->setCheckState(Qt::Unchecked);
//            item22->setEnabled(true);
        }
    }
}
void Widget::setChildUnEdit(QStandardItem *child)
{
    if(nullptr == child)
        return;
    for(int i = 0 ; i < child->rowCount() ; i++)
    {
        QStandardItem *item = child->child(i,0);
        item->setCheckState(Qt::Unchecked);
        item->setEnabled(false);
        setChildUnEdit(item);
    }
}
void Widget::sendToSvrMessage(QStringList &_filenames)
{
    //socket
    qDebug()<<m_addr[m_addrIndex];
    SendMesgToSevr * s = new SendMesgToSevr(m_addr[m_addrIndex]);
    s->fileNames = _filenames;
    qDebug()<< _filenames;
    connect(s,SIGNAL(finished()),s,SLOT(deleteLater()));
    connect(s,SIGNAL(erroMesg(int)),this,SLOT(getServMesg(int)));
    s->start();
}

//接收到服务器消息
void Widget::getServMesg(int res)
{
    if(res == 0)
    {
            //isp
            ISpVoice *pSpVoice;
            ::CoInitialize(NULL);
            CoCreateInstance(CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,IID_ISpVoice,
                             (void**)&pSpVoice);
            pSpVoice->SetVolume(80);
            bool ok = FALSE;
            QString f;
//            f = QInputDialog::getText(
//                                this,
//                                tr( "Application name" ),
//                                tr( "Please enter your name" ),
//                                QLineEdit::Normal, QString::null, &ok,Qt::Dialog  );

            f = QString::fromLocal8Bit("成功收到消息");
            //pSpVoice->Speak(LPCWSTR(f.unicode()),SPF_DEFAULT,NULL);
            qDebug()<<f;
            m_GetMesg = true;
    }
    else
    {
        m_GetMesg = false;
        QMessageBox msgBox(this);
        msgBox.setText(QString::fromLocal8Bit("无法连接到服务器"));
        msgBox.exec();
    }
    if(m_GetMesg == true)
        thread->start();
}
void Widget::Highlights()
{

}
bool Widget::TestWebService()
{
    if(m_addrIndex < 0)
    {
        return false;
    }
    soap wb_soap;
    soap_init(&wb_soap);
    QString str_endpoint = "http://"+m_addr[m_addrIndex]+":190/updataService.asmx";
    char endpoint[1024];
    strcpy(endpoint,qPrintable(str_endpoint));
    wb_soap.send_timeout = 10;
    wb_soap.recv_timeout = 10;

#define uSec *-1
#define mSec *-1000
    wb_soap.accept_timeout = 10 uSec;
    wb_soap.send_timeout = 20 mSec;
    wb_soap.recv_timeout = 20 mSec;

    //soap_call_dataStream(&add_soap,server,"",name,data,returnFlag)
    _ns1__HelloWorld helloworld;
    helloworld.helloworld = "89757";

    _ns1__HelloWorldResponse helloworldResponse;

    soap_call___ns1__HelloWorld(
                &wb_soap,
                endpoint,
                NULL,
                &helloworld,
                helloworldResponse
                );

    if(wb_soap.error)
    {
        soap_end(&wb_soap);
        return false;
    }
    qDebug() << helloworldResponse.HelloWorldResult;

    soap_end(&wb_soap);
    return true;
}
//其实应该要放到线程中去，不然如果连不上的话回卡死
QString Widget::queryData_WS()
{
    //连不上服务器
    if(m_addrIndex<0)
    {
        XmlConfig::clear();
        upInfoTreeMd->clear();
        return "";
    }
    //最后显示的时候要以一个树形结构显示出来
    QString sql = QString::fromLocal8Bit("select 单位名称,编码,上级编码,软件名称,S_UDTIME,更新成功,服务器名称,IP地址 from AU_UPLOADRECORD");

    char*  ch;
    QByteArray ba = sql.toUtf8();
    ch=ba.data();


    soap wb_soap;
    soap_init(&wb_soap);
    soap_set_mode(&wb_soap,SOAP_C_UTFSTRING);
    XmlConfig xc;
    QString str_endpoint = "http://"+m_addr[m_addrIndex]+":190/updataService.asmx";
    char endpoint[1024];
    strcpy(endpoint,qPrintable(str_endpoint));
    //soap_serve(&wb_soap);

    //soap_call_dataStream(&add_soap,server,"",name,data,returnFlag)
    _ns1__QueryData qd;
    qd.sql = ch;
    qDebug()<<qd.sql;

    _ns1__QueryDataResponse qdResponse;

    soap_call___ns1__QueryData(
                &wb_soap,
                endpoint,
                NULL,
                &qd,
                qdResponse
                );

    if(wb_soap.error)
    {
        soap_end(&wb_soap);
        return false;
    }
    QString ret = QString::fromUtf8(qdResponse.QueryDataResult);
    soap_end(&wb_soap);
    xc.getUpdateInfo(ret,upInfoTreeMd);
    return true;
}
//处理xml，用信号槽，等xmlconfig处理完了以后通知主线程的槽，完成view的刷新
void Widget::updateInfoInquire()
{
    if(TestWebService() == false)
        return;
    queryData_WS();
}

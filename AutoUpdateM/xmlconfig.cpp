#include "xmlconfig.h"
QHash<QString,treeNode> XmlConfig::treeNodes;
QMultiMap<QString,treeNode> XmlConfig::tableNodes;
XmlConfig::XmlConfig()
{

}

void XmlConfig::createXml(QStandardItemModel *treemodel, QStandardItemModel *tablemodel, QString softname)
{
    QFile file("UpdaterList.xml");
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QDomDocument doc;
    QDomProcessingInstruction instruction;

    instruction = doc.createProcessingInstruction("xml",
                                                  "version=\"1.0\" encoding=\"GB2312\"");
    doc.appendChild(instruction);

    QDomElement root = doc.createElement("AutoUpdater");

    doc.appendChild(root);


    //SoftName
    QDomElement SoftName = doc.createElement("SoftName");
    root.appendChild(SoftName);
    QDomText softText = doc.createTextNode(softname);
    SoftName.appendChild(softText);


    //Description
    QDomElement Description = doc.createElement("Desciption");
    root.appendChild(Description);
    QDomText text = doc.createTextNode("des");
    Description.appendChild(text);

    //Aims
    QDomElement Aims = doc.createElement("Aims");
    root.appendChild(Aims);
    //第一层
    for(int row = 0; row<treemodel->rowCount();row++)
    {
        //名称
        QString dwname = treemodel->index(row,0).data(Qt::DisplayRole).toString();
        bool server_checked = treemodel->index(row,0).data(Qt::CheckStateRole).toBool();
        bool client_checked = treemodel->index(row,1).data(Qt::CheckStateRole).toBool();
        QDomElement t_node = doc.createElement("FWQ");
        t_node.setAttribute("server",server_checked);
        t_node.setAttribute("client",client_checked);
        t_node.setAttribute("DwName",dwname);
        Aims.appendChild(t_node);
        //第二层
        QModelIndex index = treemodel->index(row,0).child(0,0);
        int sec_row = 0;
        while(index.isValid())
        {
            QString sec_dwname = index.data(Qt::DisplayRole).toString();
            bool server_checked = index.data(Qt::CheckStateRole).toBool();
            bool client_checked = treemodel->index(row,1).child(sec_row,1).data(Qt::CheckStateRole).toBool();

            QDomElement sec_node = doc.createElement("FWQ");
            sec_node.setAttribute("server",server_checked);
            sec_node.setAttribute("client",client_checked);
            sec_node.setAttribute("DwName",sec_dwname);
            t_node.appendChild(sec_node);


            //第三层
            QModelIndex trd_index = index.child(0,0);
            int trd_row = 0;
            while(trd_index.isValid())
            {
                QString trd_dwname = trd_index.data(Qt::DisplayRole).toString();
                bool server_checked = trd_index.data(Qt::CheckStateRole).toBool();
                bool client_checked = treemodel->index(row,1).child(sec_row,1).child(trd_row,1).data(Qt::CheckStateRole).toBool();

                QDomElement trd_node = doc.createElement("FWQ");
                trd_node.setAttribute("server",server_checked);
                trd_node.setAttribute("client",client_checked);
                trd_node.setAttribute("DwName",trd_dwname);
                sec_node.appendChild(trd_node);

                trd_index = trd_index.sibling(++trd_row,0);
            }

            index = index.sibling(++sec_row,0);
        }
    }

    //Updater
    QDomElement updater = doc.createElement("Updater");
        //url
    QDomElement url = doc.createElement("Url");
        //lastUpdateTime
    QDomElement lastUpdateTime = doc.createElement("LastUpdateTime");
    QDomText urlText = doc.createTextNode("url");
    QDomText lastUpdateTimeText = doc.createTextNode("lastupdatetime");





    url.appendChild(urlText);
    lastUpdateTime.appendChild(lastUpdateTimeText);
    updater.appendChild(url);
    updater.appendChild(lastUpdateTime);
    root.appendChild(updater);

    //Application
    QString verStr = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    //
    QDomElement application = doc.createElement("Application");
    QDomElement run = doc.createElement("Run");
    QDomElement ver = doc.createElement("Ver");
    QDomElement prover = doc.createElement("ProVer");
    QDomText runText = doc.createTextNode("runtext");
    QDomText verText = doc.createTextNode(verStr);
    QDomText proverText = doc.createTextNode("proverText");

    application.appendChild(run);
    application.appendChild(ver);
    application.appendChild(prover);
    run.appendChild(runText);
    ver.appendChild(verText);
    prover.appendChild(proverText);
    root.appendChild(application);

    //Files
    QDomElement files = doc.createElement("Files");
    root.appendChild(files);
    for(int row = 0;row<tablemodel->rowCount();row++)
    {
        QDomElement fileItem = doc.createElement("File");
        fileItem.setAttribute("dir",tablemodel->index(row,2).data(Qt::DisplayRole).toString());
        fileItem.setAttribute("options",tablemodel->index(row,1).data(Qt::DisplayRole).toString());
        fileItem.setAttribute("name",tablemodel->index(row,0).data(Qt::DisplayRole).toString());
        files.appendChild(fileItem);
    }

    //保存文件

    QTextStream out(&file);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    out.setCodec(codec);
    doc.save(out,4);

    file.close();
}
void XmlConfig::getInfo(QString _xmlStr, QStandardItemModel *_tableModel)
{
    /*
     * 由于建立的是一个树形结构
     * 分类：
     * 1.根节点，但是并不确定谁是根节点
     * 2.item都先不放到tree里面，等遍历完了所有的记录再放进去
     */
    QDomDocument doc;   //新建QDomDocument代表一个xml文档
    doc.setContent(_xmlStr);

    //解析各个节点
    QDomElement docElem = doc.documentElement();//返回根元素
    for(int index = 0 ; index < docElem.childNodes().count(); index++)
    {
        QDomNode n = docElem.childNodes().at(index);
        while(!n.isNull())
        {
            QDomElement e = n.toElement();//将其转换为元素
            qDebug() << e.tagName();        //Table
            QList<QStandardItem*>items;
            QDomNodeList list = e.childNodes();//获得元素e的所有子节点的列表
            for(int i = 0;i<list.count();i++)//遍历该列表
            {
                QDomNode node = list.at(i);
                if(node.isElement())
                {
                    qDebug()<<node.toElement().tagName()
                           <<node.toElement().text();
                    QStandardItem * node_item = new QStandardItem(node.toElement().text());
                    items.append(node_item);
                }
            }
            n = n.nextSibling();//下一个兄弟节点
            qDebug()<<"*********---------------***********";
            if(!items.isEmpty()&&_tableModel!=NULL)
                _tableModel->appendRow(items);
        }
    }
}
//Master改不了？
void XmlConfig::getUpdateInfo(QString _xmlStr, QStandardItemModel *_tableModel)
{
    /*
     * 由于建立的是一个树形结构
     * 分类：
     * 1.根节点，但是并不确定谁是根节点
     * 2.item都先不放到tree里面，等遍历完了所有的记录再放进去
     */
    QDomDocument doc;   //新建QDomDocument代表一个xml文档
    doc.setContent(_xmlStr);
    qDebug()<<_xmlStr;
    //初始化
    treeNodes.clear();
    tableNodes.clear();

    //解析各个节点
    QDomElement docElem = doc.documentElement();//返回根元素
    for(int index = 0 ; index < docElem.childNodes().count(); index++)  //实际只有一个就是NewDataSet
    {
        if(index == 1)
            break;
        QDomNode n = docElem.childNodes().at(index);
        qDebug()<<"n.nodeName is :"<<n.nodeName();
        while(1)
        {
            if(n.nextSibling().isNull())
                break;
            QDomElement e = n.toElement();//将其转换为元素
            QDomNodeList list = e.childNodes();//获得元素e的所有子节点的列表
            /*
             * 0    单位名称
             * 1    编码
             * 2    上级编码
             * 3    软件名称
             * 4    更新时间
             * 5    更新成功
             * 6    服务器名称
             * 7    服务器ip
            */
            QString dwname = list.at(0).toElement().text();
            QString code = list.at(1).toElement().text();
            QString parent_code = list.at(2).toElement().text();
            QString softname = list.at(3).toElement().text();
            QString uptime = list.at(4).toElement().text();
            QString upsuc = list.at(5).toElement().text();
            QString fwqname = list.at(6).toElement().text();
            QString fwqip = list.at(7).toElement().text();

            QStandardItem *temp_item = new QStandardItem(dwname);
            temp_item->setEditable(false);

            treeNode temp_node(temp_item,code,parent_code);
            treeNode temp_node_t(temp_item,code,parent_code,softname,uptime,upsuc,fwqname,fwqip);

            treeNodes[code]=temp_node;
            tableNodes.insert(dwname,temp_node_t);

            n = n.nextSibling();//下一个兄弟节点
        }
    }
    addTreeViewItem(_tableModel);
}
QString XmlConfig::getOneQueryData(QString _xmlStr)
{
    //同样的套路，都是解析xml文件，这个只是条件返回
    QDomDocument doc;   //新建QDomDocument代表一个xml文档
    doc.setContent(_xmlStr);

    //解析各个节点
    QDomElement docElem = doc.documentElement();//返回根元素
    for(int index = 0 ; index < docElem.childNodes().count(); index++)
    {
        QDomNode n = docElem.childNodes().at(index);
        while(!n.isNull())
        {
            QDomElement e = n.toElement();//将其转换为元素
            QDomNodeList list = e.childNodes();//获得元素e的所有子节点的列表
            for(int i = 0;i<list.count();i++)//遍历该列表
            {
                QDomNode node = list.at(i);
                if(node.isElement())
                {
                    return node.toElement().text();
                }
            }
            n = n.nextSibling();//下一个兄弟节点
        }
    }
}
void XmlConfig::addTreeViewItem(QStandardItemModel *_tableModel)
{
    _tableModel->clear();
    _tableModel->setHorizontalHeaderLabels(QStringList()<<QString::fromLocal8Bit("单位名称")<<QString::fromLocal8Bit("最新上传时间"));
    QList<QStandardItem*>topTreeNodeList;
    //遍历treeNodes
    QHashIterator<QString,treeNode> i(treeNodes);
    while(i.hasNext())
    {
        i.next();
        QString theParentCode = i.value().parent_code;
        qDebug()<<"parentCode is :"<<i.value().parent_code<<"  Code is"<<i.value().code;
        if(treeNodes.contains(theParentCode))
        {
            QHash<QString,treeNode>::const_iterator it = treeNodes.find(theParentCode);
            it.value().node_item->appendRow(i.value().node_item);
        }
        else
        {
            topTreeNodeList.append(i.value().node_item);
        }
    }

    _tableModel->appendRow(topTreeNodeList);
}
void XmlConfig::refreshUpRecordTable(QStandardItemModel *model,QString dwname)
{
    model->clear();
    model->setHorizontalHeaderLabels(QStringList()<<QString::fromLocal8Bit("软件名称")<<QString::fromLocal8Bit("更新时间")
                                             <<QString::fromLocal8Bit("更新结果")
                                             <<QString::fromLocal8Bit("更新服务器")<<QString::fromLocal8Bit("更新服务器ip"));

    if(treeNodes.isEmpty())
        return;
    QList<treeNode>values = tableNodes.values(dwname);
    for(int i = 0 ; i < values.size() ; i++)
    {
        QList<QStandardItem*>items;
        QStandardItem *softname = new QStandardItem(values.at(i).softname);
        softname->setTextAlignment(Qt::AlignHCenter);
        QStandardItem *uptime = new QStandardItem(values.at(i).uptime);
        uptime->setTextAlignment(Qt::AlignHCenter);
        QStandardItem *upsuc = new QStandardItem(values.at(i).upsuc);
        upsuc->setTextAlignment(Qt::AlignHCenter);
        QStandardItem *fwqname = new QStandardItem(values.at(i).fwqname);
        fwqname->setTextAlignment(Qt::AlignHCenter);
        QStandardItem *fwqip = new QStandardItem(values.at(i).fwqip);
        fwqip->setTextAlignment(Qt::AlignHCenter);
        items<<softname<<uptime<<upsuc<<fwqname<<fwqip;
        model->appendRow(items);
    }
}
void XmlConfig::clear()
{
    treeNodes.clear();
    tableNodes.clear();
}
void XmlConfig::parseDownloadXML(QString xmlStr,QSqlQuery sq)
{
    QDomDocument doc;   //新建QDomDocument代表一个xml文档
    doc.setContent(xmlStr);
    qDebug()<<xmlStr;
    //初始化
    treeNodes.clear();
    tableNodes.clear();

    //解析各个节点
    QDomElement docElem = doc.documentElement();//返回根元素
    for(int index = 0 ; index < docElem.childNodes().count(); index++)  //实际只有一个就是NewDataSet
    {
        if(index == 1)
            break;
        QDomNode n = docElem.childNodes().at(index);
        qDebug()<<"n.nodeName is :"<<n.nodeName();
        while(1)
        {
            if(n.nextSibling().isNull())
                break;
            QDomElement e = n.toElement();//将其转换为元素
            QDomNodeList list = e.childNodes();//获得元素e的所有子节点的列表
            /*
             * 0    ID
             * 1    分类编码
             * 2    序号
             * 3    单位编号
             * 4    类码
             * 5    名称
             * 6    编码
             * 7    上级编码
             * 8    管辖电务段数量
             * 9    电务职工总数
             * 10   换算道岔组数
             * 11   管辖里程
             * 12   所属铁路局
             * 13   站场数量
             * 14   车间数量
             * 15   生产班组数量
             * 16   在岗人数
             * 17   固定资产
             * 18   IP
            */
            QString id = list.at(0).toElement().text();
            QString code = list.at(1).toElement().text();
            QString number = list.at(2).toElement().text();
            QString dwCode = list.at(3).toElement().text();
            QString classCode = list.at(4).toElement().text();
            QString name = list.at(5).toElement().text();
            QString bCode = list.at(6).toElement().text();
            QString upbCode = list.at(7).toElement().text();
            QString eight = list.at(8).toElement().text();
            QString nine = list.at(9).toElement().text();
            QString ten = list.at(10).toElement().text();
            QString eleven = list.at(11).toElement().text();
            QString twelve = list.at(12).toElement().text();
            QString thirteen = list.at(13).toElement().text();
            QString fourteen = list.at(14).toElement().text();
            QString fifteen = list.at(15).toElement().text();
            QString sixteen = list.at(16).toElement().text();
            QString seventeen = list.at(17).toElement().text();
            QString ip = list.at(18).toElement().text();

            QString sql = QString::fromLocal8Bit("insert into BD_AU_单位字典表 values("
                                                 "%1,%2,%3,%4,'%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19')"
                                                 "").arg(id).arg(code).arg(number).arg(dwCode).arg(classCode).arg(name).arg(bCode).arg(upbCode)
                    .arg(eight).arg(nine).arg(ten).arg(eleven).arg(twelve).arg(thirteen).arg(fourteen).arg(fifteen).arg(sixteen).arg(seventeen).arg(ip);
            qDebug()<<sql;
            n = n.nextSibling();
        }
    }
}

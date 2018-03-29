#ifndef XMLCONFIG_H
#define XMLCONFIG_H

#include <QtXml>
#include <QStandardItemModel>
#include <QSqlQuery>
//自定义树节点
class treeNode
{
public:
    treeNode(QStandardItem *_node_item,QString _code,
             QString _parent_code,QString _softname="",QString _uptime="",QString _upsuc="",QString _fwqname="",
             QString _fwqip=""):node_item(_node_item),code(_code),parent_code(_parent_code),
    softname(_softname),uptime(_uptime),upsuc(_upsuc),fwqname(_fwqname),fwqip(_fwqip){}
    treeNode(){}
    //因为要放到QHash里面需要重载==
    bool operator == (const treeNode &t)const{
        return (code==t.code);
    }
public:
    QStandardItem *node_item;
    QString code;
    QString parent_code;

    /*
     * 软件名称
     * 更新时间
     * 更新成功
     * 服务器名称
     * 服务器ip
     */
    QString softname,uptime,upsuc,fwqname,fwqip;
};


class XmlConfig
{
public:
    XmlConfig();

    static void createXml(QStandardItemModel *treemodel, QStandardItemModel *tablemodel,QString softname);
    //更新upRecordTable
    static void refreshUpRecordTable(QStandardItemModel *model, QString dwname);

    //解析下载数据
    static void parseDownloadXML(QString xmlStr, QSqlQuery sq);

    //处理WebServices返回的xml数据
    void getUpdateInfo(QString _xmlStr, QStandardItemModel *_tableModel);
    QString getOneQueryData(QString _xmlStr);
signals:
    void parseComplete();
private:
    static XmlConfig *m_pXmlConfig;
    void addTreeViewItem(QStandardItemModel *_tableModel);

    void getInfo(QString _xmlStr, QStandardItemModel *_tableModel);

    //存放node节点(code,..)
    static QHash<QString,treeNode>treeNodes;
    //存放所有数据(dwname,..)
    static QMultiMap<QString,treeNode>tableNodes;

public:
    //清空容器并刷新
    void static clear();

};

#endif // XMLCONFIG_H

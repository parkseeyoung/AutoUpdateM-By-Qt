#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <QMutex>   //线程保护序列化
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class MyDataBase
{
public:
    MyDataBase();
    ~MyDataBase();
private:
    MyDataBase(const MyDataBase &);
    MyDataBase operator =(const MyDataBase &);
public:
    static MyDataBase * GetInstance();
    //弄清楚每个const的作用！
    bool ConnectAccessDB(const QString &strDBName,
                         const QString &strUser,
                         const QString &strPwd) const;
    QSqlDatabase GetDatabase() const
    {
        return QSqlDatabase::database("MyAccessDB");
    }

    QSqlQuery GetSqlQuery() const
    {
        static QSqlQuery query(m_pInstance->GetDatabase());
        return query;
    }

    bool IsValid() const
    {
        return this->GetDatabase().isValid();
    }
    bool IsConnected() const
    {
        return this->GetDatabase().isOpen();
    }

private:
    static MyDataBase *m_pInstance;

};

#endif // MYDATABASE_H

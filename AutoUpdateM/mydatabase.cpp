#include "mydatabase.h"
#include <QDebug>
MyDataBase * MyDataBase::m_pInstance(NULL);

MyDataBase::MyDataBase()
{

}
MyDataBase::~MyDataBase()
{

}

MyDataBase * MyDataBase::GetInstance()
{
    if(NULL == m_pInstance)
    {
        static QMutex mutex;
        mutex.lock();
        if( NULL == m_pInstance)
        {
            static MyDataBase myDB;
            m_pInstance = &myDB;
        }
        mutex.unlock();
    }

    return m_pInstance;
}

bool MyDataBase::ConnectAccessDB(const QString &strDBName, const QString &strUser, const QString &strPwd) const
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "MyAccessDB");
    const QString strName(QString("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=%1;Uid=%2;Pwd=%3")
                          .arg(strDBName)
                          .arg(strUser)
                          .arg(strPwd));

    db.setDatabaseName(strName);
    qDebug()<<strName;
    if (!db.isValid())
    {
        return false;
    }

    if (db.isOpen())
    {
        return true;
    }

    if (db.open())
    {
        return true;
    }
    else
    {
        qDebug() << db.lastError().text();
        return false;
    }
}

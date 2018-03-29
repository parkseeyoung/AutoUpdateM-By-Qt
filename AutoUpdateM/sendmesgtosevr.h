#ifndef SENDMESGTOSEVR_H
#define SENDMESGTOSEVR_H

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <QDebug>
#include <QThread>

class SendMesgToSevr : public QThread
{
    Q_OBJECT
public:
    SendMesgToSevr(QString _addr);
    ~SendMesgToSevr();
    void run () ;
    static int IniSocket();

    //文件列表，传的字符串以问号隔开
    QStringList fileNames;

signals:
    void erroMesg(int erroType);
private:
    SOCKET sHost;
    SOCKADDR_IN servAddr; //服务器地址
    int retVal;
    //当前服务器
    QString addr;

private:
    QString createMesg();
};

#endif // SENDMESGTOSEVR_H

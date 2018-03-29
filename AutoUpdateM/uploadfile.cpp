#include "uploadfile.h"
#include <QUrlQuery>
#include <QFile>
#include <QDebug>
#include <QEventLoop>
#include <mutex>
#include <sys/types.h>
#include <iostream>
#include <fstream>
using namespace std;
const int BUFSIZE = 1024;

UploadFile::UploadFile(QVector<theFile *> &_files, QString _addr)
    :files(_files),addr(_addr)
{

}
//析构，关闭套接字，防止异常关闭
UploadFile::~UploadFile()
{
    if(sock_clt)
        closesocket(sock_clt);
}
//伪非阻塞
void UploadFile::start()
{
    //开始上传
    for(int i = 0 ; i<files.length() ; i++)
    {
        m_strFilePath = files.at(i)->strFilePath;

        if(m_strFilePath.isEmpty() ||files.at(i)->strFileName.isEmpty())
            continue;
        row = files.at(i)->row;

        int ret = sendFile(m_strFilePath,files.at(i)->strFileName);
        if(ret==0)
            emit replyFinished(row,100);
        else
        {
            emit replyFinished(row,-1);
            //直接全部算失败
            return;
        }
    }
    emit finished();
    //全部上传成功
}
QString UploadFile::get_file_contents(const string filename)
{
    ifstream in;
    in.open(filename,ios::in);
    QString str;
    return str;
}
//还是用我们的socket发送文件算咯
int UploadFile::sendFile(QString filePath,QString fileName)
{
    int nRes;
    IniClientSocket();

    while (true)
    {
        nRes = ::connect(sock_clt, (sockaddr*)&servAddr, sizeof(sockaddr));
        if (SOCKET_ERROR == nRes)
        {
            nRes = WSAGetLastError();
            if (WSAEWOULDBLOCK == nRes || WSAEINVAL == nRes)
            {
                Sleep(5);
                continue;
            }
            else if (WSAEISCONN == nRes)
            {
                break;
            }
            else
            {
                closesocket(sock_clt);
                WSACleanup();
                return 0;
            }
        }
    }
    SYSTEMTIME systime;
    GetLocalTime(&systime);

    char chName[24];

    sprintf(chName,"&d-%d-%d.exe",systime.wHour,systime.wMinute,systime.wSecond);

    //先发送名字

    //开始上传
    unsigned int nRecvByte = 0;
    char chBuf[BUFSIZE];
    FILE * f = fopen(qPrintable(filePath),"rb");

    UINT64 nFileLen = 0 ;
    if(f)
    {
        fseek(f,0L,SEEK_END);
        nFileLen = ftell(f);
        fseek(f,0,SEEK_SET);
    }
    else
    {
        qDebug()<<"the file was not opened";
        return 1;
    }

    UINT64 nRead = 0;
    UINT64 nSendLen = 0;

    clock_t start, finish;
    double dDuration = 0.0;
    start = clock();

    int i = 0;

    while(true)
    {
        ZeroMemory(chBuf,BUFSIZE);

        if((nFileLen - nSendLen) >= BUFSIZE)
        {
            nRead = fread(chBuf , sizeof(char),BUFSIZE,f);
        }
        else    // 最后剩下的一点
        {
            if(nFileLen <= nSendLen)
            {
                break;
            }
            nRead = fread(chBuf,sizeof(char),nFileLen - nSendLen , f);
        }

        for(i = 0 ; i < nRead ;)
        {
            nRes = send(sock_clt, chBuf + i , nRead-i , 0);
            if( 0 < nRes)
            {
                nSendLen += nRes;
                i += nRes;
            }
            else
            {
                i = 0;
            }
        }

        if( 0 == (nSendLen % 102400))
        {
            int id = GetCurrentThreadId();
            qDebug()<<id<<"has recived:"<<nSendLen << "/"<<nFileLen;
            qDebug()<<nSendLen*100/nFileLen;
            emit replyFinished(row,nSendLen*100/nFileLen);
        }

        if(SOCKET_ERROR == nRes)
        {
            nRes = GetLastError();
            if(WSAEWOULDBLOCK == nRes)
            {
                Sleep(1);
                continue;
            }
        }
    }

    finish = clock();
    dDuration = (double)(finish - start) / CLOCKS_PER_SEC;

    qDebug()<<QString("---trans finish-%1 seconds\t").arg(dDuration);
    emit replyFinished(row,100);
    Sleep(1000);

    shutdown(sock_clt,SD_SEND);
    closesocket(sock_clt);

    if(f)
    {
        fclose(f);
    }
    qDebug()<<fileName+QString::fromLocal8Bit(" 发送完成(")<<nRecvByte<<")";
    Sleep(2000);
    return 0;
}
int UploadFile::IniClientSocket()
{
    WSADATA wsd; //WSADATA变量
    //初始化套结字动态库
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        qDebug() << "WSAStartup failed!" << endl;
        return -1;
    }
    sock_clt = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(INVALID_SOCKET == sock_clt)
        return -1;
    //设置服务器地址
    servAddr.sin_family =AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(qPrintable(addr));
    servAddr.sin_port = htons((short)7802);
    int nServAddlen  = sizeof(servAddr);
    return 0;
}

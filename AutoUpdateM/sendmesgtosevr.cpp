#include "sendmesgtosevr.h"
const int BUF_SIZE = 64;


SendMesgToSevr::SendMesgToSevr(QString _addr)
    :addr(_addr)
{

    sHost; //服务器套接字
    if(SendMesgToSevr::IniSocket()==-1)
    {
        emit erroMesg(-1);
        terminate();
    }
    //创建套接字
    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
    {
        qDebug() << "socket failed!" << endl;
        WSACleanup();//释放套接字资源
        terminate();
        emit erroMesg(-1);
    }
    //设置服务器地址
    servAddr.sin_family =AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(qPrintable(addr));
    servAddr.sin_port = htons((short)7801);
    int nServAddlen  = sizeof(servAddr);
}
int SendMesgToSevr::IniSocket()
{
    WSADATA wsd; //WSADATA变量
    //初始化套结字动态库
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        qDebug() << "WSAStartup failed!" << endl;
        return -1;
    }
}

void SendMesgToSevr::run()
{
    QString files = createMesg();
    //连接服务器
    qDebug()<<addr;
    retVal=::connect(sHost,(LPSOCKADDR)&servAddr, sizeof(servAddr));
    if(SOCKET_ERROR == retVal)
    {
        qDebug()<<QString::fromLocal8Bit("连接失败");
        closesocket(sHost); //关闭套接字
        WSACleanup(); //释放套接字资源

        emit erroMesg(-1);
        terminate();
    }

    //send
    qDebug() << QString::fromLocal8Bit(" 向服务器发送数据:  ")<<files;
    const int filesLen = files.count();
    retVal = send(sHost,qPrintable(files), filesLen, 0);
    if (SOCKET_ERROR == retVal)
    {
        qDebug() << "send failed!" ;
        closesocket(sHost); //关闭套接字
        WSACleanup(); //释放套接字资源
        emit erroMesg(-1);
        terminate();
    }

    char bufRecv[BUF_SIZE];//接收数据缓冲区
    //recv
    ZeroMemory(bufRecv, BUF_SIZE);
    recv(sHost, bufRecv,BUF_SIZE , 0); // 接收服务器端的数据， 只接收5个字符
    qDebug() <<QString::fromLocal8Bit("从服务器接收数据：")<< QString::fromLocal8Bit(bufRecv);
    if(strcmp(bufRecv,"ok")==0)
        emit erroMesg(0);
    else {
        emit erroMesg(-1);
    }
}
SendMesgToSevr::~SendMesgToSevr()
{
    //退出
    closesocket(sHost); //关闭套接字
    WSACleanup(); //释放套接字资源
}
QString SendMesgToSevr::createMesg()
{
    QString temp;
    temp = "FileNameList:";
    for(int i =0 ; i < fileNames.length() ; i++)
    {
        temp+=fileNames.at(i);
        if(i!=fileNames.length()-1)
        {
            temp+="?";
        }
    }
    return temp;
}

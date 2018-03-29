#ifndef UPLOADFILE_H
#define UPLOADFILE_H

#include <QObject>
#include <sendmesgtosevr.h>
enum MesgType{UploadSoft};

class theFile
{
public:
    theFile(QString _strFileName,QString _strFilePath, int _row)
        :strFileName(_strFileName),strFilePath(_strFilePath),row(_row){}
    QString strFileName;
    QString strFilePath;
    int row;
};

class UploadFile : public QObject
{
    Q_OBJECT
public:
    UploadFile(QVector<theFile*>&_files,QString _addr);
    ~UploadFile();
    Q_INVOKABLE void setServerAddr(QString addr){m_strServerAddr = addr;}
    Q_INVOKABLE void setPostFilePath(QString filePath){m_strFilePath = filePath;}
signals:
    void replyFinished(const int row,const qreal);
public slots:
    void start();
signals:
    void finished();
private:
    QString m_strServerAddr;
    QString m_strFilePath;
    int row;
    QVector<theFile*>files;
    SOCKET sock_clt;
    //服务器地址
    SOCKADDR_IN servAddr;
    //主服务器id
    QString addr;

private:
    QString get_file_contents(const std::string filename);
    int sendFile(QString filePath, QString fileName);
    int IniClientSocket();
};

#endif // UPLOADFILE_H

#-------------------------------------------------
#
# Project created by QtCreator 2018-01-12T13:48:18
#
#-------------------------------------------------

QT       += core gui sql xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += $$PWD/include

TARGET = AutoUpdateM
TEMPLATE = app
CONFIG+=c++11
LIBS += ws2_32.lib
LIBS +=
RC_ICONS = tr.ico
win32:DEFINES += _WINSOCKAPI_
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    ./include/soapClient.cpp \
    ./include/stdsoap2.cpp \
    ./include/soapC.cpp \
        widget.cpp \
    mydatabase.cpp \
    uploaddelegate.cpp \
    uploadfile.cpp \
    xmlconfig.cpp \
    sendmesgtosevr.cpp \
    bardelegate.cpp \
    animationprogressbar.cpp

HEADERS  += widget.h \
    mydatabase.h \
    head.h \
    uploaddelegate.h \
    uploadfile.h \
    xmlconfig.h \
    sendmesgtosevr.h \
    pthread/pthread.h \
    pthread/sched.h \
    pthread/semaphore.h \
    pthread/threadpool.h \
    bardelegate.h \
    animationprogressbar.h

DISTFILES +=

RESOURCES += \
    autoupdatem.qrc \
    other/main.qrc \
    other/qss.qrc

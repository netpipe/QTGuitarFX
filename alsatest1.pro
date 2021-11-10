#-------------------------------------------------
#
# Project created by QtCreator 2011-11-20T20:59:16
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = alsatest1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myAlsa.cpp \
    FX.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

LIBS += -L/usr/include/alsa -lasound
LIBS += -L/usr/include/jack -ljack

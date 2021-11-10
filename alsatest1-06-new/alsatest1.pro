#-------------------------------------------------
#
# Project created by QtCreator 2011-11-20T20:59:16
#
#-------------------------------------------------

QT       += core gui

TARGET = alsatest1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myAlsa.cpp \
    FX.cpp \
    handleMIDI.cpp \
    sine_osc.cpp \
    env.cpp \
    looper.cpp

HEADERS  += mainwindow.h \
    sine_osc.h \
    env.h \
    looper.h

FORMS    += mainwindow.ui

LIBS += -L/usr/include/alsa -lasound
LIBS += -L/usr/include/jack -ljack

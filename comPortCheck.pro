#-------------------------------------------------
#
# Project created by QtCreator 2022-03-14T17:20:39
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = comPortCheck
TEMPLATE = app


SOURCES += main.cpp\
        checkdata.cpp

HEADERS  += checkdata.h

FORMS    += checkdata.ui

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG -= qt

QT += serialport core

SOURCES += \
        main.cpp \
        stmuart.cpp

HEADERS += \
    stmuart.h

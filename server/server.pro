TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        sqlite3.c
LIBS += -lws2_32

HEADERS += \
    sqlite3.h

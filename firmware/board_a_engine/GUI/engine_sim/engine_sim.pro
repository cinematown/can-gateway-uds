QT += widgets

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = engine_sim

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialconnection.cpp

HEADERS += \
    mainwindow.h \
    serialconnection.h

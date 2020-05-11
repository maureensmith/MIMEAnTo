#-------------------------------------------------
#
# Project created by QtCreator 2015-12-01T16:58:12
#
#-------------------------------------------------

QT       += core gui svg

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11
TARGET = MIMEAnTo
TEMPLATE = app
#QMAKE_CC = gcc-9
#QMAKE_CXX = g++-9
#QMAKE_LINK = g++9
LIBS += -L/Users/msmith/homebrew/Cellar/boost/1.72.0/lib


SOURCES += main.cpp\
        mimemainwindow.cpp \
    ioTools.cpp \
    plot.cpp \
    processing.cpp \
    utils.cpp \
    datatablelineedit.cpp \
    sampleactivationcheckbox.cpp \
    messages.cpp \
    mimeexception.cpp

HEADERS  += mimemainwindow.hpp \
    ioTools.hpp \
    plot.hpp \
    processing.hpp \
    utils.hpp \
    gnuplot-iostream/gnuplot-iostream.h \
    datatablelineedit.hpp \
    sampleactivationcheckbox.hpp \
    messages.hpp \
    mimeexception.hpp

FORMS    += mimemainwindow.ui

#QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += /Users/msmith/homebrew/Cellar/boost/1.72.0/include

LIBS += -lboost_system -lboost_filesystem -lboost_iostreams


RESOURCES += \
    icons.qrc

RC_FILE = icons.rc

ICON = MIMEAnTo_Icon.icns


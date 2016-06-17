#-------------------------------------------------
#
# Project created by QtCreator 2015-12-01T16:58:12
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#macx {
#    QMAKE_MACOSX_DEPLOYMENT_TARGET = macosx10.9
    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ##QMAKE_MAC_SDK = macosx10.9
    #QMAKE_CXXFLAGS += -stdlib=libc++
    ##QMAKE_CXXFLAGS += -std=c++11
    ##CONFIG += c++11
#}

QMAKE_CXXFLAGS += -std=c++11
TARGET = MIMEAnTo
TEMPLATE = app


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

# if libraries are linked dynamically add qmake parameter CONFIG+=use_libs, if static CONFIG+=use_staticlibs
use_libs {
  LIBS += -lboost_system -lboost_filesystem -lboost_iostreams
}

use_staticlibs {
    LIBS += /usr/local/lib/libboost_filesystem.a /usr/local/lib/libboost_system.a /usr/lib/x86_64-linux-gnu/libboost_iostreams.a
}

# boost libs for cross compiling for windows
use_winlibs {
    LIBS += ~/Programmierung/mxe/usr/i686-w64-mingw32.static/lib/libboost_system-mt.a
    LIBS += ~/Programmierung/mxe/usr/i686-w64-mingw32.static/lib/libboost_filesystem-mt.a
    LIBS += ~/Programmierung/mxe/usr/i686-w64-mingw32.static/lib/libboost_iostreams-mt.a
}

use_maclibs {
    QMAKE_CXXFLAGS += -I ~/homebrew/Cellar/boost/1.60.0_2/include
    LIBS += ~/homebrew/Cellar/boost/1.60.0_2/lib/libboost_system.a
    LIBS += ~/homebrew/Cellar/boost/1.60.0_2/lib/libboost_filesystem.a
    LIBS += ~/homebrew/Cellar/boost/1.60.0_2/lib/libboost_iostreams.a
}

#CC=$(CROSS)gcc
#LD=$(CROSS)ld
#AR=$(CROSS)ar
#PKG_CONFIG=$(CROSS)pkg-config



RESOURCES += \
    icons.qrc

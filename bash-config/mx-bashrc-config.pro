#-------------------------------------------------
#
# Project created by QtCreator 2019-04-27T11:02:42
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG   += warn_on strict_c++ c++17

QMAKE_CXXFLAGS += -Wpedantic -pedantic -Werror=return-type -Werror=switch
QMAKE_CXXFLAGS += -Werror=uninitialized -Werror=return-local-addr

TARGET = bash-config
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp      \
           window.cpp    \
           buffer.cpp    \
           searcher.cpp  \
           tab.cpp       \
           prompttab.cpp \
           aliastab.cpp  \
           aliasstream.cpp \
           global.cpp    \
           othertab.cpp  \
           datetimeformatting.cpp \
           fuzzybashstream.cpp \
           dateformatpreview.cpp

HEADERS += window.h      \
           buffer.h      \
           searcher.h    \
           tab.h         \
           global.h      \
           prompttab.h   \
           aliastab.h    \
           aliasstream.h \
           othertab.h    \
           datetimeformatting.h \
           fuzzybashstream.h \
           dateformatpreview.h

FORMS    += \
           aliastab.ui   \
           window_fix.ui \
           prompttab_fix.ui \
           othertab.ui

TRANSLATIONS += translations/bash-config_en.ts

RESOURCES += \
    resource.qrc

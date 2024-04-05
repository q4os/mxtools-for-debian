QT       += core gui widgets

TARGET = mx-datetime
TEMPLATE = app
CONFIG += debug_and_release warn_on strict_c++
CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -flto=auto
    QMAKE_LFLAGS += -flto=auto
}
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00

SOURCES += \
    main.cpp \ 
    datetime.cpp \
    about.cpp \
    clockface.cpp
HEADERS += \
    datetime.h \
    about.h \
    clockface.h \
    version.h
FORMS += datetime.ui

RESOURCES += \
    images.qrc

TRANSLATIONS += translations/mx-datetime_en.ts

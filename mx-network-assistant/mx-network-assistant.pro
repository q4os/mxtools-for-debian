QT       += widgets
CONFIG   += debug_and_release warn_on strict_c++ c++17

CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -flto=auto
    QMAKE_LFLAGS += -flto=auto
}

QMAKE_CXXFLAGS += -Wpedantic -pedantic -Werror=return-type -Werror=switch
QMAKE_CXXFLAGS += -Werror=uninitialized -Werror=return-local-addr -Werror

DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = mx-network-assistant

HEADERS += \
    common.h \
    mainwindow.h \
    about.h \
    cmd.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    about.cpp \
    cmd.cpp

FORMS += \
    mainwindow.ui

RESOURCES += \
    images.qrc

TRANSLATIONS += \
    translations/mx-network-assistant_en.ts

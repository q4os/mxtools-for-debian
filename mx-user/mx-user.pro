QT       += widgets
CONFIG   += c++17 release warn_on thread qt

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = mx-user

TRANSLATIONS += \
    translations/mx-user_en.ts

FORMS += \
    mainwindow.ui

HEADERS += \
    common.h \
    mainwindow.h \
    passedit.h \
    cmd.h \
    about.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    cmd.cpp \
    about.cpp \
    passedit.cpp

LIBS += -lcrypt -lzxcvbn

RESOURCES += \
    images.qrc

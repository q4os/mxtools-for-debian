# **********************************************************************
# * Copyright (C) 2017 MX Authors
# *
# * Authors: Adrian
# *          Dolphin_Oracle
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-packageinstaller.
# *
# * mx-packageinstaller is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-packageinstaller is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-packageinstaller.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui xml network widgets
CONFIG   += debug_and_release warn_on strict_c++ c++20

CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -flto=auto
    QMAKE_LFLAGS += -flto=auto
    QMAKE_CXXFLAGS_RELEASE = -O3
}

QMAKE_CXXFLAGS += -Wpedantic -pedantic -Werror=return-type -Werror=switch
QMAKE_CXXFLAGS += -Werror=uninitialized -Werror=return-local-addr -Werror

TARGET = mx-packageinstaller
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    lockfile.cpp \
    pmfiles.cpp \
    versionnumber.cpp \
    aptcache.cpp \
    remotes.cpp \
    about.cpp \
    cmd.cpp

HEADERS  += \
    log.h \
    mainwindow.h \
    lockfile.h \
    pmfiles.h \
    versionnumber.h \
    aptcache.h \
    remotes.h \
    about.h \
    cmd.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += \
    translations/mx-packageinstaller_en.ts

RESOURCES += \
    images.qrc

DISTFILES += \
    icons/package-installed-outdated.png \
    icons/package-installed-updated.png

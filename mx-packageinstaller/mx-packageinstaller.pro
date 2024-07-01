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
CONFIG   += c++17

TARGET = mx-packageinstaller
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

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

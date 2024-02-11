# **********************************************************************
# * Copyright (C) 2024 MX Authors
# *
# * Authors: Dolphin Oracle
# *          MX Linux <http://mxlinux.org>
# *
# * This is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this package. If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui widgets
CONFIG   += c++1z

TARGET = mx-locale
TEMPLATE = app


SOURCES += main.cpp\
    choosedialog.cpp \
    mainwindow.cpp \
    about.cpp \
    cmd.cpp

HEADERS  += \
    choosedialog.h \
    mainwindow.h \
    about.h \
    cmd.h

FORMS    += \
    choosedialog.ui \
    mainwindow.ui

TRANSLATIONS += translations/mx-locale_en.ts


RESOURCES += \
    images.qrc

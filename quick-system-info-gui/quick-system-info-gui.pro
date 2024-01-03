# **********************************************************************
# * Copyright (C) 2015 MX Authors
# *
# * Authors: Dolphin Oracle
#*           Adrian, Paul David Callahan
# *          MX & MEPIS Community <http://forum.mepiscommunity.org>
# *
# * This file is part of mx-welcome.
# *
# * mx-welcome is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-welcome is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-welcome.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = quick-system-info-gui
TEMPLATE = app


SOURCES += main.cpp\
    about.cpp \
    mainwindow.cpp

HEADERS  += \
    about.h \
    mainwindow.h \
    version.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/quick-system-info-gui_en.ts \
                

RESOURCES += \
    qsi_resource.qrc

LIBS += -larchive

CONFIG += release warn_on c++17

#-------------------------------------------------
#
# Project created by QtCreator 2014-02-14T11:35:17
#
#-------------------------------------------------

#/*****************************************************************************
#* mx-codecs.pro
# *****************************************************************************
# * Copyright (C) 2014 MX Authors
# *
# * Authors: Jerry 3904
# *          Anticaptilista
# *          Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * MX Codecs is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with MX Codecs.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/


QT       += core gui network
CONFIG   += c++1z

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mx-codecs
TEMPLATE = app


SOURCES += main.cpp\
    lockfile.cpp \
    cmd.cpp \
    mainwindow.cpp \
    about.cpp

HEADERS  += \
    lockfile.h \
    version.h \
    cmd.h \
    mainwindow.h \
    about.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-codecs_en.ts

RESOURCES += \
    images.qrc

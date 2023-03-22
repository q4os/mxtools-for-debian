#/*****************************************************************************
#* mx-viewer.pro
# *****************************************************************************
# * Copyright (C) 2022 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * MX Viewer is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += webenginewidgets
CONFIG   += c++17

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = mx-viewer
TEMPLATE = app

SOURCES += \
    main.cpp \
    addressbar.cpp \
    downloadwidget.cpp \
    mainwindow.cpp

HEADERS  += \
    addressbar.h \
    downloadwidget.h \
    version.h \
    mainwindow.h

TRANSLATIONS += translations/mx-viewer_fr.ts

RESOURCES += \
    images.qrc

FORMS += \
    downloadwidget.ui

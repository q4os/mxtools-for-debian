#/*****************************************************************************
#* mx-viewer.pro
# *****************************************************************************
# * Copyright (C) 2022 MX Authors
# *
# * Authors: Adrian <adrian@mxlinux.org>
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

QT       += core gui widgets webenginewidgets
CONFIG   += debug_and_release warn_on strict_c++ c++20

CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -flto=auto
    QMAKE_LFLAGS += -flto=auto
    QMAKE_CXXFLAGS_RELEASE = -O3
}

QMAKE_CXXFLAGS += -Wpedantic -pedantic -Werror=return-type -Werror=switch
QMAKE_CXXFLAGS += -Werror=uninitialized -Werror=return-local-addr -Werror

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
    mainwindow.cpp \
    tabwidget.cpp \
    webview.cpp

HEADERS  += \
    addressbar.h \
    downloadwidget.h \
    tabwidget.h \
    mainwindow.h \
    webview.h

TRANSLATIONS += \
    translations/mx-viewer_en.ts

RESOURCES += \
    images.qrc

FORMS += \
    downloadwidget.ui

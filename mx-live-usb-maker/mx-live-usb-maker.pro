# **********************************************************************
# * Copyright (C) 2017 MX Authors
# *
# * Authors: Adrian
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

TARGET = mx-live-usb-maker
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    mainwindow.cpp \
    cmd.cpp \
    about.cpp

HEADERS  += \
    mainwindow.h \
    version.h \
    cmd.h \
    about.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-live-usb-maker_am.ts \
                translations/mx-live-usb-maker_ar.ts \
                translations/mx-live-usb-maker_bg.ts \
                translations/mx-live-usb-maker_ca.ts \
                translations/mx-live-usb-maker_cs.ts \
                translations/mx-live-usb-maker_da.ts \
                translations/mx-live-usb-maker_de.ts \
                translations/mx-live-usb-maker_el.ts \
                translations/mx-live-usb-maker_en.ts \
                translations/mx-live-usb-maker_es.ts \
                translations/mx-live-usb-maker_et.ts \
                translations/mx-live-usb-maker_eu.ts \
                translations/mx-live-usb-maker_fa.ts \
                translations/mx-live-usb-maker_fi.ts \
                translations/mx-live-usb-maker_fr.ts \
                translations/mx-live-usb-maker_fr_BE.ts \
                translations/mx-live-usb-maker_he_IL.ts \
                translations/mx-live-usb-maker_hi.ts \
                translations/mx-live-usb-maker_hr.ts \
                translations/mx-live-usb-maker_hu.ts \
                translations/mx-live-usb-maker_id.ts \
                translations/mx-live-usb-maker_is.ts \
                translations/mx-live-usb-maker_it.ts \
                translations/mx-live-usb-maker_ja.ts \
                translations/mx-live-usb-maker_kk.ts \
                translations/mx-live-usb-maker_ko.ts \
                translations/mx-live-usb-maker_lt.ts \
                translations/mx-live-usb-maker_mk.ts \
                translations/mx-live-usb-maker_mr.ts \
                translations/mx-live-usb-maker_nb.ts \
                translations/mx-live-usb-maker_nl.ts \
                translations/mx-live-usb-maker_pl.ts \
                translations/mx-live-usb-maker_pt.ts \
                translations/mx-live-usb-maker_pt_BR.ts \
                translations/mx-live-usb-maker_ro.ts \
                translations/mx-live-usb-maker_ru.ts \
                translations/mx-live-usb-maker_sk.ts \
                translations/mx-live-usb-maker_sl.ts \
                translations/mx-live-usb-maker_sq.ts \
                translations/mx-live-usb-maker_sr.ts \
                translations/mx-live-usb-maker_sv.ts \
                translations/mx-live-usb-maker_tr.ts \
                translations/mx-live-usb-maker_uk.ts \
                translations/mx-live-usb-maker_zh_CN.ts \
                translations/mx-live-usb-maker_zh_TW.ts

RESOURCES += \
    images.qrc


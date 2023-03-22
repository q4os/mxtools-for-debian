# **********************************************************************
# * Copyright (C) 2017 MX Authors
# *
# * Authors: Dolphin Oracle
# *          MX Linux <http://mxlinux.org>
# *          using live-usb-maker by BitJam
# *          and mx-live-usb-maker gui by adrian
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

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = formatusb
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    about.cpp \
    cmd.cpp

HEADERS  += \
    mainwindow.h \
    version.h \
    about.h \
    cmd.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/formatusb_am.ts \
                translations/formatusb_ar.ts \
                translations/formatusb_bg.ts \
                translations/formatusb_ca.ts \
                translations/formatusb_cs.ts \
                translations/formatusb_da.ts \
                translations/formatusb_de.ts \
                translations/formatusb_el.ts \
                translations/formatusb_es.ts \
                translations/formatusb_es_ES.ts \
                translations/formatusb_et.ts \
                translations/formatusb_eu.ts \
                translations/formatusb_fa.ts \
                translations/formatusb_fil_PH.ts \
                translations/formatusb_fi.ts \
                translations/formatusb_fr.ts \
                translations/formatusb_fr_BE.ts \
                translations/formatusb_he_IL.ts \
                translations/formatusb_hi.ts \
                translations/formatusb_hr.ts \
                translations/formatusb_hu.ts \
                translations/formatusb_id.ts \
                translations/formatusb_is.ts \
                translations/formatusb_it.ts \
                translations/formatusb_ja.ts \
                translations/formatusb_kk.ts \
                translations/formatusb_ko.ts \
                translations/formatusb_lt.ts \
                translations/formatusb_mk.ts \
                translations/formatusb_mr.ts \
                translations/formatusb_nb.ts \
                translations/formatusb_nl.ts \
                translations/formatusb_pl.ts \
                translations/formatusb_pt.ts \
                translations/formatusb_pt_BR.ts \
                translations/formatusb_ro.ts \
                translations/formatusb_ru.ts \
                translations/formatusb_sk.ts \
                translations/formatusb_sl.ts \
                translations/formatusb_sq.ts \
                translations/formatusb_sr.ts \
                translations/formatusb_sv.ts \
                translations/formatusb_tr.ts \
                translations/formatusb_uk.ts \
                translations/formatusb_vi.ts \
                translations/formatusb_zh_CN.ts \
                translations/formatusb_zh_TW.ts

RESOURCES += \
    images.qrc

# **********************************************************************
# * Copyright (C) 2017 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-conky.
# *
# * mx-conky is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-conky is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui widgets
CONFIG   += c++1z

TARGET = mx-conky
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    mainwindow.cpp \
    cmd.cpp \
    versionnumber.cpp

HEADERS  += \
    mainwindow.h \
    version.h \
    cmd.h \
    versionnumber.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-conky_am.ts \
                translations/mx-conky_ar.ts \
                translations/mx-conky_bg.ts \
                translations/mx-conky_ca.ts \
                translations/mx-conky_cs.ts \
                translations/mx-conky_da.ts \
                translations/mx-conky_de.ts \
                translations/mx-conky_el.ts \
                translations/mx-conky_en.ts \
                translations/mx-conky_es.ts \
                translations/mx-conky_et.ts \
                translations/mx-conky_eu.ts \
                translations/mx-conky_fa.ts \
                translations/mx-conky_fi.ts \
                translations/mx-conky_fr.ts \
                translations/mx-conky_fr_BE.ts \
                translations/mx-conky_he_IL.ts \
                translations/mx-conky_hi.ts \
                translations/mx-conky_hr.ts \
                translations/mx-conky_hu.ts \
                translations/mx-conky_id.ts \
                translations/mx-conky_is.ts \
                translations/mx-conky_it.ts \
                translations/mx-conky_ja.ts \
                translations/mx-conky_kk.ts \
                translations/mx-conky_ko.ts \
                translations/mx-conky_lt.ts \
                translations/mx-conky_mk.ts \
                translations/mx-conky_mr.ts \
                translations/mx-conky_nb.ts \
                translations/mx-conky_nl.ts \
                translations/mx-conky_pl.ts \
                translations/mx-conky_pt.ts \
                translations/mx-conky_pt_BR.ts \
                translations/mx-conky_ro.ts \
                translations/mx-conky_ru.ts \
                translations/mx-conky_sk.ts \
                translations/mx-conky_sl.ts \
                translations/mx-conky_sq.ts \
                translations/mx-conky_sr.ts \
                translations/mx-conky_sv.ts \
                translations/mx-conky_tr.ts \
                translations/mx-conky_uk.ts \
                translations/mx-conky_zh_CN.ts \
                translations/mx-conky_zh_TW.ts

RESOURCES += \
    images.qrc



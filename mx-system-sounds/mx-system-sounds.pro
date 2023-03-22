# **********************************************************************
# * Copyright (C) 2016 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-system-sounds.
# *
# * mx-system-sounds is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-system-sounds is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-system-sounds.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui widgets
CONFIG   += c++1z

TARGET = mx-system-sounds
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp \
    about.cpp \
    mainwindow.cpp

HEADERS  += \
    about.h \
    mainwindow.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-system-sounds_am.ts \
                translations/mx-system-sounds_ar.ts \
                translations/mx-system-sounds_bg.ts \
                translations/mx-system-sounds_ca.ts \
                translations/mx-system-sounds_cs.ts \
                translations/mx-system-sounds_da.ts \
                translations/mx-system-sounds_de.ts \
                translations/mx-system-sounds_el.ts \
                translations/mx-system-sounds_en.ts \
                translations/mx-system-sounds_es.ts \
                translations/mx-system-sounds_et.ts \
                translations/mx-system-sounds_eu.ts \
                translations/mx-system-sounds_fa.ts \
                translations/mx-system-sounds_fi.ts \
                translations/mx-system-sounds_fr.ts \
                translations/mx-system-sounds_fr_BE.ts \
                translations/mx-system-sounds_he_IL.ts \
                translations/mx-system-sounds_hi.ts \
                translations/mx-system-sounds_hr.ts \
                translations/mx-system-sounds_hu.ts \
                translations/mx-system-sounds_id.ts \
                translations/mx-system-sounds_is.ts \
                translations/mx-system-sounds_it.ts \
                translations/mx-system-sounds_ja.ts \
                translations/mx-system-sounds_kk.ts \
                translations/mx-system-sounds_ko.ts \
                translations/mx-system-sounds_lt.ts \
                translations/mx-system-sounds_mk.ts \
                translations/mx-system-sounds_mr.ts \
                translations/mx-system-sounds_nb.ts \
                translations/mx-system-sounds_nl.ts \
                translations/mx-system-sounds_pl.ts \
                translations/mx-system-sounds_pt.ts \
                translations/mx-system-sounds_pt_BR.ts \
                translations/mx-system-sounds_ro.ts \
                translations/mx-system-sounds_ru.ts \
                translations/mx-system-sounds_sk.ts \
                translations/mx-system-sounds_sl.ts \
                translations/mx-system-sounds_sq.ts \
                translations/mx-system-sounds_sr.ts \
                translations/mx-system-sounds_sv.ts \
                translations/mx-system-sounds_tr.ts \
                translations/mx-system-sounds_uk.ts \
                translations/mx-system-sounds_zh_CN.ts \
                translations/mx-system-sounds_zh_TW.ts

RESOURCES += \
    images.qrc



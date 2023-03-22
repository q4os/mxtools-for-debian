# **********************************************************************
# * Copyright (C) 2015 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-select-sound.
# *
# * mx-select-sound is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-select-sound is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-select-sound.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += core gui widgets
CONFIG   += c++1z

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = mx-select-sound
TEMPLATE = app


SOURCES += main.cpp \
    about.cpp \
    mainwindow.cpp

HEADERS  += \
    about.h \
    version.h \
    mainwindow.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-select-sound_am.ts \
                translations/mx-select-sound_ar.ts \
                translations/mx-select-sound_bg.ts \
                translations/mx-select-sound_ca.ts \
                translations/mx-select-sound_cs.ts \
                translations/mx-select-sound_da.ts \
                translations/mx-select-sound_de.ts \
                translations/mx-select-sound_el.ts \
                translations/mx-select-sound_en.ts \
                translations/mx-select-sound_es.ts \
                translations/mx-select-sound_et.ts \
                translations/mx-select-sound_eu.ts \
                translations/mx-select-sound_fa.ts \
                translations/mx-select-sound_fi.ts \
                translations/mx-select-sound_fr.ts \
                translations/mx-select-sound_he_IL.ts \
                translations/mx-select-sound_hi.ts \
                translations/mx-select-sound_hr.ts \
                translations/mx-select-sound_hu.ts \
                translations/mx-select-sound_id.ts \
                translations/mx-select-sound_is.ts \
                translations/mx-select-sound_it.ts \
                translations/mx-select-sound_ja.ts \
                translations/mx-select-sound_kk.ts \
                translations/mx-select-sound_ko.ts \
                translations/mx-select-sound_lt.ts \
                translations/mx-select-sound_mk.ts \
                translations/mx-select-sound_mr.ts \
                translations/mx-select-sound_nb.ts \
                translations/mx-select-sound_nl.ts \
                translations/mx-select-sound_pl.ts \
                translations/mx-select-sound_pt.ts \
                translations/mx-select-sound_pt_BR.ts \
                translations/mx-select-sound_ro.ts \
                translations/mx-select-sound_ru.ts \
                translations/mx-select-sound_sk.ts \
                translations/mx-select-sound_sl.ts \
                translations/mx-select-sound_sq.ts \
                translations/mx-select-sound_sr.ts \
                translations/mx-select-sound_sv.ts \
                translations/mx-select-sound_tr.ts \
                translations/mx-select-sound_uk.ts \
                translations/mx-select-sound_zh_CN.ts \
                translations/mx-select-sound_zh_TW.ts

RESOURCES += \
    images.qrc



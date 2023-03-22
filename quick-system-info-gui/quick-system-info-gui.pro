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
    mainwindow.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/quick-system-info-gui_af.ts \
				translations/quick-system-info-gui_am.ts \
                translations/quick-system-info-gui_ar.ts \
                translations/quick-system-info-gui_be.ts \
                translations/quick-system-info-gui_bg.ts \
                translations/quick-system-info-gui_bn.ts \
                translations/quick-system-info-gui_ca.ts \
                translations/quick-system-info-gui_cs.ts \
                translations/quick-system-info-gui_da.ts \
                translations/quick-system-info-gui_de.ts \
                translations/quick-system-info-gui_el.ts \
                translations/quick-system-info-gui_en_GB.ts \
                translations/quick-system-info-gui_en.ts \
                translations/quick-system-info-gui_eo.ts \
                translations/quick-system-info-gui_es.ts \
                translations/quick-system-info-gui_es_ES.ts \
                translations/quick-system-info-gui_et.ts \
                translations/quick-system-info-gui_eu.ts \
                translations/quick-system-info-gui_fa.ts \
                translations/quick-system-info-gui_fi.ts \
                translations/quick-system-info-gui_fil_PH.ts \
                translations/quick-system-info-gui_fr.ts \
                translations/quick-system-info-gui_fr_BE.ts \
                translations/quick-system-info-gui_gu.ts \
                translations/quick-system-info-gui_he.ts \
                translations/quick-system-info-gui_he_IL.ts \
                translations/quick-system-info-gui_hi.ts \
                translations/quick-system-info-gui_hr.ts \
                translations/quick-system-info-gui_hu.ts \
                translations/quick-system-info-gui_id.ts \
                translations/quick-system-info-gui_is.ts \
                translations/quick-system-info-gui_it.ts \
                translations/quick-system-info-gui_ja.ts \
                translations/quick-system-info-gui_kk.ts \
                translations/quick-system-info-gui_ko.ts \
                translations/quick-system-info-gui_ku.ts \
                translations/quick-system-info-gui_lt.ts \
                translations/quick-system-info-gui_mk.ts \
                translations/quick-system-info-gui_mr.ts \
                translations/quick-system-info-gui_nb.ts \
                #translations/quick-system-info-gui_nb_NO.ts \
                translations/quick-system-info-gui_nl.ts \
                #translations/quick-system-info-gui_nl_BE.ts \
                translations/quick-system-info-gui_or.ts \
                translations/quick-system-info-gui_pl.ts \
                translations/quick-system-info-gui_pt.ts \
                translations/quick-system-info-gui_pt_BR.ts \
                translations/quick-system-info-gui_ro.ts \
                translations/quick-system-info-gui_ru.ts \
                translations/quick-system-info-gui_sk.ts \
                translations/quick-system-info-gui_sl.ts \
                translations/quick-system-info-gui_so.ts \
                translations/quick-system-info-gui_sq.ts \
                translations/quick-system-info-gui_sr.ts \
                translations/quick-system-info-gui_sv.ts \
                translations/quick-system-info-gui_th.ts \
                translations/quick-system-info-gui_tr.ts \
                translations/quick-system-info-gui_uk.ts \
                translations/quick-system-info-gui_vi.ts \
                translations/quick-system-info-gui_zh_CN.ts \
                translations/quick-system-info-gui_zh_HK.ts \
                translations/quick-system-info-gui_zh_TW.ts

RESOURCES +=



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

TRANSLATIONS += translations/mx-conky_af.ts \
                translations/mx-conky_am.ts \
                translations/mx-conky_ar.ts \
                translations/mx-conky_be.ts \
                translations/mx-conky_bg.ts \
                translations/mx-conky_bn.ts \
                translations/mx-conky_bs_BA.ts \
                translations/mx-conky_bs.ts \
                translations/mx-conky_ca.ts \
                translations/mx-conky_ceb.ts \
                translations/mx-conky_co.ts \
                translations/mx-conky_cs.ts \
                translations/mx-conky_cy.ts \
                translations/mx-conky_da.ts \
                translations/mx-conky_de.ts \
                translations/mx-conky_el.ts \
                translations/mx-conky_en_GB.ts \
                translations/mx-conky_en.ts \
                translations/mx-conky_en_US.ts \
                translations/mx-conky_eo.ts \
                translations/mx-conky_es_ES.ts \
                translations/mx-conky_es.ts \
                translations/mx-conky_et.ts \
                translations/mx-conky_eu.ts \
                translations/mx-conky_fa.ts \
                translations/mx-conky_fi_FI.ts \
                translations/mx-conky_fil_PH.ts \
                translations/mx-conky_fil.ts \
                translations/mx-conky_fi.ts \
                translations/mx-conky_fr_BE.ts \
                translations/mx-conky_fr_FR.ts \
                translations/mx-conky_fr.ts \
                translations/mx-conky_fy.ts \
                translations/mx-conky_ga.ts \
                translations/mx-conky_gd.ts \
                translations/mx-conky_gl_ES.ts \
                translations/mx-conky_gl.ts \
                translations/mx-conky_gu.ts \
                translations/mx-conky_ha.ts \
                translations/mx-conky_haw.ts \
                translations/mx-conky_he_IL.ts \
                translations/mx-conky_he.ts \
                translations/mx-conky_hi.ts \
                translations/mx-conky_hr.ts \
                translations/mx-conky_ht.ts \
                translations/mx-conky_hu.ts \
                translations/mx-conky_hy.ts \
                translations/mx-conky_id.ts \
                translations/mx-conky_is.ts \
                translations/mx-conky_it.ts \
                translations/mx-conky_ja.ts \
                translations/mx-conky_jv.ts \
                translations/mx-conky_ka.ts \
                translations/mx-conky_kk.ts \
                translations/mx-conky_km.ts \
                translations/mx-conky_kn.ts \
                translations/mx-conky_ko.ts \
                translations/mx-conky_ku.ts \
                translations/mx-conky_ky.ts \
                translations/mx-conky_lb.ts \
                translations/mx-conky_lo.ts \
                translations/mx-conky_lt.ts \
                translations/mx-conky_lv.ts \
                translations/mx-conky_mg.ts \
                translations/mx-conky_mi.ts \
                translations/mx-conky_mk.ts \
                translations/mx-conky_ml.ts \
                translations/mx-conky_mn.ts \
                translations/mx-conky_mr.ts \
                translations/mx-conky_ms.ts \
                translations/mx-conky_mt.ts \
                translations/mx-conky_my.ts \
                translations/mx-conky_nb_NO.ts \
                translations/mx-conky_nb.ts \
                translations/mx-conky_ne.ts \
                translations/mx-conky_nl_BE.ts \
                translations/mx-conky_nl.ts \
                translations/mx-conky_ny.ts \
                translations/mx-conky_or.ts \
                translations/mx-conky_pa.ts \
                translations/mx-conky_pl.ts \
                translations/mx-conky_ps.ts \
                translations/mx-conky_pt_BR.ts \
                translations/mx-conky_pt.ts \
                translations/mx-conky_ro.ts \
                translations/mx-conky_rue.ts \
                translations/mx-conky_ru_RU.ts \
                translations/mx-conky_ru.ts \
                translations/mx-conky_rw.ts \
                translations/mx-conky_sd.ts \
                translations/mx-conky_si.ts \
                translations/mx-conky_sk.ts \
                translations/mx-conky_sl.ts \
                translations/mx-conky_sm.ts \
                translations/mx-conky_sn.ts \
                translations/mx-conky_so.ts \
                translations/mx-conky_sq.ts \
                translations/mx-conky_sr.ts \
                translations/mx-conky_st.ts \
                translations/mx-conky_su.ts \
                translations/mx-conky_sv.ts \
                translations/mx-conky_sw.ts \
                translations/mx-conky_ta.ts \
                translations/mx-conky_te.ts \
                translations/mx-conky_tg.ts \
                translations/mx-conky_th.ts \
                translations/mx-conky_tk.ts \
                translations/mx-conky_tr.ts \
                translations/mx-conky_tt.ts \
                translations/mx-conky_ug.ts \
                translations/mx-conky_uk.ts \
                translations/mx-conky_ur.ts \
                translations/mx-conky_uz.ts \
                translations/mx-conky_vi.ts \
                translations/mx-conky_xh.ts \
                translations/mx-conky_yi.ts \
                translations/mx-conky_yo.ts \
                translations/mx-conky_yue_CN.ts \
                translations/mx-conky_zh_CN.ts \
                translations/mx-conky_zh_HK.ts \
                translations/mx-conky_zh_TW.ts

RESOURCES += \
    images.qrc



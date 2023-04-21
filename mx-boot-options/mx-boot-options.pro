# **********************************************************************
# * Copyright (C) 2017 MX Authors
# *
# * Authors: Adrian, Dolphin Oracle
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

QT       += widgets
CONFIG   += c++1z warn_on

TARGET = mx-boot-options
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    about.cpp \
    mainwindow.cpp \
    dialog.cpp \
    cmd.cpp

HEADERS  += \
    about.h \
    mainwindow.h \
    dialog.h \
    version.h \
    cmd.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-boot-options_af.ts \
                translations/mx-boot-options_am.ts \
                translations/mx-boot-options_ar.ts \
                translations/mx-boot-options_be.ts \
                translations/mx-boot-options_bg.ts \
                translations/mx-boot-options_bn.ts \
                translations/mx-boot-options_bs_BA.ts \
                translations/mx-boot-options_bs.ts \
                translations/mx-boot-options_ca.ts \
                translations/mx-boot-options_ceb.ts \
                translations/mx-boot-options_co.ts \
                translations/mx-boot-options_cs.ts \
                translations/mx-boot-options_cy.ts \
                translations/mx-boot-options_da.ts \
                translations/mx-boot-options_de.ts \
                translations/mx-boot-options_el.ts \
                translations/mx-boot-options_en_GB.ts \
                translations/mx-boot-options_en.ts \
                translations/mx-boot-options_en_US.ts \
                translations/mx-boot-options_eo.ts \
                translations/mx-boot-options_es_ES.ts \
                translations/mx-boot-options_es.ts \
                translations/mx-boot-options_et.ts \
                translations/mx-boot-options_eu.ts \
                translations/mx-boot-options_fa.ts \
                translations/mx-boot-options_fi_FI.ts \
                translations/mx-boot-options_fil_PH.ts \
                translations/mx-boot-options_fil.ts \
                translations/mx-boot-options_fi.ts \
                translations/mx-boot-options_fr_BE.ts \
                translations/mx-boot-options_fr_FR.ts \
                translations/mx-boot-options_fr.ts \
                translations/mx-boot-options_fy.ts \
                translations/mx-boot-options_ga.ts \
                translations/mx-boot-options_gd.ts \
                translations/mx-boot-options_gl_ES.ts \
                translations/mx-boot-options_gl.ts \
                translations/mx-boot-options_gu.ts \
                translations/mx-boot-options_ha.ts \
                translations/mx-boot-options_haw.ts \
                translations/mx-boot-options_he_IL.ts \
                translations/mx-boot-options_he.ts \
                translations/mx-boot-options_hi.ts \
                translations/mx-boot-options_hr.ts \
                translations/mx-boot-options_ht.ts \
                translations/mx-boot-options_hu.ts \
                translations/mx-boot-options_hy.ts \
                translations/mx-boot-options_id.ts \
                translations/mx-boot-options_is.ts \
                translations/mx-boot-options_it.ts \
                translations/mx-boot-options_ja.ts \
                translations/mx-boot-options_jv.ts \
                translations/mx-boot-options_ka.ts \
                translations/mx-boot-options_kk.ts \
                translations/mx-boot-options_km.ts \
                translations/mx-boot-options_kn.ts \
                translations/mx-boot-options_ko.ts \
                translations/mx-boot-options_ku.ts \
                translations/mx-boot-options_ky.ts \
                translations/mx-boot-options_lb.ts \
                translations/mx-boot-options_lo.ts \
                translations/mx-boot-options_lt.ts \
                translations/mx-boot-options_lv.ts \
                translations/mx-boot-options_mg.ts \
                translations/mx-boot-options_mi.ts \
                translations/mx-boot-options_mk.ts \
                translations/mx-boot-options_ml.ts \
                translations/mx-boot-options_mn.ts \
                translations/mx-boot-options_mr.ts \
                translations/mx-boot-options_ms.ts \
                translations/mx-boot-options_mt.ts \
                translations/mx-boot-options_my.ts \
                translations/mx-boot-options_nb_NO.ts \
                translations/mx-boot-options_nb.ts \
                translations/mx-boot-options_ne.ts \
                translations/mx-boot-options_nl_BE.ts \
                translations/mx-boot-options_nl.ts \
                translations/mx-boot-options_ny.ts \
                translations/mx-boot-options_or.ts \
                translations/mx-boot-options_pa.ts \
                translations/mx-boot-options_pl.ts \
                translations/mx-boot-options_ps.ts \
                translations/mx-boot-options_pt_BR.ts \
                translations/mx-boot-options_pt.ts \
                translations/mx-boot-options_ro.ts \
                translations/mx-boot-options_rue.ts \
                translations/mx-boot-options_ru_RU.ts \
                translations/mx-boot-options_ru.ts \
                translations/mx-boot-options_rw.ts \
                translations/mx-boot-options_sd.ts \
                translations/mx-boot-options_si.ts \
                translations/mx-boot-options_sk.ts \
                translations/mx-boot-options_sl.ts \
                translations/mx-boot-options_sm.ts \
                translations/mx-boot-options_sn.ts \
                translations/mx-boot-options_so.ts \
                translations/mx-boot-options_sq.ts \
                translations/mx-boot-options_sr.ts \
                translations/mx-boot-options_st.ts \
                translations/mx-boot-options_su.ts \
                translations/mx-boot-options_sv.ts \
                translations/mx-boot-options_sw.ts \
                translations/mx-boot-options_ta.ts \
                translations/mx-boot-options_te.ts \
                translations/mx-boot-options_tg.ts \
                translations/mx-boot-options_th.ts \
                translations/mx-boot-options_tk.ts \
                translations/mx-boot-options_tr.ts \
                translations/mx-boot-options_tt.ts \
                translations/mx-boot-options_ug.ts \
                translations/mx-boot-options_uk.ts \
                translations/mx-boot-options_ur.ts \
                translations/mx-boot-options_uz.ts \
                translations/mx-boot-options_vi.ts \
                translations/mx-boot-options_xh.ts \
                translations/mx-boot-options_yi.ts \
                translations/mx-boot-options_yo.ts \
                translations/mx-boot-options_yue_CN.ts \
                translations/mx-boot-options_zh_CN.ts \
                translations/mx-boot-options_zh_HK.ts \
                translations/mx-boot-options_zh_TW.ts \

RESOURCES += \
    images.qrc

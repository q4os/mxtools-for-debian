# **********************************************************************
# * Copyright (C) 2021 MX Authors
# *
# * Authors: Adrian <adrian@mxlinux.org>
# *          Dolphin_Oracle
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

TARGET = mx-samba-config
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    editshare.cpp \
    mainwindow.cpp \
    about.cpp

HEADERS  += \
    editshare.h \
    mainwindow.h \
    version.h \
    about.h

FORMS    += \
    editshare.ui \
    mainwindow.ui

TRANSLATIONS += translations/mx-samba-config_af.ts \
                translations/mx-samba-config_am.ts \
                translations/mx-samba-config_ar.ts \
                translations/mx-samba-config_ast.ts \
                translations/mx-samba-config_be.ts \
                translations/mx-samba-config_bg.ts \
                translations/mx-samba-config_bn.ts \
                translations/mx-samba-config_bs_BA.ts \
                translations/mx-samba-config_bs.ts \
                translations/mx-samba-config_ca.ts \
                translations/mx-samba-config_ceb.ts \
                translations/mx-samba-config_co.ts \
                translations/mx-samba-config_cs.ts \
                translations/mx-samba-config_cy.ts \
                translations/mx-samba-config_da.ts \
                translations/mx-samba-config_de.ts \
                translations/mx-samba-config_el.ts \
                translations/mx-samba-config_en_GB.ts \
                translations/mx-samba-config_en.ts \
                translations/mx-samba-config_en_US.ts \
                translations/mx-samba-config_eo.ts \
                translations/mx-samba-config_es_ES.ts \
                translations/mx-samba-config_es.ts \
                translations/mx-samba-config_et.ts \
                translations/mx-samba-config_eu.ts \
                translations/mx-samba-config_fa.ts \
                translations/mx-samba-config_fi_FI.ts \
                translations/mx-samba-config_fil_PH.ts \
                translations/mx-samba-config_fil.ts \
                translations/mx-samba-config_fi.ts \
                translations/mx-samba-config_fr_BE.ts \
                translations/mx-samba-config_fr.ts \
                translations/mx-samba-config_fy.ts \
                translations/mx-samba-config_ga.ts \
                translations/mx-samba-config_gd.ts \
                translations/mx-samba-config_gl_ES.ts \
                translations/mx-samba-config_gl.ts \
                translations/mx-samba-config_gu.ts \
                translations/mx-samba-config_ha.ts \
                translations/mx-samba-config_haw.ts \
                translations/mx-samba-config_he_IL.ts \
                translations/mx-samba-config_he.ts \
                translations/mx-samba-config_hi.ts \
                translations/mx-samba-config_hr.ts \
                translations/mx-samba-config_ht.ts \
                translations/mx-samba-config_hu.ts \
                translations/mx-samba-config_hy_AM.ts \
                translations/mx-samba-config_hye.ts \
                translations/mx-samba-config_hy.ts \
                translations/mx-samba-config_id.ts \
                translations/mx-samba-config_ie.ts \
                translations/mx-samba-config_is.ts \
                translations/mx-samba-config_it.ts \
                translations/mx-samba-config_ja.ts \
                translations/mx-samba-config_jv.ts \
                translations/mx-samba-config_kab.ts \
                translations/mx-samba-config_ka.ts \
                translations/mx-samba-config_kk.ts \
                translations/mx-samba-config_km.ts \
                translations/mx-samba-config_kn.ts \
                translations/mx-samba-config_ko.ts \
                translations/mx-samba-config_ku.ts \
                translations/mx-samba-config_ky.ts \
                translations/mx-samba-config_lb.ts \
                translations/mx-samba-config_lo.ts \
                translations/mx-samba-config_lt.ts \
                translations/mx-samba-config_lv.ts \
                translations/mx-samba-config_mg.ts \
                translations/mx-samba-config_mi.ts \
                translations/mx-samba-config_mk.ts \
                translations/mx-samba-config_ml.ts \
                translations/mx-samba-config_mn.ts \
                translations/mx-samba-config_mr.ts \
                translations/mx-samba-config_ms.ts \
                translations/mx-samba-config_mt.ts \
                translations/mx-samba-config_my.ts \
                translations/mx-samba-config_nb_NO.ts \
                translations/mx-samba-config_nb.ts \
                translations/mx-samba-config_ne.ts \
                translations/mx-samba-config_nl_BE.ts \
                translations/mx-samba-config_nl.ts \
                translations/mx-samba-config_nn.ts \
                translations/mx-samba-config_ny.ts \
                translations/mx-samba-config_oc.ts \
                translations/mx-samba-config_or.ts \
                translations/mx-samba-config_pa.ts \
                translations/mx-samba-config_pl.ts \
                translations/mx-samba-config_ps.ts \
                translations/mx-samba-config_pt_BR.ts \
                translations/mx-samba-config_pt.ts \
                translations/mx-samba-config_ro.ts \
                translations/mx-samba-config_rue.ts \
                translations/mx-samba-config_ru_RU.ts \
                translations/mx-samba-config_ru.ts \
                translations/mx-samba-config_rw.ts \
                translations/mx-samba-config_sd.ts \
                translations/mx-samba-config_si.ts \
                translations/mx-samba-config_sk.ts \
                translations/mx-samba-config_sl.ts \
                translations/mx-samba-config_sm.ts \
                translations/mx-samba-config_sn.ts \
                translations/mx-samba-config_so.ts \
                translations/mx-samba-config_sq.ts \
                translations/mx-samba-config_sr.ts \
                translations/mx-samba-config_st.ts \
                translations/mx-samba-config_su.ts \
                translations/mx-samba-config_sv.ts \
                translations/mx-samba-config_sw.ts \
                translations/mx-samba-config_ta.ts \
                translations/mx-samba-config_te.ts \
                translations/mx-samba-config_tg.ts \
                translations/mx-samba-config_th.ts \
                translations/mx-samba-config_tk.ts \
                translations/mx-samba-config_tr.ts \
                translations/mx-samba-config_tt.ts \
                translations/mx-samba-config_ug.ts \
                translations/mx-samba-config_uk.ts \
                translations/mx-samba-config_ur.ts \
                translations/mx-samba-config_uz.ts \
                translations/mx-samba-config_vi.ts \
                translations/mx-samba-config_xh.ts \
                translations/mx-samba-config_yi.ts \
                translations/mx-samba-config_yo.ts \
                translations/mx-samba-config_yue_CN.ts \
                translations/mx-samba-config_zh_CN.ts \
                translations/mx-samba-config_zh_HK.ts \
                translations/mx-samba-config_zh_TW.ts

RESOURCES += \
    images.qrc


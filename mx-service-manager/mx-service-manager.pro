# **********************************************************************
# * Copyright (C) 2023 MX Authors
# *
# * Authors: Adrian <adrian@mxlinux.org>
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
CONFIG   += c++17

TARGET = mx-service-manager
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    mainwindow.cpp \
    about.cpp \
    cmd.cpp \
    service.cpp

HEADERS  += \
    mainwindow.h \
    service.h \
    version.h \
    about.h \
    cmd.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-service-manager_af.ts \
                translations/mx-service-manager_am.ts \
                translations/mx-service-manager_ar.ts \
                translations/mx-service-manager_ast.ts \
                translations/mx-service-manager_be.ts \
                translations/mx-service-manager_bg.ts \
                translations/mx-service-manager_bn.ts \
                translations/mx-service-manager_bs.ts \
                translations/mx-service-manager_ca.ts \
                translations/mx-service-manager_ceb.ts \
                translations/mx-service-manager_co.ts \
                translations/mx-service-manager_cs.ts \
                translations/mx-service-manager_cy.ts \
                translations/mx-service-manager_da.ts \
                translations/mx-service-manager_de.ts \
                translations/mx-service-manager_el.ts \
                translations/mx-service-manager_en_GB.ts \
                translations/mx-service-manager_en_US.ts \
                translations/mx-service-manager_eo.ts \
                translations/mx-service-manager_es_ES.ts \
                translations/mx-service-manager_et.ts \
                translations/mx-service-manager_eu.ts \
                translations/mx-service-manager_fa.ts \
                translations/mx-service-manager_fi.ts \
                translations/mx-service-manager_fi_FI.ts \
                translations/mx-service-manager_fil.ts \
                translations/mx-service-manager_fr.ts \
                translations/mx-service-manager_fr_BE.ts \
                translations/mx-service-manager_fy.ts \
                translations/mx-service-manager_ga.ts \
                translations/mx-service-manager_gd.ts \
                translations/mx-service-manager_gl.ts \
                translations/mx-service-manager_gl_ES.ts \
                translations/mx-service-manager_gu.ts \
                translations/mx-service-manager_ha.ts \
                translations/mx-service-manager_haw.ts \
                translations/mx-service-manager_he.ts \
                translations/mx-service-manager_he_IL.ts \
                translations/mx-service-manager_hi.ts \
                translations/mx-service-manager_hr.ts \
                translations/mx-service-manager_ht.ts \
                translations/mx-service-manager_hu.ts \
                translations/mx-service-manager_hy.ts \
                translations/mx-service-manager_hy_AM.ts \
                translations/mx-service-manager_hye.ts \
                translations/mx-service-manager_id.ts \
                translations/mx-service-manager_ie.ts \
                translations/mx-service-manager_is.ts \
                translations/mx-service-manager_it.ts \
                translations/mx-service-manager_ja.ts \
                translations/mx-service-manager_jv.ts \
                translations/mx-service-manager_ka.ts \
                translations/mx-service-manager_kab.ts \
                translations/mx-service-manager_kk.ts \
                translations/mx-service-manager_km.ts \
                translations/mx-service-manager_kn.ts \
                translations/mx-service-manager_ko.ts \
                translations/mx-service-manager_ku.ts \
                translations/mx-service-manager_ky.ts \
                translations/mx-service-manager_lb.ts \
                translations/mx-service-manager_lo.ts \
                translations/mx-service-manager_lt.ts \
                translations/mx-service-manager_lv.ts \
                translations/mx-service-manager_mg.ts \
                translations/mx-service-manager_mi.ts \
                translations/mx-service-manager_mk.ts \
                translations/mx-service-manager_ml.ts \
                translations/mx-service-manager_mn.ts \
                translations/mx-service-manager_mr.ts \
                translations/mx-service-manager_ms.ts \
                translations/mx-service-manager_mt.ts \
                translations/mx-service-manager_my.ts \
                translations/mx-service-manager_nb.ts \
                translations/mx-service-manager_nb_NO.ts \
                translations/mx-service-manager_ne.ts \
                translations/mx-service-manager_nl.ts \
                translations/mx-service-manager_nl_BE.ts \
                translations/mx-service-manager_nn.ts \
                translations/mx-service-manager_ny.ts \
                translations/mx-service-manager_oc.ts \
                translations/mx-service-manager_or.ts \
                translations/mx-service-manager_pa.ts \
                translations/mx-service-manager_pl.ts \
                translations/mx-service-manager_ps.ts \
                translations/mx-service-manager_pt.ts \
                translations/mx-service-manager_pt_BR.ts \
                translations/mx-service-manager_ro.ts \
                translations/mx-service-manager_ru.ts \
                translations/mx-service-manager_rue.ts \
                translations/mx-service-manager_rw.ts \
                translations/mx-service-manager_sd.ts \
                translations/mx-service-manager_si.ts \
                translations/mx-service-manager_sk.ts \
                translations/mx-service-manager_sl.ts \
                translations/mx-service-manager_sm.ts \
                translations/mx-service-manager_sn.ts \
                translations/mx-service-manager_so.ts \
                translations/mx-service-manager_sq.ts \
                translations/mx-service-manager_sr.ts \
                translations/mx-service-manager_st.ts \
                translations/mx-service-manager_su.ts \
                translations/mx-service-manager_sv.ts \
                translations/mx-service-manager_sw.ts \
                translations/mx-service-manager_ta.ts \
                translations/mx-service-manager_te.ts \
                translations/mx-service-manager_tg.ts \
                translations/mx-service-manager_th.ts \
                translations/mx-service-manager_tk.ts \
                translations/mx-service-manager_tr.ts \
                translations/mx-service-manager_tt.ts \
                translations/mx-service-manager_ug.ts \
                translations/mx-service-manager_uk.ts \
                translations/mx-service-manager_ur.ts \
                translations/mx-service-manager_uz.ts \
                translations/mx-service-manager_vi.ts \
                translations/mx-service-manager_xh.ts \
                translations/mx-service-manager_yi.ts \
                translations/mx-service-manager_yo.ts \
                translations/mx-service-manager_yue_CN.ts \
                translations/mx-service-manager_zh_CN.ts \
                translations/mx-service-manager_zh_HK.ts \
                translations/mx-service-manager_zh_TW.ts

RESOURCES += \
    images.qrc


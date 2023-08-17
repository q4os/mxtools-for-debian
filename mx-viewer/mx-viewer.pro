#/*****************************************************************************
#* mx-viewer.pro
# *****************************************************************************
# * Copyright (C) 2022 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * MX Viewer is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += webenginewidgets
CONFIG   += c++1z warn_on

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = mx-viewer
TEMPLATE = app

SOURCES += \
    main.cpp \
    addressbar.cpp \
    downloadwidget.cpp \
    mainwindow.cpp

HEADERS  += \
    addressbar.h \
    downloadwidget.h \
    version.h \
    mainwindow.h

TRANSLATIONS += \
                translations/mx-viewer_af.ts \
                translations/mx-viewer_am.ts \
                translations/mx-viewer_ar.ts \
                translations/mx-viewer_ast.ts \
                translations/mx-viewer_be.ts \
                translations/mx-viewer_bg.ts \
                translations/mx-viewer_bn.ts \
                translations/mx-viewer_bs.ts \
                translations/mx-viewer_ca.ts \
                translations/mx-viewer_ceb.ts \
                translations/mx-viewer_co.ts \
                translations/mx-viewer_cs.ts \
                translations/mx-viewer_cy.ts \
                translations/mx-viewer_da.ts \
                translations/mx-viewer_de.ts \
                translations/mx-viewer_el.ts \
                translations/mx-viewer_en_GB.ts \
                translations/mx-viewer_en.ts \
                translations/mx-viewer_en_US.ts \
                translations/mx-viewer_eo.ts \
                translations/mx-viewer_es_ES.ts \
                translations/mx-viewer_es.ts \
                translations/mx-viewer_et.ts \
                translations/mx-viewer_eu.ts \
                translations/mx-viewer_fa.ts \
                translations/mx-viewer_fil_PH.ts \
                translations/mx-viewer_fil.ts \
                translations/mx-viewer_fi.ts \
                translations/mx-viewer_fr_BE.ts \
                translations/mx-viewer_fr.ts \
                translations/mx-viewer_fy.ts \
                translations/mx-viewer_ga.ts \
                translations/mx-viewer_gd.ts \
                translations/mx-viewer_gl_ES.ts \
                translations/mx-viewer_gl.ts \
                translations/mx-viewer_gu.ts \
                translations/mx-viewer_ha.ts \
                translations/mx-viewer_haw.ts \
                translations/mx-viewer_he_IL.ts \
                translations/mx-viewer_he.ts \
                translations/mx-viewer_hi.ts \
                translations/mx-viewer_hr.ts \
                translations/mx-viewer_ht.ts \
                translations/mx-viewer_hu.ts \
                translations/mx-viewer_hye.ts \
                translations/mx-viewer_hy.ts \
                translations/mx-viewer_id.ts \
                translations/mx-viewer_ie.ts \
                translations/mx-viewer_is.ts \
                translations/mx-viewer_it.ts \
                translations/mx-viewer_ja.ts \
                translations/mx-viewer_jv.ts \
                translations/mx-viewer_kab.ts \
                translations/mx-viewer_ka.ts \
                translations/mx-viewer_kk.ts \
                translations/mx-viewer_km.ts \
                translations/mx-viewer_kn.ts \
                translations/mx-viewer_ko.ts \
                translations/mx-viewer_ku.ts \
                translations/mx-viewer_ky.ts \
                translations/mx-viewer_lb.ts \
                translations/mx-viewer_lo.ts \
                translations/mx-viewer_lt.ts \
                translations/mx-viewer_lv.ts \
                translations/mx-viewer_mg.ts \
                translations/mx-viewer_mi.ts \
                translations/mx-viewer_mk.ts \
                translations/mx-viewer_ml.ts \
                translations/mx-viewer_mn.ts \
                translations/mx-viewer_mr.ts \
                translations/mx-viewer_ms.ts \
                translations/mx-viewer_mt.ts \
                translations/mx-viewer_my.ts \
                translations/mx-viewer_nb_NO.ts \
                translations/mx-viewer_nb.ts \
                translations/mx-viewer_ne.ts \
                translations/mx-viewer_nl_BE.ts \
                translations/mx-viewer_nl.ts \
                translations/mx-viewer_nn.ts \
                translations/mx-viewer_ny.ts \
                translations/mx-viewer_oc.ts \
                translations/mx-viewer_or.ts \
                translations/mx-viewer_pa.ts \
                translations/mx-viewer_pl.ts \
                translations/mx-viewer_ps.ts \
                translations/mx-viewer_pt_BR.ts \
                translations/mx-viewer_pt.ts \
                translations/mx-viewer_ro.ts \
                translations/mx-viewer_rue.ts \
                translations/mx-viewer_ru.ts \
                translations/mx-viewer_rw.ts \
                translations/mx-viewer_sd.ts \
                translations/mx-viewer_si.ts \
                translations/mx-viewer_sk.ts \
                translations/mx-viewer_sl.ts \
                translations/mx-viewer_sm.ts \
                translations/mx-viewer_sn.ts \
                translations/mx-viewer_so.ts \
                translations/mx-viewer_sq.ts \
                translations/mx-viewer_sr.ts \
                translations/mx-viewer_st.ts \
                translations/mx-viewer_su.ts \
                translations/mx-viewer_sv.ts \
                translations/mx-viewer_sw.ts \
                translations/mx-viewer_ta.ts \
                translations/mx-viewer_te.ts \
                translations/mx-viewer_tg.ts \
                translations/mx-viewer_th.ts \
                translations/mx-viewer_tk.ts \
                translations/mx-viewer_tr.ts \
                translations/mx-viewer_tt.ts \
                translations/mx-viewer_ug.ts \
                translations/mx-viewer_uk.ts \
                translations/mx-viewer_ur.ts \
                translations/mx-viewer_uz.ts \
                translations/mx-viewer_vi.ts \
                translations/mx-viewer_xh.ts \
                translations/mx-viewer_yi.ts \
                translations/mx-viewer_yo.ts \
                translations/mx-viewer_yue_CN.ts \
                translations/mx-viewer_zh_CN.ts \
                translations/mx-viewer_zh_HK.ts \
                translations/mx-viewer_zh_TW.ts

RESOURCES += \
    images.qrc

FORMS += \
    downloadwidget.ui

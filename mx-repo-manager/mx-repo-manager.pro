# **********************************************************************
# * Copyright (C) 2016 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-repo-manager.
# *
# * mx-repo-manager is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * mx-repo-manager is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with mx-repo-manager.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += network widgets
CONFIG   += c++1z

TARGET = mx-repo-manager
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    mainwindow.cpp \
    cmd.cpp \
    about.cpp

HEADERS  += mainwindow.h \
    version.h \
    cmd.h \
    about.h

FORMS    += mainwindow.ui

TRANSLATIONS += translations/mx-repo-manager_af.ts \
                translations/mx-repo-manager_am.ts \
                translations/mx-repo-manager_ar.ts \
                translations/mx-repo-manager_be.ts \
                translations/mx-repo-manager_bg.ts \
                translations/mx-repo-manager_bn.ts \
                translations/mx-repo-manager_bs_BA.ts \
                translations/mx-repo-manager_bs.ts \
                translations/mx-repo-manager_ca.ts \
                translations/mx-repo-manager_ceb.ts \
                translations/mx-repo-manager_co.ts \
                translations/mx-repo-manager_cs.ts \
                translations/mx-repo-manager_cy.ts \
                translations/mx-repo-manager_da.ts \
                translations/mx-repo-manager_de.ts \
                translations/mx-repo-manager_el.ts \
                translations/mx-repo-manager_en_GB.ts \
                translations/mx-repo-manager_en.ts \
                translations/mx-repo-manager_en_US.ts \
                translations/mx-repo-manager_eo.ts \
                translations/mx-repo-manager_es_ES.ts \
                translations/mx-repo-manager_es.ts \
                translations/mx-repo-manager_et.ts \
                translations/mx-repo-manager_eu.ts \
                translations/mx-repo-manager_fa.ts \
                translations/mx-repo-manager_fi_FI.ts \
                translations/mx-repo-manager_fil_PH.ts \
                translations/mx-repo-manager_fil.ts \
                translations/mx-repo-manager_fi.ts \
                translations/mx-repo-manager_fr_BE.ts \
                translations/mx-repo-manager_fr_FR.ts \
                translations/mx-repo-manager_fr.ts \
                translations/mx-repo-manager_fy.ts \
                translations/mx-repo-manager_ga.ts \
                translations/mx-repo-manager_gd.ts \
                translations/mx-repo-manager_gl_ES.ts \
                translations/mx-repo-manager_gl.ts \
                translations/mx-repo-manager_gu.ts \
                translations/mx-repo-manager_ha.ts \
                translations/mx-repo-manager_haw.ts \
                translations/mx-repo-manager_he_IL.ts \
                translations/mx-repo-manager_he.ts \
                translations/mx-repo-manager_hi.ts \
                translations/mx-repo-manager_hr.ts \
                translations/mx-repo-manager_ht.ts \
                translations/mx-repo-manager_hu.ts \
                translations/mx-repo-manager_hy.ts \
                translations/mx-repo-manager_id.ts \
                translations/mx-repo-manager_is.ts \
                translations/mx-repo-manager_it.ts \
                translations/mx-repo-manager_ja.ts \
                translations/mx-repo-manager_jv.ts \
                translations/mx-repo-manager_ka.ts \
                translations/mx-repo-manager_kk.ts \
                translations/mx-repo-manager_km.ts \
                translations/mx-repo-manager_kn.ts \
                translations/mx-repo-manager_ko.ts \
                translations/mx-repo-manager_ku.ts \
                translations/mx-repo-manager_ky.ts \
                translations/mx-repo-manager_lb.ts \
                translations/mx-repo-manager_lo.ts \
                translations/mx-repo-manager_lt.ts \
                translations/mx-repo-manager_lv.ts \
                translations/mx-repo-manager_mg.ts \
                translations/mx-repo-manager_mi.ts \
                translations/mx-repo-manager_mk.ts \
                translations/mx-repo-manager_ml.ts \
                translations/mx-repo-manager_mn.ts \
                translations/mx-repo-manager_mr.ts \
                translations/mx-repo-manager_ms.ts \
                translations/mx-repo-manager_mt.ts \
                translations/mx-repo-manager_my.ts \
                translations/mx-repo-manager_nb_NO.ts \
                translations/mx-repo-manager_nb.ts \
                translations/mx-repo-manager_ne.ts \
                translations/mx-repo-manager_nl_BE.ts \
                translations/mx-repo-manager_nl.ts \
                translations/mx-repo-manager_ny.ts \
                translations/mx-repo-manager_or.ts \
                translations/mx-repo-manager_pa.ts \
                translations/mx-repo-manager_pl.ts \
                translations/mx-repo-manager_ps.ts \
                translations/mx-repo-manager_pt_BR.ts \
                translations/mx-repo-manager_pt.ts \
                translations/mx-repo-manager_ro.ts \
                translations/mx-repo-manager_rue.ts \
                translations/mx-repo-manager_ru_RU.ts \
                translations/mx-repo-manager_ru.ts \
                translations/mx-repo-manager_rw.ts \
                translations/mx-repo-manager_sd.ts \
                translations/mx-repo-manager_si.ts \
                translations/mx-repo-manager_sk.ts \
                translations/mx-repo-manager_sl.ts \
                translations/mx-repo-manager_sm.ts \
                translations/mx-repo-manager_sn.ts \
                translations/mx-repo-manager_so.ts \
                translations/mx-repo-manager_sq.ts \
                translations/mx-repo-manager_sr.ts \
                translations/mx-repo-manager_st.ts \
                translations/mx-repo-manager_su.ts \
                translations/mx-repo-manager_sv.ts \
                translations/mx-repo-manager_sw.ts \
                translations/mx-repo-manager_ta.ts \
                translations/mx-repo-manager_te.ts \
                translations/mx-repo-manager_tg.ts \
                translations/mx-repo-manager_th.ts \
                translations/mx-repo-manager_tk.ts \
                translations/mx-repo-manager_tr.ts \
                translations/mx-repo-manager_tt.ts \
                translations/mx-repo-manager_ug.ts \
                translations/mx-repo-manager_uk.ts \
                translations/mx-repo-manager_ur.ts \
                translations/mx-repo-manager_uz.ts \
                translations/mx-repo-manager_vi.ts \
                translations/mx-repo-manager_xh.ts \
                translations/mx-repo-manager_yi.ts \
                translations/mx-repo-manager_yo.ts \
                translations/mx-repo-manager_yue_CN.ts \
                translations/mx-repo-manager_zh_CN.ts \
                translations/mx-repo-manager_zh_HK.ts \
                translations/mx-repo-manager_zh_TW.ts 

RESOURCES += \
    images.qrc

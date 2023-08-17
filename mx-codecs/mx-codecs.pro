#-------------------------------------------------
#
# Project created by QtCreator 2014-02-14T11:35:17
#
#-------------------------------------------------

#/*****************************************************************************
#* mx-codecs.pro
# *****************************************************************************
# * Copyright (C) 2014 MX Authors
# *
# * Authors: Jerry 3904
# *          Anticaptilista
# *          Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * MX Codecs is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with MX Codecs.  If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/


QT       += core gui network
CONFIG   += c++1z

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mx-codecs
TEMPLATE = app


SOURCES += main.cpp\
    lockfile.cpp \
    cmd.cpp \
    mainwindow.cpp \
    about.cpp

HEADERS  += \
    lockfile.h \
    version.h \
    cmd.h \
    mainwindow.h \
    about.h

FORMS    += \
    mainwindow.ui

TRANSLATIONS += translations/mx-codecs_af.ts \
                translations/mx-codecs_af.ts \
                translations/mx-codecs_am.ts \
                translations/mx-codecs_ar.ts \
                translations/mx-codecs_ast.ts \
                translations/mx-codecs_be.ts \
                translations/mx-codecs_bg.ts \
                translations/mx-codecs_bn.ts \
                translations/mx-codecs_bs.ts \
                translations/mx-codecs_ca.ts \
                translations/mx-codecs_ceb.ts \
                translations/mx-codecs_co.ts \
                translations/mx-codecs_cs.ts \
                translations/mx-codecs_cy.ts \
                translations/mx-codecs_da.ts \
                translations/mx-codecs_de.ts \
                translations/mx-codecs_el.ts \
                translations/mx-codecs_en_GB.ts \
                translations/mx-codecs_en.ts \
                translations/mx-codecs_en_US.ts \
                translations/mx-codecs_eo.ts \
                translations/mx-codecs_es_ES.ts \
                translations/mx-codecs_es.ts \
                translations/mx-codecs_et.ts \
                translations/mx-codecs_eu.ts \
                translations/mx-codecs_fa.ts \
                translations/mx-codecs_fil_PH.ts \
                translations/mx-codecs_fil.ts \
                translations/mx-codecs_fi.ts \
                translations/mx-codecs_fr_BE.ts \
                translations/mx-codecs_fr.ts \
                translations/mx-codecs_fy.ts \
                translations/mx-codecs_ga.ts \
                translations/mx-codecs_gd.ts \
                translations/mx-codecs_gl_ES.ts \
                translations/mx-codecs_gl.ts \
                translations/mx-codecs_gu.ts \
                translations/mx-codecs_ha.ts \
                translations/mx-codecs_haw.ts \
                translations/mx-codecs_he_IL.ts \
                translations/mx-codecs_he.ts \
                translations/mx-codecs_hi.ts \
                translations/mx-codecs_hr.ts \
                translations/mx-codecs_ht.ts \
                translations/mx-codecs_hu.ts \
                translations/mx-codecs_hye.ts \
                translations/mx-codecs_hy.ts \
                translations/mx-codecs_id.ts \
                translations/mx-codecs_ie.ts \
                translations/mx-codecs_is.ts \
                translations/mx-codecs_it.ts \
                translations/mx-codecs_ja.ts \
                translations/mx-codecs_jv.ts \
                translations/mx-codecs_kab.ts \
                translations/mx-codecs_ka.ts \
                translations/mx-codecs_kk.ts \
                translations/mx-codecs_km.ts \
                translations/mx-codecs_kn.ts \
                translations/mx-codecs_ko.ts \
                translations/mx-codecs_ku.ts \
                translations/mx-codecs_ky.ts \
                translations/mx-codecs_lb.ts \
                translations/mx-codecs_lo.ts \
                translations/mx-codecs_lt.ts \
                translations/mx-codecs_lv.ts \
                translations/mx-codecs_mg.ts \
                translations/mx-codecs_mi.ts \
                translations/mx-codecs_mk.ts \
                translations/mx-codecs_ml.ts \
                translations/mx-codecs_mn.ts \
                translations/mx-codecs_mr.ts \
                translations/mx-codecs_ms.ts \
                translations/mx-codecs_mt.ts \
                translations/mx-codecs_my.ts \
                translations/mx-codecs_nb_NO.ts \
                translations/mx-codecs_nb.ts \
                translations/mx-codecs_ne.ts \
                translations/mx-codecs_nl_BE.ts \
                translations/mx-codecs_nl.ts \
                translations/mx-codecs_nn.ts \
                translations/mx-codecs_ny.ts \
                translations/mx-codecs_oc.ts \
                translations/mx-codecs_or.ts \
                translations/mx-codecs_pa.ts \
                translations/mx-codecs_pl.ts \
                translations/mx-codecs_ps.ts \
                translations/mx-codecs_pt_BR.ts \
                translations/mx-codecs_pt.ts \
                translations/mx-codecs_ro.ts \
                translations/mx-codecs_rue.ts \
                translations/mx-codecs_ru.ts \
                translations/mx-codecs_rw.ts \
                translations/mx-codecs_sd.ts \
                translations/mx-codecs_si.ts \
                translations/mx-codecs_sk.ts \
                translations/mx-codecs_sl.ts \
                translations/mx-codecs_sm.ts \
                translations/mx-codecs_sn.ts \
                translations/mx-codecs_so.ts \
                translations/mx-codecs_sq.ts \
                translations/mx-codecs_sr.ts \
                translations/mx-codecs_st.ts \
                translations/mx-codecs_su.ts \
                translations/mx-codecs_sv.ts \
                translations/mx-codecs_sw.ts \
                translations/mx-codecs_ta.ts \
                translations/mx-codecs_te.ts \
                translations/mx-codecs_tg.ts \
                translations/mx-codecs_th.ts \
                translations/mx-codecs_tk.ts \
                translations/mx-codecs_tr.ts \
                translations/mx-codecs_tt.ts \
                translations/mx-codecs_ug.ts \
                translations/mx-codecs_uk.ts \
                translations/mx-codecs_ur.ts \
                translations/mx-codecs_uz.ts \
                translations/mx-codecs_vi.ts \
                translations/mx-codecs_xh.ts \
                translations/mx-codecs_yi.ts \
                translations/mx-codecs_yo.ts \
                translations/mx-codecs_yue_CN.ts \
                translations/mx-codecs_zh_CN.ts \
                translations/mx-codecs_zh_HK.ts \
                translations/mx-codecs_zh_TW.ts

RESOURCES += \
    images.qrc

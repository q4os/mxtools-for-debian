QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mx-datetime
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050000

SOURCES += main.cpp datetime.cpp \
    about.cpp \
    clockface.cpp
HEADERS += datetime.h \
    about.h \
    clockface.h \
    version.h
FORMS += datetime.ui

RESOURCES += \
    images.qrc

TRANSLATIONS += translations/mx-datetime_af.ts \
                translations/mx-datetime_am.ts \
                translations/mx-datetime_ar.ts \
                translations/mx-datetime_be.ts \
                translations/mx-datetime_bg.ts \
                translations/mx-datetime_bn.ts \
                translations/mx-datetime_bs_BA.ts \
                translations/mx-datetime_bs.ts \
                translations/mx-datetime_ca.ts \
                translations/mx-datetime_ceb.ts \
                translations/mx-datetime_co.ts \
                translations/mx-datetime_cs.ts \
                translations/mx-datetime_cy.ts \
                translations/mx-datetime_da.ts \
                translations/mx-datetime_de.ts \
                translations/mx-datetime_el.ts \
                translations/mx-datetime_en_GB.ts \
                translations/mx-datetime_en.ts \
                translations/mx-datetime_en_US.ts \
                translations/mx-datetime_eo.ts \
                translations/mx-datetime_es_ES.ts \
                translations/mx-datetime_es.ts \
                translations/mx-datetime_et.ts \
                translations/mx-datetime_eu.ts \
                translations/mx-datetime_fa.ts \
                translations/mx-datetime_fi_FI.ts \
                translations/mx-datetime_fil_PH.ts \
                translations/mx-datetime_fil.ts \
                translations/mx-datetime_fi.ts \
                translations/mx-datetime_fr_BE.ts \
                translations/mx-datetime_fr_FR.ts \
                translations/mx-datetime_fr.ts \
                translations/mx-datetime_fy.ts \
                translations/mx-datetime_ga.ts \
                translations/mx-datetime_gd.ts \
                translations/mx-datetime_gl_ES.ts \
                translations/mx-datetime_gl.ts \
                translations/mx-datetime_gu.ts \
                translations/mx-datetime_ha.ts \
                translations/mx-datetime_haw.ts \
                translations/mx-datetime_he_IL.ts \
                translations/mx-datetime_he.ts \
                translations/mx-datetime_hi.ts \
                translations/mx-datetime_hr.ts \
                translations/mx-datetime_ht.ts \
                translations/mx-datetime_hu.ts \
                translations/mx-datetime_hy.ts \
                translations/mx-datetime_id.ts \
                translations/mx-datetime_is.ts \
                translations/mx-datetime_it.ts \
                translations/mx-datetime_ja.ts \
                translations/mx-datetime_jv.ts \
                translations/mx-datetime_ka.ts \
                translations/mx-datetime_kk.ts \
                translations/mx-datetime_km.ts \
                translations/mx-datetime_kn.ts \
                translations/mx-datetime_ko.ts \
                translations/mx-datetime_ku.ts \
                translations/mx-datetime_ky.ts \
                translations/mx-datetime_lb.ts \
                translations/mx-datetime_lo.ts \
                translations/mx-datetime_lt.ts \
                translations/mx-datetime_lv.ts \
                translations/mx-datetime_mg.ts \
                translations/mx-datetime_mi.ts \
                translations/mx-datetime_mk.ts \
                translations/mx-datetime_ml.ts \
                translations/mx-datetime_mn.ts \
                translations/mx-datetime_mr.ts \
                translations/mx-datetime_ms.ts \
                translations/mx-datetime_mt.ts \
                translations/mx-datetime_my.ts \
                translations/mx-datetime_nb_NO.ts \
                translations/mx-datetime_nb.ts \
                translations/mx-datetime_ne.ts \
                translations/mx-datetime_nl_BE.ts \
                translations/mx-datetime_nl.ts \
                translations/mx-datetime_ny.ts \
                translations/mx-datetime_or.ts \
                translations/mx-datetime_pa.ts \
                translations/mx-datetime_pl.ts \
                translations/mx-datetime_ps.ts \
                translations/mx-datetime_pt_BR.ts \
                translations/mx-datetime_pt.ts \
                translations/mx-datetime_ro.ts \
                translations/mx-datetime_rue.ts \
                translations/mx-datetime_ru_RU.ts \
                translations/mx-datetime_ru.ts \
                translations/mx-datetime_rw.ts \
                translations/mx-datetime_sd.ts \
                translations/mx-datetime_si.ts \
                translations/mx-datetime_sk.ts \
                translations/mx-datetime_sl.ts \
                translations/mx-datetime_sm.ts \
                translations/mx-datetime_sn.ts \
                translations/mx-datetime_so.ts \
                translations/mx-datetime_sq.ts \
                translations/mx-datetime_sr.ts \
                translations/mx-datetime_st.ts \
                translations/mx-datetime_su.ts \
                translations/mx-datetime_sv.ts \
                translations/mx-datetime_sw.ts \
                translations/mx-datetime_ta.ts \
                translations/mx-datetime_te.ts \
                translations/mx-datetime_tg.ts \
                translations/mx-datetime_th.ts \
                translations/mx-datetime_tk.ts \
                translations/mx-datetime_tr.ts \
                translations/mx-datetime_tt.ts \
                translations/mx-datetime_ug.ts \
                translations/mx-datetime_uk.ts \
                translations/mx-datetime_ur.ts \
                translations/mx-datetime_uz.ts \
                translations/mx-datetime_vi.ts \
                translations/mx-datetime_xh.ts \
                translations/mx-datetime_yi.ts \
                translations/mx-datetime_yo.ts \
                translations/mx-datetime_yue_CN.ts \
                translations/mx-datetime_zh_CN.ts \
                translations/mx-datetime_zh_HK.ts \
                translations/mx-datetime_zh_TW.ts
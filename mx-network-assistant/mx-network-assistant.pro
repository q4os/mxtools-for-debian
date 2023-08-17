QT       += widgets
CONFIG   += release warn_on c++1z

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = mx-network-assistant

TRANSLATIONS += translations/mx-network-assistant_af.ts \
                translations/mx-network-assistant_am.ts \
                translations/mx-network-assistant_ar.ts \
                translations/mx-network-assistant_ast.ts \
                translations/mx-network-assistant_be.ts \
                translations/mx-network-assistant_bg.ts \
                translations/mx-network-assistant_bn.ts \
                translations/mx-network-assistant_bs_BA.ts \
                translations/mx-network-assistant_bs.ts \
                translations/mx-network-assistant_ca.ts \
                translations/mx-network-assistant_ceb.ts \
                translations/mx-network-assistant_co.ts \
                translations/mx-network-assistant_cs.ts \
                translations/mx-network-assistant_cy.ts \
                translations/mx-network-assistant_da.ts \
                translations/mx-network-assistant_de.ts \
                translations/mx-network-assistant_el.ts \
                translations/mx-network-assistant_en_GB.ts \
                translations/mx-network-assistant_en.ts \
                translations/mx-network-assistant_en_US.ts \
                translations/mx-network-assistant_eo.ts \
                translations/mx-network-assistant_es_ES.ts \
                translations/mx-network-assistant_es.ts \
                translations/mx-network-assistant_et.ts \
                translations/mx-network-assistant_eu.ts \
                translations/mx-network-assistant_fa.ts \
                translations/mx-network-assistant_fi_FI.ts \
                translations/mx-network-assistant_fil_PH.ts \
                translations/mx-network-assistant_fil.ts \
                translations/mx-network-assistant_fi.ts \
                translations/mx-network-assistant_fr_BE.ts \
                translations/mx-network-assistant_fr.ts \
                translations/mx-network-assistant_fy.ts \
                translations/mx-network-assistant_ga.ts \
                translations/mx-network-assistant_gd.ts \
                translations/mx-network-assistant_gl_ES.ts \
                translations/mx-network-assistant_gl.ts \
                translations/mx-network-assistant_gu.ts \
                translations/mx-network-assistant_ha.ts \
                translations/mx-network-assistant_haw.ts \
                translations/mx-network-assistant_he_IL.ts \
                translations/mx-network-assistant_he.ts \
                translations/mx-network-assistant_hi.ts \
                translations/mx-network-assistant_hr.ts \
                translations/mx-network-assistant_ht.ts \
                translations/mx-network-assistant_hu.ts \
                translations/mx-network-assistant_hy_AM.ts \
                translations/mx-network-assistant_hye.ts \
                translations/mx-network-assistant_hy.ts \
                translations/mx-network-assistant_id.ts \
                translations/mx-network-assistant_ie.ts \
                translations/mx-network-assistant_is.ts \
                translations/mx-network-assistant_it.ts \
                translations/mx-network-assistant_ja.ts \
                translations/mx-network-assistant_jv.ts \
                translations/mx-network-assistant_kab.ts \
                translations/mx-network-assistant_ka.ts \
                translations/mx-network-assistant_kk.ts \
                translations/mx-network-assistant_km.ts \
                translations/mx-network-assistant_kn.ts \
                translations/mx-network-assistant_ko.ts \
                translations/mx-network-assistant_ku.ts \
                translations/mx-network-assistant_ky.ts \
                translations/mx-network-assistant_lb.ts \
                translations/mx-network-assistant_lo.ts \
                translations/mx-network-assistant_lt.ts \
                translations/mx-network-assistant_lv.ts \
                translations/mx-network-assistant_mg.ts \
                translations/mx-network-assistant_mi.ts \
                translations/mx-network-assistant_mk.ts \
                translations/mx-network-assistant_ml.ts \
                translations/mx-network-assistant_mn.ts \
                translations/mx-network-assistant_mr.ts \
                translations/mx-network-assistant_ms.ts \
                translations/mx-network-assistant_mt.ts \
                translations/mx-network-assistant_my.ts \
                translations/mx-network-assistant_nb_NO.ts \
                translations/mx-network-assistant_nb.ts \
                translations/mx-network-assistant_ne.ts \
                translations/mx-network-assistant_nl_BE.ts \
                translations/mx-network-assistant_nl.ts \
                translations/mx-network-assistant_nn.ts \
                translations/mx-network-assistant_ny.ts \
                translations/mx-network-assistant_oc.ts \
                translations/mx-network-assistant_or.ts \
                translations/mx-network-assistant_pa.ts \
                translations/mx-network-assistant_pl.ts \
                translations/mx-network-assistant_ps.ts \
                translations/mx-network-assistant_pt_BR.ts \
                translations/mx-network-assistant_pt.ts \
                translations/mx-network-assistant_ro.ts \
                translations/mx-network-assistant_rue.ts \
                translations/mx-network-assistant_ru_RU.ts \
                translations/mx-network-assistant_ru.ts \
                translations/mx-network-assistant_rw.ts \
                translations/mx-network-assistant_sd.ts \
                translations/mx-network-assistant_si.ts \
                translations/mx-network-assistant_sk.ts \
                translations/mx-network-assistant_sl.ts \
                translations/mx-network-assistant_sm.ts \
                translations/mx-network-assistant_sn.ts \
                translations/mx-network-assistant_so.ts \
                translations/mx-network-assistant_sq.ts \
                translations/mx-network-assistant_sr.ts \
                translations/mx-network-assistant_st.ts \
                translations/mx-network-assistant_su.ts \
                translations/mx-network-assistant_sv.ts \
                translations/mx-network-assistant_sw.ts \
                translations/mx-network-assistant_ta.ts \
                translations/mx-network-assistant_te.ts \
                translations/mx-network-assistant_tg.ts \
                translations/mx-network-assistant_th.ts \
                translations/mx-network-assistant_tk.ts \
                translations/mx-network-assistant_tr.ts \
                translations/mx-network-assistant_tt.ts \
                translations/mx-network-assistant_ug.ts \
                translations/mx-network-assistant_uk.ts \
                translations/mx-network-assistant_ur.ts \
                translations/mx-network-assistant_uz.ts \
                translations/mx-network-assistant_vi.ts \
                translations/mx-network-assistant_xh.ts \
                translations/mx-network-assistant_yi.ts \
                translations/mx-network-assistant_yo.ts \
                translations/mx-network-assistant_yue_CN.ts \
                translations/mx-network-assistant_zh_CN.ts \
                translations/mx-network-assistant_zh_HK.ts \
                translations/mx-network-assistant_zh_TW.ts

FORMS += \
    mainwindow.ui
HEADERS += \
    mainwindow.h \
    version.h \
    about.h \
    cmd.h
SOURCES += main.cpp \
    mainwindow.cpp \
    about.cpp \
    cmd.cpp

RESOURCES += \
    images.qrc


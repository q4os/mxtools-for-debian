QT       += core gui widgets
CONFIG   += c++1z

TEMPLATE = app
TARGET = job-scheduler

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += Clib.h \
           CronModel.h \
           Crontab.h \
           CronTime.h \
           CronView.h \
           Execute.h \
           ExecuteList.h \
           ExecuteModel.h \
           ExecuteView.h \
           MainWindow.h \
           SaveDialog.h \
           TCommandEdit.h \
           TimeDialog.h \
           VariableEdit.h \
           VariableModel.h \
           VariableView.h \
           Version.h \
           about.h
SOURCES += CronModel.cpp \
           Crontab.cpp \
           CronTime.cpp \
           CronView.cpp \
           ExecuteList.cpp \
           ExecuteModel.cpp \
           ExecuteView.cpp \
           about.cpp \
           main.cpp \
           MainWindow.cpp \
           SaveDialog.cpp \
           TCommandEdit.cpp \
           TimeDialog.cpp \
           VariableEdit.cpp \
           VariableModel.cpp \
           VariableView.cpp
RESOURCES += application.qrc

TRANSLATIONS += translations/job-scheduler_af.ts \
                translations/job-scheduler_am.ts \
                translations/job-scheduler_ar.ts \
                translations/job-scheduler_ast.ts \
                translations/job-scheduler_be.ts \
                translations/job-scheduler_bg.ts \
                translations/job-scheduler_bn.ts \
                translations/job-scheduler_bs_BA.ts \
                translations/job-scheduler_bs.ts \
                translations/job-scheduler_ca.ts \
                translations/job-scheduler_ceb.ts \
                translations/job-scheduler_co.ts \
                translations/job-scheduler_cs.ts \
                translations/job-scheduler_cy.ts \
                translations/job-scheduler_da.ts \
                translations/job-scheduler_de.ts \
                translations/job-scheduler_el.ts \
                translations/job-scheduler_en_GB.ts \
                translations/job-scheduler_en.ts \
                translations/job-scheduler_en_US.ts \
                translations/job-scheduler_eo.ts \
                translations/job-scheduler_es_ES.ts \
                translations/job-scheduler_es.ts \
                translations/job-scheduler_et.ts \
                translations/job-scheduler_eu.ts \
                translations/job-scheduler_fa.ts \
                translations/job-scheduler_fi_FI.ts \
                translations/job-scheduler_fil_PH.ts \
                translations/job-scheduler_fil.ts \
                translations/job-scheduler_fi.ts \
                translations/job-scheduler_fr_BE.ts \
                translations/job-scheduler_fr.ts \
                translations/job-scheduler_fy.ts \
                translations/job-scheduler_ga.ts \
                translations/job-scheduler_gd.ts \
                translations/job-scheduler_gl_ES.ts \
                translations/job-scheduler_gl.ts \
                translations/job-scheduler_gu.ts \
                translations/job-scheduler_ha.ts \
                translations/job-scheduler_haw.ts \
                translations/job-scheduler_he_IL.ts \
                translations/job-scheduler_he.ts \
                translations/job-scheduler_hi.ts \
                translations/job-scheduler_hr.ts \
                translations/job-scheduler_ht.ts \
                translations/job-scheduler_hu.ts \
                translations/job-scheduler_hy_AM.ts \
                translations/job-scheduler_hye.ts \
                translations/job-scheduler_hy.ts \
                translations/job-scheduler_id.ts \
                translations/job-scheduler_ie.ts \
                translations/job-scheduler_is.ts \
                translations/job-scheduler_it.ts \
                translations/job-scheduler_ja.ts \
                translations/job-scheduler_jv.ts \
                translations/job-scheduler_kab.ts \
                translations/job-scheduler_ka.ts \
                translations/job-scheduler_kk.ts \
                translations/job-scheduler_km.ts \
                translations/job-scheduler_kn.ts \
                translations/job-scheduler_ko.ts \
                translations/job-scheduler_ku.ts \
                translations/job-scheduler_ky.ts \
                translations/job-scheduler_lb.ts \
                translations/job-scheduler_lo.ts \
                translations/job-scheduler_lt.ts \
                translations/job-scheduler_lv.ts \
                translations/job-scheduler_mg.ts \
                translations/job-scheduler_mi.ts \
                translations/job-scheduler_mk.ts \
                translations/job-scheduler_ml.ts \
                translations/job-scheduler_mn.ts \
                translations/job-scheduler_mr.ts \
                translations/job-scheduler_ms.ts \
                translations/job-scheduler_mt.ts \
                translations/job-scheduler_my.ts \
                translations/job-scheduler_nb_NO.ts \
                translations/job-scheduler_nb.ts \
                translations/job-scheduler_ne.ts \
                translations/job-scheduler_nl_BE.ts \
                translations/job-scheduler_nl.ts \
                translations/job-scheduler_nn.ts \
                translations/job-scheduler_ny.ts \
                translations/job-scheduler_oc.ts \
                translations/job-scheduler_or.ts \
                translations/job-scheduler_pa.ts \
                translations/job-scheduler_pl.ts \
                translations/job-scheduler_ps.ts \
                translations/job-scheduler_pt_BR.ts \
                translations/job-scheduler_pt.ts \
                translations/job-scheduler_ro.ts \
                translations/job-scheduler_rue.ts \
                translations/job-scheduler_ru_RU.ts \
                translations/job-scheduler_ru.ts \
                translations/job-scheduler_rw.ts \
                translations/job-scheduler_sd.ts \
                translations/job-scheduler_si.ts \
                translations/job-scheduler_sk.ts \
                translations/job-scheduler_sl.ts \
                translations/job-scheduler_sm.ts \
                translations/job-scheduler_sn.ts \
                translations/job-scheduler_so.ts \
                translations/job-scheduler_sq.ts \
                translations/job-scheduler_sr.ts \
                translations/job-scheduler_st.ts \
                translations/job-scheduler_su.ts \
                translations/job-scheduler_sv.ts \
                translations/job-scheduler_sw.ts \
                translations/job-scheduler_ta.ts \
                translations/job-scheduler_te.ts \
                translations/job-scheduler_tg.ts \
                translations/job-scheduler_th.ts \
                translations/job-scheduler_tk.ts \
                translations/job-scheduler_tr.ts \
                translations/job-scheduler_tt.ts \
                translations/job-scheduler_ug.ts \
                translations/job-scheduler_uk.ts \
                translations/job-scheduler_ur.ts \
                translations/job-scheduler_uz.ts \
                translations/job-scheduler_vi.ts \
                translations/job-scheduler_xh.ts \
                translations/job-scheduler_yi.ts \
                translations/job-scheduler_yo.ts \
                translations/job-scheduler_yue_CN.ts \
                translations/job-scheduler_zh_CN.ts \
                translations/job-scheduler_zh_HK.ts \
                translations/job-scheduler_zh_TW.ts

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

TRANSLATIONS += translations/job-scheduler_am.ts \
                translations/job-scheduler_ar.ts \
                translations/job-scheduler_bg.ts \
                translations/job-scheduler_bn.ts \
                translations/job-scheduler_ca.ts \
                translations/job-scheduler_cs.ts \
                translations/job-scheduler_da.ts \
                translations/job-scheduler_de.ts \
                translations/job-scheduler_el.ts \
                translations/job-scheduler_en.ts \
                translations/job-scheduler_es.ts \
                translations/job-scheduler_et.ts \
                translations/job-scheduler_eu.ts \
                translations/job-scheduler_fa.ts \
                translations/job-scheduler_fi.ts \
                translations/job-scheduler_fil_PH.ts \
                translations/job-scheduler_fr.ts \
                translations/job-scheduler_fr_BE.ts \
                translations/job-scheduler_he_IL.ts \
                translations/job-scheduler_hi.ts \
                translations/job-scheduler_hr.ts \
                translations/job-scheduler_hu.ts \
                translations/job-scheduler_id.ts \
                translations/job-scheduler_is.ts \
                translations/job-scheduler_it.ts \
                translations/job-scheduler_ja.ts \
                translations/job-scheduler_kk.ts \
                translations/job-scheduler_ko.ts \
                translations/job-scheduler_lt.ts \
                translations/job-scheduler_mk.ts \
                translations/job-scheduler_mr.ts \
                translations/job-scheduler_nb.ts \
                translations/job-scheduler_nl.ts \
                translations/job-scheduler_pl.ts \
                translations/job-scheduler_pt.ts \
                translations/job-scheduler_pt_BR.ts \
                translations/job-scheduler_ro.ts \
                translations/job-scheduler_ru.ts \
                translations/job-scheduler_sk.ts \
                translations/job-scheduler_sl.ts \
                translations/job-scheduler_sq.ts \
                translations/job-scheduler_sr.ts \
                translations/job-scheduler_sv.ts \
                translations/job-scheduler_tr.ts \
                translations/job-scheduler_uk.ts \
                translations/job-scheduler_vi.ts \
                translations/job-scheduler_zh_CN.ts \
                translations/job-scheduler_zh_TW.ts

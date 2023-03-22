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

TRANSLATIONS += translations/mx-datetime_am.ts \
                translations/mx-datetime_ar.ts \
                translations/mx-datetime_bg.ts \
                translations/mx-datetime_ca.ts \
                translations/mx-datetime_cs.ts \
                translations/mx-datetime_da.ts \
                translations/mx-datetime_de.ts \
                translations/mx-datetime_el.ts \
                translations/mx-datetime_en.ts \
                translations/mx-datetime_es.ts \
                translations/mx-datetime_et.ts \
                translations/mx-datetime_eu.ts \
                translations/mx-datetime_fa.ts \
                translations/mx-datetime_fi.ts \
                translations/mx-datetime_fr.ts \
                translations/mx-datetime_fr_BE.ts \
                translations/mx-datetime_he_IL.ts \
                translations/mx-datetime_hi.ts \
                translations/mx-datetime_hr.ts \
                translations/mx-datetime_hu.ts \
                translations/mx-datetime_id.ts \
                translations/mx-datetime_is.ts \
                translations/mx-datetime_it.ts \
                translations/mx-datetime_ja.ts \
                translations/mx-datetime_kk.ts \
                translations/mx-datetime_ko.ts \
                translations/mx-datetime_lt.ts \
                translations/mx-datetime_mk.ts \
                translations/mx-datetime_mr.ts \
                translations/mx-datetime_nb.ts \
                translations/mx-datetime_nl.ts \
                translations/mx-datetime_pl.ts \
                translations/mx-datetime_pt.ts \
                translations/mx-datetime_pt_BR.ts \
                translations/mx-datetime_ro.ts \
                translations/mx-datetime_ru.ts \
                translations/mx-datetime_sk.ts \
                translations/mx-datetime_sl.ts \
                translations/mx-datetime_sq.ts \
                translations/mx-datetime_sr.ts \
                translations/mx-datetime_sv.ts \
                translations/mx-datetime_tr.ts \
                translations/mx-datetime_uk.ts \
                translations/mx-datetime_zh_CN.ts \
                translations/mx-datetime_zh_TW.ts


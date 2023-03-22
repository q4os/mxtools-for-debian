#-------------------------------------------------
#
# Project created by QtCreator 2019-04-27T11:02:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bash-config
TEMPLATE = app
CONFIG += c++11 Wall

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        window.cpp \
    buffer.cpp \
    searcher.cpp \
    tab.cpp \
    prompttab.cpp \
    aliastab.cpp \
    aliasstream.cpp \
    global.cpp \
    othertab.cpp \
    datetimeformatting.cpp \
    fuzzybashstream.cpp \
    dateformatpreview.cpp

HEADERS  += window.h \
    buffer.h \
    searcher.h \
    tab.h \
    global.h \
    prompttab.h \
    aliastab.h \
    aliasstream.h \
    othertab.h \
    datetimeformatting.h \
    fuzzybashstream.h \
    dateformatpreview.h

FORMS    += \
    aliastab.ui \
    window_fix.ui \
    prompttab_fix.ui \
    othertab.ui

TRANSLATIONS += translations/bash-config_am.ts \
                translations/bash-config_ar.ts \
                translations/bash-config_bg.ts \
                translations/bash-config_ca.ts \
                translations/bash-config_cs.ts \
                translations/bash-config_da.ts \
                translations/bash-config_de.ts \
                translations/bash-config_el.ts \
                translations/bash-config_es.ts \
                translations/bash-config_et.ts \
                translations/bash-config_eu.ts \
                translations/bash-config_fa.ts \
                translations/bash-config_fi.ts \
                translations/bash-config_fr.ts \
                translations/bash-config_he_IL.ts \
                translations/bash-config_hi.ts \
                translations/bash-config_hr.ts \
                translations/bash-config_hu.ts \
                translations/bash-config_id.ts \
                translations/bash-config_is.ts \
                translations/bash-config_it.ts \
                translations/bash-config_ja.ts \
                translations/bash-config_kk.ts \
                translations/bash-config_ko.ts \
                translations/bash-config_lt.ts \
                translations/bash-config_mk.ts \
                translations/bash-config_mr.ts \
                translations/bash-config_nb.ts \
                translations/bash-config_nl.ts \
                translations/bash-config_pl.ts \
                translations/bash-config_pt.ts \
                translations/bash-config_pt_BR.ts \
                translations/bash-config_ro.ts \
                translations/bash-config_ru.ts \
                translations/bash-config_sk.ts \
                translations/bash-config_sl.ts \
                translations/bash-config_sq.ts \
                translations/bash-config_sr.ts \
                translations/bash-config_sv.ts \
                translations/bash-config_tr.ts \
                translations/bash-config_uk.ts \
                translations/bash-config_zh_CN.ts \
                translations/bash-config_zh_TW.ts


RESOURCES += \
    resource.qrc

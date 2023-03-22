QT       += core gui widgets
CONFIG   += c++1z

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = mx-network-assistant

TRANSLATIONS += translations/mx-network-assistant_am.ts \
                translations/mx-network-assistant_ar.ts \
                translations/mx-network-assistant_bg.ts \
                translations/mx-network-assistant_ca.ts \
                translations/mx-network-assistant_cs.ts \
                translations/mx-network-assistant_da.ts \
                translations/mx-network-assistant_de.ts \
                translations/mx-network-assistant_el.ts \
                translations/mx-network-assistant_en.ts \
                translations/mx-network-assistant_es.ts \
                translations/mx-network-assistant_et.ts \
                translations/mx-network-assistant_eu.ts \
                translations/mx-network-assistant_fa.ts \
                translations/mx-network-assistant_fi.ts \
                translations/mx-network-assistant_fr.ts \
                translations/mx-network-assistant_fr_BE.ts \
                translations/mx-network-assistant_he_IL.ts \
                translations/mx-network-assistant_hi.ts \
                translations/mx-network-assistant_hr.ts \
                translations/mx-network-assistant_hu.ts \
                translations/mx-network-assistant_id.ts \
                translations/mx-network-assistant_is.ts \
                translations/mx-network-assistant_it.ts \
                translations/mx-network-assistant_ja.ts \
                translations/mx-network-assistant_kk.ts \
                translations/mx-network-assistant_ko.ts \
                translations/mx-network-assistant_lt.ts \
                translations/mx-network-assistant_mk.ts \
                translations/mx-network-assistant_mr.ts \
                translations/mx-network-assistant_nb.ts \
                translations/mx-network-assistant_nl.ts \
                translations/mx-network-assistant_pl.ts \
                translations/mx-network-assistant_pt.ts \
                translations/mx-network-assistant_pt_BR.ts \
                translations/mx-network-assistant_ro.ts \
                translations/mx-network-assistant_ru.ts \
                translations/mx-network-assistant_sk.ts \
                translations/mx-network-assistant_sl.ts \
                translations/mx-network-assistant_sq.ts \
                translations/mx-network-assistant_sr.ts \
                translations/mx-network-assistant_sv.ts \
                translations/mx-network-assistant_tr.ts \
                translations/mx-network-assistant_uk.ts \
                translations/mx-network-assistant_zh_CN.ts \
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
CONFIG += release warn_on thread qt c++11

RESOURCES += \
    images.qrc


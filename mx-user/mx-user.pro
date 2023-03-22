QT       += widgets
CONFIG   += c++1z

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TEMPLATE = app
TARGET = mx-user

TRANSLATIONS += translations/mx-user_af.ts \
    translations/mx-user_am.ts \
    translations/mx-user_ar.ts \
    translations/mx-user_be.ts \
    translations/mx-user_bg.ts \
    translations/mx-user_bn.ts \
    translations/mx-user_ca.ts \
    translations/mx-user_cs.ts \
    translations/mx-user_da.ts \
    translations/mx-user_de.ts \
    translations/mx-user_el.ts \
    translations/mx-user_en_GB.ts \
    translations/mx-user_en.ts \
    translations/mx-user_eo.ts \
    translations/mx-user_es_ES.ts \
    translations/mx-user_es.ts \
    translations/mx-user_et.ts \
    translations/mx-user_eu.ts \
    translations/mx-user_fa.ts \
    translations/mx-user_fil_PH.ts \
    translations/mx-user_fi.ts \
    translations/mx-user_fr_BE.ts \
    translations/mx-user_fr_FR.ts \
    translations/mx-user_fr.ts \
    translations/mx-user_gl_ES.ts \
    translations/mx-user_gu.ts \
    translations/mx-user_he_IL.ts \
    translations/mx-user_he.ts \
    translations/mx-user_hi.ts \
    translations/mx-user_hr.ts \
    translations/mx-user_hu.ts \
    translations/mx-user_id.ts \
    translations/mx-user_is.ts \
    translations/mx-user_it.ts \
    translations/mx-user_ja.ts \
    translations/mx-user_kk.ts \
    translations/mx-user_ko.ts \
    translations/mx-user_ku.ts \
    translations/mx-user_lt.ts \
    translations/mx-user_mk.ts \
    translations/mx-user_mr.ts \
    translations/mx-user_nb_NO.ts \
    translations/mx-user_nb.ts \
    translations/mx-user_nl_BE.ts \
    translations/mx-user_nl.ts \
    translations/mx-user_or.ts \
    translations/mx-user_pl.ts \
    translations/mx-user_pt_BR.ts \
    translations/mx-user_pt.ts \
    translations/mx-user_ro.ts \
    translations/mx-user_rue.ts \
    translations/mx-user_ru.ts \
    translations/mx-user_sk.ts \
    translations/mx-user_sl.ts \
    translations/mx-user_so.ts \
    translations/mx-user_sq.ts \
    translations/mx-user_sr.ts \
    translations/mx-user_sv.ts \
    translations/mx-user_th.ts \
    translations/mx-user_tr.ts \
    translations/mx-user_uk.ts \
    translations/mx-user_vi.ts \
    translations/mx-user_zh_CN.ts \
    translations/mx-user_zh_HK.ts \
    translations/mx-user_zh_TW.ts \

FORMS += \
    mainwindow.ui
HEADERS += \
    mainwindow.h \
    version.h \
    cmd.h \
    about.h
SOURCES += main.cpp \
    mainwindow.cpp \
    cmd.cpp \
    about.cpp
LIBS += -lcrypt
CONFIG += release warn_on thread qt

RESOURCES += \
    images.qrc

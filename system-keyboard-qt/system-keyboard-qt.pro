#-------------------------------------------------
#
# Project created by QtCreator 2020-11-25T16:14:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = system-keyboard-qt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        window.cpp \
    keyboardlayouts.cpp \
    selectlayoutdialog.cpp \
    posixconfigparser.cpp \
    translation.cpp \
    filterablecombobox.cpp

HEADERS += \
        window.h \
    keyboardlayouts.h \
    selectlayoutdialog.h \
    translation.h \
    posixconfigparser.h \
    filterablecombobox.h

FORMS += \
        window.ui \
    selectlayoutdialog.ui

TRANSLATIONS += translations/system-keyboard-qt_am.ts \
                translations/system-keyboard-qt_ar.ts \
                translations/system-keyboard-qt_bg.ts \
                translations/system-keyboard-qt_bn.ts \
                translations/system-keyboard-qt_ca.ts \
                translations/system-keyboard-qt_cs.ts \
                translations/system-keyboard-qt_da.ts \
                translations/system-keyboard-qt_de.ts \
                translations/system-keyboard-qt_el.ts \
                translations/system-keyboard-qt_es.ts \
                translations/system-keyboard-qt_et.ts \
                translations/system-keyboard-qt_eu.ts \
                translations/system-keyboard-qt_fa.ts \
                translations/system-keyboard-qt_fi.ts \
                translations/system-keyboard-qt_fil_PH.ts \
                translations/system-keyboard-qt_fr.ts \
                translations/system-keyboard-qt_he_IL.ts \
                translations/system-keyboard-qt_hi.ts \
                translations/system-keyboard-qt_hr.ts \
                translations/system-keyboard-qt_hu.ts \
                translations/system-keyboard-qt_id.ts \
                translations/system-keyboard-qt_is.ts \
                translations/system-keyboard-qt_it.ts \
                translations/system-keyboard-qt_ja.ts \
                translations/system-keyboard-qt_kk.ts \
                translations/system-keyboard-qt_ko.ts \
                translations/system-keyboard-qt_lt.ts \
                translations/system-keyboard-qt_mk.ts \
                translations/system-keyboard-qt_mr.ts \
                translations/system-keyboard-qt_nb.ts \
                translations/system-keyboard-qt_nl.ts \
                translations/system-keyboard-qt_pl.ts \
                translations/system-keyboard-qt_pt.ts \
                translations/system-keyboard-qt_pt_BR.ts \
                translations/system-keyboard-qt_ro.ts \
                translations/system-keyboard-qt_ru.ts \
                translations/system-keyboard-qt_sk.ts \
                translations/system-keyboard-qt_sl.ts \
                translations/system-keyboard-qt_sq.ts \
                translations/system-keyboard-qt_sr.ts \
                translations/system-keyboard-qt_sv.ts \
                translations/system-keyboard-qt_tr.ts \
                translations/system-keyboard-qt_uk.ts \
                translations/system-keyboard-qt_vi.ts \
                translations/system-keyboard-qt_zh_CN.ts \
                translations/system-keyboard-qt_zh_TW.ts

## Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    system-keyboard-qt-resource.qrc

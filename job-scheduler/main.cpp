/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "Clib.h"
#include "MainWindow.h"
#include <unistd.h>

const extern QString starting_home = qEnvironmentVariable("HOME");

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(application);

    if (getuid() == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qputenv("HOME", "/root");
    } else { // when switching between modes we need to reset the HOME and XDG_RUNTME_DIR
        qputenv("XDG_RUNTIME_DIR", "/run/user/" + QString::number(getuid()).toUtf8());
        qputenv("HOME", Clib::uHome().toUtf8());
    }
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));
    QApplication::setOrganizationName(QStringLiteral("MX-Linux"));

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), QStringLiteral("qt"), QStringLiteral("_"),
                    QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale::system().name(),
                     "/usr/share/" + QApplication::applicationName() + "/locale")) {
        QApplication::installTranslator(&appTran);
    }

    MainWindow window;
    window.show();
    return QApplication::exec();
}

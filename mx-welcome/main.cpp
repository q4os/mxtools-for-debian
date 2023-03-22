/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Adrian
 *          Paul David Callahan
 *          Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-welcome.
 *
 * mx-welcome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-welcome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-welcome.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "mainwindow.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(app.applicationName()));
    app.setOrganizationName(QStringLiteral("MX-Linux"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("This tool displays a welcome screen with two tabs."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"a", "about"}, QObject::tr("Start with About tab selected. "
                      "The About tab provides basic information about the current "
                      "MX Linux version, the user's hardware, and access to a full system report.")});
    parser.addOption({{"t", "test"}, QObject::tr("Run a test mode.")});
    parser.process(app);

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), QStringLiteral("qt"), QStringLiteral("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTran);

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtBaseTran);

    QTranslator appTran;
    if (appTran.load(app.applicationName() + "_" + QLocale::system().name(), "/usr/share/" + app.applicationName() + "/locale"))
        app.installTranslator(&appTran);

    if (getuid() != 0) {
        MainWindow w(parser);
        w.show();
        return app.exec();
    } else {
        QApplication::beep();
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("You must run this program as normal user."));
        return EXIT_FAILURE;
    }
}

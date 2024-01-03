/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian
 *          Dolphin_Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-packageinstaller.
 *
 * mx-packageinstaller is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-packageinstaller is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-packageinstaller.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "lockfile.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDate>
#include <QDebug>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "log.h"
#include "mainwindow.h"
#include "version.h"

#include <unistd.h>

extern const QString starting_home = qEnvironmentVariable("HOME");

int main(int argc, char *argv[])
{
    if (getuid() == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qunsetenv("SESSION_MANAGER");
    }
    QApplication app(argc, argv);
    if (getuid() == 0) {
        qputenv("HOME", "/root");
    }

    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));
    QApplication::setOrganizationName(QStringLiteral("MX-Linux"));
    QApplication::setApplicationVersion(VERSION);

    QTranslator qtTran;
    if (qtTran.load("qt_" + QLocale().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale().name(),
                     "/usr/share/" + QApplication::applicationName() + "/locale")) {
        QApplication::installTranslator(&appTran);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr("MX Package Installer is a tool used for managing packages on MX Linux\n\
    - installs popular programs from different sources\n\
    - installs programs from the MX Test repo\n\
    - installs programs from Debian Backports repo\n\
    - installs flatpaks"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{QStringLiteral("s"), QStringLiteral("skip-online-check")},
                      QObject::tr("Skip online check if it falsely reports lack of internet access.")});
    parser.process(app);

    // Root guard
    QFile loginUidFile {"/proc/self/loginuid"};
    if (loginUidFile.open(QIODevice::ReadOnly)) {
        QString loginUid = QString(loginUidFile.readAll()).trimmed();
        loginUidFile.close();
        if (loginUid == "0") {
            QMessageBox::critical(
                nullptr, QObject::tr("Error"),
                QObject::tr(
                    "You seem to be logged in as root, please log out and log in as normal user to use this program."));
            exit(EXIT_FAILURE);
        }
    }
    if (getuid() != 0) {
        if (!QFile::exists("/usr/bin/pkexec") && !QFile::exists("/usr/bin/gksu")) {
            QMessageBox::critical(nullptr, QObject::tr("Error"),
                                  QObject::tr("You must run this program with admin access."));
            exit(EXIT_FAILURE);
        }
    }
    Log startLog;
    MainWindow w(parser);
    w.show();
    return QApplication::exec();
}

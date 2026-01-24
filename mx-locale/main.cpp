/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2024 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "common.h"
#include "mainwindow.h"

#include <unistd.h>

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

int main(int argc, char *argv[])
{
    // Set Qt platform to XCB (X11) if not already set and we're in X11 environment
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        if (!qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
            qputenv("QT_QPA_PLATFORM", "xcb");
        }
    }

    QApplication a(argc, argv);

    if (a.arguments().contains("--version") || a.arguments().contains("-v")) {
        qDebug() << "Version:" << VERSION;
        return EXIT_SUCCESS;
    }

    QApplication::setWindowIcon(QIcon::fromTheme("preferences-desktop-locale"));
    QApplication::setOrganizationName("MX-Linux");
    QApplication::setApplicationVersion(VERSION);

    QTranslator qtTran;
    if (qtTran.load("qt_" + QLocale().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    QTranslator appTran;
    const QString localePath = QDir(Paths::usrShare).filePath(QApplication::applicationName() + "/locale");
    if (appTran.load(QApplication::applicationName() + "_" + QLocale().name(), localePath)) {
        QApplication::installTranslator(&appTran);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("MX Locale is a tool used for managing locale settings in MX Linux"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"l", "only-lang"}, QObject::tr("Show only language selection tab.")});
    parser.addOption({{"f", "full-categories"}, QObject::tr("Show all locale categories.")});
    parser.process(a);

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

    qDebug() << "Program Version:" << VERSION;

    MainWindow w(parser);
    w.show();
    return a.exec();
}

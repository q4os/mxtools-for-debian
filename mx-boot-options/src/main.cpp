/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian, Dolphin Oracle
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
#include <QDebug>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QProcess>
#include <QTranslator>

#include "common.h"
#include "mainwindow.h"

#include <unistd.h>

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

int main(int argc, char *argv[])
{
    auto uid = getuid();
    if (uid == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qunsetenv("SESSION_MANAGER");
    }
    // Set Qt platform to XCB (X11) if not already set and we're in X11 environment
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        if (!qEnvironmentVariableIsEmpty("DISPLAY") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
            qputenv("QT_QPA_PLATFORM", "xcb");
        }
    }

    QApplication app(argc, argv);
    if (uid == 0) {
        qputenv("HOME", "/root");
    }

    QApplication::setApplicationVersion(VERSION);
    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("Program for selecting common start-up choices"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    QTranslator qtTran;
    if (qtTran.load("qt_" + QLocale().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale().name(),
                     "/usr/share/" + QApplication::applicationName() + "/locale")) {
        QApplication::installTranslator(&appTran);
    }

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
            QCoreApplication::exit(EXIT_FAILURE);
        }
    }

    // Early authentication test to fill the credential cache
    if (!Cmd().procAsRoot("true", {}, nullptr, nullptr, QuietMode::Yes)) {
        qDebug().noquote() << "Error executing command as another user: Request dismissed or not authorized";
        exit(EXIT_FAILURE);
    }

    MainWindow mainWindow;
    mainWindow.show();
    return QApplication::exec();
}

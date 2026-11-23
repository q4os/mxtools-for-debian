/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
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

#define QT_USE_QSTRINGBUILDER
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QSharedPointer>
#include <QTranslator>

#include "cmd.h"
#include "common.h"
#include "mainwindow.h"

#include <unistd.h>

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

int main(int argc, char *argv[])
{
    const QByteArray option = argc == 2 ? QByteArray(argv[1]) : QByteArray();
    const bool informational = option == "-h" || option == "--help" || option == "--help-all"
                               || option == "-v" || option == "--version";
    QSharedPointer<QCoreApplication> app;
    if (informational) {
        app = QSharedPointer<QCoreApplication>::create(argc, argv);
    } else {
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM") && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
            if (qEnvironmentVariableIsEmpty("DISPLAY")) {
                qWarning("mx-service-manager: no display available (DISPLAY and WAYLAND_DISPLAY are both unset); "
                         "a graphical session is required to run this program.");
                return EXIT_FAILURE;
            }
            qputenv("QT_QPA_PLATFORM", "xcb");
        }
        if (getuid() == 0) {
            qputenv("XDG_RUNTIME_DIR", "/run/user/0");
            qunsetenv("SESSION_MANAGER");
        }
        app = QSharedPointer<QApplication>::create(argc, argv);
        if (getuid() == 0) {
            qputenv("HOME", "/root");
        }
    }
    QApplication::setOrganizationName("MX-Linux");
    QApplication::setApplicationVersion(VERSION);

    QTranslator qtTran;
    if (qtTran.load("qt_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    QTranslator appTran;
    const QString catalog = QApplication::applicationName() + "_" + QLocale::system().name();
    const QDir binaryDir(QCoreApplication::applicationDirPath());
    if (appTran.load(catalog, binaryDir.filePath(QStringLiteral(APP_LOCALE_DIR)))
        || appTran.load(catalog, binaryDir.absolutePath())) {
        QApplication::installTranslator(&appTran);
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(*app);

    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));

    // Root guard
    QFile loginUidFile {"/proc/self/loginuid"};
    if (loginUidFile.open(QIODevice::ReadOnly)) {
        QString loginUid = QString(loginUidFile.readAll()).trimmed();
        loginUidFile.close();
        if (loginUid == QLatin1String("0")) {
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
    if (!Cmd().runAsRoot({"true"}, true)) {
        QMessageBox::critical(nullptr, Cmd::tr("Administrator Access Required"),
                              Cmd::tr("This operation requires administrator privileges. Please restart the application "
                                      "and enter your password when prompted."));
        return EXIT_FAILURE;
    }
    MainWindow w;
    w.show();
    return QApplication::exec();
}

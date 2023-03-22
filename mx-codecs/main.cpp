/*****************************************************************************
 * main.cpp
 *****************************************************************************
 * Copyright (C) 2014 MX Authors
 *
 * Authors: Jerry 3904
 *          Anticaptilista
 *          Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Codecs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Codecs.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include <unistd.h>
#include "lockfile.h"
#include "mainwindow.h"
#include "version.h"

extern const QString starting_home = qEnvironmentVariable("HOME");

int main(int argc, char *argv[])
{
    if (getuid() == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qunsetenv("SESSION_MANAGER");
    }
    QApplication app(argc, argv);
    if (getuid() == 0) qputenv("HOME", "/root");
    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));
    QApplication::setApplicationVersion(VERSION);

    QTranslator qtTran;
    if (qtTran.load(QLocale(), QStringLiteral("qt"), QStringLiteral("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTran);

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtBaseTran);

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale().name(), "/usr/share/" + QApplication::applicationName() + "/locale"))
        QApplication::installTranslator(&appTran);

    // root guard
    if (QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "logname |grep -q ^root$"}) == 0) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("You seem to be logged in as root, please log out and log in as normal user to use this program."));
        exit(EXIT_FAILURE);
    }

    if (getuid() == 0) {
        // Don't start app if Synaptic/apt-get is running, lock dpkg otherwise while the program runs
        LockFile lock_file(QStringLiteral("/var/lib/dpkg/lock"));
        if (lock_file.isLocked()) {
            QApplication::beep();
            QMessageBox::critical(nullptr, QObject::tr("Unable to get exclusive lock"),
                                  QObject::tr("Another package management application (like Synaptic or apt-get), "\
                                                   "is already running. Please close that application first"));
            return EXIT_FAILURE;
        } else {
            lock_file.lock();
        }
        MainWindow w;
        w.show();
        return QApplication::exec();
    } else {
        QProcess::startDetached(QStringLiteral("/usr/bin/mx-cleanup-launcher"), {});
    }
}

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

#include "mainwindow.h"
#include "version.h"
#include <unistd.h>

static QFile logFile;
extern const QString starting_home = qEnvironmentVariable("HOME");

int main(int argc, char *argv[])
{
    void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    if (getuid() == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qunsetenv("SESSION_MANAGER");
    }
    QApplication app(argc, argv);
    if (getuid() == 0)
        qputenv("HOME", "/root");

    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));
    QApplication::setOrganizationName(QStringLiteral("MX-Linux"));
    QApplication::setApplicationVersion(VERSION);

    QTranslator qtTran;
    if (qtTran.load("qt_" + QLocale().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTran);

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtBaseTran);

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale().name(),
                     "/usr/share/" + QApplication::applicationName() + "/locale"))
        QApplication::installTranslator(&appTran);

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
    if (QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "logname |grep -q ^root$"}) == 0) {
        QMessageBox::critical(
            nullptr, QObject::tr("Error"),
            QObject::tr(
                "You seem to be logged in as root, please log out and log in as normal user to use this program."));
        exit(EXIT_FAILURE);
    }

    if (getuid() == 0) {
        // Don't start app if Synaptic/apt-get is running, lock dpkg otherwise while the program runs
        LockFile lock_file(QStringLiteral("/var/lib/dpkg/lock"));
        if (lock_file.isLocked()) {
            QApplication::beep();
            QMessageBox::critical(nullptr, QObject::tr("Unable to get exclusive lock"),
                                  QObject::tr("Another package management application (like Synaptic or apt-get), "
                                              "is already running. Please close that application first"));
            exit(EXIT_FAILURE);
        } else {
            lock_file.lock();
        }
        QString log_name = QStringLiteral("/var/log/mxpi.log");
        if (QFile::exists(log_name)) {
            QProcess::execute(QStringLiteral("/bin/bash"),
                              {"-c", "echo '-----------------------------------------------------------\n"
                                     "MXPI SESSION\n-----------------------------------------------------------' >> "
                                         + log_name.toUtf8() + ".old"});
            QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "cat " + log_name + " >> " + log_name + ".old"});
            QFile::remove(log_name);
        }
        logFile.setFileName(log_name);
        logFile.open(QFile::Append | QFile::Text);
        qInstallMessageHandler(messageHandler);

        MainWindow w(parser);
        w.show();
        return QApplication::exec();
    } else {
        QProcess::startDetached(QStringLiteral("/usr/bin/mxpi-launcher"), {});
    }
}

// The implementation of the handler
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream term_out(stdout);
    term_out << msg << QStringLiteral("\n");

    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz "));
    switch (type) {
    case QtInfoMsg:
        out << QStringLiteral("INF ");
        break;
    case QtDebugMsg:
        out << QStringLiteral("DBG ");
        break;
    case QtWarningMsg:
        out << QStringLiteral("WRN ");
        break;
    case QtCriticalMsg:
        out << QStringLiteral("CRT ");
        break;
    case QtFatalMsg:
        out << QStringLiteral("FTL ");
        break;
    }
    out << context.category << QStringLiteral(": ") << msg << QStringLiteral("\n");
}

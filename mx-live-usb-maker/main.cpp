/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian
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
#include <QFileInfo>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

#include "common.h"
#include "mainwindow.h"
#include <unistd.h>
#include <version.h>

static QFile logFile;
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

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
    QApplication::setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Program for creating a live-usb from an iso-file, another live-usb, "
                                                 "a live-cd/dvd, or a running live system."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QObject::tr("filename"), QObject::tr("Name of .iso file to open"),
                                 QObject::tr("[filename]"));
    parser.process(app);

    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
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

    QString executablePath = QStandardPaths::findExecutable("pkexec");
    if (executablePath.isEmpty() && getuid() != 0) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("You must run this program as root."));
        exit(EXIT_FAILURE);
    }

    auto const log_file_name = "/tmp/" + QApplication::applicationName() + ".log";
    logFile.setFileName(log_file_name);
    if (logFile.exists() && QFileInfo(logFile).isWritable()) {
        QFile::remove(log_file_name + ".old");
        QFile::rename(log_file_name, log_file_name + ".old");
    }
    if (logFile.exists() && !QFileInfo(logFile).isWritable()) {
        logFile.setFileName(log_file_name + "_new");
    }
    logFile.open(QIODevice::ReadWrite);
    qInstallMessageHandler(messageHandler);
    qDebug().noquote() << QApplication::applicationName() << QObject::tr("version:")
                       << QApplication::applicationVersion();
    MainWindow w(QApplication::arguments());
    w.show();
    return QApplication::exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream term_out(stdout);
    term_out << msg << '\n';

    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    switch (type) {
    case QtInfoMsg:
        out << "INF ";
        break;
    case QtDebugMsg:
        out << "DBG ";
        break;
    case QtWarningMsg:
        out << "WRN ";
        break;
    case QtCriticalMsg:
        out << "CRT ";
        break;
    case QtFatalMsg:
        out << "FTL ";
        break;
    }
    out << context.category << ": " << msg << '\n';
}

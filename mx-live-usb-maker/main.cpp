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
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QProcess>
#include <QTranslator>

#include "mainwindow.h"
#include <unistd.h>
#include <version.h>


static QFile logFile;
extern const QString starting_home = qEnvironmentVariable("HOME");
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    if (getuid() == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qunsetenv("SESSION_MANAGER");
    }
    QApplication app(argc, argv);
    if (getuid() == 0) qputenv("HOME", "/root");
    QApplication::setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Program for creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live system."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QObject::tr("filename"), QObject::tr("Name of .iso file to open"), QObject::tr("[filename]"));
    parser.process(app);

    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), QStringLiteral("qt"), QStringLiteral("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTran);

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtBaseTran);

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale::system().name(), "/usr/share/" + QApplication::applicationName() + "/locale"))
        QApplication::installTranslator(&appTran);

    // root guard
    if (QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "logname |grep -q ^root$"}) == 0) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("You seem to be logged in as root, please log out and log in as normal user to use this program."));
        exit(EXIT_FAILURE);
    }

    if (getuid() == 0) {
        QString log_name = "/var/log/" + QApplication::applicationName() + ".log";
        if (QFileInfo::exists(log_name)) {
            QFile::remove(log_name + ".old");
            QFile::rename(log_name, log_name + ".old");
        }
        logFile.setFileName(log_name);
        logFile.open(QFile::Append | QFile::Text);
        qInstallMessageHandler(messageHandler);
        qDebug().noquote() << QApplication::applicationName() << QObject::tr("version:") << QApplication::applicationVersion();
        MainWindow w(QApplication::arguments());
        w.show();
        return QApplication::exec();
    } else {
        QProcess::startDetached(QStringLiteral("/usr/bin/mxlum-launcher"), {});
    }
}


// The implementation of the handler
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QTextStream term_out(stdout);
    term_out << msg << QStringLiteral("\n");

    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz "));
    switch (type)
    {
    case QtInfoMsg:     out << QStringLiteral("INF "); break;
    case QtDebugMsg:    out << QStringLiteral("DBG "); break;
    case QtWarningMsg:  out << QStringLiteral("WRN "); break;
    case QtCriticalMsg: out << QStringLiteral("CRT "); break;
    case QtFatalMsg:    out << QStringLiteral("FTL "); break;
    }
    out << context.category << QStringLiteral(": ") << msg << QStringLiteral("\n");
}

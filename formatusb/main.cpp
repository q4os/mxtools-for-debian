/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2019 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *          using live-usb-maker by BitJam
 *          and mx-live-usb-maker gui by adrian
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
#include <QDateTime>
#include <QIcon>
#include <QLocale>
#include <QScopedPointer>
#include <QTranslator>
#include <QDebug>

#include "mainwindow.h"
#include <version.h>
#include <unistd.h>


QScopedPointer<QFile> logFile;
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (a.arguments().contains("--version") || a.arguments().contains("-v") ) {
       qDebug() << "Version:" << VERSION;
       return EXIT_SUCCESS;
    }

    a.setWindowIcon(QIcon::fromTheme("media-removable"));

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QString log_name= "/tmp/formatusb.log";
    // Set the logging files
    logFile.reset(new QFile(log_name));
    // Open the file logging
    logFile.data()->open(QFile::Append | QFile::Text);
    // Set handler
    qInstallMessageHandler(messageHandler);

    QTranslator appTran;
    appTran.load(QString("formatusb_") + QLocale::system().name(), "/usr/share/formatusb/locale");
    a.installTranslator(&appTran);

    qDebug() << "Program Version:" << VERSION;

        MainWindow w;
        w.show();
        return a.exec();
}


// The implementation of the handler
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Write to terminal
    QTextStream term_out(stdout);
    term_out << msg << "\n";

    // Open stream file writes
    QTextStream out(logFile.data());

    // Write the date of recording
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    // By type determine to what level belongs message
    switch (type)
    {
    //case QtInfoMsg:     out << "INF "; break; Not in older Qt versions
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    default:            out << "OTH"; break;
    }
    // Write to the output category of the message and the message itself
    out << context.category << ": " << msg << "\n";
    out.flush();    // Clear the buffered data
}

/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

#include "cmd.h"
#include "mainwindow.h"
#include "version.h"
#include "versionnumber.h"
#include <unistd.h>

// return the config file used for the newest conky process
QString getRunningConky()
{
    Cmd cmd;

    QString pid = cmd.getCmdOut(QStringLiteral("pgrep -u $(id -nu) -nx conky"), true);
    if (pid.isEmpty())
        return QString();
    QString conkyrc = cmd.getCmdOutUntrimmed(
        QString("sed -nrz '/^--config=/s///p; /^(-c|--config)/{n;p;}' /proc/%1/cmdline").arg(pid), true);
    if (conkyrc.startsWith("/"))
        return conkyrc;
    QString conkywd = cmd.getCmdOutUntrimmed(QString("sed -nrz '/^PWD=/s///p' /proc/%1/environ").arg(pid), true);
    return conkywd + "/" + conkyrc;
}

QString openFile(const QDir &dir)
{
    QString file_name = getRunningConky();

    if (not file_name.isEmpty())
        return file_name;

    QString selected
        = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select Conky Manager config file"), dir.path());
    if (not selected.isEmpty())
        return selected;
    return QString();
}

void messageUpdate()
{
    Cmd cmd;
    VersionNumber current_version {
        cmd.getCmdOut(QStringLiteral("dpkg -l mx-conky-data | awk 'NR==6 {print $3}'"), true)};

    QSettings settings;

    QString ver = settings.value(QStringLiteral("data-version")).toByteArray();
    VersionNumber recorded_version {ver};

    QString title = QObject::tr("Conky Data Update");
    QString message = QObject::tr("The MX Conky data set has been updated. <p><p>\
                                  Copy from the folder where it is located <a href=\"/usr/share/mx-conky-data/themes\">/usr/share/mx-conky-data/themes</a> \
                                  whatever you wish to your Home hidden conky folder <a href=\"%1/.conky\">~/.conky</a>. \
                                  Be careful not to overwrite any conkies you have changed.")
                          .arg(QDir::homePath());

    if (recorded_version.toString().isEmpty() || current_version > recorded_version) {
        settings.setValue(QStringLiteral("data-version"), current_version.toString());
        QMessageBox::information(nullptr, title, message);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
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

    if (system("dpkg -s conky-manager | grep -q 'Status: install ok installed'") != 0
        && system("dpkg -s conky-manager2 | grep -q 'Status: install ok installed'") != 0) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("Could not find conky-manager, please install it before running mx-conky"));
        return EXIT_FAILURE;
    }

    if (getuid() != 0) {

        QString dir = QDir::homePath() + "/.conky";
        if (!QFile::exists(dir))
            QDir().mkdir(dir);

        messageUpdate();

        // copy the mx-conky-data themes to the default folder
        system("cp -rn /usr/share/mx-conky-data/themes/* " + dir.toUtf8());

        QString file;
        if (QApplication::arguments().length() >= 2 && QFile::exists(QApplication::arguments().at(1)))
            file = QApplication::arguments().at(1);
        else
            file = openFile(dir);

        if (file.isEmpty())
            return EXIT_FAILURE;

        MainWindow w(nullptr, file);
        w.show();
        return QApplication::exec();

    } else {
        QApplication::beep();
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("You must run this program as normal user"));
        return EXIT_FAILURE;
    }
}

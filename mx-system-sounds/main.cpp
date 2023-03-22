/**********************************************************************
 *  main.cpp
 **********************************************************************
 * Copyright (C) 2016 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-system-sounds.
 *
 * mx-system-sounds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-system-sounds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-system-sounds.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "mainwindow.h"
#include <unistd.h>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon("/usr/share/pixmaps/mx-system-sounds.png"));

    QTranslator qtTran;
    qtTran.load(QStringLiteral("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QStringLiteral("mx-system-sounds_") + QLocale::system().name(), QStringLiteral("/usr/share/mx-system-sounds/locale"));
    a.installTranslator(&appTran);

    if (system("echo $XDG_CURRENT_DESKTOP | grep -q XFCE") != 0) {
            QMessageBox::information(nullptr, QObject::tr("MX System Sounds"), QObject::tr("This app is Xfce-only"));
            exit(EXIT_SUCCESS);
    }

    MainWindow w;
    w.show();
    return a.exec();
}

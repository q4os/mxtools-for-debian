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
#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>
#include <unistd.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon("/usr/share/pixmaps/mx-system-sounds.png"));

    QTranslator qtTran;
    qtTran.load(QStringLiteral("qt_") + QLocale::system().name());
    QApplication::installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QStringLiteral("mx-system-sounds_") + QLocale::system().name(),
                 QStringLiteral("/usr/share/mx-system-sounds/locale"));
    QApplication::installTranslator(&appTran);

    if (qgetenv("XDG_CURRENT_DESKTOP") != "xfce" && qgetenv("XDG_CURRENT_DESKTOP") != "XFCE") {
        QMessageBox::information(nullptr, QObject::tr("MX System Sounds"), QObject::tr("This app is Xfce-only"));
        exit(EXIT_SUCCESS);
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}

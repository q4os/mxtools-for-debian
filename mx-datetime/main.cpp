/**********************************************************************
 *  Main routine for MX Date/Time.
 **********************************************************************
 *   Copyright (C) 2019 by AK-47
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of mx-datetime.
 **********************************************************************/

#include <unistd.h>
#include <QApplication>
#include <QLibraryInfo>
#include <QIcon>
#include <QMessageBox>
#include <QTranslator>

#include "datetime.h"

const extern QString starting_home = qEnvironmentVariable("HOME");

int main(int argc, char *argv[])
{
    if (getuid() == 0) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        qunsetenv("SESSION_MANAGER");
    }
    QApplication a(argc, argv);
    if (getuid() == 0) qputenv("HOME", "/root");
    a.setWindowIcon(QIcon::fromTheme(a.applicationName()));

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    const QString &transpath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const QString &transpath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), "qt", "_", transpath)) {
        a.installTranslator(&qtTran);
    }
    QTranslator qtBaseTran;
    if (qtBaseTran.load(QLocale::system(), "qtbase", "_", transpath)) {
        a.installTranslator(&qtBaseTran);
    }
    QTranslator appTran;
    if (appTran.load(QLocale::system(), a.applicationName(), "_", "/usr/share/mx-datetime/locale")) {
        a.installTranslator(&appTran);
    }

    MXDateTime w;
    w.show();
    return a.exec();
}

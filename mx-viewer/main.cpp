/*****************************************************************************
 * main.cpp
 *****************************************************************************
 * Copyright (C) 2022 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Viewer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "mainwindow.h"
#include "version.h"
#include <unistd.h>

QPair<int, int> getUserIDs()
{
    QPair<uint, uint> id;
    QProcess proc;
    proc.start("logname", {}, QIODevice::ReadOnly);
    proc.waitForFinished();
    QString logname = QString::fromLatin1(proc.readAllStandardOutput().trimmed());
    proc.start("id", {"-u", logname}, QIODevice::ReadOnly);
    proc.waitForFinished();
    id.first = proc.readAllStandardOutput().trimmed().toUInt();
    proc.start("id", {"-g", logname}, QIODevice::ReadOnly);
    proc.waitForFinished();
    id.second = proc.readAllStandardOutput().trimmed().toUInt();
    return id;
}

// Drop rights of the program to regular user or 'nobody' if logname return root id or gid
// Used to drop rights to 'nobody', but normal user rights might be needed to write cache and cookies.
bool dropElevatedPrivileges(bool force_nobody)
{
    if (getuid() != 0 && geteuid() != 0) {
        return true;
    }

    // ref:
    // https://www.safaribooksonline.com/library/view/secure-programming-cookbook/0596003943/ch01s03.html#secureprgckbk-CHP-1-SECT-3.3
    auto [id, gid] = getUserIDs();
    const int nobody = 65534; // nobody (uid 65534), nogroup (gid 65534)
    if (id == 0 || gid == 0 || force_nobody) {
        id = gid = nobody;
    }

    if (setgid(id) != 0) {
        return false;
    }
    if (setuid(gid) != 0) {
        return false;
    }

    // On systems with defined _POSIX_SAVED_IDS in the unistd.h file, it should be
    // impossible to regain elevated privs after the setuid() call, above.  Test, try to regain elev priv:
    if (setuid(0) != -1 || seteuid(0) != -1) {
        return false; // and the calling fn should EXIT/abort the program
    }

    // change cwd, for good measure (if unable to, treat as overall failure)
    if (chdir("/tmp") != 0) {
        qDebug() << "Can't change working directory to /tmp";
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon::fromTheme(QApplication::applicationName()));
    QApplication::setApplicationVersion(VERSION);
    QApplication::setOrganizationName(QStringLiteral("MX-Linux"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr("This tool will display the URL content in a window, window title is optional"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(
        {{QStringLiteral("f"), QStringLiteral("full-screen")}, QObject::tr("Start program in full-screen mode")});
    parser.addOption({{QStringLiteral("i"), QStringLiteral("disable-images")},
                      QObject::tr("Disable load images automatically from websites")});
    parser.addOption({{QStringLiteral("j"), QStringLiteral("disable-js")}, QObject::tr("Disable JavaScript")});
    if (getuid() == 0 || geteuid() == 0) {
        parser.addOption(
            {{QStringLiteral("n"), QStringLiteral("force-nobody")},
             QObject::tr("Drop program's rights to 'nobody'. By default, if run as root, the rights are "
                         "dropped to normal user. This option might provide additional protection, but the program "
                         "would not be able to write its cache and cookies to the user directory, so it might break "
                         "some functionality.")});
    }
    parser.addOption({{QStringLiteral("s"), QStringLiteral("enable-spatial-navigation")},
                      QObject::tr("Enable spatial navigation with keyboard")});
    parser.addPositionalArgument(QObject::tr("URL"),
                                 QObject::tr("URL of the page you want to load")
                                     + "\ne.g., https://google.com, google.com, file:///home/user/file.html");
    parser.addPositionalArgument(QObject::tr("Title"), QObject::tr("Window title for the viewer"),
                                 QStringLiteral("[title]"));
    parser.process(app);

    bool force_nobody = (getuid() == 0 || geteuid() == 0) ? parser.isSet("force-nobody") : false;
    if (!dropElevatedPrivileges(force_nobody)) {
        qDebug() << "Could not drop elevated privileges";
        exit(EXIT_FAILURE);
    }

    QTranslator qtTran;
    if (qtTran.load(QLocale::system(), QStringLiteral("qt"), QStringLiteral("_"),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTran);
    }

    QTranslator qtBaseTran;
    if (qtBaseTran.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtBaseTran);
    }

    QTranslator appTran;
    if (appTran.load(QApplication::applicationName() + "_" + QLocale::system().name(),
                     "/usr/share/" + QApplication::applicationName() + "/locale")) {
        QApplication::installTranslator(&appTran);
    }

    MainWindow w(parser);
    w.show();

    return QApplication::exec();
}

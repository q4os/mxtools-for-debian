/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
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
#include "log.h"

#include <QDate>
#include <QDateTime>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

Log::Log(const QString &fileName)
{
    logFile.setFileName(fileName);
    if (!openLogFile()) {
        qDebug() << "Could not open log file:" << fileName;
        return;
    }
    qInstallMessageHandler(Log::messageHandler);
}

QString Log::defaultLogPath()
{
    // Keep the log out of world-writable /tmp. As root use /run (root-only);
    // as the user use the private per-user runtime dir ($XDG_RUNTIME_DIR,
    // i.e. /run/user/<uid>, mode 0700). Fall back to /tmp only if no runtime
    // dir is available (openLogFile() still refuses to follow a symlink there).
    if (geteuid() == 0) {
        return QStringLiteral("/run/mxpi.log");
    }
    const QString runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (!runtimeDir.isEmpty() && QFileInfo(runtimeDir).isDir()) {
        return runtimeDir + QStringLiteral("/mxpi.log");
    }
    return QStringLiteral("/tmp/mxpi.log");
}

bool Log::openLogFile()
{
    const QByteArray name = logFile.fileName().toLocal8Bit();
    // O_NOFOLLOW: never follow a symlink at the final path component, so even
    // the world-writable /tmp fallback cannot be redirected at another file.
    const int fd = ::open(name.constData(), O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return false;
    }
    if (!logFile.open(fd, QIODevice::ReadWrite, QFileDevice::AutoCloseHandle)) {
        ::close(fd);
        return false;
    }
    return true;
}

void Log::messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    static QMutex logMutex;
    const QMutexLocker locker(&logMutex);

    QTextStream termOut(stdout);
    if (msg.contains('\r')) {
        termOut << msg;
        return;
    }
    termOut << msg << '\n';

    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz "));
    switch (type) {
    case QtInfoMsg:
        out << QStringLiteral("INF");
        break;
    case QtDebugMsg:
        out << QStringLiteral("DBG");
        break;
    case QtWarningMsg:
        out << QStringLiteral("WRN");
        break;
    case QtCriticalMsg:
        out << QStringLiteral("CRT");
        break;
    case QtFatalMsg:
        out << QStringLiteral("FTL");
        break;
    }
    out << QStringLiteral(": ") << msg << '\n';
}

QString Log::getLog()
{
    return logFile.fileName();
}

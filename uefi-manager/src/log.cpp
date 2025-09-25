/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2024-2025 MX Authors
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

#include <QDateTime>

Log::Log(const QString &fileName)
{
    logFile.setFileName(fileName);
    if (!logFile.open(QIODevice::ReadWrite)) {
        qDebug() << "Could not open log file:" << fileName;
        return;
    }
    qInstallMessageHandler(Log::messageHandler);
}

void Log::messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
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

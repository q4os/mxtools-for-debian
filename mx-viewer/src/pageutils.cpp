/*****************************************************************************
 * pageutils.cpp
 *****************************************************************************
 * Copyright (C) 2022-2026 MX Authors
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

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QUrlQuery>
#include <QWebEngineProfile>

#include "mainwindowhelpers.h"
#include <QUrl>

using namespace MainWindowHelpers;

namespace MainWindowHelpers {
qint64 directorySize(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return -1;
    }
    qint64 total = 0;
    QDirIterator it(path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

qint64 totalDirectorySize(const QStringList &paths)
{
    qint64 total = 0;
    bool found = false;
    for (const auto &path : paths) {
        if (!QDir(path).exists()) {
            continue;
        }
        const qint64 size = directorySize(path);
        if (size >= 0) {
            total += size;
            found = true;
        }
    }
    return found ? total : -1;
}

QStringList collectCachePaths(const QWebEngineProfile *profile)
{
    if (!profile) return {};
    const QString cachePath = profile->cachePath();
    if (cachePath.isEmpty()) return {};
    QDir dir(cachePath);
    QStringList subdirs;
    for (const QString &entry : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        subdirs << cachePath + "/" + entry;
    }
    return subdirs;
}

// A value from a form submitted with GET, where '+' stands for a space; decoded before %2B becomes a
// literal '+'.
QString formValue(const QUrlQuery &query, const QString &name)
{
    QString encoded = query.queryItemValue(name, QUrl::FullyEncoded);
    encoded.replace(QLatin1Char('+'), QLatin1Char(' '));
    return QUrl::fromPercentEncoding(encoded.toUtf8()).trimmed();
}
} // namespace MainWindowHelpers

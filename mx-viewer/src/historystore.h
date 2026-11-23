/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2026 MX Authors
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
#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

class QUrl;

// Browsing history, kept in an SQLite database next to the other application data rather than in the
// settings file. Every running instance opens the same database, which serializes their writes.
namespace HistoryStore
{
struct Entry {
    qint64 id {};
    QString title;
    QString url;
    QByteArray icon; // PNG of the site's favicon; shared by all pages of the site
    qint64 time {};  // seconds since epoch; 0 for entries logged before timestamps were recorded
};

// Oldest first, in the order the pages were visited.
QList<Entry> entries();

struct Site {
    QString site;    // scheme://host[:port]
    int visits {};
    qint64 lastId {}; // the most recent visit, for ordering
    QByteArray icon;
};
// Every web site visited, with its visit count. Callers rank them, so that sites they count as one
// (http and https of a host) are added up first.
QList<Site> sites();
// Each URL once, most recently visited first, at most limit of them.
QStringList recentUrls(int limit);
// Returns false if the visit could not be recorded.
bool addVisit(const QUrl &url, const QString &title);
void setIcon(const QUrl &url, const QByteArray &png);
void remove(qint64 id);
// Removes the visits made at or after cutoff (seconds since epoch).
void removeSince(qint64 cutoff);
void clear();
void close();
} // namespace HistoryStore

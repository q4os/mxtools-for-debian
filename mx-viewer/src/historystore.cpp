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
#include "historystore.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUrl>
#include <QVariant>

namespace
{
const QString connectionName = QStringLiteral("history");
constexpr int schemaVersion = 1;
constexpr qint64 retentionSeconds = 365LL * 24 * 60 * 60;

// Icons are stored once per site instead of once per visit.
QString siteKey(const QUrl &url)
{
    return url.adjusted(QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment)
        .toString();
}

bool exec(QSqlQuery &query)
{
    if (!query.exec()) {
        qWarning() << "History database:" << query.lastError().text();
        return false;
    }
    return true;
}

bool exec(const QSqlDatabase &db, const QString &statement)
{
    QSqlQuery query(db);
    query.prepare(statement);
    return exec(query);
}

// Copies the history that older versions kept in the settings file, once.
void migrateFromSettings(const QSqlDatabase &db)
{
    QSettings settings;
    const int size = settings.beginReadArray("History");
    QSqlQuery visit(db);
    visit.prepare("INSERT INTO visits (url, title, time, site) VALUES (?, ?, ?, ?)");
    QSqlQuery icon(db);
    icon.prepare("INSERT OR REPLACE INTO icons (site, icon) VALUES (?, ?)");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        const QString url = settings.value("url").toString();
        if (url.isEmpty()) {
            continue;
        }
        const QString site = siteKey(QUrl(url));
        visit.addBindValue(url);
        visit.addBindValue(settings.value("title").toString());
        visit.addBindValue(settings.value("time").toLongLong());
        visit.addBindValue(site);
        exec(visit);
        // Later entries overwrite earlier ones, so each site keeps its most recent icon.
        const QByteArray png = settings.value("icon").toByteArray();
        if (!png.isEmpty()) {
            icon.addBindValue(site);
            icon.addBindValue(png);
            exec(icon);
        }
    }
    settings.endArray();
}

void removeUnusedIcons(const QSqlDatabase &db)
{
    exec(db, "DELETE FROM icons WHERE site NOT IN (SELECT site FROM visits WHERE site IS NOT NULL)");
}

// In WAL mode deleted rows stay in the log's frames until a checkpoint writes them back; truncating it
// leaves no copy of removed history on disk.
void purgeDeleted(const QSqlDatabase &db)
{
    exec(db, "PRAGMA wal_checkpoint(TRUNCATE)");
}

bool setUp(QSqlDatabase &db)
{
    // WAL lets instances read while another one writes; it can't be changed inside a transaction.
    exec(db, "PRAGMA journal_mode=WAL");
    // Deleted history is overwritten on disk rather than left in free pages, and the write-ahead log
    // is truncated after each checkpoint instead of keeping old pages around.
    exec(db, "PRAGMA secure_delete=ON");
    exec(db, "PRAGMA journal_size_limit=0");
    // Take the write lock up front so two instances starting together don't both migrate.
    if (!exec(db, "BEGIN IMMEDIATE")) {
        return false;
    }
    QSqlQuery version(db);
    version.prepare("PRAGMA user_version");
    const bool fresh = exec(version) && version.next() && version.value(0).toInt() < schemaVersion;
    if (fresh) {
        const bool created
            = exec(db, "CREATE TABLE IF NOT EXISTS visits (id INTEGER PRIMARY KEY, url TEXT NOT NULL, "
                       "title TEXT, time INTEGER NOT NULL DEFAULT 0, site TEXT)")
              && exec(db, "CREATE INDEX IF NOT EXISTS visits_time ON visits (time)")
              && exec(db, "CREATE INDEX IF NOT EXISTS visits_url ON visits (url)")
              && exec(db, "CREATE TABLE IF NOT EXISTS icons (site TEXT PRIMARY KEY, icon BLOB)");
        if (!created) {
            exec(db, "ROLLBACK");
            return false;
        }
        migrateFromSettings(db);
        exec(db, QStringLiteral("PRAGMA user_version=%1").arg(schemaVersion));
    }
    if (!exec(db, "COMMIT")) {
        exec(db, "ROLLBACK");
        return false;
    }
    // Drop the copy in the settings file only once the database holds it; also covers an instance
    // that found the migration already done by another one.
    QSettings settings;
    if (settings.childGroups().contains("History")) {
        settings.remove("History");
    }
    // History is kept for a year; visits without a time predate timestamps and are left alone.
    QSqlQuery prune(db);
    prune.prepare("DELETE FROM visits WHERE time > 0 AND time < ?");
    prune.addBindValue(QDateTime::currentSecsSinceEpoch() - retentionSeconds);
    if (exec(prune) && prune.numRowsAffected() > 0) {
        removeUnusedIcons(db);
        purgeDeleted(db);
    }
    return true;
}

// Opened on first use; an empty, invalid database means history is unavailable (e.g. no writable
// data directory after dropping privileges), and every call then does nothing.
QSqlDatabase database()
{
    if (QSqlDatabase::contains(connectionName)) {
        return QSqlDatabase::database(connectionName, false);
    }
    auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty() || !QDir().mkpath(dir)) {
        qWarning() << "History database: no writable data directory";
        return db;
    }
    db.setDatabaseName(dir + "/history.db");
    // Wait for another instance's write instead of failing at once.
    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000");
    if (!db.open()) {
        qWarning() << "History database:" << db.lastError().text();
        return db;
    }
    if (!setUp(db)) {
        db.close();
    }
    return db;
}
} // namespace

QList<HistoryStore::Entry> HistoryStore::entries()
{
    QList<Entry> result;
    const auto db = database();
    if (!db.isOpen()) {
        return result;
    }
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare("SELECT visits.id, visits.title, visits.url, icons.icon, visits.time FROM visits "
                  "LEFT JOIN icons ON icons.site = visits.site ORDER BY visits.id");
    if (!exec(query)) {
        return result;
    }
    while (query.next()) {
        result.append({query.value(0).toLongLong(), query.value(1).toString(), query.value(2).toString(),
                       query.value(3).toByteArray(), query.value(4).toLongLong()});
    }
    return result;
}

QList<HistoryStore::Site> HistoryStore::sites()
{
    QList<Site> result;
    const auto db = database();
    if (!db.isOpen()) {
        return result;
    }
    // Counted in the database, so one row per site leaves it, not every visit with its icon.
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare("SELECT visits.site, COUNT(*), MAX(visits.id), icons.icon FROM visits "
                  "LEFT JOIN icons ON icons.site = visits.site "
                  "WHERE visits.site LIKE 'http://%' OR visits.site LIKE 'https://%' "
                  "GROUP BY visits.site");
    if (!exec(query)) {
        return result;
    }
    while (query.next()) {
        result.append({query.value(0).toString(), query.value(1).toInt(), query.value(2).toLongLong(),
                       query.value(3).toByteArray()});
    }
    return result;
}

QStringList HistoryStore::recentUrls(int limit)
{
    QStringList result;
    const auto db = database();
    if (!db.isOpen()) {
        return result;
    }
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare("SELECT url FROM visits GROUP BY url ORDER BY MAX(id) DESC LIMIT ?");
    query.addBindValue(limit);
    if (!exec(query)) {
        return result;
    }
    while (query.next()) {
        result.append(query.value(0).toString());
    }
    return result;
}

bool HistoryStore::addVisit(const QUrl &url, const QString &title)
{
    // Generated content has no address worth going back to.
    if (url.scheme() == "data" || url.scheme() == "blob") {
        return false;
    }
    const auto db = database();
    if (!db.isOpen()) {
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO visits (url, title, time, site) VALUES (?, ?, ?, ?)");
    // A user name and password in the address are never written to disk.
    query.addBindValue(url.adjusted(QUrl::RemoveUserInfo).toString());
    query.addBindValue(title);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(siteKey(url));
    return exec(query);
}

void HistoryStore::setIcon(const QUrl &url, const QByteArray &png)
{
    const auto db = database();
    if (!db.isOpen() || png.isEmpty()) {
        return;
    }
    QSqlQuery query(db);
    query.prepare("INSERT OR REPLACE INTO icons (site, icon) VALUES (?, ?)");
    query.addBindValue(siteKey(url));
    query.addBindValue(png);
    exec(query);
}

void HistoryStore::remove(qint64 id)
{
    const auto db = database();
    if (!db.isOpen()) {
        return;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM visits WHERE id = ?");
    query.addBindValue(id);
    if (exec(query)) {
        removeUnusedIcons(db);
        purgeDeleted(db);
    }
}

void HistoryStore::removeSince(qint64 cutoff)
{
    const auto db = database();
    if (!db.isOpen()) {
        return;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM visits WHERE time >= ?");
    query.addBindValue(cutoff);
    if (exec(query)) {
        removeUnusedIcons(db);
        purgeDeleted(db);
    }
}

void HistoryStore::clear()
{
    const auto db = database();
    if (!db.isOpen()) {
        return;
    }
    exec(db, "DELETE FROM visits");
    exec(db, "DELETE FROM icons");
    // Give the space back.
    exec(db, "VACUUM");
    purgeDeleted(db);
}

void HistoryStore::close()
{
    if (!QSqlDatabase::contains(connectionName)) {
        return;
    }
    {
        auto db = QSqlDatabase::database(connectionName, false);
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

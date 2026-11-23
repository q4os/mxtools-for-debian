/*****************************************************************************
 * newtabpage.cpp
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
#include "mainwindow.h"
#include "historystore.h"

#include <algorithm>

#include <QBuffer>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

void MainWindow::renderNewTabPage(WebView *view)
{
    if (view) {
        view->setHtml(buildNewTabPageHtml(), QUrl("mx-newtab://"));
    }
}

// Speed dial: bookmarks and the most visited sites from history (bookmarks only in a private window).
QString MainWindow::buildNewTabPageHtml()
{
    constexpr int maxTiles = 12;
    auto tile = [](const QString &url, const QString &label, const QString &detail, const QByteArray &icon) {
        const QString iconHtml = icon.isEmpty()
            ? QStringLiteral("<span class=\"icon letter\">%1</span>").arg(label.left(1).toUpper().toHtmlEscaped())
            : QStringLiteral("<img class=\"icon\" alt=\"\" src=\"data:image/png;base64,%1\">")
                  .arg(QString::fromLatin1(icon.toBase64()));
        return QStringLiteral("<a class=\"tile\" href=\"%1\" title=\"%2\">%3<span class=\"label\">%4</span>"
                              "<span class=\"detail\">%5</span></a>")
            .arg(url.toHtmlEscaped(), (label + "\n" + url).toHtmlEscaped(), iconHtml, label.toHtmlEscaped(),
                 detail.toHtmlEscaped());
    };

    QStringList bookmarkTiles;
    const auto actions = bookmarks->actions();
    for (const QAction *action : actions) {
        const QUrl url = action->property("url").toUrl();
        if (!url.isValid() || url.isEmpty()) {
            continue;
        }
        QByteArray icon;
        if (!action->icon().isNull()) {
            QBuffer buffer(&icon);
            if (buffer.open(QIODevice::WriteOnly)) {
                action->icon().pixmap(QSize(32, 32)).save(&buffer, "PNG");
            }
        }
        const QString label = bookmarkTitle(action).isEmpty() ? url.host() : bookmarkTitle(action);
        bookmarkTiles.append(tile(url.toString(), label, url.host(), icon));
        if (bookmarkTiles.size() == maxTiles) {
            break;
        }
    }

    QStringList siteTiles;
    if (!privateWindow) {
        struct Site {
            QUrl root;
            int visits {};
            qint64 lastId {};
            QByteArray icon;
        };
        // http and https of one host:port count as one site; different ports are different sites.
        QHash<QString, Site> sites;
        const QList<HistoryStore::Site> visited = HistoryStore::sites();
        for (const HistoryStore::Site &entry : visited) {
            const QUrl url(entry.site);
            if (url.host().isEmpty()) {
                continue;
            }
            Site &site = sites[url.host() + ':' + QString::number(url.port())];
            site.visits += entry.visits;
            if (entry.lastId > site.lastId) {
                // The most recently used scheme; adjusted() keeps the port and IPv6 brackets.
                site.lastId = entry.lastId;
                site.root = url.adjusted(QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
                site.root.setPath("/");
            }
            if (site.icon.isEmpty()) {
                site.icon = entry.icon;
            }
        }
        QList<Site> ranked = sites.values();
        std::sort(ranked.begin(), ranked.end(), [](const Site &a, const Site &b) {
            return a.visits != b.visits ? a.visits > b.visits : a.lastId > b.lastId;
        });
        for (const Site &site : std::as_const(ranked)) {
            // Host with brackets and port, as typed; user info was removed from root.
            QString label = site.root.authority();
            if (label.startsWith("www.")) {
                label = label.mid(4);
            }
            siteTiles.append(tile(site.root.toString(), label, tr("%n visit(s)", nullptr, site.visits), site.icon));
            if (siteTiles.size() == maxTiles) {
                break;
            }
        }
    }

    auto section = [](const QString &heading, const QStringList &tiles) {
        return tiles.isEmpty() ? QString()
                               : QStringLiteral("<h2>%1</h2><div class=\"grid\">%2</div>")
                                     .arg(heading.toHtmlEscaped(), tiles.join("\n"));
    };
    QString body = section(tr("Bookmarks"), bookmarkTiles) + section(tr("Most visited"), siteTiles);
    if (body.isEmpty()) {
        body = QStringLiteral("<p class=\"empty\">%1</p>")
                   .arg(tr("Type an address or search terms in the address bar.").toHtmlEscaped());
    }
    return QStringLiteral(R"(<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>%1</title>
  <style>
    :root { color-scheme: light dark; }
    body { font-family: sans-serif; margin: 32px auto; max-width: 960px; padding: 0 24px; }
    h2 { font-size: 15px; font-weight: 600; opacity: 0.7; margin: 24px 0 12px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(140px, 1fr)); gap: 12px; }
    .tile { display: flex; flex-direction: column; align-items: center; gap: 6px; padding: 16px 8px 12px;
            border: 1px solid rgba(128, 128, 128, 0.3); border-radius: 10px; text-decoration: none; color: inherit; }
    .tile:hover, .tile:focus { background: rgba(128, 128, 128, 0.12); outline: none; }
    .icon { width: 32px; height: 32px; border-radius: 6px; }
    .letter { display: flex; align-items: center; justify-content: center; font-weight: 700; font-size: 18px;
              background: rgba(128, 128, 128, 0.2); }
    .label, .detail { max-width: 100%; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
    .label { font-size: 14px; }
    .detail { font-size: 12px; opacity: 0.6; }
    .empty { text-align: center; opacity: 0.6; margin-top: 15%; }
  </style>
</head>
<body>%2</body>
</html>)")
        .arg(tr("New Tab").toHtmlEscaped(), body);
}

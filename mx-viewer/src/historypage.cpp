/*****************************************************************************
 * historypage.cpp
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

#include <QDateTime>
#include <QLabel>
#include <QLocale>
#include <QSet>
#include <QMessageBox>
#include <QTimer>
#include <QUrlQuery>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

void MainWindow::listHistory()
{
    history->clear();
    // Owned by the menu, so the next clear() deletes it.
    auto *showHistory = new QAction(QIcon::fromTheme("view-list-text"), tr("History"), history);
    showHistory->setShortcut(Qt::CTRL | Qt::Key_H);
    connect(showHistory, &QAction::triggered, this, &MainWindow::openHistoryPage);
    history->addAction(showHistory);
    history->addAction(clearDataAction);
    history->addSeparator();
    auto *recentTitle = new QWidgetAction(history);
    auto *recentLabel = new QLabel(tr("Recent tabs"), history);
    QFont recentFont = recentLabel->font();
    recentFont.setUnderline(true);
    recentFont.setBold(true);
    recentLabel->setFont(recentFont);
    recentLabel->setStyleSheet("padding: 4px 18px 4px 18px;");
    // In the text color the menu is really painted with, which the palette may not match.
    QPalette recentPalette = recentLabel->palette();
    recentPalette.setColor(QPalette::WindowText, renderedMenuColors(history).second);
    recentLabel->setPalette(recentPalette);
    recentTitle->setDefaultWidget(recentLabel);
    history->addAction(recentTitle);
    if (closedTabs.isEmpty()) {
        auto *emptyAction = history->addAction(tr("No recent tabs"));
        emptyAction->setEnabled(false);
    } else {
        for (int i = closedTabs.size() - 1; i >= 0; --i) {
            const int tabIndex = i;
            const auto entry = closedTabs.at(i);
            const QUrl url = entry.first;
            auto *action = history->addAction(entry.second, url.toDisplayString());
            connect(action, &QAction::triggered, this, [this, tabIndex] {
                if (tabIndex < 0 || tabIndex >= closedTabs.size()) {
                    return;
                }
                const auto entry = closedTabs.takeAt(tabIndex);
                openSavedTab(entry.first, true);
            });
        }
    }
    adaptMenuIcons(history);
    refreshHistoryCompleter();
}

QString MainWindow::buildHistoryPageHtml(const QString &filter)
{
    // The history on disk belongs to regular windows; a private window lists none of it.
    const QList<HistoryStore::Entry> entries = privateWindow ? QList<HistoryStore::Entry>() : HistoryStore::entries();

    // Newest first, under a heading per day. Entries are stored in the order they were visited, and
    // the oldest ones may have no time recorded.
    const QLocale locale;
    const QDate today = QDate::currentDate();
    QStringList groups;
    QStringList rows;
    QString groupLabel;
    const auto closeGroup = [&] {
        if (!rows.isEmpty()) {
            groups.append(QStringLiteral("<section class=\"group\"><h2 class=\"day\">%1</h2><ul class=\"list\">%2</ul></section>")
                              .arg(groupLabel.toHtmlEscaped(), rows.join("\n")));
            rows.clear();
        }
    };
    for (qsizetype i = entries.size() - 1; i >= 0; --i) {
        const HistoryStore::Entry &entry = entries.at(i);
        const QDateTime visited = entry.time > 0 ? QDateTime::fromSecsSinceEpoch(entry.time) : QDateTime();
        QString label;
        if (!visited.isValid()) {
            label = tr("Earlier");
        } else if (visited.date() == today) {
            label = tr("Today");
        } else if (visited.date() == today.addDays(-1)) {
            label = tr("Yesterday");
        } else {
            label = locale.toString(visited.date(), QLocale::LongFormat);
        }
        if (label != groupLabel) {
            closeGroup();
            groupLabel = label;
        }
        const QString title = entry.title.isEmpty() ? entry.url : entry.title;
        const QString searchText = (title + " " + entry.url).toLower();
        // The page filters as the user types; this serves a search submitted without JavaScript.
        if (!searchText.contains(filter.toLower())) {
            continue;
        }
        const QString titleEscaped = title.toHtmlEscaped();
        const QString urlEscaped = entry.url.toHtmlEscaped();
        const QString timeText = visited.isValid() ? locale.toString(visited.time(), QLocale::ShortFormat) : QString();
        QString iconHtml;
        if (!entry.icon.isEmpty()) {
            const QString iconBase64 = QString::fromLatin1(entry.icon.toBase64());
            iconHtml = QStringLiteral("<img class=\"icon\" alt=\"\" src=\"data:image/png;base64,%1\">")
                           .arg(iconBase64);
        } else {
            iconHtml = QStringLiteral("<span class=\"icon placeholder\"></span>");
        }
        // Plain links and a form, so the page also works with JavaScript turned off.
        const QString deleteUrl = QStringLiteral("mx-history://delete?id=%1&q=%2")
                                      .arg(QString::number(entry.id), QString::fromLatin1(QUrl::toPercentEncoding(filter)));
        rows.append(QStringLiteral(
                        "<li class=\"entry\" data-search=\"%1\">"
                        "<span class=\"time\">%8</span>"
                        "%2"
                        "<div class=\"content\">"
                        "<div class=\"row\">"
                        "<a class=\"title\" href=\"%3\">%4</a>"
                        "<a class=\"delete\" href=\"%5\">%6</a>"
                        "</div>"
                        "<div class=\"url\">%7</div>"
                        "</div>"
                        "</li>")
                        .arg(searchText.toHtmlEscaped(), iconHtml, urlEscaped, titleEscaped, deleteUrl.toHtmlEscaped(),
                             tr("Delete").toHtmlEscaped(), urlEscaped, timeText.toHtmlEscaped()));
    }
    closeGroup();

    const QString emptyText = tr("No history entries.");
    const QString html = QStringLiteral(R"(<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="cache-control" content="no-cache">
  <title>%1</title>
  <style>
    :root { color-scheme: light; }
    body { font-family: sans-serif; margin: 24px; color: #1f2328; background: #ffffff; }
    h1 { font-size: 22px; margin: 0 0 12px; }
    .controls { display: flex; gap: 12px; align-items: center; margin-bottom: 16px; flex-wrap: wrap; }
    .search-form { flex: 1 1 240px; display: flex; }
    .search { flex: 1 1 auto; padding: 8px 10px; border: 1px solid #d0d7de; border-radius: 6px; }
    .clear { padding: 8px 12px; border: 1px solid #d0d7de; background: #f6f8fa; border-radius: 6px; cursor: pointer; color: inherit; text-decoration: none; }
    .list { list-style: none; padding: 0; margin: 0; display: flex; flex-direction: column; gap: 10px; }
    .group { margin-bottom: 20px; }
    .day { font-size: 15px; margin: 0 0 8px; color: #57606a; }
    .entry { display: grid; grid-template-columns: auto 24px 1fr; gap: 10px; padding: 10px 12px; border: 1px solid #eaeef2; border-radius: 8px; }
    .time { color: #57606a; font-size: 13px; width: 5.5em; text-align: right; padding-top: 2px; font-variant-numeric: tabular-nums; }
    .icon { width: 20px; height: 20px; border-radius: 4px; }
    .icon.placeholder { background: #eaeef2; }
    .content { display: flex; flex-direction: column; gap: 4px; }
    .row { display: flex; align-items: center; gap: 10px; }
    .title { color: #0969da; text-decoration: none; font-weight: 600; flex: 1 1 auto; }
    .url { color: #57606a; font-size: 12px; word-break: break-all; }
    .delete { padding: 4px 8px; border: 1px solid #d0d7de; background: #fff; border-radius: 6px; cursor: pointer; color: inherit; text-decoration: none; font-size: 13px; }
    .empty { padding: 16px; border: 1px dashed #d0d7de; border-radius: 8px; color: #57606a; }
  </style>
</head>
<body>
  <h1>%1</h1>
  <div class="controls">
    <form class="search-form" action="mx-history://list" method="get">
      <input id="search" name="q" class="search" type="search" placeholder="%2" value="%5" autofocus>
    </form>
    <a id="clear" class="clear" href="mx-history://clear">%3</a>
  </div>
  %4
  <script>
    const search = document.getElementById('search');
    const entries = Array.from(document.querySelectorAll('li.entry'));
    function applyFilter() {
      const term = search.value.trim().toLowerCase();
      entries.forEach(entry => {
        entry.style.display = entry.dataset.search.includes(term) ? '' : 'none';
      });
      document.querySelectorAll('section.group').forEach(group => {
        const visible = Array.from(group.querySelectorAll('li.entry')).some(entry => entry.style.display !== 'none');
        group.style.display = visible ? '' : 'none';
      });
    }
    search.addEventListener('input', applyFilter);
    // The form is for pages without JavaScript; submitting would drop the rows the live filter hides.
    search.form.addEventListener('submit', event => event.preventDefault());
  </script>
</body>
</html>)")
                            .arg(tr("History").toHtmlEscaped(), tr("Search history").toHtmlEscaped(),
                                 tr("Clear history").toHtmlEscaped(),
                                 groups.isEmpty()
                                     ? QStringLiteral("<div class=\"empty\">%1</div>").arg(emptyText.toHtmlEscaped())
                                     : groups.join("\n"),
                                 filter.toHtmlEscaped());

    return html;
}

void MainWindow::renderHistoryPage(WebView *view, const QString &filter)
{
    if (!view) {
        return;
    }
    view->setHtml(buildHistoryPageHtml(filter), QUrl("mx-history://list"));
    view->show();
    tabWidget->setTabTitle(tabWidget->indexOf(view), tr("History"));
    setWindowTitle(tr("History"));
    updateUrl();
}

void MainWindow::openHistoryPage()
{
    if (auto *view = currentWebView()) {
        if (view->url().scheme() == "mx-history") {
            renderHistoryPage(view);
            return;
        }
    }
    auto *view = tabWidget->createTab(true);
    if (!view) {
        return;
    }
    setConnections();
    renderHistoryPage(view);
}

void MainWindow::removeHistoryEntry(qint64 id)
{
    if (privateWindow) {
        return;
    }
    HistoryStore::remove(id);
}

void MainWindow::clearHistoryEntries()
{
    if (privateWindow) {
        return;
    }
    HistoryStore::clear();
}

bool MainWindow::handleHistoryRequest(const QUrl &url)
{
    if (url.scheme() != "mx-history") {
        return false;
    }
    const QString action = url.host();
    const QUrlQuery query(url);
    if (action == "list") {
        renderHistoryPage(currentWebView(), formValue(query, "q"));
        return true;
    }
    if (action == "clear") {
        // Confirmed here rather than in the page, which may run without JavaScript. Deferred, so the
        // dialog's event loop does not run inside the navigation request.
        QTimer::singleShot(0, this, [this] {
            if (QMessageBox::question(this, tr("Clear history"), tr("Clear all history entries?"))
                != QMessageBox::Yes) {
                return;
            }
            clearHistoryEntries();
            refreshHistoryCompleter();
            if (auto *view = currentWebView(); view && view->url().scheme() == "mx-history") {
                renderHistoryPage(view);
            }
        });
        return true;
    }
    if (action != "delete") {
        return false;
    }
    removeHistoryEntry(query.queryItemValue("id").toLongLong());
    refreshHistoryCompleter();
    renderHistoryPage(currentWebView(), formValue(query, "q"));
    return true;
}

void MainWindow::refreshHistoryCompleter()
{
    QStringList completions;
    QStringList hosts;
    QSet<QString> seenHosts;
    // The most recent ones are plenty for completion and keep this quick on every address bar focus.
    constexpr int maxCompletions = 2000;
    const QStringList urls = HistoryStore::recentUrls(maxCompletions);
    completions.reserve(urls.size());
    for (const QString &urlValue : urls) {
        if (urlValue.isEmpty() || urlValue == "about:blank") {
            continue;
        }
        const QUrl url = QUrl::fromUserInput(urlValue);
        if (url.scheme() == "mx-history" || url.scheme() == "mx-settings") {
            continue;
        }
        const QString host = url.host();
        if (!host.isEmpty() && !seenHosts.contains(host)) {
            seenHosts.insert(host);
            hosts.append(host);
            if (host.startsWith("www.", Qt::CaseInsensitive)) {
                const QString stripped = host.mid(4);
                if (!seenHosts.contains(stripped)) {
                    seenHosts.insert(stripped);
                    hosts.append(stripped);
                }
            }
        }
        completions.append(urlValue);
    }
    historyCompletionModel->setStringList(completions);
    historyCompletionHosts = hosts;
}

/*****************************************************************************
 * mainwindow.cpp
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

#include <QAbstractItemView>
#include <QCheckBox>
#include <QCompleter>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSet>
#include <QSpinBox>
#include <QtGlobal>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineView>
#include <QStandardPaths>

namespace {
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

bool removeCachePath(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists()) {
        return false;
    }
    if (info.isFile() || info.isSymLink()) {
        return QFile::remove(path);
    }
    QDir dir(path);
    return dir.removeRecursively();
}
} // namespace

MainWindow::MainWindow(const QCommandLineParser &argParser, QWidget *parent)
    : QMainWindow(parent),
      downloadWidget {new DownloadWidget},
      searchBox {new QLineEdit(this)},
      progressBar {new QProgressBar(this)},
      toolBar {new QToolBar(this)},
      webProfile {new QWebEngineProfile("mx-viewer", this)},
      tabWidget {new TabWidget(webProfile, this)},
      args {&argParser}
{
    init();
    if (argParser.isSet("full-screen")) {
        showFullScreen();
        toolBar->hide();
    }
    QString url;
    QString title;
    if (args && !args->positionalArguments().isEmpty()) {
        url = args->positionalArguments().at(0);
        title = (args->positionalArguments().size() > 1) ? args->positionalArguments().at(1) : url;
    }
    if (!restoredTabs) {
        displaySite(url, title);
    } else if (!url.isEmpty()) {
        addNewTab(QUrl::fromUserInput(url), true);
    }
}

MainWindow::MainWindow(const QUrl &url, QWidget *parent)
    : QMainWindow(parent),
      downloadWidget {new DownloadWidget},
      searchBox {new QLineEdit(this)},
      progressBar {new QProgressBar(this)},
      toolBar {new QToolBar(this)},
      webProfile {new QWebEngineProfile("mx-viewer", this)},
      tabWidget {new TabWidget(webProfile, this)},
      args {nullptr}
{
    init();
    if (!restoredTabs) {
        displaySite(url.toString(), QString());
    }
}

void MainWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose);
    toolBar->toggleViewAction()->setVisible(false);
    connect(tabWidget, &TabWidget::currentChanged, this, [this] { tabChanged(); });
    connect(tabWidget, &TabWidget::newTabButtonClicked, this, [this] { addNewTab(); });
    connect(tabWidget, &TabWidget::tabClosed, this, [this](const QUrl &url) {
        if (url.isValid()) {
            QIcon icon;
            if (auto *view = currentWebView()) {
                if (view->url() == url) {
                    icon = view->icon();
                }
            }
            if (icon.isNull()) {
                for (int i = 0; i < tabWidget->count(); ++i) {
                    if (auto *view = qobject_cast<WebView *>(tabWidget->widget(i))) {
                        if (view->url() == url) {
                            icon = view->icon();
                            break;
                        }
                    }
                }
            }
            closedTabs.append({url, icon});
        }
    });
    websettings = webProfile->settings();
    loadSettings();
    addToolbar();
    addActions();
    setConnections();

    restoredTabs = settings.value("SaveTabs", false).toBool() && restoreSavedTabs();

    auto *closeTabAction = new QAction(this);
    closeTabAction->setShortcut(QKeySequence::Close);
    connect(closeTabAction, &QAction::triggered, this, &MainWindow::closeCurrentTab);
    addAction(closeTabAction);
    auto *reopenTabAction = new QAction(this);
    reopenTabAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    connect(reopenTabAction, &QAction::triggered, this, &MainWindow::reopenClosedTab);
    addAction(reopenTabAction);
}

MainWindow::~MainWindow()
{
    settings.setValue("Geometry", saveGeometry());
    saveMenuItems(bookmarks, 2);
}

void MainWindow::addActions()
{
    auto *full = new QAction(tr("Full screen"));
    full->setShortcut(Qt::Key_F11);
    addAction(full);
    connect(full, &QAction::triggered, this, &MainWindow::toggleFullScreen);
}

void MainWindow::addBookmarksSubmenu()
{
    bookmarks->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookmarks, &QMenu::customContextMenuRequested, this, [this](QPoint pos) {
        QAction *currentAction = bookmarks->actionAt(pos);
        if (!currentAction || !currentAction->property("url").isValid()) {
            return;
        }
        QPoint globalPos = bookmarks->mapToGlobal(pos);
        QMenu submenu;
        QList<QAction *> bookmarkActions;
        for (auto *action : bookmarks->actions()) {
            if (action->property("url").isValid()) {
                bookmarkActions.append(action);
            }
        }
        const int currentIndex = bookmarkActions.indexOf(currentAction);
        if (currentIndex > 0) {
            submenu.addAction(QIcon::fromTheme("arrow-up"), tr("Move up"), bookmarks, [this, pos] {
                QAction *action = bookmarks->actionAt(pos);
                if (!action || !action->property("url").isValid()) {
                    return;
                }
                QList<QAction *> actions;
                for (auto *entry : bookmarks->actions()) {
                    if (entry->property("url").isValid()) {
                        actions.append(entry);
                    }
                }
                const int index = actions.indexOf(action);
                if (index > 0) {
                    bookmarks->insertAction(actions.at(index - 1), action);
                }
            });
        }
        if (currentIndex >= 0 && currentIndex < bookmarkActions.count() - 1) {
            submenu.addAction(QIcon::fromTheme("arrow-down"), tr("Move down"), bookmarks, [this, pos] {
                QAction *action = bookmarks->actionAt(pos);
                if (!action || !action->property("url").isValid()) {
                    return;
                }
                QList<QAction *> actions;
                for (auto *entry : bookmarks->actions()) {
                    if (entry->property("url").isValid()) {
                        actions.append(entry);
                    }
                }
                const int index = actions.indexOf(action);
                if (index >= 0 && index < actions.count() - 1) {
                    QAction *insertBefore = (index + 2 < actions.count()) ? actions.at(index + 2) : nullptr;
                    bookmarks->insertAction(insertBefore, action);
                }
            });
        }
        submenu.addAction(QIcon::fromTheme("edit-symbolic"), tr("Rename"), bookmarks, [this, pos] {
            QInputDialog edit(this);
            edit.setInputMode(QInputDialog::TextInput);
            edit.setOkButtonText(tr("Save"));
            edit.setTextValue(bookmarks->actionAt(pos)->text());
            edit.setLabelText(tr("Rename bookmark:"));
            edit.resize(300, edit.height());
            if (edit.exec() == QDialog::Accepted) {
                bookmarks->actionAt(pos)->setText(edit.textValue());
            }
        });
        submenu.addAction(QIcon::fromTheme("user-trash"), tr("Delete"), bookmarks,
                          [this, pos] { bookmarks->removeAction(bookmarks->actionAt(pos)); });
        submenu.exec(globalPos);
    });
}

void MainWindow::addHistorySubmenu()
{
    history->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(history, &QMenu::customContextMenuRequested, this, [this](QPoint pos) {
        if (history->actionAt(pos) == history->actions().at(0)) { // skip first "Clear history" action.
            return;
        }
        QPoint globalPos = history->mapToGlobal(pos);
        QMenu submenu;
        submenu.addAction(QIcon::fromTheme("user-trash"), tr("Delete"), history, [this, pos] {
            history->removeAction(history->actionAt(pos));
            saveMenuItems(history, 3); // skip "clear history", separator, and first item added at menu refresh
        });
        submenu.exec(globalPos);
    });
}

void MainWindow::addNewTab(const QUrl &url, bool makeCurrent)
{
    WebView *view = tabWidget->createTab(makeCurrent);
    if (!view) {
        return;
    }
    if (makeCurrent) {
        setConnections();
    }
    QUrl finalUrl = url;
    if (finalUrl.isEmpty() && openNewTabWithHome) {
        finalUrl = QUrl::fromUserInput(homeAddress);
    }
    if (finalUrl.isEmpty()) {
        finalUrl = QUrl("about:blank");
    }
    view->setUrl(finalUrl);
    view->show();
    if (makeCurrent) {
        QTimer::singleShot(0, this, &MainWindow::focusAddressBarIfBlank);
        QMetaObject::Connection once;
        once = connect(view, &QWebEngineView::loadFinished, this, [this, view, once](bool) mutable {
            if (view == currentWebView()) {
                focusAddressBarIfBlank();
            }
            disconnect(once);
        });
    }
}

void MainWindow::listHistory()
{
    history->clear();
    auto *showHistory = new QAction(QIcon::fromTheme("view-list-text"), tr("History"));
    showHistory->setShortcut(Qt::CTRL | Qt::Key_H);
    connect(showHistory, &QAction::triggered, this, &MainWindow::openHistoryPage);
    history->addAction(showHistory);
    history->addSeparator();
    auto *recentTitle = new QWidgetAction(history);
    auto *recentLabel = new QLabel(tr("Recent tabs"), history);
    QFont recentFont = recentLabel->font();
    recentFont.setUnderline(true);
    recentFont.setBold(true);
    recentLabel->setFont(recentFont);
    recentLabel->setStyleSheet("color: #4a4a4a; padding: 4px 18px 4px 18px;");
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
    refreshHistoryCompleter();
}

QString MainWindow::buildHistoryPageHtml()
{
    struct HistoryEntry {
        QString title;
        QString url;
        QByteArray icon;
    };

    QList<HistoryEntry> entries;
    int size = settings.beginReadArray("History");
    entries.reserve(size);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        const QString url = settings.value("url").toString();
        if (url.isEmpty()) {
            continue;
        }
        QString title = settings.value("title").toString();
        if (title.isEmpty()) {
            title = url;
        }
        entries.append({title, url, settings.value("icon").toByteArray()});
    }
    settings.endArray();

    QStringList rows;
    rows.reserve(entries.size());
    for (int i = 0; i < entries.size(); ++i) {
        const HistoryEntry &entry = entries.at(i);
        const QString titleEscaped = entry.title.toHtmlEscaped();
        const QString urlEscaped = entry.url.toHtmlEscaped();
        const QString searchText = (entry.title + " " + entry.url).toLower().toHtmlEscaped();
        QString iconHtml;
        if (!entry.icon.isEmpty()) {
            const QString iconBase64 = QString::fromLatin1(entry.icon.toBase64());
            iconHtml = QStringLiteral("<img class=\"icon\" alt=\"\" src=\"data:image/png;base64,%1\">")
                           .arg(iconBase64);
        } else {
            iconHtml = QStringLiteral("<span class=\"icon placeholder\"></span>");
        }
        rows.append(QStringLiteral(
                        "<li class=\"entry\" data-search=\"%1\">"
                        "%2"
                        "<div class=\"content\">"
                        "<div class=\"row\">"
                        "<a class=\"title\" href=\"%3\">%4</a>"
                        "<button class=\"delete\" data-index=\"%5\">%6</button>"
                        "</div>"
                        "<div class=\"url\">%7</div>"
                        "</div>"
                        "</li>")
                        .arg(searchText, iconHtml, urlEscaped, titleEscaped, QString::number(i),
                             tr("Delete").toHtmlEscaped(), urlEscaped));
    }

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
    .search { flex: 1 1 240px; padding: 8px 10px; border: 1px solid #d0d7de; border-radius: 6px; }
    .clear { padding: 8px 12px; border: 1px solid #d0d7de; background: #f6f8fa; border-radius: 6px; cursor: pointer; }
    .list { list-style: none; padding: 0; margin: 0; display: flex; flex-direction: column; gap: 10px; }
    .entry { display: grid; grid-template-columns: 24px 1fr; gap: 10px; padding: 10px 12px; border: 1px solid #eaeef2; border-radius: 8px; }
    .icon { width: 20px; height: 20px; border-radius: 4px; }
    .icon.placeholder { background: #eaeef2; }
    .content { display: flex; flex-direction: column; gap: 4px; }
    .row { display: flex; align-items: center; gap: 10px; }
    .title { color: #0969da; text-decoration: none; font-weight: 600; flex: 1 1 auto; }
    .url { color: #57606a; font-size: 12px; word-break: break-all; }
    .delete { padding: 4px 8px; border: 1px solid #d0d7de; background: #fff; border-radius: 6px; cursor: pointer; }
    .empty { padding: 16px; border: 1px dashed #d0d7de; border-radius: 8px; color: #57606a; }
  </style>
</head>
<body>
  <h1>%1</h1>
  <div class="controls">
    <input id="search" class="search" type="search" placeholder="%2" autofocus>
    <button id="clear" class="clear">%3</button>
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
    }
    search.addEventListener('input', applyFilter);
    document.querySelectorAll('button.delete').forEach(btn => {
      btn.addEventListener('click', event => {
        event.preventDefault();
        location.href = 'mx-history://delete?index=' + btn.dataset.index;
      });
    });
    const clearButton = document.getElementById('clear');
    clearButton.addEventListener('click', event => {
      event.preventDefault();
      if (confirm('%5')) {
        location.href = 'mx-history://clear';
      }
    });
  </script>
</body>
</html>)")
                            .arg(tr("History").toHtmlEscaped(), tr("Search history").toHtmlEscaped(),
                                 tr("Clear history").toHtmlEscaped(),
                                 rows.isEmpty()
                                     ? QStringLiteral("<div class=\"empty\">%1</div>").arg(emptyText.toHtmlEscaped())
                                     : QStringLiteral("<ul class=\"list\">%1</ul>").arg(rows.join("\n")),
                                 tr("Clear all history entries?").toHtmlEscaped());

    return html;
}

void MainWindow::renderHistoryPage(WebView *view)
{
    if (!view) {
        return;
    }
    view->setHtml(buildHistoryPageHtml(), QUrl("mx-history://list"));
    view->show();
    tabWidget->setTabText(tabWidget->indexOf(view), tr("History"));
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

void MainWindow::removeHistoryEntry(int index)
{
    if (index < 0) {
        return;
    }
    struct HistoryEntry {
        QString title;
        QString url;
        QByteArray icon;
    };
    QList<HistoryEntry> entries;
    int size = settings.beginReadArray("History");
    entries.reserve(size);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        const QString url = settings.value("url").toString();
        if (url.isEmpty()) {
            continue;
        }
        entries.append({settings.value("title").toString(), url, settings.value("icon").toByteArray()});
    }
    settings.endArray();
    if (index >= entries.size()) {
        return;
    }
    entries.removeAt(index);
    settings.beginWriteArray("History");
    for (int i = 0; i < entries.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("title", entries.at(i).title);
        settings.setValue("url", entries.at(i).url);
        if (!entries.at(i).icon.isEmpty()) {
            settings.setValue("icon", entries.at(i).icon);
        }
    }
    settings.endArray();
    settings.setValue("History/size", entries.size());
}

void MainWindow::clearHistoryEntries()
{
    settings.remove("History");
    settings.setValue("History/size", 0);
}

bool MainWindow::handleHistoryRequest(const QUrl &url)
{
    if (url.scheme() != "mx-history") {
        return false;
    }
    const QString action = url.host();
    if (action == "delete") {
        QUrlQuery query(url);
        removeHistoryEntry(query.queryItemValue("index").toInt());
    } else if (action == "clear") {
        clearHistoryEntries();
    } else {
        return false;
    }
    refreshHistoryCompleter();
    renderHistoryPage(currentWebView());
    return true;
}

void MainWindow::refreshHistoryCompleter()
{
    QStringList completions;
    QStringList hosts;
    QSet<QString> seenUrls;
    QSet<QString> seenHosts;
    int size = settings.beginReadArray("History");
    completions.reserve(size);
    for (int i = size - 1; i >= 0; --i) {
        settings.setArrayIndex(i);
        const QString urlValue = settings.value("url").toString();
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
        if (!seenUrls.contains(urlValue)) {
            seenUrls.insert(urlValue);
            completions.append(urlValue);
        }
    }
    settings.endArray();
    historyCompletionModel->setStringList(completions);
    historyCompletionHosts = hosts;
}

void MainWindow::addToolbar()
{
    addToolBar(toolBar);
    setCentralWidget(tabWidget);
    addNavigationActions();
    addHomeAction();
    setupAddressBar();
    setupSearchBox();
    addZoomActions();
    setupMenuButton();
    buildMenu();
    toolBar->show();
}

void MainWindow::addNavigationActions()
{
    auto *back = pageAction(QWebEnginePage::Back);
    auto *forward = pageAction(QWebEnginePage::Forward);
    auto *reload = pageAction(QWebEnginePage::Reload);
    auto *stop = pageAction(QWebEnginePage::Stop);
    toolBar->addAction(back);
    toolBar->addAction(forward);
    reloadAction = new QAction(reload->icon(), reload->text(), this);
    reloadAction->setShortcutContext(Qt::ApplicationShortcut);
    toolBar->addAction(reloadAction);
    toolBar->addAction(stop);
    back->setShortcut(QKeySequence::Back);
    forward->setShortcut(QKeySequence::Forward);
    reloadAction->setShortcuts(QKeySequence::Refresh);
    stop->setShortcut(QKeySequence::Cancel);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reloadCurrentView);
    connect(stop, &QAction::triggered, this, [this] { done(true); });
}

void MainWindow::addHomeAction()
{
    auto *home {new QAction(QIcon::fromTheme("go-home", QIcon(":/icons/go-home.svg")), tr("Home"))};
    toolBar->addAction(home);
    home->setShortcut(Qt::ALT | Qt::Key_Home);
    connect(home, &QAction::triggered, this, [this] { displaySite(); });
}

void MainWindow::setupAddressBar()
{
    addressBar = new AddressBar(this);
    addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addressBar->setClearButtonEnabled(true);
    historyCompletionModel = new QStringListModel(this);
    historyCompleter = new QCompleter(historyCompletionModel, this);
    historyCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    historyCompleter->setCompletionMode(QCompleter::PopupCompletion);
    historyCompleter->setFilterMode(Qt::MatchContains);
    addressBar->setCompleter(historyCompleter);
    connect(historyCompleter, QOverload<const QString &>::of(&QCompleter::activated), this,
            [this](const QString &text) {
                addressBar->setText(text);
                openFromAddressBarText(text);
            });
    connect(addressBar, &AddressBar::focused, this, [this] {
        lastAddressEditLength = addressBar->text().size();
        refreshHistoryCompleter();
    });
    connect(addressBar, &AddressBar::keyPressed, this, [this](int key) {
        lastAddressEditWasDeletion = (key == Qt::Key_Backspace || key == Qt::Key_Delete);
    });
    connect(addressBar, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (completingHistory) {
            return;
        }
        if (addressBar->cursorPosition() < text.size()) {
            return;
        }
        if (lastAddressEditWasDeletion) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty() || trimmed.contains(' ')) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        if (historyCompletionHosts.isEmpty()) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        QString prefix;
        QString hostInput = trimmed;
        const int schemeIndex = trimmed.indexOf("://");
        if (schemeIndex >= 0) {
            prefix = trimmed.left(schemeIndex + 3);
            hostInput = trimmed.mid(schemeIndex + 3);
        }
        const int pathIndex = hostInput.indexOf('/');
        if (pathIndex >= 0) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        QString match;
        for (const QString &entry : historyCompletionHosts) {
            if (entry.startsWith(hostInput, Qt::CaseInsensitive)) {
                match = entry;
                break;
            }
        }
        if (match.isEmpty() || match.compare(hostInput, Qt::CaseInsensitive) == 0) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        completingHistory = true;
        addressBar->setText(prefix + match);
        addressBar->setSelection(prefix.size() + hostInput.size(), match.size() - hostInput.size());
        lastAddressEditLength = addressBar->text().size();
        completingHistory = false;
        lastAddressEditWasDeletion = false;
    });
    refreshHistoryCompleter();
    addBookmark = addressBar->addAction(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/emblem-favorite.png")),
                                        QLineEdit::TrailingPosition);
    addBookmark->setToolTip(tr("Add bookmark"));
    connect(addressBar, &QLineEdit::returnPressed, this, &MainWindow::openFromAddressBar);
    toolBar->addWidget(addressBar);
}

void MainWindow::setupSearchBox()
{
    searchBox->setPlaceholderText(tr("search in page"));
    searchBox->setClearButtonEnabled(true);
    searchBox->setMaximumWidth(searchWidth);
    searchBox->addAction(QIcon::fromTheme("search", QIcon(":/icons/system-search.png")), QLineEdit::LeadingPosition);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::findForward);
    connect(searchBox, &QLineEdit::returnPressed, this, &MainWindow::findForward);
    toolBar->addWidget(searchBox);
}

void MainWindow::addZoomActions()
{
    auto *zoomout {new QAction(QIcon::fromTheme("zoom-out", QIcon(":/icons/zoom-out.svg")), tr("Zoom out"))};
    zoomPercentAction = new QAction("100%");
    auto *zoomin {new QAction(QIcon::fromTheme("zoom-in", QIcon(":/icons/zoom-in.svg")), tr("Zoom In"))};
    toolBar->addAction(zoomout);
    toolBar->addAction(zoomPercentAction);
    toolBar->addAction(zoomin);
    zoomin->setShortcuts({QKeySequence::ZoomIn, Qt::CTRL | Qt::Key_Equal});
    zoomout->setShortcut(QKeySequence::ZoomOut);
    zoomPercentAction->setShortcut(Qt::CTRL | Qt::Key_0);
    connect(zoomout, &QAction::triggered, this, [this] {
        setZoomPercent(zoomPercent - 10, true);
    });
    connect(zoomin, &QAction::triggered, this, [this] {
        setZoomPercent(zoomPercent + 10, true);
    });
    connect(zoomPercentAction, &QAction::triggered, this, [this] {
        setZoomPercent(100, true);
    });
    setZoomPercent(zoomPercent, false);
}

void MainWindow::setupMenuButton()
{
    menuButton = new QAction(QIcon::fromTheme("open-menu", QIcon(":/icons/open-menu.png")), tr("Settings"));
    toolBar->addAction(menuButton);
    menuButton->setShortcut(Qt::Key_F10);
}

void MainWindow::openBrowseDialog()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select file to open"), QDir::homePath(),
                                                tr("Hypertext Files (*.htm *.html);;All Files (*.*)"));
    if (QFileInfo::exists(file)) {
        displaySite(file, file);
    }
}

// Display a URL in the current view.
void MainWindow::displaySite(QString url, const QString &title)
{
    if (url.isEmpty()) {
        if (tabWidget->currentIndex() == 0) {
            url = homeAddress;
        } else {
            return;
        }
    }
    if (QFile::exists(url)) {
        url = QFileInfo(url).absoluteFilePath();
    }
    QUrl qurl = QUrl::fromUserInput(url);
    auto *view = currentWebView();
    if (!view) {
        progressBar->hide();
        setWindowTitle(title);
        return;
    }
    view->setUrl(qurl);
    view->show();
    showProgress ? loading() : progressBar->hide();
    setWindowTitle(title);
}

void MainWindow::loadBookmarks()
{
    int size = settings.beginReadArray("Bookmarks");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark
                             = new QAction(settings.value("icon").value<QIcon>(), settings.value("title").toString()));
        bookmark->setProperty("url", settings.value("url"));
        connectAddress(bookmark, bookmarks);
    }
    settings.endArray();
}

void MainWindow::loadHistory()
{
    int size = settings.beginReadArray("History");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *histItem {nullptr};
        QIcon restoredIcon;
        QByteArray iconByteArray = settings.value("icon").toByteArray();
        if (!iconByteArray.isEmpty()) {
            QPixmap restoredIconPixmap;
            if (restoredIconPixmap.loadFromData(iconByteArray)) {
                restoredIcon.addPixmap(restoredIconPixmap);
            }
        }
        history->addAction(histItem = new QAction(restoredIcon, settings.value("title").toString()));
        histItem->setProperty("url", settings.value("url"));
        connectAddress(histItem, history);
    }
    settings.endArray();
}

void MainWindow::loadSettings()
{
    // Load first from system .conf file and then overwrite with CLI switches where available
    websettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    websettings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    webProfile->setHttpAcceptLanguage(QLocale::system().name());

    homeAddress = settings.value("Home", "https://start.duckduckgo.com").toString();
    showProgress = settings.value("ShowProgressBar", false).toBool();
    openNewTabWithHome = settings.value("OpenNewTabWithHome", true).toBool();
    zoomPercent = settings.value("ZoomPercent", 100).toInt();
    cookiesEnabled = settings.value("EnableCookies", true).toBool();
    clearCookiesAtExit = settings.value("ClearCookiesAtExit", false).toBool();
    searchEngine = settings.value("SearchEngine", "DuckDuckGo").toString();
    searchEngineCustom = settings.value("SearchEngineCustom", QString()).toString();
    if (!settings.contains("EnableJavaScript") && settings.contains("DisableJava")) {
        settings.setValue("EnableJavaScript", !settings.value("DisableJava", false).toBool());
    }

    applyWebSettings();

    QSize size {defaultWidth, defaultHeight};
    const bool canRestore = settings.contains("Geometry") && (!args || !args->isSet("full-screen"));
    if (canRestore) {
        const bool restored = restoreGeometry(settings.value("Geometry").toByteArray());
        if (!restored) {
            resize(size);
            centerWindow();
        }
    } else {
        resize(size);
        centerWindow();
    }
}

void MainWindow::centerWindow()
{
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::openQuickInfo()
{
    QMessageBox::about(this, tr("Keyboard Shortcuts"),
                       tr("Ctrl-F, or F3") + "\t - " + tr("Find") + "\n" + tr("Shift-F3") + "\t - "
                           + tr("Find previous") + "\n" + tr("Ctrl-R, or F5") + "\t - " + tr("Reload") + "\n"
                           + tr("Ctrl-H") + "\t - " + tr("History") + "\n"
                           + tr("Ctrl-O") + "\t - " + tr("Browse file to open") + "\n" + tr("Esc") + "\t - "
                           + tr("Stop loading/clear Find field") + "\n" + tr("Alt→, Alt←") + "\t - "
                           + tr("Back/Forward") + "\n" + tr("F1, or ?") + "\t - " + tr("Open this help dialog"));
}

bool MainWindow::isLocalHostInput(const QString &input) const
{
    const QString lower = input.toLower();
    if (lower == "localhost") {
        return true;
    }
    if (lower == "localhost.localdomain") {
        return true;
    }
    if (lower.endsWith(".local")) {
        return true;
    }
    if (lower == "127.0.0.1" || lower == "::1") {
        return true;
    }
    return false;
}

QString MainWindow::searchUrlForQuery(const QString &query) const
{
    const QByteArray encoded = QUrl::toPercentEncoding(query);
    if (searchEngine == "Custom" && !searchEngineCustom.isEmpty()) {
        const QString encodedText = QString::fromLatin1(encoded);
        if (searchEngineCustom.contains("%s")) {
            return QString(searchEngineCustom).replace("%s", encodedText);
        }
        if (searchEngineCustom.contains("?q=")) {
            return searchEngineCustom + encodedText;
        }
        return searchEngineCustom;
    }
    if (searchEngine == "Google") {
        return QString::fromLatin1("https://www.google.com/search?q=%1").arg(QString::fromLatin1(encoded));
    }
    if (searchEngine == "Bing") {
        return QString::fromLatin1("https://www.bing.com/search?q=%1").arg(QString::fromLatin1(encoded));
    }
    return QString::fromLatin1("https://duckduckgo.com/?q=%1").arg(QString::fromLatin1(encoded));
}

void MainWindow::displaySearchResults(const QString &query)
{
    const QString searchUrl = searchUrlForQuery(query);
    lastAddressMaySearch = false;
    displaySite(searchUrl, query);
}

void MainWindow::openFromAddressBar()
{
    openFromAddressBarText(addressBar->text());
}

void MainWindow::openFromAddressBarText(const QString &inputText)
{
    const QString input = inputText.trimmed();
    if (input.isEmpty()) {
        return;
    }
    if (input.startsWith("http://", Qt::CaseInsensitive) || input.startsWith("https://", Qt::CaseInsensitive)) {
        lastAddressInput = input;
        lastAddressUrl = QUrl::fromUserInput(input);
        lastAddressMaySearch = false;
        lastAddressExplicitScheme = true;
        displaySite(input);
        return;
    }
    if (QFile::exists(input)) {
        displaySite(input, input);
        return;
    }
    const bool hasExplicitScheme = input.contains("://");
    const bool hasSpace = input.contains(' ');
    const bool hasDot = input.contains('.');
    if (hasSpace || (!hasDot && !hasExplicitScheme && !isLocalHostInput(input))) {
        displaySearchResults(input);
        return;
    }
    QUrl qurl = QUrl::fromUserInput(input);
    lastAddressInput = input;
    lastAddressUrl = qurl;
    lastAddressMaySearch = true;
    lastAddressExplicitScheme = hasExplicitScheme;
    displaySite(input);
}

void MainWindow::saveMenuItems(const QMenu *menu, int offset)
{
    // Offset is for skipping "Clear history" item, separator, etc.
    settings.beginWriteArray(menu->objectName());
    if (menu->objectName() == "Bookmarks") {
        int index = 0;
        for (auto *action : menu->actions()) {
            if (!action->property("url").isValid()) {
                continue;
            }
            settings.setArrayIndex(index++);
            settings.setValue("title", action->text());
            settings.setValue("url", action->property("url").toString());
            settings.setValue("icon", action->icon());
        }
    } else {
        for (int i = offset; i < menu->actions().count(); ++i) {
            settings.setArrayIndex(i - offset);
            settings.setValue("title", menu->actions().at(i)->text());
            settings.setValue("url", menu->actions().at(i)->property("url").toString());

            QPixmap iconPixmap = menu->actions().at(i)->icon().pixmap(QSize(16, 16));
            QByteArray iconByteArray;
            QBuffer buffer(&iconByteArray);
            buffer.open(QIODevice::WriteOnly);
            iconPixmap.save(&buffer, "PNG");
            settings.setValue("icon", iconByteArray);
        }
    }
    settings.endArray();
}

void MainWindow::setConnections()
{
    if (!currentWebView()) {
        return;
    }
    websettings = currentWebView()->settings();
    applyWebSettings();
    if (loadStartedConn) {
        disconnect(loadStartedConn);
    }
    loadStartedConn = connect(currentWebView(), &QWebEngineView::loadStarted, toolBar, &QToolBar::show);
    if (loadingConn) {
        disconnect(loadingConn);
    }
    if (showProgress) {
        loadingConn = connect(currentWebView(), &QWebEngineView::loadStarted, this, &MainWindow::loading);
    }
    if (urlChangedConn) {
        disconnect(urlChangedConn);
    }
    urlChangedConn = connect(currentWebView(), &QWebEngineView::urlChanged, this, &MainWindow::updateUrl);
    connect(webProfile, &QWebEngineProfile::downloadRequested, downloadWidget,
            &DownloadWidget::downloadRequested, Qt::UniqueConnection);
    if (loadFinishedConn) {
        disconnect(loadFinishedConn);
    }
    loadFinishedConn = connect(currentWebView(), &QWebEngineView::loadFinished, this, &MainWindow::done);
    if (linkHoveredConn) {
        disconnect(linkHoveredConn);
    }
    linkHoveredConn = connect(currentWebView()->page(), &QWebEnginePage::linkHovered, this, [this](const QString &url) {
        if (url.isEmpty()) {
            statusBar()->hide();
        } else {
            statusBar()->show();
            statusBar()->showMessage(url);
        }
    });
    setZoomPercent(zoomPercent, false);
}

void MainWindow::showFullScreenNotification()
{
    constexpr int distanceTop = 100;
    constexpr int durationMs = 800;
    constexpr double start = 0;
    constexpr double end = 0.85;
    auto *label = new QLabel(this);
    auto *effect = new QGraphicsOpacityEffect;
    label->setGraphicsEffect(effect);
    label->setStyleSheet("padding: 15px; background-color:#787878; color:white");
    label->setText(tr("Press [F11] to exit full screen"));
    label->adjustSize();
    label->move(QApplication::primaryScreen()->geometry().width() / 2 - label->width() / 2, distanceTop);
    auto *a = new QPropertyAnimation(effect, "opacity");
    a->setDuration(durationMs);
    a->setStartValue(start);
    a->setEndValue(end);
    a->setEasingCurve(QEasingCurve::InBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    label->show();
    QTimer::singleShot(4000, this, [label, effect, end, start] {
        auto *a = new QPropertyAnimation(effect, "opacity");
        a->setDuration(durationMs);
        a->setStartValue(end);
        a->setEndValue(start);
        a->setEasingCurve(QEasingCurve::OutBack);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        connect(a, &QPropertyAnimation::finished, label, &QLabel::deleteLater);
    });
}

void MainWindow::tabChanged()
{
    if (!currentWebView()) {
        return;
    }
    auto *back = pageAction(QWebEnginePage::Back);
    auto *forward = pageAction(QWebEnginePage::Forward);
    auto *reload = pageAction(QWebEnginePage::Reload);
    auto *stop = pageAction(QWebEnginePage::Stop);
    QMap<QString, QAction *> actionMap = {{"Back", back}, {"Forward", forward}, {"Reload", reloadAction}, {"Stop", stop}};
    auto actionList = toolBar->actions();
    toolBar->setUpdatesEnabled(false);
    for (int i = 0; i < actionList.size() - 1; ++i) {
        auto *currentAction = actionList.at(i);
        auto *nextAction = actionList.at(i + 1);
        auto it = actionMap.find(currentAction->text());
        if (it != actionMap.end()) {
            QAction *replacementAction = it.value();
            toolBar->removeAction(currentAction);
            toolBar->insertAction(nextAction, replacementAction);
        }
    }
    toolBar->setUpdatesEnabled(true);
    if (reloadAction) {
        reloadAction->setIcon(reload->icon());
        reloadAction->setText(reload->text());
        reloadAction->setToolTip(reload->toolTip());
        reloadAction->setEnabled(reload->isEnabled());
    }
    addressBar->setText(currentWebView()->url().toString());
    if (addressBar->text().isEmpty()) {
        addressBar->setFocus();
    }
    setWindowTitle(currentWebView()->title());
    setConnections();
    if (devToolsWindow && devToolsView) {
        currentWebView()->page()->setDevToolsPage(devToolsView->page());
    }
}

// Show the hovered URL in the status bar and connect it to launch it.
void MainWindow::connectAddress(const QAction *action, const QMenu *menu)
{
    connect(action, &QAction::hovered, this, [this, action] {
        QString url = action->property("url").toString();
        if (url.isEmpty()) {
            statusBar()->hide();
        } else {
            statusBar()->show();
            statusBar()->showMessage(url);
        }
    });
    connect(action, &QAction::triggered, this, [this, action] {
        QString url = action->property("url").toString();
        displaySite(url);
    });
    connect(menu, &QMenu::aboutToHide, statusBar(), &QStatusBar::hide);
}

void MainWindow::buildMenu()
{
    auto *menu = new QMenu(this);
    history = new QMenu(menu);
    bookmarks = new QMenu(menu);
    bookmarks->setObjectName("Bookmarks");
    history->setStyleSheet("QMenu { menu-scrollable: 1; }");
    history->setObjectName("History");
    bookmarks->setStyleSheet("QMenu { menu-scrollable: 1; }");
    menuButton->setMenu(menu);

    addFileMenuActions(menu);
    addViewMenuActions(menu);
    addHelpMenuActions(menu);

    loadBookmarks();
    addBookmarksSubmenu();

    setupMenuConnections(menu);
}

void MainWindow::addFileMenuActions(QMenu *menu)
{
    QAction *newTab {nullptr};
    menu->addAction(newTab = new QAction(QIcon::fromTheme("tab-new"), tr("&New tab")));
    newTab->setShortcut(Qt::CTRL | Qt::Key_T);
    connect(newTab, &QAction::triggered, this, [this] { addNewTab(); });
}

void MainWindow::addViewMenuActions(QMenu *menu)
{
    QAction *fullScreen {nullptr};
    QAction *devTools {nullptr};
    QAction *historyAction {nullptr};
    QAction *downloadAction {nullptr};
    QAction *bookmarkAction {nullptr};
    QAction *manageBookmarks {nullptr};
    menu->addAction(fullScreen = new QAction(QIcon::fromTheme("view-fullscreen"), tr("&Full screen")));
    menu->addSeparator();
    menu->addAction(devTools = new QAction(QIcon::fromTheme("applications-development"), tr("&Developer Tools")));
    devTools->setShortcut(Qt::Key_F12);
    menu->addAction(historyAction = new QAction(QIcon::fromTheme("history"), tr("H&istory")));
    historyAction->setMenu(history);
    menu->addAction(downloadAction = new QAction(QIcon::fromTheme("folder-download"), tr("&Downloads")));
    downloadAction->setShortcut(Qt::CTRL | Qt::Key_J);
    menu->addAction(bookmarkAction = new QAction(QIcon::fromTheme("emblem-favorite"), tr("&Bookmarks")));
    bookmarkAction->setMenu(bookmarks);
    bookmarks->addAction(addBookmark);
    addBookmark->setText(tr("Bookmark current address"));
    addBookmark->setShortcut(Qt::CTRL | Qt::Key_D);
    bookmarks->addAction(manageBookmarks = new QAction(QIcon::fromTheme("document-edit"), tr("Manage &bookmarks")));
    manageBookmarks->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    bookmarks->addSeparator();
    connect(fullScreen, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    connect(devTools, &QAction::triggered, this, &MainWindow::openDevTools);
    connect(downloadAction, &QAction::triggered, downloadWidget, &QWidget::show);
    connect(manageBookmarks, &QAction::triggered, this, &MainWindow::openBookmarksEditor);
    connect(addBookmark, &QAction::triggered, this, [this] {
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark = new QAction(currentWebView()->icon(), currentWebView()->title()));
        bookmark->setProperty("url", currentWebView()->url());
        connectAddress(bookmark, bookmarks);
    });
}

void MainWindow::addHelpMenuActions(QMenu *menu)
{
    QAction *settingsAction {nullptr};
    QAction *help {nullptr};
    QAction *about {nullptr};
    QAction *quit {nullptr};
    menu->addSeparator();
    menu->addAction(settingsAction = new QAction(QIcon::fromTheme("preferences-system"), tr("&Settings")));
    settingsAction->setShortcuts({Qt::CTRL | Qt::Key_Comma, QKeySequence::Preferences});
    menu->addSeparator();
    menu->addAction(help = new QAction(QIcon::fromTheme("help-contents"), tr("&Help")));
    menu->addAction(about = new QAction(QIcon::fromTheme("help-about"), tr("&About")));
    menu->addSeparator();
    menu->addAction(quit = new QAction(QIcon::fromTheme("window-close"), tr("&Exit")));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    connect(help, &QAction::triggered, this, &MainWindow::openQuickInfo);
    connect(quit, &QAction::triggered, this, &MainWindow::close);
    connect(about, &QAction::triggered, this, [this] {
        QMessageBox::about(this, tr("About MX Viewer"),
                           tr("This is a VERY basic browser based on Qt WebEngine.\n\n"
                              "The main purpose is to provide a basic document viewer for MX documentation. "
                              "It could be used for LIMITED internet browsing, but it's not recommended to be "
                              "used for anything important or secure because it's not a fully featured browser "
                              "and its security/privacy features were not tested.\n\n"
                              "This program is free software: you can redistribute it and/or modify "
                              "it under the terms of the GNU General Public License as published by "
                              "the Free Software Foundation, either version 3 of the License, or "
                              "(at your option) any later version.\n\n"
                              "MX Viewer is distributed in the hope that it will be useful, "
                              "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                              "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
                              "GNU General Public License for more details.\n\n"
                              "You should have received a copy of the GNU General Public License "
                              "along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>."));
    });
}

void MainWindow::setupMenuConnections(QMenu *menu)
{
    connect(menuButton, &QAction::triggered, this, [this, menu] {
        QPoint pos = mapToParent(toolBar->widgetForAction(menuButton)->pos());
        pos.setY(pos.y() + toolBar->widgetForAction(menuButton)->size().height());
        menu->popup(pos);
        listHistory();
    });
}

void MainWindow::openDevTools()
{
    if (!currentWebView()) {
        return;
    }
    if (!devToolsWindow) {
        devToolsWindow = new QMainWindow(this);
        devToolsWindow->setAttribute(Qt::WA_DeleteOnClose);
        devToolsWindow->setWindowTitle(tr("Developer Tools"));
        devToolsView = new QWebEngineView(devToolsWindow);
        devToolsWindow->setCentralWidget(devToolsView);
        devToolsWindow->resize(900, 700);
        connect(devToolsWindow.data(), &QObject::destroyed, this, [this] {
            devToolsWindow = nullptr;
            devToolsView = nullptr;
        });
    }
    currentWebView()->page()->setDevToolsPage(devToolsView->page());
    devToolsWindow->show();
    devToolsWindow->raise();
    devToolsWindow->activateWindow();
}

void MainWindow::openSettings()
{
    openSettingsPage();
}

QString MainWindow::buildSettingsPageHtml()
{
    const bool spatialNav = settings.value("SpatialNavigation", false).toBool();
    const bool enableJs = settings.value("EnableJavaScript", true).toBool();
    const bool loadImages = settings.value("LoadImages", true).toBool();
    const bool enableCookies = settings.value("EnableCookies", true).toBool();
    const bool enableThirdPartyCookies = settings.value("EnableThirdPartyCookies", true).toBool();
    const bool allowPopups = settings.value("AllowPopups", true).toBool();
    const bool saveTabs = settings.value("SaveTabs", false).toBool();
    const bool clearCookiesAtExit = settings.value("ClearCookiesAtExit", false).toBool();

    auto check = [](bool value) { return value ? QStringLiteral("checked") : QString(); };
    auto disabled = [](bool value) { return value ? QString() : QStringLiteral("disabled"); };

    QString cacheSizeText = clearingCache ? tr("Clearing...") : tr("unknown");
    if (!clearingCache) {
        const auto *profile = webProfile;
        const QStringList cacheCandidates = collectCachePaths(profile);
        const qint64 cacheBytes = totalDirectorySize(cacheCandidates);
        if (cacheBytes >= 0) {
            if (cacheBytes < 1024) {
                cacheSizeText = tr("Cleared");
            } else {
                cacheSizeText = DownloadWidget::withUnit(cacheBytes);
            }
        }
    }

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
    form { display: grid; gap: 12px; max-width: 720px; }
    label { font-weight: 600; display: block; margin-bottom: 4px; }
    .row { display: grid; gap: 6px; }
    .input, select { padding: 8px 10px; border: 1px solid #d0d7de; border-radius: 6px; }
    .check { display: flex; gap: 8px; align-items: center; }
    .check-row { display: flex; gap: 12px; align-items: center; }
    .cache-label { font-weight: 600; }
    .actions { display: flex; gap: 12px; margin-top: 8px; }
    .btn { padding: 8px 12px; border: 1px solid #d0d7de; background: #f6f8fa; border-radius: 6px; cursor: pointer; }
    .btn-inline { padding: 4px 8px; font-size: 12px; }
  </style>
</head>
<body>
  <h1>%1</h1>
  <form id="settings">
    <div class="row">
      <label for="home">%2</label>
      <input id="home" class="input" type="text" value="%3">
    </div>
    <div class="row">
      <label for="search">%4</label>
      <select id="search" class="input">
        <option value="DuckDuckGo" %5>%6</option>
        <option value="Google" %7>%8</option>
        <option value="Bing" %9>%10</option>
        <option value="Custom" %11>%12</option>
      </select>
    </div>
    <div class="row">
      <label for="customSearch">%13</label>
      <input id="customSearch" class="input" type="text" value="%14" placeholder="https://example.com/?q=%s">
    </div>
    <div class="row">
      <label for="zoom">%15</label>
      <input id="zoom" class="input" type="number" min="25" max="500" value="%16">
    </div>
    <label class="check"><input id="openNewTab" type="checkbox" %17> %18</label>
    <label class="check"><input id="showProgress" type="checkbox" %19> %20</label>
    <label class="check"><input id="spatialNav" type="checkbox" %21> %22</label>
    <label class="check"><input id="enableJs" type="checkbox" %23> %24</label>
    <label class="check"><input id="loadImages" type="checkbox" %25> %26</label>
    <div class="check-row">
      <label class="check"><input id="enableCookies" type="checkbox" %27> %28</label>
      <button class="btn btn-inline" id="clearCookies" type="button">%41</button>
    </div>
    <label class="check"><input id="thirdPartyCookies" type="checkbox" %29 %30> %31</label>
    <label class="check"><input id="clearCookiesAtExit" type="checkbox" %32> %33</label>
    <label class="check"><input id="allowPopups" type="checkbox" %34> %35</label>
    <label class="check"><input id="saveTabs" type="checkbox" %36> %37</label>
    <div class="check-row">
      <div class="cache-label">%42</div>
      <button class="btn btn-inline" id="clearCache" type="button">%43</button>
    </div>
    <div class="actions">
      <button class="btn" id="save">%38</button>
      <button class="btn" id="reset" type="button">%39</button>
    </div>
  </form>
  <script>
    const form = document.getElementById('settings');
    const saveBtn = document.getElementById('save');
    const resetBtn = document.getElementById('reset');
    const searchSelect = document.getElementById('search');
    const customSearch = document.getElementById('customSearch');
    const inputs = Array.from(form.querySelectorAll('input, select'));
    saveBtn.disabled = true;
    resetBtn.disabled = true;
    window.mxSettingsDirty = false;
    function snapshot() {
      const data = {};
      inputs.forEach(el => {
        if (!el.id) {
          return;
        }
        data[el.id] = el.type === 'checkbox' ? el.checked : el.value;
      });
      return JSON.stringify(data);
    }
    let baseline = snapshot();
    function updateDirtyState() {
      const dirty = snapshot() !== baseline;
      window.mxSettingsDirty = dirty;
      saveBtn.disabled = !dirty;
      resetBtn.disabled = !dirty;
    }
    function boolValue(id) {
      return document.getElementById(id).checked ? '1' : '0';
    }
    function normalizeCustomUrl(url) {
      if (!url.includes('%s') && !url.includes('?q=')) {
        const add = confirm('%40');
        if (add) {
          const join = url.includes('?') ? '&' : '?';
          return url + join + 'q=%s';
        }
      }
      return url;
    }
    function syncCustom() {
      const isCustom = searchSelect.value === 'Custom';
      customSearch.disabled = !isCustom;
      customSearch.parentElement.style.display = isCustom ? '' : 'none';
    }
    saveBtn.addEventListener('click', event => {
      event.preventDefault();
      if (saveBtn.disabled) {
        return;
      }
      let customUrl = customSearch.value.trim();
      if (searchSelect.value === 'Custom' && customUrl.length > 0) {
        customUrl = normalizeCustomUrl(customUrl);
        customSearch.value = customUrl;
      }
      const params = new URLSearchParams();
      params.set('home', document.getElementById('home').value);
      params.set('search', document.getElementById('search').value);
      params.set('customSearch', customUrl);
      params.set('zoom', document.getElementById('zoom').value);
      params.set('openNewTab', boolValue('openNewTab'));
      params.set('showProgress', boolValue('showProgress'));
      params.set('spatialNav', boolValue('spatialNav'));
      params.set('enableJs', boolValue('enableJs'));
      params.set('loadImages', boolValue('loadImages'));
      params.set('enableCookies', boolValue('enableCookies'));
      params.set('thirdPartyCookies', boolValue('thirdPartyCookies'));
      params.set('allowPopups', boolValue('allowPopups'));
      params.set('saveTabs', boolValue('saveTabs'));
      params.set('clearCookiesAtExit', boolValue('clearCookiesAtExit'));
      baseline = snapshot();
      updateDirtyState();
      location.href = 'mx-settings://save?' + params.toString();
    });
    resetBtn.addEventListener('click', event => {
      event.preventDefault();
      location.href = 'mx-settings://list';
    });
    const clearCookiesBtn = document.getElementById('clearCookies');
    clearCookiesBtn.addEventListener('click', event => {
      event.preventDefault();
      if (confirm('%44')) {
        location.href = 'mx-settings://clearCookies';
      }
    });
    const clearCacheBtn = document.getElementById('clearCache');
    clearCacheBtn.addEventListener('click', event => {
      event.preventDefault();
      if (confirm('%45')) {
        location.href = 'mx-settings://clearCache';
      }
    });
    searchSelect.addEventListener('change', syncCustom);
    inputs.forEach(el => {
      el.addEventListener('input', updateDirtyState);
      el.addEventListener('change', updateDirtyState);
    });
    const cookiesToggle = document.getElementById('enableCookies');
    const thirdPartyToggle = document.getElementById('thirdPartyCookies');
    function syncThirdParty() {
      thirdPartyToggle.disabled = !cookiesToggle.checked;
      if (!cookiesToggle.checked) {
        thirdPartyToggle.checked = false;
      }
    }
    syncCustom();
    cookiesToggle.addEventListener('change', syncThirdParty);
    syncThirdParty();
    baseline = snapshot();
    updateDirtyState();
  </script>
</body>
</html>)")
                            .arg(tr("Settings").toHtmlEscaped(),
                                 tr("Home address").toHtmlEscaped(),
                                 homeAddress.toHtmlEscaped(),
                                 tr("Search engine").toHtmlEscaped(),
                                 searchEngine == "DuckDuckGo" ? QStringLiteral("selected") : QString(),
                                 tr("DuckDuckGo").toHtmlEscaped(),
                                 searchEngine == "Google" ? QStringLiteral("selected") : QString(),
                                 tr("Google").toHtmlEscaped(),
                                 searchEngine == "Bing" ? QStringLiteral("selected") : QString(),
                                 tr("Bing").toHtmlEscaped(),
                                 searchEngine == "Custom" ? QStringLiteral("selected") : QString(),
                                 tr("Custom").toHtmlEscaped(),
                                 tr("Custom search URL").toHtmlEscaped(),
                                 searchEngineCustom.toHtmlEscaped(),
                                 tr("Zoom level").toHtmlEscaped(),
                                 QString::number(zoomPercent),
                                 check(openNewTabWithHome),
                                 tr("Open new tabs with home page").toHtmlEscaped(),
                                 check(showProgress),
                                 tr("Show progress bar").toHtmlEscaped(),
                                 check(spatialNav),
                                 tr("Enable spatial navigation").toHtmlEscaped(),
                                 check(enableJs),
                                 tr("Enable JavaScript").toHtmlEscaped(),
                                 check(loadImages),
                                 tr("Load images").toHtmlEscaped(),
                                 check(enableCookies),
                                 tr("Enable cookies").toHtmlEscaped(),
                                 check(enableThirdPartyCookies),
                                 disabled(enableCookies),
                                 tr("Enable third-party cookies").toHtmlEscaped(),
                                 check(clearCookiesAtExit),
                                 tr("Clear cookies at exit").toHtmlEscaped(),
                                 check(allowPopups),
                                 tr("Allow pop-up windows").toHtmlEscaped(),
                                 check(saveTabs),
                                 tr("Save tabs on closing").toHtmlEscaped(),
                                 tr("Save settings").toHtmlEscaped(),
                                 tr("Reset").toHtmlEscaped(),
                                 tr("Custom URL has no ?q= or %s placeholder. Append ?q=%s automatically?")
                                     .toHtmlEscaped(),
                                 tr("Clear cookies").toHtmlEscaped(),
                                 tr("Cache size: %1").arg(cacheSizeText).toHtmlEscaped(),
                                 tr("Clear cache").toHtmlEscaped(),
                                 tr("Clear all cookies?").toHtmlEscaped(),
                                 tr("Clear the cache?").toHtmlEscaped());

    return html;
}

void MainWindow::renderSettingsPage(WebView *view)
{
    if (!view) {
        return;
    }
    const QString ts = QString::number(QDateTime::currentMSecsSinceEpoch());
    view->setHtml(buildSettingsPageHtml(), QUrl("mx-settings://list?ts=" + ts));
    view->show();
    tabWidget->setTabText(tabWidget->indexOf(view), tr("Settings"));
    setWindowTitle(tr("Settings"));
    updateUrl();
}

void MainWindow::openSettingsPage()
{
    if (auto *view = currentWebView()) {
        if (view->url().scheme() == "mx-settings") {
            renderSettingsPage(view);
            return;
        }
    }
    auto *view = tabWidget->createTab(true);
    if (!view) {
        return;
    }
    setConnections();
    renderSettingsPage(view);
}

bool MainWindow::handleSettingsRequest(const QUrl &url)
{
    if (url.scheme() != "mx-settings") {
        return false;
    }
    const QString action = url.host();
    if (action == "clearcookies") {
        webProfile->cookieStore()->deleteAllCookies();
        renderSettingsPage(currentWebView());
        return true;
    } else if (action == "clearcache") {
        clearingCache = true;
        renderSettingsPage(currentWebView());
        webProfile->clearHttpCache();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        connect(webProfile, &QWebEngineProfile::clearHttpCacheCompleted, this, [this]() {
            clearingCache = false;
            if (auto *view = currentWebView()) {
                if (view->url().scheme() == "mx-settings") {
                    renderSettingsPage(view);
                }
            }
        }, Qt::SingleShotConnection);
#else
        // Qt < 6.7 doesn't have clearHttpCacheCompleted signal, use timer fallback
        QTimer::singleShot(500, this, [this]() {
            clearingCache = false;
            if (auto *view = currentWebView()) {
                if (view->url().scheme() == "mx-settings") {
                    renderSettingsPage(view);
                }
            }
        });
#endif
        return true;
    }
    QUrlQuery query(url);
    const QString newHome = QUrl::fromPercentEncoding(query.queryItemValue("home").toUtf8()).trimmed();
    const QString newSearch = query.queryItemValue("search").trimmed();
    const QString newCustomSearch = QUrl::fromPercentEncoding(query.queryItemValue("customSearch").toUtf8()).trimmed();
    const int newZoom = query.queryItemValue("zoom").toInt();
    const bool newOpenNewTab = query.queryItemValue("openNewTab") == "1";
    const bool newShowProgress = query.queryItemValue("showProgress") == "1";
    const bool newSpatialNav = query.queryItemValue("spatialNav") == "1";
    const bool newEnableJs = query.queryItemValue("enableJs") == "1";
    const bool newLoadImages = query.queryItemValue("loadImages") == "1";
    const bool newEnableCookies = query.queryItemValue("enableCookies") == "1";
    bool newThirdParty = query.queryItemValue("thirdPartyCookies") == "1";
    const bool newAllowPopups = query.queryItemValue("allowPopups") == "1";
    const bool newSaveTabs = query.queryItemValue("saveTabs") == "1";
    const bool newClearCookiesAtExit = query.queryItemValue("clearCookiesAtExit") == "1";
    if (!newEnableCookies) {
        newThirdParty = false;
    }

    homeAddress = newHome.isEmpty() ? homeAddress : newHome;
    if (!newSearch.isEmpty()) {
        searchEngine = newSearch;
    }
    searchEngineCustom = newCustomSearch;
    settings.setValue("SearchEngineCustom", searchEngineCustom);
    openNewTabWithHome = newOpenNewTab;
    showProgress = newShowProgress;
    settings.setValue("Home", homeAddress);
    settings.setValue("SearchEngine", searchEngine);
    settings.setValue("OpenNewTabWithHome", openNewTabWithHome);
    settings.setValue("ShowProgressBar", showProgress);
    settings.setValue("SpatialNavigation", newSpatialNav);
    settings.setValue("EnableJavaScript", newEnableJs);
    settings.setValue("LoadImages", newLoadImages);
    settings.setValue("EnableCookies", newEnableCookies);
    settings.setValue("EnableThirdPartyCookies", newThirdParty);
    settings.setValue("AllowPopups", newAllowPopups);
    settings.setValue("SaveTabs", newSaveTabs);
    clearCookiesAtExit = newClearCookiesAtExit;
    settings.setValue("ClearCookiesAtExit", newClearCookiesAtExit);
    if (newZoom > 0) {
        setZoomPercent(newZoom, true);
    }

    applyWebSettings();
    renderSettingsPage(currentWebView());
    return true;
}

void MainWindow::applyWebSettings()
{
    bool spatialNav = settings.value("SpatialNavigation", false).toBool();
    bool enableJs = settings.value("EnableJavaScript", true).toBool();
    bool loadImages = settings.value("LoadImages", true).toBool();
    bool enableCookies = settings.value("EnableCookies", true).toBool();
    bool enableThirdPartyCookies = settings.value("EnableThirdPartyCookies", true).toBool();
    bool allowPopups = settings.value("AllowPopups", true).toBool();

    if (args && args->isSet("enable-spatial-navigation")) {
        spatialNav = true;
    }
    if (args && args->isSet("disable-js")) {
        enableJs = false;
    }
    if (args && args->isSet("disable-images")) {
        loadImages = false;
    }

    websettings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, spatialNav);
    websettings->setAttribute(QWebEngineSettings::JavascriptEnabled, enableJs);
    websettings->setAttribute(QWebEngineSettings::AutoLoadImages, loadImages);
    websettings->setAttribute(QWebEngineSettings::LocalStorageEnabled, enableCookies);
    websettings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, allowPopups);

    auto *profile = webProfile;
    profile->setHttpAcceptLanguage(QLocale::system().name());

    if (cookiesEnabled && !enableCookies) {
        profile->cookieStore()->deleteAllCookies();
    }
    cookiesEnabled = enableCookies;

    profile->setPersistentCookiesPolicy(enableCookies ? QWebEngineProfile::ForcePersistentCookies
                                                     : QWebEngineProfile::NoPersistentCookies);

    if (!enableCookies) {
        profile->cookieStore()->setCookieFilter([](const QWebEngineCookieStore::FilterRequest &) {
            return false;
        });

        QString jsCode = R"(
            Object.defineProperty(navigator, 'cookieEnabled', {
                value: false,
                configurable: true
            });
        )";
        cookieScript.setName("cookieDisabled");
        cookieScript.setSourceCode(jsCode);
        cookieScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
        cookieScript.setRunsOnSubFrames(true);
        cookieScript.setWorldId(QWebEngineScript::MainWorld);
    } else {
        if (!enableThirdPartyCookies) {
            profile->cookieStore()->setCookieFilter([](const QWebEngineCookieStore::FilterRequest &request) {
                return !request.thirdParty;
            });
        } else {
            profile->cookieStore()->setCookieFilter(nullptr);
        }

        QString jsCode = R"(
            Object.defineProperty(navigator, 'cookieEnabled', {
                value: true,
                configurable: true
            });
        )";
        cookieScript.setName("cookieEnabled");
        cookieScript.setSourceCode(jsCode);
        cookieScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
        cookieScript.setRunsOnSubFrames(true);
        cookieScript.setWorldId(QWebEngineScript::MainWorld);
    }

    for (int i = 0; i < tabWidget->count(); ++i) {
        if (auto *view = qobject_cast<WebView *>(tabWidget->widget(i))) {
            view->page()->scripts().clear();
            view->page()->scripts().insert(cookieScript);
        }
    }
}

void MainWindow::setZoomPercent(int percent, bool persist)
{
    zoomPercent = qBound(25, percent, 500);
    if (zoomPercentAction) {
        zoomPercentAction->setText(QString::number(zoomPercent) + "%");
    }
    if (auto *view = currentWebView()) {
        view->setZoomFactor(zoomPercent / 100.0);
    }
    if (persist) {
        settings.setValue("ZoomPercent", zoomPercent);
    }
}

void MainWindow::openBookmarksEditor()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Manage bookmarks"));
    dialog.resize(520, 420);

    auto *layout = new QVBoxLayout(&dialog);
    auto *list = new QListWidget(&dialog);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(list);

    auto *formLayout = new QFormLayout;
    auto *titleEdit = new QLineEdit(&dialog);
    auto *urlEdit = new QLineEdit(&dialog);
    formLayout->addRow(tr("Title"), titleEdit);
    formLayout->addRow(tr("URL"), urlEdit);
    layout->addLayout(formLayout);

    auto *controlsLayout = new QHBoxLayout;
    auto *moveUpButton = new QPushButton(QIcon::fromTheme("arrow-up"), tr("Move up"), &dialog);
    auto *moveDownButton = new QPushButton(QIcon::fromTheme("arrow-down"), tr("Move down"), &dialog);
    auto *removeButton = new QPushButton(QIcon::fromTheme("user-trash"), tr("Remove"), &dialog);
    controlsLayout->addWidget(moveUpButton);
    controlsLayout->addWidget(moveDownButton);
    controlsLayout->addWidget(removeButton);
    controlsLayout->addStretch();
    layout->addLayout(controlsLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    for (auto *action : bookmarks->actions()) {
        if (!action->property("url").isValid()) {
            continue;
        }
        auto *item = new QListWidgetItem(action->icon(), action->text(), list);
        item->setData(Qt::UserRole, action->property("url").toString());
    }

    auto syncEditors = [list, titleEdit, urlEdit, moveUpButton, moveDownButton, removeButton] {
        auto *item = list->currentItem();
        const bool hasItem = (item != nullptr);
        titleEdit->setEnabled(hasItem);
        urlEdit->setEnabled(hasItem);
        moveUpButton->setEnabled(hasItem && list->currentRow() > 0);
        moveDownButton->setEnabled(hasItem && list->currentRow() < list->count() - 1);
        removeButton->setEnabled(hasItem);
        if (!hasItem) {
            titleEdit->clear();
            urlEdit->clear();
            return;
        }
        titleEdit->setText(item->text());
        urlEdit->setText(item->data(Qt::UserRole).toString());
    };

    connect(list, &QListWidget::currentRowChanged, &dialog, [syncEditors] { syncEditors(); });
    connect(titleEdit, &QLineEdit::textEdited, &dialog, [list](const QString &text) {
        if (auto *item = list->currentItem()) {
            item->setText(text);
        }
    });
    connect(urlEdit, &QLineEdit::textEdited, &dialog, [list](const QString &text) {
        if (auto *item = list->currentItem()) {
            item->setData(Qt::UserRole, text);
        }
    });
    connect(moveUpButton, &QPushButton::clicked, &dialog, [list] {
        const int row = list->currentRow();
        if (row > 0) {
            auto *item = list->takeItem(row);
            list->insertItem(row - 1, item);
            list->setCurrentItem(item);
        }
    });
    connect(moveDownButton, &QPushButton::clicked, &dialog, [list] {
        const int row = list->currentRow();
        if (row >= 0 && row < list->count() - 1) {
            auto *item = list->takeItem(row);
            list->insertItem(row + 1, item);
            list->setCurrentItem(item);
        }
    });
    connect(removeButton, &QPushButton::clicked, &dialog, [list, syncEditors] {
        const int row = list->currentRow();
        if (row >= 0) {
            delete list->takeItem(row);
            syncEditors();
        }
    });
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (list->count() > 0) {
        list->setCurrentRow(0);
    } else {
        syncEditors();
    }

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    auto actions = bookmarks->actions();
    for (auto *action : actions) {
        if (!action->property("url").isValid()) {
            continue;
        }
        bookmarks->removeAction(action);
        action->deleteLater();
    }
    for (int i = 0; i < list->count(); ++i) {
        auto *item = list->item(i);
        auto *bookmark = new QAction(item->icon(), item->text(), bookmarks);
        bookmark->setProperty("url", item->data(Qt::UserRole).toString());
        bookmarks->addAction(bookmark);
        connectAddress(bookmark, bookmarks);
    }
    saveMenuItems(bookmarks, 2);
}

void MainWindow::closeCurrentTab()
{
    if (tabWidget->count() > 1) {
        tabWidget->removeTab(tabWidget->currentIndex());
    } else {
        close();
    }
}

void MainWindow::reopenClosedTab()
{
    if (!closedTabs.isEmpty()) {
        openSavedTab(closedTabs.takeLast().first, true);
    }
}

void MainWindow::openLinkInNewTab(const QUrl &url)
{
    addNewTab(url, false);
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        if (!normalGeometry.isEmpty()) {
            restoreGeometry(normalGeometry);
        }
        toolBar->show();
    } else {
        normalGeometry = saveGeometry();
        showFullScreen();
        toolBar->hide();
        showFullScreenNotification();
    }
}

void MainWindow::updateUrl()
{
    auto *view = currentWebView();
    if (!view) {
        addressBar->clear();
        addressBar->hide();
        return;
    }
    addressBar->show();
    addressBar->setText(view->url().toDisplayString());
    addressBar->setCursorPosition(0);
}

bool MainWindow::restoreSavedTabs()
{
    int size = settings.beginReadArray("SavedTabs");
    if (size == 0) {
        settings.endArray();
        return false;
    }

    QList<QUrl> savedUrls;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString url = settings.value("url").toString();
        if (!url.isEmpty()) {
            savedUrls.append(QUrl::fromUserInput(url));
        }
    }
    settings.endArray();
    settings.remove("SavedTabs");

    if (!savedUrls.isEmpty()) {
        tabWidget->removeTab(0);
        openSavedTab(savedUrls.first(), true);
    }

    for (int i = 1; i < savedUrls.size(); ++i) {
        openSavedTab(savedUrls.at(i), false);
    }
    return !savedUrls.isEmpty();
}

void MainWindow::openSavedTab(const QUrl &url, bool makeCurrent)
{
    if (url.scheme() == "mx-history") {
        auto *view = tabWidget->createTab(makeCurrent);
        if (!view) {
            return;
        }
        if (makeCurrent) {
            setConnections();
        }
        renderHistoryPage(view);
        return;
    }
    if (url.scheme() == "mx-settings") {
        auto *view = tabWidget->createTab(makeCurrent);
        if (!view) {
            return;
        }
        if (makeCurrent) {
            setConnections();
        }
        renderSettingsPage(view);
        return;
    }
    addNewTab(url, makeCurrent);
}

void MainWindow::focusAddressBar()
{
    if (addressBar) {
        addressBar->setFocus();
    }
}

void MainWindow::focusAddressBarIfBlank()
{
    if (!addressBar) {
        return;
    }
    const QString text = addressBar->text().trimmed();
    if (text.isEmpty() || text == "about:blank") {
        focusAddressBar();
    }
}

void MainWindow::findBackward()
{
    searchBox->setFocus();
    currentWebView()->findText(searchBox->text(), QWebEnginePage::FindBackward);
}

void MainWindow::findForward()
{
    searchBox->setFocus();
    currentWebView()->findText(searchBox->text());
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_H && event->modifiers() == Qt::ControlModifier) {
        openHistoryPage();
        return;
    }
    if (event->matches(QKeySequence::FindNext) || event->matches(QKeySequence::Find) || event->key() == Qt::Key_Slash) {
        findForward();
        return;
    }
    if (event->matches(QKeySequence::FindPrevious)) {
        findBackward();
        return;
    }
    if (event->matches(QKeySequence::Open)) {
        openBrowseDialog();
        return;
    }
    if (event->matches(QKeySequence::HelpContents) || event->key() == Qt::Key_Question) {
        openQuickInfo();
        return;
    }
    if (event->matches(QKeySequence::Cancel) && !searchBox->text().isEmpty() && searchBox->hasFocus()) {
        searchBox->clear();
        return;
    }
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        toggleFullScreen();
        return;
    }
    if (event->matches(QKeySequence::Cancel) && searchBox->text().isEmpty()) {
        if (auto *view = currentWebView()) {
            view->setFocus();
        }
        return;
    }
    if (event->key() == Qt::Key_L && event->modifiers() == Qt::ControlModifier) {
        focusAddressBar();
        return;
    }
    if (event->key() == Qt::Key_R && event->modifiers() == Qt::ControlModifier) {
        reloadCurrentView();
        return;
    }
    if (event->key() == Qt::Key_Left && event->modifiers() == Qt::AltModifier) {
        if (auto *view = currentWebView()) {
            view->back();
        }
        return;
    }
    if (event->key() == Qt::Key_Right && event->modifiers() == Qt::AltModifier) {
        if (auto *view = currentWebView()) {
            view->forward();
        }
        return;
    }
}

// resize event
void MainWindow::resizeEvent(QResizeEvent * /*event*/)
{
    if (showProgress) {
        progressBar->move(geometry().width() / 2 - progressBar->width() / 2, geometry().height() - progBarVerticalAdj);
    }
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    downloadWidget->close();
    if (clearCookiesAtExit) {
        webProfile->cookieStore()->deleteAllCookies();
    }
    settings.setValue("Geometry", saveGeometry());

    if (settings.value("SaveTabs", false).toBool()) {
        settings.beginWriteArray("SavedTabs");
        for (int i = 0; i < tabWidget->count(); ++i) {
            settings.setArrayIndex(i);
            auto *webView = qobject_cast<WebView *>(tabWidget->widget(i));
            if (webView) {
                settings.setValue("url", webView->url().toString());
            }
        }
        settings.endArray();
    }
}

QAction *MainWindow::pageAction(QWebEnginePage::WebAction webAction)
{
    return currentWebView()->pageAction(webAction);
}

WebView *MainWindow::currentWebView()
{
    return tabWidget->currentWebView();
}

void MainWindow::reloadCurrentView()
{
    auto *view = currentWebView();
    if (!view) {
        return;
    }
    if (view->url().scheme() == "mx-settings") {
        renderSettingsPage(view);
        return;
    }
    view->reload();
}

// display progressbar while loading page
void MainWindow::loading()
{
    progressBar->setFixedHeight(progBarWidth);
    progressBar->setTextVisible(false);
    progressBar->move(geometry().width() / 2 - progressBar->width() / 2, geometry().height() - progBarVerticalAdj);
    progressBar->setFocus();
    progressBar->show();
    progressBar->setRange(0, 0);
}

// done loading
void MainWindow::done(bool ok)
{
    auto *view = currentWebView();
    if (!view) {
        searchBox->clear();
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        progressBar->hide();
        return;
    }
    if (!ok && lastAddressMaySearch && !lastAddressExplicitScheme && view->url() == lastAddressUrl
        && !lastAddressInput.isEmpty()) {
        displaySearchResults(lastAddressInput);
        return;
    }
    if (!ok) {
        qDebug() << "Error loading:" << view->url().toString();
    }
    view->stop();
    view->setFocus();
    searchBox->clear();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->hide();
    tabWidget->setTabText(tabWidget->currentIndex(), view->title());
    setWindowTitle(view->title());
}

/*****************************************************************************
 * navigation.cpp
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
#include "findbar.h"

#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QTimer>
#include <QWebEngineFindTextResult>
#include <QWebEngineLoadingInfo>
#include <QWebEngineView>

#include <memory>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

WebView *MainWindow::addNewTab(const QUrl &url, bool makeCurrent)
{
    WebView *view = tabWidget->createTab(makeCurrent);
    if (!view) {
        return nullptr;
    }
    if (makeCurrent) {
        setConnections();
    }
    QUrl finalUrl = url.scheme() == "mx-newtab" ? QUrl() : url;
    if (finalUrl.isEmpty() && openNewTabWithHome) {
        finalUrl = QUrl::fromUserInput(homeAddress);
    }
    if (finalUrl.isEmpty()) {
        renderNewTabPage(view);
    } else {
        view->setUrl(finalUrl);
    }
    view->show();
    if (makeCurrent) {
        QTimer::singleShot(0, this, &MainWindow::focusAddressBarIfBlank);
        connect(
            view, &QWebEngineView::loadFinished, this,
            [this, view](bool) {
                if (view == currentWebView()) {
                    focusAddressBarIfBlank();
                }
            },
            Qt::SingleShotConnection);
    }
    return view;
}

// Display a URL in the current view.
void MainWindow::displaySite(QString url, const QString &title)
{
    if (url.isEmpty()) {
        url = homeAddress;
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

bool MainWindow::isLocalHostInput(const QString &input) const
{
    // Only the host counts, so a port or path ("localhost:8080/admin") still opens the address.
    const QString host = QUrl::fromUserInput(input).host().toLower();
    return host == "localhost" || host == "localhost.localdomain" || host.endsWith(".local") || host == "127.0.0.1"
           || host == "::1";
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
    displaySite(searchUrlForQuery(query), query);
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
    // A new address replaces the one still being watched.
    disconnect(typedAddressConn);
    if (input.startsWith("http://", Qt::CaseInsensitive) || input.startsWith("https://", Qt::CaseInsensitive)) {
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
    displaySite(input);
    if (!hasExplicitScheme) {
        searchIfUnresolved(input);
    }
}

// Typed text with a dot ("foo.bar") may not be an address after all: if the first load of it fails
// because the host name does not resolve, search for the text instead. Nothing else counts, so a later
// failure of the same address (Stop, a reload while offline) never sends it to the search engine.
void MainWindow::searchIfUnresolved(const QString &input)
{
    auto *view = currentWebView();
    if (!view) {
        return;
    }
    constexpr int nameNotResolved = -105;     // net::ERR_NAME_NOT_RESOLVED
    constexpr int nameResolutionFailed = -137; // net::ERR_NAME_RESOLUTION_FAILED
    // Chromium reports "http://host" as "http://host/".
    const auto normalized = [](QUrl url) {
        if (url.path().isEmpty()) {
            url.setPath(QStringLiteral("/"));
        }
        return url;
    };
    const QUrl expected = normalized(QUrl::fromUserInput(input));
    const QPointer<WebView> target = view;
    auto started = std::make_shared<bool>(false);
    typedAddressConn = connect(view->page(), &QWebEnginePage::loadingChanged, this,
                               [this, target, input, expected, normalized, started](const QWebEngineLoadingInfo &info) {
        if (info.status() == QWebEngineLoadingInfo::LoadStartedStatus) {
            // The end of a load that was already running is not this address's answer.
            *started = *started || normalized(info.url()) == expected;
            return;
        }
        if (!*started) {
            return;
        }
        disconnect(typedAddressConn);
        const int error = info.errorCode();
        if (target && info.status() == QWebEngineLoadingInfo::LoadFailedStatus
            && info.errorDomain() == QWebEngineLoadingInfo::ConnectionErrorDomain
            && (error == nameNotResolved || error == nameResolutionFailed)) {
            // After the failed load has finished putting up its error page.
            QTimer::singleShot(0, target, [this, target, input] { target->setUrl(QUrl(searchUrlForQuery(input))); });
        }
    });
}

void MainWindow::setConnections()
{
    if (!currentWebView()) {
        return;
    }
    if (loadStartedConn) {
        disconnect(loadStartedConn);
    }
    loadStartedConn = connect(currentWebView(), &QWebEngineView::loadStarted, toolBar, &QToolBar::show);
    updateProgressConnection();
    if (urlChangedConn) {
        disconnect(urlChangedConn);
    }
    urlChangedConn = connect(currentWebView(), &QWebEngineView::urlChanged, this, &MainWindow::updateUrl);
    if (loadFinishedConn) {
        disconnect(loadFinishedConn);
    }
    loadFinishedConn = connect(currentWebView(), &QWebEngineView::loadFinished, this, &MainWindow::done);
    if (linkHoveredConn) {
        disconnect(linkHoveredConn);
    }
    if (iconChangedConn) {
        disconnect(iconChangedConn);
    }
    iconChangedConn = connect(currentWebView(), &QWebEngineView::iconChanged, this, &MainWindow::updateSiteIcon);
    linkHoveredConn = connect(currentWebView()->page(), &QWebEnginePage::linkHovered, this, [this](const QString &url) {
        if (url.isEmpty()) {
            statusBar()->hide();
        } else {
            statusBar()->show();
            statusBar()->showMessage(url);
        }
    });
    applyZoom();
}

void MainWindow::tabChanged()
{
    // The search follows the current tab; the previous one keeps no highlights.
    if (findBar && findBar->isVisible()) {
        if (findView && findView != currentWebView()) {
            findView->findText(QString());
        }
        findBar->setPage(currentWebView());
        findForward();
    }
    if (pageFullScreen && currentWebView() != pageFullScreenView) {
        exitPageFullScreen();
    }
    if (!currentWebView()) {
        // The last tab just closed; its page (and these actions) will be
        // deleted shortly, so drop the references before they dangle.
        backAction = forwardAction = stopAction = nullptr;
        return;
    }
    auto *back = pageAction(QWebEnginePage::Back);
    auto *forward = pageAction(QWebEnginePage::Forward);
    auto *reload = pageAction(QWebEnginePage::Reload);
    auto *stop = pageAction(QWebEnginePage::Stop);
    back->setShortcut(QKeySequence::Back);
    forward->setShortcut(QKeySequence::Forward);
    stop->setShortcut(QKeySequence::Cancel);
    toolBar->setUpdatesEnabled(false);
    // Anchor inserts on the permanent reloadAction/homeAction rather than the
    // previous tab's (about-to-be-replaced) actions, so this stays correct
    // even right after the last tab closed and backAction/etc. were nulled.
    toolBar->insertAction(reloadAction, back);
    toolBar->insertAction(reloadAction, forward);
    toolBar->insertAction(homeAction, stop);
    if (backAction) {
        toolBar->removeAction(backAction);
    }
    if (forwardAction) {
        toolBar->removeAction(forwardAction);
    }
    if (stopAction) {
        toolBar->removeAction(stopAction);
    }
    backAction = back;
    forwardAction = forward;
    stopAction = stop;
    disconnect(stopAction, &QAction::triggered, this, nullptr);
    connect(stopAction, &QAction::triggered, this, [this] { done(true); });
    toolBar->setUpdatesEnabled(true);
    if (reloadAction) {
        reloadAction->setIcon(reload->icon());
        reloadAction->setText(reload->text());
        reloadAction->setToolTip(reload->toolTip());
    }
    // Each tab brings its own page actions with the theme's icons.
    const QPalette palette = toolBar->palette();
    for (QAction *action : {backAction, forwardAction, stopAction, reloadAction}) {
        if (action) {
            action->setIcon(iconForBackground(action->icon(), palette.color(QPalette::Window),
                                              palette.color(QPalette::WindowText)));
        }
    }
    // As updateUrl() shows it, without a password.
    addressBar->setText(currentWebView()->url().scheme() == "mx-newtab" ? QString() : currentWebView()->url().toDisplayString());
    if (addressBar->text().isEmpty()) {
        addressBar->setFocus();
    }
    setWindowTitle(currentWebView()->title());
    setConnections();
    if (devToolsWindow && devToolsView) {
        currentWebView()->page()->setDevToolsPage(devToolsView->page());
    }
}

void MainWindow::openInNewWindow(const QUrl &url, bool privateMode)
{
    auto *window = new MainWindow(url, privateMode, false);
    window->move(pos() + QPoint(40, 40));
    window->show();
}

void MainWindow::openFromOtherInstance(const QString &argument)
{
    // Links from other applications never go to a private window.
    MainWindow *target = lastActiveWindow;
    if (!target || !target->isVisible()) {
        target = nullptr;
        const auto widgets = QApplication::topLevelWidgets();
        for (auto *widget : widgets) {
            auto *window = qobject_cast<MainWindow *>(widget);
            if (window && !window->privateWindow && window->isVisible()) {
                target = window;
                break;
            }
        }
    }
    QUrl url = argument.isEmpty() ? QUrl() : QUrl::fromUserInput(argument);
    // Any local process can send this, so only ordinary locations are opened, never internal pages
    // such as mx-history://clear; anything else just opens a new tab.
    static const QStringList allowedSchemes {"http", "https", "file", "ftp"};
    if (!url.isEmpty() && !allowedSchemes.contains(url.scheme())) {
        url.clear();
    }
    if (!target) {
        target = new MainWindow(url, false);
        // Like a first launch: restored tabs replace the start page, so the link needs its own tab.
        if (target->restoredTabs && !url.isEmpty()) {
            target->addNewTab(url, true);
        }
        target->show();
    } else {
        target->addNewTab(url, true);
        if (target->isMinimized()) {
            // showNormal() would also drop a maximized or full-screen state.
            target->setWindowState(target->windowState() & ~Qt::WindowMinimized);
        }
    }
    target->raise();
    target->activateWindow();
}

void MainWindow::openPrivateWindow()
{
    auto *window = new MainWindow(QUrl(), true);
    window->move(pos() + QPoint(40, 40));
    window->show();
}

void MainWindow::closeCurrentTab()
{
    if (!tabWidget->closeCurrentTabByShortcut()) {
        close();
    }
}

void MainWindow::reopenClosedTab()
{
    if (!closedTabs.isEmpty()) {
        openSavedTab(closedTabs.takeLast().first, true);
    }
}

void MainWindow::openLinkInNewTab(const QUrl &url, WebView *opener)
{
    WebView *view = addNewTab(url, false);
    if (view && opener) {
        tabWidget->placeAfterOpener(view, opener);
    }
}

void MainWindow::searchInNewTab(const QString &text)
{
    addNewTab(QUrl(searchUrlForQuery(text)), true);
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
    addressBar->setText(view->url().scheme() == "mx-newtab" ? QString() : view->url().toDisplayString());
    addressBar->setCursorPosition(0);
    updateSiteIcon();
    applyZoom();
}

bool MainWindow::restoreSavedTabs()
{
    int size = settings.beginReadArray("SavedTabs");
    if (size == 0) {
        settings.endArray();
        return false;
    }

    QList<QUrl> savedUrls;
    QList<bool> savedPinned;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString url = settings.value("url").toString();
        if (!url.isEmpty()) {
            savedUrls.append(QUrl::fromUserInput(url));
            savedPinned.append(settings.value("pinned", false).toBool());
        }
    }
    settings.endArray();
    settings.remove("SavedTabs");

    if (!savedUrls.isEmpty()) {
        // Open the first saved tab before dropping the initial one, since closing the only tab
        // closes the window.
        openSavedTab(savedUrls.first(), true);
        tabWidget->removeTab(0);
    }

    for (int i = 1; i < savedUrls.size(); ++i) {
        openSavedTab(savedUrls.at(i), false);
    }
    // Each saved entry opened one tab in order; pinning left to right keeps the saved order.
    for (int i = 0; i < savedPinned.size() && i < tabWidget->count(); ++i) {
        if (savedPinned.at(i)) {
            tabWidget->setPinned(i, true);
        }
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
        addressBar->setFocus(Qt::ShortcutFocusReason);
        addressBar->selectAll();
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
    findInPage(QWebEnginePage::FindBackward);
}

void MainWindow::findForward()
{
    findInPage({});
}

void MainWindow::findInPage(QWebEnginePage::FindFlags flags)
{
    auto *view = currentWebView();
    if (!view) {
        return;
    }
    if (!findBar->isVisible()) {
        openFindBar();
        if (findBar->text().isEmpty()) {
            return;
        }
    }
    if (findBar->isCaseSensitive()) {
        flags |= QWebEnginePage::FindCaseSensitively;
    }
    const QString text = findBar->text();
    findView = view;
    QPointer<MainWindow> self = this;
    QPointer<WebView> guard = view;
    view->findText(text, flags, [this, self, guard, text](const QWebEngineFindTextResult &result) {
        // Ignore results for a tab that is no longer shown or a search that was replaced.
        if (!self || !guard || guard != currentWebView() || text != findBar->text() || !findBar->isVisible()) {
            return;
        }
        if (text.isEmpty()) {
            findBar->clearResult();
            return;
        }
        findBar->setResult(result.activeMatch(), result.numberOfMatches());
    });
}

void MainWindow::openFindBar()
{
    findBar->open(currentWebView());
}

void MainWindow::closeFindBar()
{
    findBar->hide();
    findBar->clearResult();
    if (findView) {
        findView->findText(QString());
    }
    if (auto *view = currentWebView()) {
        view->setFocus();
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
    view->reloadPage();
}

// display progressbar while loading page
void MainWindow::loading()
{
    progressBar->setFixedHeight(progBarWidth);
    progressBar->setTextVisible(false);
    progressBar->move(geometry().width() / 2 - progressBar->width() / 2, geometry().height() - progBarVerticalAdj);
    progressBar->show();
    progressBar->setRange(0, 0);
}

// done loading
void MainWindow::done(bool ok)
{
    auto *view = currentWebView();
    if (!view) {
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        progressBar->hide();
        return;
    }
    // Navigation to another site may have reset the zoom.
    applyZoom();
    if (!ok) {
        qDebug() << "Error loading:" << view->url().toString();
    }
    // The new page has no highlights, so the old count no longer applies.
    if (findBar->isVisible()) {
        findBar->clearResult();
    } else {
        view->setFocus();
    }
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->hide();
    tabWidget->setTabTitle(tabWidget->currentIndex(), view->title());
    setWindowTitle(view->title());
}

/*****************************************************************************
 * browsercontext.cpp
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

#include <algorithm>

#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QPointer>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineView>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

QWebEngineProfile *MainWindow::sharedProfile()
{
    if (!s_sharedProfile) {
        // One browser context per storage path: QtWebEngine does not support two profiles on the same data.
        s_sharedProfile = new QWebEngineProfile("mx-viewer");
        QObject::connect(s_sharedProfile, &QWebEngineProfile::downloadRequested, s_sharedProfile,
                         [](QWebEngineDownloadRequest *download) { routeDownload(s_sharedProfile, download); });
    }
    return s_sharedProfile;
}

void MainWindow::releaseSharedProfile()
{
    // Collect first: deleting a window also deletes other top-level widgets it owns (its download list).
    QList<QPointer<MainWindow>> windows;
    const auto widgets = QApplication::topLevelWidgets();
    for (auto *widget : widgets) {
        if (auto *window = qobject_cast<MainWindow *>(widget)) {
            windows.append(window);
        }
    }
    for (const auto &window : std::as_const(windows)) {
        delete window.data();
    }
    delete s_sharedProfile;
    s_sharedProfile = nullptr;
}

// A download goes to the window whose page started it, so it is listed, and cancelled, with that window.
void MainWindow::routeDownload(QWebEngineProfile *profile, QWebEngineDownloadRequest *download)
{
    MainWindow *target {nullptr};
    if (const auto *page = download->page()) {
        if (const auto *view = QWebEngineView::forPage(page)) {
            target = qobject_cast<MainWindow *>(view->window());
        }
    }
    if (!target || target->webProfile != profile) {
        target = (lastActiveWindow && lastActiveWindow->webProfile == profile) ? lastActiveWindow.data() : nullptr;
    }
    if (!target) {
        const auto widgets = QApplication::topLevelWidgets();
        for (auto *widget : widgets) {
            auto *window = qobject_cast<MainWindow *>(widget);
            if (window && window->webProfile == profile && window->isVisible()) {
                target = window;
                break;
            }
        }
    }
    // Not accepted, so QtWebEngine cancels it.
    if (target) {
        target->downloadWidget->downloadRequested(download, profile);
    }
}

bool MainWindow::otherRegularWindowOpen() const
{
    const auto widgets = QApplication::topLevelWidgets();
    return std::any_of(widgets.cbegin(), widgets.cend(), [this](QWidget *widget) {
        const auto *window = qobject_cast<MainWindow *>(widget);
        return window && window != this && !window->privateWindow && window->isVisible();
    });
}

void MainWindow::loadSettings()
{
    // Load first from system .conf file and then overwrite with CLI switches where available
    webProfile->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    webProfile->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    setupSpellCheck();

    tabsInTitleBar = settings.value("TabsInTitleBar", true).toBool();
    cookiesEnabled = settings.value("EnableCookies", true).toBool();
    readPreferences();
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

void MainWindow::readPreferences()
{
    homeAddress = settings.value("Home", "https://start.duckduckgo.com").toString();
    showProgress = settings.value("ShowProgressBar", false).toBool();
    openNewTabWithHome = settings.value("OpenNewTabWithHome", true).toBool();
    zoomPercent = settings.value("ZoomPercent", 100).toInt();
    clearCookiesAtExit = settings.value("ClearCookiesAtExit", false).toBool();
    searchEngine = settings.value("SearchEngine", "DuckDuckGo").toString();
    searchEngineCustom = settings.value("SearchEngineCustom", QString()).toString();
}

void MainWindow::updateProgressConnection()
{
    if (loadingConn) {
        disconnect(loadingConn);
    }
    if (showProgress && currentWebView()) {
        loadingConn = connect(currentWebView(), &QWebEngineView::loadStarted, this, &MainWindow::loading);
    }
}

void MainWindow::applyPageSettings(QWebEngineSettings *target) const
{
    bool spatialNav = settings.value("SpatialNavigation", false).toBool();
    bool enableJs = settings.value("EnableJavaScript", true).toBool();
    bool loadImages = settings.value("LoadImages", true).toBool();
    const bool enableCookies = settings.value("EnableCookies", true).toBool();
    const bool allowPopups = settings.value("AllowPopups", true).toBool();

    if (commandLineOverrides.spatialNavigation) {
        spatialNav = true;
    }
    if (commandLineOverrides.disableJavaScript) {
        enableJs = false;
    }
    if (commandLineOverrides.disableImages) {
        loadImages = false;
    }

    target->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, spatialNav);
    target->setAttribute(QWebEngineSettings::JavascriptEnabled, enableJs);
    target->setAttribute(QWebEngineSettings::AutoLoadImages, loadImages);
    target->setAttribute(QWebEngineSettings::LocalStorageEnabled, enableCookies);
    target->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, allowPopups);
}

void MainWindow::applyWebSettings()
{
    const bool enableCookies = settings.value("EnableCookies", true).toBool();
    const bool enableThirdPartyCookies = settings.value("EnableThirdPartyCookies", true).toBool();

    // Set on each page rather than as profile defaults, since private windows have profiles of their
    // own. New pages get them through TabWidget::viewAdded.
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (auto *view = qobject_cast<WebView *>(tabWidget->widget(i))) {
            applyPageSettings(view->settings());
        }
    }

    auto *profile = webProfile;
    profile->setHttpAcceptLanguage(QLocale::system().uiLanguages().join(u','));

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
    } else if (!enableThirdPartyCookies) {
        profile->cookieStore()->setCookieFilter([](const QWebEngineCookieStore::FilterRequest &request) {
            return !request.thirdParty;
        });
    } else {
        profile->cookieStore()->setCookieFilter(nullptr);
    }

    QWebEngineScript cookieScript;
    cookieScript.setName(enableCookies ? QStringLiteral("cookieEnabled") : QStringLiteral("cookieDisabled"));
    cookieScript.setSourceCode(QStringLiteral(R"(
            Object.defineProperty(navigator, 'cookieEnabled', {
                value: %1,
                configurable: true
            });
        )")
                                   .arg(enableCookies ? "true" : "false"));
    cookieScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    cookieScript.setRunsOnSubFrames(true);
    cookieScript.setWorldId(QWebEngineScript::MainWorld);
    // On the profile, so every page gets it from its first load, including tabs opened in the background.
    QWebEngineScriptCollection *scripts = profile->scripts();
    for (const QString &name : {QStringLiteral("cookieEnabled"), QStringLiteral("cookieDisabled")}) {
        for (const QWebEngineScript &old : scripts->find(name)) {
            scripts->remove(old);
        }
    }
    scripts->insert(cookieScript);
}

// Enable spell checking when a dictionary for the system language is installed
// (e.g. en_US.bdic from hunspell-en-us); an exact locale match wins over one for the language only.
void MainWindow::setupSpellCheck()
{
    const QString dir = qEnvironmentVariable("QTWEBENGINE_DICTIONARIES_PATH");
    if (dir.isEmpty()) {
        return;
    }
    const QStringList files = QDir(dir).entryList({"*.bdic"}, QDir::Files);
    const QString locale = QLocale::system().name();
    const QString language = locale.section('_', 0, 0);
    QString match;
    for (const QString &file : files) {
        const QString name = QFileInfo(file).completeBaseName();
        if (name == locale) {
            match = name;
            break;
        }
        if (match.isEmpty() && name.section('_', 0, 0) == language) {
            match = name;
        }
    }
    if (!match.isEmpty()) {
        webProfile->setSpellCheckLanguages({match});
        webProfile->setSpellCheckEnabled(true);
    }
}

// Sets the default zoom, used for sites without their own zoom level.
void MainWindow::setZoomPercent(int percent, bool persist)
{
    zoomPercent = qBound(minZoom, percent, maxZoom);
    if (persist) {
        settings.setValue("ZoomPercent", zoomPercent);
    }
    applyZoom();
}

// Zoom levels are remembered per host (per scheme for host-less pages such as file://).
QString MainWindow::zoomKey(const QUrl &url)
{
    const QString site = url.host().isEmpty() ? url.scheme() : url.host();
    return site.isEmpty() ? QString() : "SiteZoom/" + site;
}

int MainWindow::currentZoomPercent() const
{
    const auto *view = tabWidget->currentWebView();
    return view ? qRound(view->zoomFactor() * 100) : zoomPercent;
}

void MainWindow::setSiteZoom(int percent)
{
    auto *view = currentWebView();
    if (!view) {
        return;
    }
    percent = qBound(minZoom, percent, maxZoom);
    const QString key = zoomKey(view->url());
    if (key.isEmpty()) {
        view->setZoomFactor(percent / 100.0);
        if (zoomPercentAction) {
            zoomPercentAction->setText(QString::number(percent) + "%");
        }
        return;
    }
    if (privateWindow) {
        privateZoom.insert(key, percent);
    } else if (percent == zoomPercent) {
        settings.remove(key);
    } else {
        settings.setValue(key, percent);
    }
    applyZoom();
}

void MainWindow::applyZoom()
{
    auto *view = currentWebView();
    if (!view) {
        return;
    }
    const QString key = zoomKey(view->url());
    int percent = zoomPercent;
    if (!key.isEmpty()) {
        percent = privateZoom.contains(key) ? privateZoom.value(key) : settings.value(key, zoomPercent).toInt();
        percent = qBound(minZoom, percent, maxZoom);
    }
    if (qRound(view->zoomFactor() * 100) != percent) {
        view->setZoomFactor(percent / 100.0);
    }
    if (zoomPercentAction) {
        zoomPercentAction->setText(QString::number(percent) + "%");
    }
}

void MainWindow::setQuitting()
{
    quitting = true;
    // Collected anew from the windows that close now, not from one closed earlier in the session.
    sessionTabs.clear();
}

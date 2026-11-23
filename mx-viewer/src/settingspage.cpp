/*****************************************************************************
 * settingspage.cpp
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

#include <QApplication>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPointer>
#include <QThreadPool>
#include <QTimer>
#include <QUrlQuery>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

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

    // The real size is filled in by updateCacheSize() once the cache has been walked off the GUI thread.
    const QString cacheSizeText = clearingCache ? tr("Clearing...") : tr("Calculating...");

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
  <form id="settings" action="mx-settings://save" method="get">
    <div class="row">
      <label for="home">%2</label>
      <input id="home" name="home" class="input" type="text" value="%3">
    </div>
    <div class="row">
      <label for="search">%4</label>
      <select id="search" name="search" class="input">
        <option value="DuckDuckGo" %5>%6</option>
        <option value="Google" %7>%8</option>
        <option value="Bing" %9>%10</option>
        <option value="Custom" %11>%12</option>
      </select>
    </div>
    <div class="row">
      <label for="customSearch">%13</label>
      <input id="customSearch" name="customSearch" class="input" type="text" value="%14" placeholder="https://example.com/?q=%s" data-confirm="%40">
    </div>
    <div class="row">
      <label for="zoom">%15</label>
      <input id="zoom" name="zoom" class="input" type="number" min="25" max="500" value="%16">
    </div>
    <label class="check"><input id="openNewTab" name="openNewTab" type="checkbox" value="1" %17> %18</label>
    <label class="check"><input id="showProgress" name="showProgress" type="checkbox" value="1" %19> %20</label>
    <label class="check"><input id="spatialNav" name="spatialNav" type="checkbox" value="1" %21> %22</label>
    <label class="check"><input id="enableJs" name="enableJs" type="checkbox" value="1" %23> %24</label>
    <label class="check"><input id="loadImages" name="loadImages" type="checkbox" value="1" %25> %26</label>
    <div class="check-row">
      <label class="check"><input id="enableCookies" name="enableCookies" type="checkbox" value="1" %27> %28</label>
      <button class="btn btn-inline" id="clearCookies" type="submit" form="clearCookiesForm">%41</button>
    </div>
    <label class="check"><input id="thirdPartyCookies" name="thirdPartyCookies" type="checkbox" value="1" %29 %30> %31</label>
    <label class="check"><input id="clearCookiesAtExit" name="clearCookiesAtExit" type="checkbox" value="1" %32> %33</label>
    <label class="check"><input id="allowPopups" name="allowPopups" type="checkbox" value="1" %34> %35</label>
    <label class="check"><input id="saveTabs" name="saveTabs" type="checkbox" value="1" %36> %37</label>
    <label class="check"><input id="tabsInTitleBar" name="tabsInTitleBar" type="checkbox" value="1" %44> %45</label>
    <div class="check-row">
      <div class="cache-label" id="cacheSize">%42</div>
      <button class="btn btn-inline" id="clearCache" type="submit" form="clearCacheForm">%43</button>
    </div>
    <div class="actions">
      <button class="btn" id="save" type="submit">%38</button>
      <button class="btn" id="reset" type="reset">%39</button>
    </div>
  </form>
  <!-- The Clear buttons submit these forms, so they work without JavaScript and are never the main form's
       default button for Enter. -->
  <form id="clearCookiesForm" action="mx-settings://clearCookies" method="get"></form>
  <form id="clearCacheForm" action="mx-settings://clearCache" method="get"></form>
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
        const add = confirm(customSearch.dataset.confirm);
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
    }
    // Collects the form into a save request; closing a changed settings tab asks for it too.
    function saveUrl() {
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
      params.set('tabsInTitleBar', boolValue('tabsInTitleBar'));
      params.set('clearCookiesAtExit', boolValue('clearCookiesAtExit'));
      baseline = snapshot();
      updateDirtyState();
      return 'mx-settings://save?' + params.toString();
    }
    window.mxSettingsSaveUrl = saveUrl;
    function saveSettings(event) {
      if (event) {
        event.preventDefault();
      }
      if (saveBtn.disabled) {
        return;
      }
      location.href = saveUrl();
    }
    form.addEventListener('submit', saveSettings);
    resetBtn.addEventListener('click', event => {
      event.preventDefault();
      location.href = 'mx-settings://list';
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
                                 check(tabsInTitleBar),
                                 tr("Show tabs in the title bar").toHtmlEscaped());

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
    tabWidget->setTabTitle(tabWidget->indexOf(view), tr("Settings"));
    setWindowTitle(tr("Settings"));
    updateUrl();
    if (!clearingCache) {
        updateCacheSize(view);
    }
}

// Walking a large cache takes a while, so it runs on a worker thread; the result goes into the page
// that asked, unless a clear started meanwhile.
void MainWindow::updateCacheSize(WebView *view)
{
    const QStringList paths = collectCachePaths(webProfile);
    const QPointer<WebView> target = view;
    QThreadPool::globalInstance()->start([paths, target] {
        const qint64 bytes = totalDirectorySize(paths);
        QMetaObject::invokeMethod(qApp, [bytes, target] {
            if (target && !clearingCache) {
                setCacheSizeLabel(target, bytes < 0     ? tr("unknown")
                                          : bytes < 1024 ? tr("Cleared")
                                                         : DownloadWidget::withUnit(bytes));
            }
        });
    });
}

// Changes only the label, so whatever the user is editing on the page stays; once loaded, if the view
// still shows the settings.
void MainWindow::setCacheSizeLabel(WebView *view, const QString &size)
{
    const QPointer<WebView> target = view;
    const QString text = QString::fromUtf8(
        QJsonDocument(QJsonArray {tr("Cache size: %1").arg(size)}).toJson(QJsonDocument::Compact));
    const auto apply = [target, text] {
        if (!target || target->url().scheme() != "mx-settings") {
            return;
        }
        // The application world runs even when JavaScript is turned off for pages.
        target->page()->runJavaScript(
            QStringLiteral("{const label = document.getElementById('cacheSize'); if (label) label.textContent = %1[0];}")
                .arg(text),
            QWebEngineScript::ApplicationWorld);
    };
    if (view->page()->isLoading()) {
        connect(view, &QWebEngineView::loadFinished, view, apply, Qt::SingleShotConnection);
    } else {
        apply();
    }
}

void MainWindow::openSettingsPage()
{
    if (privateWindow) {
        return;
    }
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
    // Private windows read shared preferences but must never edit them, even through a typed URL.
    if (privateWindow) {
        return true;
    }
    const QString action = url.host();
    if (action == "clearcookies" || action == "clearcache") {
        // Confirmed here rather than in the page, which may run without JavaScript. Deferred, so the
        // dialog's event loop does not run inside the navigation request.
        QTimer::singleShot(0, this, [this, action] {
            const bool cookies = action == "clearcookies";
            if (QMessageBox::question(this, cookies ? tr("Clear cookies") : tr("Clear cache"),
                                      cookies ? tr("Clear all cookies?") : tr("Clear the cache?"))
                != QMessageBox::Yes) {
                return;
            }
            // Nothing on the page shows the cookies, so it is left as it is, edits included.
            if (cookies) {
                webProfile->cookieStore()->deleteAllCookies();
            } else {
                clearCache();
            }
        });
        return true;
    }
    if (action == "list") {
        renderSettingsPage(currentWebView());
        return true;
    }
    if (action != "save") {
        return false;
    }
    QUrlQuery query(url);
    const QString newHome = formValue(query, "home");
    const QString newSearch = query.queryItemValue("search").trimmed();
    const QString newCustomSearch = formValue(query, "customSearch");
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
    const bool newTabsInTitleBar = query.queryItemValue("tabsInTitleBar") == "1";
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
    settings.setValue("TabsInTitleBar", newTabsInTitleBar);
    if (newTabsInTitleBar != tabsInTitleBar) {
        const auto widgets = QApplication::topLevelWidgets();
        for (auto *widget : widgets) {
            if (auto *window = qobject_cast<MainWindow *>(widget)) {
                window->tabsInTitleBar = newTabsInTitleBar;
                window->updateTitleBar();
            }
        }
    }
    clearCookiesAtExit = newClearCookiesAtExit;
    settings.setValue("ClearCookiesAtExit", newClearCookiesAtExit);
    if (newZoom > 0) {
        setZoomPercent(newZoom, true);
    }

    // Every window, private ones included, keeps its own copy of the shared preferences.
    for (auto *widget : QApplication::topLevelWidgets()) {
        if (auto *window = qobject_cast<MainWindow *>(widget)) {
            window->readPreferences();
            window->updateProgressConnection();
            window->applyZoom();
            window->applyWebSettings();
        }
    }
    renderSettingsPage(currentWebView());
    return true;
}

// The regular windows share one cache, so a clear already running for any of them is not started again.
// Every open settings page shows the progress in its label.
void MainWindow::clearCache()
{
    if (clearingCache) {
        return;
    }
    const auto forEachSettingsPage = [](const auto &action) {
        for (auto *widget : QApplication::topLevelWidgets()) {
            auto *window = qobject_cast<MainWindow *>(widget);
            if (!window || window->privateWindow) {
                continue;
            }
            for (int i = 0; i < window->tabWidget->count(); ++i) {
                auto *view = qobject_cast<WebView *>(window->tabWidget->widget(i));
                if (view && view->url().scheme() == "mx-settings") {
                    action(window, view);
                }
            }
        }
    };
    clearingCache = true;
    forEachSettingsPage([](MainWindow *, WebView *view) { setCacheSizeLabel(view, tr("Clearing...")); });
    const auto finished = [forEachSettingsPage] {
        clearingCache = false;
        forEachSettingsPage([](MainWindow *window, WebView *view) { window->updateCacheSize(view); });
    };
    // Tied to the profile, which outlives any window that asked.
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(webProfile, &QWebEngineProfile::clearHttpCacheCompleted, webProfile, finished, Qt::SingleShotConnection);
    webProfile->clearHttpCache();
#else
    // Qt < 6.7 has no clearHttpCacheCompleted signal.
    webProfile->clearHttpCache();
    QTimer::singleShot(500, webProfile, finished);
#endif
}

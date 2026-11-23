/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
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

#include <QSettings>
#include <QUrl>
#include <QWebEngineCertificateError>
#include <QWebEnginePage>
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QWebEnginePermission>
#endif
#include <QWebEngineView>

#include <functional>

class QAuthenticator;
class WebView;

class WebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WebPage(QWebEngineProfile *profile, WebView *parent);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber,
                                  const QString &sourceID) override;

private:
    WebView *m_webView;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    static QString permissionDescription(QWebEnginePermission::PermissionType type);
    void handlePermissionRequest(QWebEnginePermission permission);
#else
    static QString permissionDescription(QWebEnginePage::Feature feature);
    void handleFeaturePermissionRequest(const QUrl &origin, QWebEnginePage::Feature feature);
#endif
    // Asks whether the site may do "what"; the answer is true when the user allows it.
    void askPermission(const QUrl &origin, const QString &what, const std::function<void(bool)> &answer);
    void handleCertificateError(QWebEngineCertificateError error);
    void askCredentials(const QString &message, const QString &keychainKey, QAuthenticator *auth);
};

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QWebEngineProfile *profile, QWidget *parent = nullptr);
    WebView *createWindow(QWebEnginePage::WebWindowType type) override;

    static bool lastClickWasNewTabRequest();
    static bool consumeIfNewTabRequest();  // Check, mark consumed, and clear - returns true if was new tab request
    static void clearClickState();
    static bool wasClickConsumed();

    // Reader view is generated HTML shown in place of the page, with the page's URL kept as base.
    [[nodiscard]] bool isReaderMode() const { return readerMode; }
    void showReaderPage(const QString &html);
    // A crash notice or reader view: generated HTML shown under the page's own URL.
    [[nodiscard]] bool isGeneratedPage() const;
    // Like reload(), but a generated page is replaced by a fresh load of the real page.
    void reloadPage();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool event(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void handleLoadFinished(bool ok);
    void handleIconChanged();
    void handleRenderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus status);

signals:
    void newWebView(WebView *wv, bool makeCurrent);

private:
    QUrl lastHistoryUrl; // The page most recently logged to history, which gets its site's icon.
    bool readerMode {};
    QWidget *m_currentProxy = nullptr;
    QWebEngineProfile *profile {};

    // Static because Chromium creates new WebViews for navigation,
    // but the click is captured on the original view
    static bool s_ctrlHeld;
    static bool s_middleClick;
    static bool s_consumed;  // Set when acceptNavigationRequest handles the click
    static qint64 s_clickTime; // When that click happened (ms since the epoch)
    static constexpr qint64 clickLifetimeMs {1000};

    void installEventFilterOnFocusProxy();
};

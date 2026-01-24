/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023 MX Authors
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
#include <QWebEnginePage>
#include <QWebEngineView>

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

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void handleLoadFinished(bool ok);
    void handleIconChanged();

signals:
    void newWebView(WebView *wv, bool makeCurrent);

private:
    QSettings historyLog;
    int index;
    int lastHistoryIndex = -1;
    QUrl lastHistoryUrl;
    QWidget *m_currentProxy = nullptr;
    QWebEngineProfile *profile {};

    // Static because Chromium creates new WebViews for navigation,
    // but the click is captured on the original view
    static bool s_ctrlHeld;
    static bool s_middleClick;
    static bool s_consumed;  // Set when acceptNavigationRequest handles the click

    void installEventFilterOnFocusProxy();
};

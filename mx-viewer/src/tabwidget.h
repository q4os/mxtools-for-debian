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

#include <QTabWidget>
#include <QWebEngineProfile>
#include "webview.h"

class QPushButton;

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QWebEngineProfile *profile, QWidget *parent = nullptr);
    WebView *currentWebView();

    WebView *createTab(bool makeCurrent = true);
    void addNewTab(WebView *webView, bool makeCurrent = true);
    void removeTab(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void newTabButtonClicked();
    void tabClosed(const QUrl &url);

private:
    QPushButton *newTabButton {};
    QWebEngineProfile *profile {};
    void handleCurrentChanged(int index);
    void finalizeRemoveTab(int index);
    void updateNewTabButton();
    void positionNewTabButton();
};

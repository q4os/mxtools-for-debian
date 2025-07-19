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
#include "webview.h"
#include "mainwindow.h"

#include <QBuffer>
#include <QPointer>
#include <QWebEngineHistoryItem>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent),
      index(historyLog.value("History/size", 0).toInt())
{
    connect(this, &WebView::loadFinished, this, &WebView::handleLoadFinished);
    connect(this, &WebView::iconChanged, this, &WebView::handleIconChanged);
}

WebView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    auto *newView = new WebView;
    if (type == QWebEnginePage::WebBrowserTab) {
        QPointer<WebView> viewPtr(newView);
        connect(newView->page(), &QWebEnginePage::urlChanged, this, [this, viewPtr] {
            if (viewPtr) {
                emit newWebView(viewPtr.data());
            }
        });
    } else if (type == QWebEnginePage::WebBrowserWindow) {
        connect(newView->page(), &QWebEnginePage::urlChanged, this, [](const QUrl &url) {
            auto *main = new MainWindow(url);
            main->show();
        });
    }
    return newView;
}

void WebView::handleLoadFinished()
{
    index = historyLog.value("History/size", 0).toInt();
    historyLog.beginWriteArray("History");
    historyLog.setArrayIndex(index);
    historyLog.setValue("title", title());
    historyLog.setValue("url", url().toString());
    historyLog.endArray();
}

// Assumes this trigger after loadFinished, that might not be correct all the time
void WebView::handleIconChanged()
{
    if (icon().isNull()) {
        return;
    }
    index = historyLog.value("History/size", 0).toInt();
    checkRecordComplete();
    historyLog.beginWriteArray("History");
    historyLog.setArrayIndex(index);
    QPixmap iconPixmap = icon().pixmap(QSize(22, 22));
    QByteArray iconByteArray;
    QBuffer buffer(&iconByteArray);
    if (buffer.open(QIODevice::WriteOnly)) {
        iconPixmap.save(&buffer, "PNG");
    }
    historyLog.setValue("icon", iconByteArray);
    historyLog.endArray();
}

// Check if the last record is complete, if not, decrement index and go back.
// loadFinished and iconChanged trigger independently, assumption is iconChange triggers after loadFinished
void WebView::checkRecordComplete()
{
    if (index <= 0) {
        return;
    }
    historyLog.beginReadArray("History");
    if (index > 0) {
        historyLog.setArrayIndex(index - 1);
        if (historyLog.allKeys().count() != 3) { // if not all 3 keys were written
            --index;
        }
    }
    historyLog.endArray();
}

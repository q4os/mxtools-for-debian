/*****************************************************************************
 * readermode.cpp
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

#include <QFile>
#include <QRegularExpression>
#include <QMessageBox>
#include <QPointer>
#include <QWebEngineScript>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

void MainWindow::toggleReaderMode()
{
    QPointer<WebView> view = currentWebView();
    if (!view) {
        return;
    }
    if (view->isReaderMode()) {
        // Reader view was pushed on top of the original page, so going back restores it without a reload.
        const auto *history = view->history();
        if (history->canGoBack() && history->backItem().url() == view->url()) {
            view->back();
        } else {
            view->load(view->url());
        }
        return;
    }
    const QString scheme = view->url().scheme();
    if (scheme != "http" && scheme != "https" && scheme != "file") {
        return;
    }
    static const QString readability = [] {
        QFile file(":/readability/Readability.js");
        return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
    }();
    if (readability.isEmpty()) {
        return;
    }
    // Parse a copy so the page itself is untouched; the isolated world keeps page scripts out of it.
    const QString script = readability + QStringLiteral(R"(
;(function() {
    const article = new Readability(document.cloneNode(true)).parse();
    if (!article || !article.content) {
        return null;
    }
    return {title: article.title || document.title, byline: article.byline || '', siteName: article.siteName || '',
            content: article.content, lang: article.lang || document.documentElement.lang || '',
            dir: article.dir || ''};
})();)");
    const QUrl pageUrl = view->url();
    view->page()->runJavaScript(script, QWebEngineScript::ApplicationWorld, [this, view, pageUrl](const QVariant &result) {
        if (!view || view->url() != pageUrl || view->isReaderMode()) {
            return;
        }
        const QVariantMap article = result.toMap();
        if (article.isEmpty()) {
            auto *box = new QMessageBox(QMessageBox::Information, tr("Reader view"),
                                        tr("No article was found on this page."), QMessageBox::Ok, this);
            box->setAttribute(Qt::WA_DeleteOnClose);
            box->open();
            return;
        }
        const auto field = [&article](const char *key) { return article.value(key).toString().toHtmlEscaped(); };
        const QString meta = QStringList {field("siteName"), field("byline")}.filter(QRegularExpression("\\S")).join(" · ");
        // The article HTML comes from the page, so the reader page may not run scripts, submit forms
        // or load anything but images, media and fonts.
        const QString html = QStringLiteral(
            "<!DOCTYPE html><html lang=\"%1\" dir=\"%2\" data-mx-reader><head><meta charset=\"utf-8\">"
            "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; img-src * data:; "
            "media-src *; font-src *; style-src 'unsafe-inline'; form-action 'none'; base-uri 'none'\">"
            "<title>%3</title><style>"
            ":root{color-scheme:light dark}"
            "body{margin:0;background:#f8f6f0;color:#222;font:1.15rem/1.65 Georgia,'DejaVu Serif',serif}"
            "a{color:#1a5fb4}"
            "@media (prefers-color-scheme:dark){body{background:#1f1f1f;color:#ddd}a{color:#8ab4f8}}"
            "main{max-width:40em;margin:0 auto;padding:2em 1.25em 4em}"
            "h1{font-family:sans-serif;line-height:1.25;margin-bottom:.3em}"
            ".meta{color:#888;font:.9rem sans-serif;margin-top:0}"
            "img,video,svg,iframe{max-width:100%;height:auto}figure{margin:1.5em 0}"
            "pre{overflow:auto;white-space:pre-wrap}table{display:block;max-width:100%;overflow:auto}"
            "</style></head><body><main><h1>%3</h1><p class=\"meta\">%4</p>%5</main></body></html>")
                                 .arg(field("lang"), field("dir"), field("title"), meta,
                                      article.value("content").toString());
        // setHtml() passes the page as a data: URL, which is limited to 2 MB once encoded.
        constexpr qsizetype maxReaderSize {1400 * 1024};
        if (html.toUtf8().size() > maxReaderSize) {
            auto *box = new QMessageBox(QMessageBox::Information, tr("Reader view"),
                                        tr("This article is too large for reader view."), QMessageBox::Ok, this);
            box->setAttribute(Qt::WA_DeleteOnClose);
            box->open();
            return;
        }
        view->showReaderPage(html);
    });
}

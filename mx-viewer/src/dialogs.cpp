/*****************************************************************************
 * dialogs.cpp
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

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <memory>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QStandardPaths>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

namespace
{
// Keycap chips for one shortcut text like "Ctrl+Tab, Ctrl+PgDn" or "Ctrl+1 … Ctrl+9".
QWidget *shortcutKeys(const QString &text, QWidget *parent)
{
    auto *keysWidget = new QWidget(parent);
    auto *keysLayout = new QHBoxLayout(keysWidget);
    keysLayout->setContentsMargins(0, 0, 0, 0);
    keysLayout->setSpacing(3);
    const auto addText = [keysWidget, keysLayout](const QString &separator) {
        auto *label = new QLabel(separator, keysWidget);
        label->setObjectName("keySeparator");
        keysLayout->addWidget(label);
    };
    const QStringList alternatives = text.split(QStringLiteral(", "));
    for (qsizetype alt = 0; alt < alternatives.size(); ++alt) {
        if (alt > 0) {
            addText(MainWindow::tr("or"));
        }
        const QStringList ends = alternatives.at(alt).split(QStringLiteral(" … "));
        for (qsizetype end = 0; end < ends.size(); ++end) {
            if (end > 0) {
                addText(QStringLiteral("…"));
            }
            // "+" joins keys, except right after a join, where it is the key itself ("Ctrl++").
            QStringList keys;
            QString key;
            for (const QChar c : ends.at(end)) {
                if (c == u'+' && !key.isEmpty()) {
                    keys.append(key);
                    key.clear();
                } else {
                    key += c;
                }
            }
            if (!key.isEmpty()) {
                keys.append(key);
            }
            for (const QString &name : std::as_const(keys)) {
                auto *cap = new QLabel(name.trimmed(), keysWidget);
                cap->setObjectName("keycap");
                cap->setAlignment(Qt::AlignCenter);
                keysLayout->addWidget(cap);
            }
        }
    }
    keysLayout->addStretch();
    return keysWidget;
}
} // namespace

void MainWindow::openClearDataDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Clear browsing data"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    auto *range = new QComboBox(&dialog);
    // Values are ages in seconds; 0 means everything.
    range->addItem(tr("Last hour"), 3600);
    range->addItem(tr("Last 24 hours"), 24 * 3600);
    range->addItem(tr("Last 7 days"), 7 * 24 * 3600);
    range->addItem(tr("Last 4 weeks"), 28 * 24 * 3600);
    range->addItem(tr("All time"), 0);
    range->setCurrentIndex(range->count() - 1);
    form->addRow(tr("Time range:"), range);
    layout->addLayout(form);

    // A private window keeps no history of its own and must not touch the regular one on disk.
    auto *historyBox = new QCheckBox(privateWindow ? tr("Recently closed tabs")
                                                   : tr("Browsing history and recently closed tabs"),
                                     &dialog);
    auto *cookiesBox = new QCheckBox(tr("Cookies"), &dialog);
    auto *cacheBox = new QCheckBox(tr("Cached images and files"), &dialog);
    auto *permissionsBox = new QCheckBox(tr("Site permissions"), &dialog);
    historyBox->setChecked(true);
    cookiesBox->setChecked(true);
    cacheBox->setChecked(true);
    for (auto *box : {historyBox, cookiesBox, cacheBox, permissionsBox}) {
        layout->addWidget(box);
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // QtWebEngine can only clear these completely.
    auto *note = new QLabel(tr("Cookies, cache and site permissions are always cleared for all time."), &dialog);
#else
    // Before Qt 6.8 site permissions are only kept until the page closes, so there is nothing to clear.
    permissionsBox->hide();
    auto *note = new QLabel(tr("Cookies and cache are always cleared for all time."), &dialog);
#endif
    note->setWordWrap(true);
    note->setEnabled(false);
    layout->addWidget(note);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    auto *clear = buttons->addButton(tr("Clear data"), QDialogButtonBox::AcceptRole);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    auto updateButton = [=] {
        clear->setEnabled(historyBox->isChecked() || cookiesBox->isChecked() || cacheBox->isChecked()
                          || permissionsBox->isChecked());
    };
    for (auto *box : {historyBox, cookiesBox, cacheBox, permissionsBox}) {
        connect(box, &QCheckBox::toggled, &dialog, updateButton);
    }
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const qint64 age = range->currentData().toLongLong();
    if (historyBox->isChecked()) {
        if (privateWindow) {
            webProfile->clearAllVisitedLinks(); // The window's own off-the-record profile.
            closedTabs.clear();
        } else {
            if (age == 0) {
                clearHistoryEntries();
                webProfile->clearAllVisitedLinks();
            } else {
                // Entries without a timestamp predate this feature, so they are older than any range offered.
                HistoryStore::removeSince(QDateTime::currentSecsSinceEpoch() - age);
            }
            // Closed tabs carry no time; they are all from this session, which is usually recent. Every
            // regular window keeps its own list, and all of them belong to the history being cleared.
            for (auto *widget : QApplication::topLevelWidgets()) {
                auto *window = qobject_cast<MainWindow *>(widget);
                if (window && !window->privateWindow) {
                    window->closedTabs.clear();
                }
            }
        }
        if (auto *view = currentWebView(); view && view->url().scheme() == "mx-history") {
            renderHistoryPage(view);
        }
    }
    if (cookiesBox->isChecked()) {
        webProfile->cookieStore()->deleteAllCookies();
    }
    if (cacheBox->isChecked()) {
        webProfile->clearHttpCache();
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    if (permissionsBox->isChecked()) {
        const auto permissions = webProfile->listAllPermissions();
        for (const auto &permission : permissions) {
            permission.reset();
        }
    }
#endif
}

// namespace
void MainWindow::openQuickInfo()
{
    struct Section {
        QString title;
        QList<std::pair<QString, QString>> shortcuts;
    };
    const QList<Section> sections {
        {tr("Tabs and windows"),
         {
             {tr("Ctrl+T"), tr("New tab")},
             {tr("Ctrl+Shift+N"), tr("New private window")},
             {tr("Ctrl+W"), tr("Close tab")},
             {tr("Ctrl+Shift+T"), tr("Reopen closed tab")},
             {tr("Ctrl+Tab, Ctrl+PgDn"), tr("Next tab")},
             {tr("Ctrl+Shift+Tab, Ctrl+PgUp"), tr("Previous tab")},
             {tr("Ctrl+1 … Ctrl+9"), tr("Go to tab 1 … 9")},
         }},
        {tr("Navigation"),
         {
             {tr("Ctrl+L, Alt+D, F6"), tr("Focus the address bar")},
             {tr("Alt+←, Alt+→"), tr("Back/Forward")},
             {tr("Alt+Home"), tr("Home page")},
             {tr("Ctrl+R, F5"), tr("Reload")},
             {tr("Esc"), tr("Stop loading/close Find bar")},
         }},
        {tr("Page"),
         {
             {tr("Ctrl+F, /"), tr("Find")},
             {tr("F3, Shift+F3"), tr("Find next/previous")},
             {tr("Ctrl++, Ctrl+-"), tr("Zoom in/out")},
             {tr("Ctrl+0"), tr("Reset zoom")},
             {tr("F9, Ctrl+Alt+R"), tr("Reader view")},
             {tr("F11"), tr("Full screen")},
             {tr("Hold Esc"), tr("Exit full screen")},
             {tr("Ctrl+S"), tr("Save page")},
             {tr("Ctrl+P"), tr("Print")},
         }},
        {tr("Browser"),
         {
             {tr("Ctrl+D"), tr("Bookmark current address")},
             {tr("Ctrl+Shift+O"), tr("Manage bookmarks")},
             {tr("Ctrl+Shift+B"), tr("Show or hide the bookmarks bar")},
             {tr("Ctrl+H"), tr("History")},
             {tr("Ctrl+J"), tr("Downloads")},
             {tr("Ctrl+Shift+Del"), tr("Clear browsing data")},
             {tr("Ctrl+O"), tr("Browse file to open")},
             {tr("Ctrl+,"), tr("Settings")},
             {tr("F10"), tr("Menu")},
             {tr("F12"), tr("Developer Tools")},
             {tr("F1, ?"), tr("Open this help dialog")},
         }},
    };

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Keyboard Shortcuts"));
    // Palette roles keep the keycaps readable on light and dark themes.
    dialog.setStyleSheet(QStringLiteral(
        "QLabel#keycap { background: palette(button); color: palette(button-text);"
        " border: 1px solid palette(mid); border-bottom-width: 2px; border-radius: 4px;"
        " padding: 1px 6px; min-width: 12px; }"
        "QLabel#keySeparator { color: palette(placeholder-text); }"
        "QLabel#sectionTitle { font-weight: bold; }"));

    auto *content = new QWidget;
    auto *grid = new QGridLayout(content);
    grid->setContentsMargins(16, 12, 16, 12);
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(6);
    int row = 0;
    for (const Section &section : sections) {
        if (row > 0) {
            grid->setRowMinimumHeight(row++, 10);
        }
        auto *title = new QLabel(section.title, content);
        title->setObjectName("sectionTitle");
        grid->addWidget(title, row++, 0, 1, 2);
        for (const auto &[keys, action] : section.shortcuts) {
            grid->addWidget(new QLabel(action, content), row, 0);
            grid->addWidget(shortcutKeys(keys, content), row, 1);
            ++row;
        }
    }
    grid->setColumnStretch(1, 1);
    grid->setRowStretch(row, 1);

    auto *scroll = new QScrollArea(&dialog);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0, 0, 0, 12);
    layout->addWidget(scroll);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    buttons->setContentsMargins(12, 0, 12, 0);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    dialog.resize(qMax(520, content->sizeHint().width() + scroll->verticalScrollBar()->sizeHint().width()), 620);
    dialog.exec();
}

void MainWindow::openAbout()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("About MX Viewer"));

    auto *iconLabel = new QLabel(&dialog);
    iconLabel->setPixmap(QApplication::windowIcon().pixmap(QSize(64, 64), devicePixelRatioF()));
    iconLabel->setAlignment(Qt::AlignTop);

    auto *name = new QLabel(QStringLiteral("MX Viewer"), &dialog);
    QFont nameFont = name->font();
    nameFont.setPointSizeF(nameFont.pointSizeF() * 1.6);
    nameFont.setBold(true);
    name->setFont(nameFont);
    auto *version = new QLabel(tr("Version %1").arg(QApplication::applicationVersion()), &dialog);
    auto *website = new QLabel(QStringLiteral("<a href=\"https://mxlinux.org\">mxlinux.org</a>"), &dialog);
    website->setOpenExternalLinks(true);

    auto *titleLayout = new QVBoxLayout;
    titleLayout->setSpacing(2);
    titleLayout->addWidget(name);
    titleLayout->addWidget(version);
    titleLayout->addWidget(website);
    titleLayout->addStretch();
    auto *header = new QHBoxLayout;
    header->setSpacing(16);
    header->addWidget(iconLabel);
    header->addLayout(titleLayout, 1);

    // The first paragraph describes the program, the rest is the license notice.
    const QStringList paragraphs
        = tr("This is a VERY basic browser based on Qt WebEngine.\n\n"
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
             "along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>.")
              .split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);
    const auto toHtml = [](const QString &text) {
        static const QRegularExpression link(QStringLiteral("&lt;(https?://[^&]+)&gt;"));
        return text.toHtmlEscaped().replace(link, QStringLiteral("<a href=\"\\1\">\\1</a>"));
    };
    const int descriptionCount = paragraphs.size() > 2 ? 2 : static_cast<int>(paragraphs.size());
    QString description;
    for (int i = 0; i < descriptionCount; ++i) {
        description += QStringLiteral("<p>%1</p>").arg(toHtml(paragraphs.at(i)));
    }
    auto *descriptionLabel = new QLabel(description, &dialog);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    descriptionLabel->setOpenExternalLinks(true);

    auto *license = new QTextBrowser(&dialog);
    license->setOpenExternalLinks(true);
    QString licenseHtml = QStringLiteral("<p>Copyright © 2022–2026 MX Authors</p>");
    for (qsizetype i = descriptionCount; i < paragraphs.size(); ++i) {
        licenseHtml += QStringLiteral("<p>%1</p>").arg(toHtml(paragraphs.at(i)));
    }
    license->setHtml(licenseHtml);
    license->setVisible(false);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    QPushButton *licenseButton = buttons->addButton(tr("License"), QDialogButtonBox::HelpRole);
    licenseButton->setCheckable(true);
    connect(licenseButton, &QPushButton::toggled, license, &QWidget::setVisible);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 12);
    layout->setSpacing(12);
    layout->addLayout(header);
    layout->addWidget(descriptionLabel);
    layout->addWidget(license);
    layout->addWidget(buttons);
    // Fixed size follows the content, so the dialog grows and shrinks as the license is shown and hidden.
    layout->setSizeConstraint(QLayout::SetFixedSize);
    descriptionLabel->setFixedWidth(440);
    license->setFixedSize(440, 220);
    dialog.exec();
}

void MainWindow::openBrowseDialog()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select file to open"), QDir::homePath(),
                                                tr("Hypertext Files (*.htm *.html);;All Files (*.*)"));
    if (QFileInfo::exists(file)) {
        displaySite(file, file);
    }
}

void MainWindow::printPage(WebView *webView)
{
    QPointer<WebView> view = webView;
    if (!view || printingView) {
        return;
    }
    // QWebEngineView::print() is asynchronous. The printer is owned by a connection whose context is
    // the view, so it stays alive until printFinished and is never freed before the view itself.
    auto printer = std::make_shared<QPrinter>(QPrinter::HighResolution);
    // Print to file defaults to the user's Downloads folder, named after the page, rather than the
    // working directory.
    QString folder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (folder.isEmpty() || !QDir(folder).exists()) {
        folder = QDir::homePath();
    }
    static const QRegularExpression unsafe(QStringLiteral("[/\\\\\\x00-\\x1f]"));
    QString name = view->title().simplified().replace(unsafe, QStringLiteral("_")).left(100);
    // File names are limited to 255 bytes, and a title in a non-Latin script takes up to 3 bytes per
    // character; leave room for ".pdf" and never cut a surrogate pair.
    while (!name.isEmpty() && name.back().isHighSurrogate()) {
        name.chop(1);
    }
    while (name.toUtf8().size() > 250) {
        name.chop(name.size() >= 2 && name.back().isLowSurrogate() ? 2 : 1);
    }
    while (name.startsWith(QLatin1Char('.'))) {
        name.remove(0, 1);
    }
    if (name.isEmpty()) {
        name = view->url().host().isEmpty() ? QStringLiteral("page") : view->url().host();
    }
    printer->setOutputFileName(QDir(folder).filePath(name + QStringLiteral(".pdf")));
    // A .pdf name switches the printer to PDF; switch back so a real printer, if any, stays the default.
    printer->setOutputFormat(QPrinter::NativeFormat);
    QPrintDialog dialog(printer.get(), this);
    dialog.setWindowTitle(tr("Print page"));
    if (dialog.exec() != QDialog::Accepted || !view) {
        return;
    }
    printingView = view;
    QPointer<MainWindow> self = this;
    connect(
        view, &QWebEngineView::printFinished, view,
        [self, view, printer](bool success) {
            if (self) {
                self->printingView = nullptr;
            }
            if (!success && view) {
                QMessageBox::warning(view, tr("Print page"), tr("Printing failed."));
            }
        },
        Qt::SingleShotConnection);
    view->print(printer.get());
}

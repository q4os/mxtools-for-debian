/*****************************************************************************
 * mainwindow.cpp
 *****************************************************************************
 * Copyright (C) 2022 MX Authors
 *
 * Authors: Adrian
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

#include "version.h"
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(const QCommandLineParser &arg_parser, QWidget *parent)
    : QMainWindow(parent)
    , args {arg_parser}
{
    timer = new QTimer(this);
    toolBar = new QToolBar(this);
    toolBar->toggleViewAction()->setVisible(false);
    webview = new QWebEngineView(this);
    downloadWidget = new DownloadWidget;
    progressBar = new QProgressBar(this);
    searchBox = new QLineEdit(this);
    websettings = webview->settings();

    loadSettings();
    addToolbar();
    addActions();
    setConnections();

    if (arg_parser.isSet(QStringLiteral("full-screen"))) {
        this->showFullScreen();
        toolBar->hide();
    }
    QString url;
    QString title;
    if (!args.positionalArguments().isEmpty()) {
        url = args.positionalArguments().at(0);
        title = (args.positionalArguments().size() > 1) ? args.positionalArguments().at(1) : url;
    }
    displaySite(url, title);
}

MainWindow::~MainWindow()
{
    settings.setValue(QStringLiteral("Geometry"), saveGeometry());
    listHistory();
    saveMenuItems(history, 2);
    saveMenuItems(bookmarks, 2);
}

void MainWindow::addActions()
{
    auto *full = new QAction(tr("Full screen"));
    full->setShortcut(Qt::Key_F11);
    this->addAction(full);
    connect(full, &QAction::triggered, this, &MainWindow::toggleFullScreen);
}

void MainWindow::addBookmarksSubmenu()
{
    bookmarks->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookmarks, &QMenu::customContextMenuRequested, this, [this](QPoint pos) {
        if (bookmarks->actionAt(pos) == bookmarks->actions().at(0)) // skip first "Add bookmark" action.
            return;
        QPoint globalPos = bookmarks->mapToGlobal(pos);
        QMenu submenu;
        if (bookmarks->actionAt(pos) != bookmarks->actions().at(1)) {
            submenu.addAction(QIcon::fromTheme(QStringLiteral("arrow-up")), tr("Move up"), bookmarks, [this, pos]() {
                for (int i = 2; i < bookmarks->actions().count();
                     ++i) { // starting from third action (ignoring "Add bookmark" and first bookmark we cannot move up)
                    if (bookmarks->actions().at(i) == bookmarks->actionAt(pos))
                        bookmarks->insertAction(bookmarks->actions().at(i - 1), bookmarks->actions().at(i));
                }
            });
        }
        if (bookmarks->actionAt(pos) != bookmarks->actions().at(bookmarks->actions().count() - 1)) {
            submenu.addAction(
                QIcon::fromTheme(QStringLiteral("arrow-down")), tr("Move down"), bookmarks, [this, pos]() {
                    for (int i = 1; i < bookmarks->actions().count();
                         ++i) { // starting from second action (ignoring "Add bookmark")
                        if (bookmarks->actions().at(i) == bookmarks->actionAt(pos))
                            bookmarks->insertAction(bookmarks->actions().at(i + 2), bookmarks->actions().at(i));
                    }
                });
        }
        submenu.addAction(QIcon::fromTheme(QStringLiteral("edit-symbolic")), tr("Rename"), bookmarks, [this, pos]() {
            auto *edit = new QInputDialog(this);
            edit->setInputMode(QInputDialog::TextInput);
            edit->setOkButtonText(tr("Save"));
            edit->setTextValue(bookmarks->actionAt(pos)->text());
            edit->setLabelText(tr("Rename bookmark:"));
            edit->resize(300, edit->height());
            if (edit->exec() == QDialog::Accepted)
                bookmarks->actionAt(pos)->setText(edit->textValue());
        });
        submenu.addAction(QIcon::fromTheme(QStringLiteral("user-trash")), tr("Delete"), bookmarks,
                          [this, pos]() { bookmarks->removeAction(bookmarks->actionAt(pos)); });
        submenu.exec(globalPos);
    });
}

void MainWindow::addHistorySubmenu()
{
    history->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(history, &QMenu::customContextMenuRequested, this, [this](QPoint pos) {
        if (history->actionAt(pos) == history->actions().at(0)) // skip first "Clear history" action.
            return;
        QPoint globalPos = history->mapToGlobal(pos);
        QMenu submenu;
        submenu.addAction(QIcon::fromTheme(QStringLiteral("user-trash")), tr("Delete"), history, [this, pos]() {
            history->removeAction(history->actionAt(pos));
            saveMenuItems(history,
                          3); // skip "clear history", separator, and first item which gets added at menu refresh
            webview->history()->clear(); // next menu refresh will load from saved file not from webview->history()
        });
        submenu.exec(globalPos);
    });
}

void MainWindow::listHistory()
{
    history->clear();
    auto *deleteHistory = new QAction(QIcon::fromTheme(QStringLiteral("user-trash")), tr("&Clear history"));
    connect(deleteHistory, &QAction::triggered, this, [this]() {
        history->clear();
        webview->history()->clear();
        saveMenuItems(history, 2);
    });
    history->addAction(deleteHistory);
    history->addSeparator();
    auto *hist = webview->history();

    for (int i = hist->items().count() - 1; i >= 0; --i) {
        auto item = hist->itemAt(i);
        QAction *histItem {nullptr};
        history->addAction(histItem = new QAction(histIcons.value(item.url()), item.title()));
        histItem->setProperty("url", item.url());
        connectAddress(histItem, history);
    }
    loadHistory();
}

void MainWindow::addToolbar()
{
    this->addToolBar(toolBar);
    this->setCentralWidget(webview);

    QAction *back {nullptr};
    QAction *forward {nullptr};
    QAction *reload {nullptr};
    QAction *stop {nullptr};
    //    QAction *tab {nullptr};
    QAction *home {nullptr};
    QAction *zoomin {nullptr};
    QAction *zoompercent {nullptr};
    QAction *zoomout {nullptr};

    toolBar->addAction(back = webview->pageAction(QWebEnginePage::Back));
    toolBar->addAction(forward = webview->pageAction(QWebEnginePage::Forward));
    toolBar->addAction(reload = webview->pageAction(QWebEnginePage::Reload));
    toolBar->addAction(stop = webview->pageAction(QWebEnginePage::Stop));
    toolBar->addAction(
        home = new QAction(QIcon::fromTheme(QStringLiteral("go-home"), QIcon(QStringLiteral(":/icons/go-home.svg"))),
                           tr("Home")));
    back->setShortcut(QKeySequence::Back);
    forward->setShortcut(QKeySequence::Forward);
    home->setShortcut(Qt::CTRL + Qt::Key_H);
    reload->setShortcuts(QKeySequence::Refresh);
    stop->setShortcut(QKeySequence::Cancel);
    //    tab = webview->pageAction(QWebEnginePage::OpenLinkInNewTab);
    //    tab->setShortcut(Qt::CTRL + Qt::Key_T);
    connect(stop, &QAction::triggered, this, &MainWindow::done);
    connect(home, &QAction::triggered, this, [this]() { displaySite(); });

    searchBox->setPlaceholderText(tr("search in page"));
    searchBox->setClearButtonEnabled(true);
    searchBox->setMaximumWidth(searchWidth);
    searchBox->setClearButtonEnabled(true);
    searchBox->addAction(QIcon::fromTheme(QStringLiteral("search"), QIcon(QStringLiteral(":/icons/system-search.png"))),
                         QLineEdit::LeadingPosition);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::findForward);
    connect(searchBox, &QLineEdit::returnPressed, this, &MainWindow::findForward);
    addressBar = new AddressBar(this);
    addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addressBar->setClearButtonEnabled(true);
    addBookmark = addressBar->addAction(
        QIcon::fromTheme(QStringLiteral("emblem-favorite"), QIcon(QStringLiteral(":/icons/emblem-favorite.png"))),
        QLineEdit::TrailingPosition);
    addBookmark->setToolTip(tr("Add bookmark"));
    connect(addressBar, &QLineEdit::returnPressed, this, [this]() { displaySite(addressBar->text()); });
    toolBar->addWidget(addressBar);
    toolBar->addWidget(searchBox);
    toolBar->addAction(zoomout = new QAction(
                           QIcon::fromTheme(QStringLiteral("zoom-out"), QIcon(QStringLiteral(":/icons/zoom-out.svg"))),
                           tr("Zoom out")));
    toolBar->addAction(zoompercent = new QAction(QStringLiteral("100%")));
    toolBar->addAction(
        zoomin = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in"), QIcon(QStringLiteral(":/icons/zoom-in.svg"))),
                             tr("Zoom In")));
    toolBar->addAction(menuButton = new QAction(QIcon::fromTheme(QStringLiteral("open-menu"),
                                                                 QIcon(QStringLiteral(":/icons/open-menu.png"))),
                                                tr("Settings")));
    const auto step = 0.1;
    zoomin->setShortcuts({QKeySequence::ZoomIn, Qt::CTRL + Qt::Key_Equal});
    zoomout->setShortcut(QKeySequence::ZoomOut);
    zoompercent->setShortcut(Qt::CTRL + Qt::Key_0);
    connect(zoomout, &QAction::triggered, this, [this, step, zoompercent]() {
        webview->setZoomFactor(webview->zoomFactor() - step);
        zoompercent->setText(QString::number(webview->zoomFactor() * 100) + "%");
    });
    connect(zoomin, &QAction::triggered, this, [this, step, zoompercent]() {
        webview->setZoomFactor(webview->zoomFactor() + step);
        zoompercent->setText(QString::number(webview->zoomFactor() * 100) + "%");
    });
    connect(zoompercent, &QAction::triggered, this, [this, zoompercent]() {
        webview->setZoomFactor(1);
        zoompercent->setText(QStringLiteral("100%"));
    });
    menuButton->setShortcut(Qt::Key_F10);
    buildMenu();
    toolBar->show();
}

void MainWindow::openBrowseDialog()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select file to open"), QDir::homePath(),
                                                tr("Hypertext Files (*.htm *.html);;All Files (*.*)"));
    if (QFileInfo::exists(file))
        displaySite(file, file);
}

// pop up a window and display website
void MainWindow::displaySite(QString url, const QString &title)
{
    if (url.isEmpty())
        url = homeAddress;
    disconnect(conn);
    conn = connect(webview, &QWebEngineView::loadFinished, [url](bool ok) {
        if (!ok)
            qDebug() << "Error :" << url;
    });
    if (QFile::exists(url))
        url = QFileInfo(url).absoluteFilePath();
    QUrl qurl = QUrl::fromUserInput(url);
    webview->setUrl(qurl);
    webview->load(qurl);
    webview->show();
    showProgress ? loading() : progressBar->hide();
    this->setWindowTitle(title);
}

void MainWindow::loadBookmarks()
{
    int size = settings.beginReadArray(QStringLiteral("Bookmarks"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark = new QAction(settings.value(QStringLiteral("icon")).value<QIcon>(),
                                                    settings.value(QStringLiteral("title")).toString()));
        bookmark->setProperty("url", settings.value(QStringLiteral("url")));
        connectAddress(bookmark, bookmarks);
    }
    settings.endArray();
}

void MainWindow::loadHistory()
{
    int size = settings.beginReadArray(QStringLiteral("History"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *histItem {nullptr};
        history->addAction(histItem = new QAction(settings.value(QStringLiteral("icon")).value<QIcon>(),
                                                  settings.value(QStringLiteral("title")).toString()));
        histItem->setProperty("url", settings.value(QStringLiteral("url")));
        connectAddress(histItem, history);
    }
    settings.endArray();
}

void MainWindow::loadSettings()
{
    // Load first from system .conf file and then overwrite with CLI switches where available
    websettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    websettings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QWebEngineProfile::defaultProfile()->setUseForGlobalCertificateVerification();

    homeAddress = settings.value(QStringLiteral("Home"), QStringLiteral("https://duckduckgo.com")).toString();
    showProgress = settings.value(QStringLiteral("ShowProgressBar"), false).toBool();

    websettings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled,
                              settings.value(QStringLiteral("SpatialNavigation"), false).toBool());
    websettings->setAttribute(QWebEngineSettings::JavascriptEnabled,
                              !settings.value(QStringLiteral("DisableJava"), false).toBool());
    websettings->setAttribute(QWebEngineSettings::AutoLoadImages,
                              settings.value(QStringLiteral("LoadImages"), true).toBool());

    if (args.isSet(QStringLiteral("enable-spatial-navigation")))
        websettings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, true);
    if (args.isSet(QStringLiteral("disable-js")))
        websettings->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    if (args.isSet(QStringLiteral("disable-images")))
        websettings->setAttribute(QWebEngineSettings::AutoLoadImages, false);

    QSize size {defaultWidth, defaultHeight};
    this->resize(size);
    if (settings.contains(QStringLiteral("Geometry")) && !args.isSet(QStringLiteral("full-screen"))) {
        restoreGeometry(settings.value(QStringLiteral("Geometry")).toByteArray());
        if (this->isMaximized()) { // add option to resize if maximized
            this->resize(size);
            centerWindow();
        }
    } else {
        centerWindow();
    }
}

// center main window
void MainWindow::centerWindow()
{
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

void MainWindow::openQuickInfo()
{
    QMessageBox::about(this, tr("Keyboard Shortcuts"),
                       tr("Ctrl-F, or F3") + "\t - " + tr("Find") + "\n" + tr("Shift-F3") + "\t - "
                           + tr("Find previous") + "\n" + tr("Ctrl-R, or F5") + "\t - " + tr("Reload") + "\n"
                           + tr("Ctrl-O") + "\t - " + tr("Browse file to open") + "\n" + tr("Esc") + "\t - "
                           + tr("Stop loading/clear Find field") + "\n" + tr("Alt→, Alt←") + "\t - "
                           + tr("Back/Forward") + "\n" + tr("F1, or ?") + "\t - " + tr("Open this help dialog"));
}

void MainWindow::saveMenuItems(const QMenu *menu, int offset)
{
    // Offset is for skipping "Clear history" item, separator, etc.
    settings.beginWriteArray(menu->objectName());
    for (int i = offset; i < menu->actions().count(); ++i) {
        settings.setArrayIndex(i - offset);
        settings.setValue(QStringLiteral("title"), menu->actions().at(i)->text());
        settings.setValue(QStringLiteral("url"), menu->actions().at(i)->property("url").toString());
        settings.setValue(QStringLiteral("icon"), menu->actions().at(i)->icon());
    }
    settings.endArray();
}

void MainWindow::setConnections()
{
    connect(webview, &QWebEngineView::loadStarted, toolBar, &QToolBar::show); // show toolbar when loading a new page
    connect(webview, &QWebEngineView::urlChanged, this, &MainWindow::updateUrl);
    connect(webview, &QWebEngineView::iconChanged, this,
            [this]() { histIcons.insert(webview->url(), webview->icon()); });
    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, downloadWidget,
            &DownloadWidget::downloadRequested);
    if (showProgress)
        connect(webview, &QWebEngineView::loadStarted, this, &MainWindow::loading);
    connect(webview, &QWebEngineView::loadFinished, this, &MainWindow::done);
    connect(webview->page(), &QWebEnginePage::linkHovered, this, [this](const QString &url) {
        if (url.isEmpty()) {
            this->statusBar()->hide();
        } else {
            this->statusBar()->show();
            this->statusBar()->showMessage(url);
        }
    });
}

void MainWindow::showFullScreenNotification()
{
    const int distance_top = 100;
    const int duration_ms = 800;
    const double start = 0;
    const double end = 0.85;
    auto *label = new QLabel(this);
    auto *effect = new QGraphicsOpacityEffect;
    label->setGraphicsEffect(effect);
    label->setStyleSheet(QStringLiteral("padding: 15px; background-color:#787878; color:white"));
    label->setText(tr("Press [F11] to exit full screen"));
    label->adjustSize();
    label->move(QApplication::primaryScreen()->geometry().width() / 2 - label->width() / 2, distance_top);
    auto *a = new QPropertyAnimation(effect, "opacity");
    a->setDuration(duration_ms);
    a->setStartValue(start);
    a->setEndValue(end);
    a->setEasingCurve(QEasingCurve::InBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    label->show();
    QTimer::singleShot(4s, this, [label, effect, end, start]() {
        auto *a = new QPropertyAnimation(effect, "opacity");
        a->setDuration(duration_ms);
        a->setStartValue(end);
        a->setEndValue(start);
        a->setEasingCurve(QEasingCurve::OutBack);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        connect(a, &QPropertyAnimation::finished, label, &QLabel::deleteLater);
    });
}

// Show the address in the toolbar and also connect it to launch it
void MainWindow::connectAddress(const QAction *action, const QMenu *menu)
{
    connect(action, &QAction::hovered, this, [this, action]() {
        QString url = action->property("url").toString();
        if (url.isEmpty()) {
            this->statusBar()->hide();
        } else {
            this->statusBar()->show();
            this->statusBar()->showMessage(url);
        }
    });
    connect(action, &QAction::triggered, this, [this, action]() {
        QString url = action->property("url").toString();
        displaySite(url);
    });
    connect(menu, &QMenu::aboutToHide, this->statusBar(), &QStatusBar::hide);
}

void MainWindow::buildMenu()
{
    auto *menu = new QMenu(this);
    history = new QMenu(menu);
    bookmarks = new QMenu(menu);
    bookmarks->setObjectName("Bookmarks");
    history->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    history->setObjectName("History");
    bookmarks->setStyleSheet(QStringLiteral("QMenu { menu-scrollable: 1; }"));
    QAction *fullscreen {nullptr};
    QAction *about {nullptr};
    QAction *quit {nullptr};
    QAction *help {nullptr};
    QAction *downloadAction {nullptr};
    QAction *bookmarkAction {nullptr};
    QAction *historyAction {nullptr};
    menuButton->setMenu(menu);

    menu->addAction(fullscreen = new QAction(QIcon::fromTheme(QStringLiteral("view-fullscreen")), tr("&Full screen")));
    menu->addSeparator();
    menu->addAction(historyAction = new QAction(QIcon::fromTheme(QStringLiteral("history")), tr("H&istory")));
    historyAction->setMenu(history);
    menu->addAction(downloadAction
                    = new QAction(QIcon::fromTheme(QStringLiteral("folder-download")), tr("&Downloads")));
    downloadAction->setShortcut(Qt::CTRL + Qt::Key_J);
    menu->addAction(bookmarkAction
                    = new QAction(QIcon::fromTheme(QStringLiteral("emblem-favorite")), tr("&Bookmarks")));
    bookmarkAction->setMenu(bookmarks);
    bookmarks->addAction(addBookmark);
    addBookmark->setText(tr("Bookmark current address"));
    addBookmark->setShortcut(Qt::CTRL + Qt::Key_D);
    bookmarks->addSeparator();
    menu->addSeparator();
    menu->addAction(help = new QAction(QIcon::fromTheme(QStringLiteral("help-contents")), tr("&Help")));
    menu->addAction(about = new QAction(QIcon::fromTheme(QStringLiteral("help-about")), tr("&About")));
    menu->addSeparator();
    menu->addAction(quit = new QAction(QIcon::fromTheme(QStringLiteral("window-close")), tr("&Exit")));

    loadBookmarks();
    addBookmarksSubmenu();
    addHistorySubmenu();

    connect(addBookmark, &QAction::triggered, this, [this]() {
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark = new QAction(webview->icon(), webview->title()));
        bookmark->setProperty("url", webview->url());
        connectAddress(bookmark, bookmarks);
    });

    connect(menuButton, &QAction::triggered, this, [this, menu]() {
        QPoint pos = mapToParent(toolBar->widgetForAction(menuButton)->pos());
        pos.setY(pos.y() + toolBar->widgetForAction(menuButton)->size().height());
        menu->popup(pos);
        listHistory();
    });

    connect(fullscreen, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    QApplication::processEvents();

    connect(downloadAction, &QAction::triggered, downloadWidget, &QWidget::show);
    connect(help, &QAction::triggered, this, &MainWindow::openQuickInfo);
    connect(quit, &QAction::triggered, this, &MainWindow::close);
    connect(about, &QAction::triggered, this, [this]() {
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

void MainWindow::toggleFullScreen()
{
    if (this->isFullScreen()) {
        this->showNormal();
        toolBar->show();
    } else {
        this->showFullScreen();
        toolBar->hide();
        showFullScreenNotification();
    }
}

void MainWindow::updateUrl()
{
    addressBar->show();
    addressBar->setText(webview->url().toDisplayString());
}

void MainWindow::findBackward()
{
    searchBox->setFocus();
    webview->findText(searchBox->text(), QWebEnginePage::FindBackward);
}

void MainWindow::findForward()
{
    searchBox->setFocus();
    webview->findText(searchBox->text());
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::FindNext) || event->matches(QKeySequence::Find) || event->key() == Qt::Key_Slash)
        findForward();
    if (event->matches(QKeySequence::FindPrevious))
        findBackward();
    else if (event->matches(QKeySequence::Open))
        openBrowseDialog();
    else if (event->matches(QKeySequence::HelpContents) || event->key() == Qt::Key_Question)
        openQuickInfo();
    else if (event->matches(QKeySequence::Cancel) && !searchBox->text().isEmpty() && searchBox->hasFocus())
        searchBox->clear();
    else if (event->matches(QKeySequence::Cancel) && searchBox->text().isEmpty())
        webview->setFocus();
    else if (event->key() == Qt::Key_L && event->modifiers() == Qt::ControlModifier)
        addressBar->setFocus();
}

// resize event
void MainWindow::resizeEvent(QResizeEvent * /*event*/)
{
    if (showProgress)
        progressBar->move(this->geometry().width() / 2 - progressBar->width() / 2,
                          this->geometry().height() - progBarVerticalAdj);
}

void MainWindow::closeEvent(QCloseEvent * /*event*/) { downloadWidget->close(); }

// display progressbar while loading page
void MainWindow::loading()
{
    progressBar->setFixedHeight(progBarWidth);
    progressBar->setTextVisible(false);
    progressBar->move(this->geometry().width() / 2 - progressBar->width() / 2,
                      this->geometry().height() - progBarVerticalAdj);
    progressBar->setFocus();
    progressBar->show();
    timer->start(100ms);
    connect(timer, &QTimer::timeout, this, &MainWindow::procTime);
}

// done loading
void MainWindow::done()
{
    timer->stop();
    timer->disconnect();
    webview->stop();
    webview->setFocus();
    searchBox->clear();
    progressBar->hide();
    this->setWindowTitle(webview->title());
}

// advance progressbar
void MainWindow::procTime()
{
    const int step = 5;
    progressBar->setValue((progressBar->value() + step) % progressBar->maximum());
}

/*****************************************************************************
 * mainwindow.cpp
 *****************************************************************************
 * Copyright (C) 2022 MX Authors
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

#include "version.h"
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(const QCommandLineParser &arg_parser, QWidget *parent)
    : QMainWindow(parent),
      downloadWidget {new DownloadWidget},
      searchBox {new QLineEdit(this)},
      progressBar {new QProgressBar(this)},
      timer {new QTimer(this)},
      toolBar {new QToolBar(this)},
      tabWidget {new TabWidget(this)},
      args {&arg_parser}
{
    setAttribute(Qt::WA_DeleteOnClose);
    toolBar->toggleViewAction()->setVisible(false);
    connect(tabWidget, &TabWidget::currentChanged, this, [this] { tabChanged(); });
    auto *webView = currentWebView();
    if (webView) {
        websettings = webView->settings();
    }
    loadSettings();
    addToolbar();
    addActions();
    setConnections();

    if (arg_parser.isSet("full-screen")) {
        showFullScreen();
        toolBar->hide();
    }
    QString url;
    QString title;
    if (args && !args->positionalArguments().isEmpty()) {
        url = args->positionalArguments().at(0);
        title = (args->positionalArguments().size() > 1) ? args->positionalArguments().at(1) : url;
    }
    displaySite(url, title);
}

MainWindow::MainWindow(const QUrl &url, QWidget *parent)
    : QMainWindow(parent),
      downloadWidget {new DownloadWidget},
      searchBox {new QLineEdit(this)},
      progressBar {new QProgressBar(this)},
      timer {new QTimer(this)},
      toolBar {new QToolBar(this)},
      tabWidget {new TabWidget(this)},
      args {nullptr}
{
    setAttribute(Qt::WA_DeleteOnClose);
    toolBar->toggleViewAction()->setVisible(false);
    connect(tabWidget, &TabWidget::currentChanged, this, [this] { tabChanged(); });
    auto *webView = currentWebView();
    if (webView) {
        websettings = webView->settings();
    }
    loadSettings();
    addToolbar();
    addActions();
    setConnections();
    displaySite(url.toString(), QString());
}

MainWindow::~MainWindow()
{
    settings.setValue("Geometry", saveGeometry());
    listHistory();
    saveMenuItems(history, 2);
    saveMenuItems(bookmarks, 2);
}

void MainWindow::addActions()
{
    auto *full = new QAction(tr("Full screen"));
    full->setShortcut(Qt::Key_F11);
    addAction(full);
    connect(full, &QAction::triggered, this, &MainWindow::toggleFullScreen);
}

void MainWindow::addBookmarksSubmenu()
{
    bookmarks->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookmarks, &QMenu::customContextMenuRequested, this, [this](QPoint pos) {
        if (bookmarks->actionAt(pos) == bookmarks->actions().at(0)) { // skip first "Add bookmark" action.
            return;
        }
        QPoint globalPos = bookmarks->mapToGlobal(pos);
        QMenu submenu;
        if (bookmarks->actionAt(pos) != bookmarks->actions().at(1)) {
            submenu.addAction(QIcon::fromTheme("arrow-up"), tr("Move up"), bookmarks, [this, pos] {
                for (int i = 2; i < bookmarks->actions().count();
                     ++i) { // starting from third action (ignoring "Add bookmark" and first bookmark we cannot move up)
                    if (bookmarks->actions().at(i) == bookmarks->actionAt(pos)) {
                        bookmarks->insertAction(bookmarks->actions().at(i - 1), bookmarks->actions().at(i));
                    }
                }
            });
        }
        if (bookmarks->actionAt(pos) != bookmarks->actions().at(bookmarks->actions().count() - 1)) {
            submenu.addAction(QIcon::fromTheme("arrow-down"), tr("Move down"), bookmarks, [this, pos] {
                for (int i = 1; i < bookmarks->actions().count();
                     ++i) { // starting from second action (ignoring "Add bookmark")
                    if (bookmarks->actions().at(i) == bookmarks->actionAt(pos)) {
                        bookmarks->insertAction(bookmarks->actions().at(i + 2), bookmarks->actions().at(i));
                    }
                }
            });
        }
        submenu.addAction(QIcon::fromTheme("edit-symbolic"), tr("Rename"), bookmarks, [this, pos] {
            auto *edit = new QInputDialog(this);
            edit->setInputMode(QInputDialog::TextInput);
            edit->setOkButtonText(tr("Save"));
            edit->setTextValue(bookmarks->actionAt(pos)->text());
            edit->setLabelText(tr("Rename bookmark:"));
            edit->resize(300, edit->height());
            if (edit->exec() == QDialog::Accepted) {
                bookmarks->actionAt(pos)->setText(edit->textValue());
            }
        });
        submenu.addAction(QIcon::fromTheme("user-trash"), tr("Delete"), bookmarks,
                          [this, pos] { bookmarks->removeAction(bookmarks->actionAt(pos)); });
        submenu.exec(globalPos);
    });
}

void MainWindow::addHistorySubmenu()
{
    history->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(history, &QMenu::customContextMenuRequested, this, [this](QPoint pos) {
        if (history->actionAt(pos) == history->actions().at(0)) { // skip first "Clear history" action.
            return;
        }
        QPoint globalPos = history->mapToGlobal(pos);
        QMenu submenu;
        submenu.addAction(QIcon::fromTheme("user-trash"), tr("Delete"), history, [this, pos] {
            history->removeAction(history->actionAt(pos));
            saveMenuItems(history, 3); // skip "clear history", separator, and first item added at menu refresh
        });
        submenu.exec(globalPos);
    });
}

void MainWindow::addNewTab(const QString &url)
{
    tabWidget->createTab();
    displaySite(url);
    setConnections();
}

void MainWindow::listHistory()
{
    history->clear();
    auto *deleteHistory = new QAction(QIcon::fromTheme("user-trash"), tr("&Clear history"));
    connect(deleteHistory, &QAction::triggered, this, [this] {
        history->clear();
        saveMenuItems(history, 2);
    });
    history->addAction(deleteHistory);
    history->addSeparator();
    loadHistory();
}

void MainWindow::addToolbar()
{
    addToolBar(toolBar);
    setCentralWidget(tabWidget);

    auto *back = pageAction(QWebEnginePage::Back);
    auto *forward = pageAction(QWebEnginePage::Forward);
    auto *reload = pageAction(QWebEnginePage::Reload);
    auto *stop = pageAction(QWebEnginePage::Stop);
    auto *home {new QAction(QIcon::fromTheme("go-home", QIcon(":/icons/go-home.svg")), tr("Home"))};
    auto *zoomin {new QAction(QIcon::fromTheme("zoom-in", QIcon(":/icons/zoom-in.svg")), tr("Zoom In"))};
    auto *zoompercent {new QAction("100%")};
    auto *zoomout {new QAction(QIcon::fromTheme("zoom-out", QIcon(":/icons/zoom-out.svg")), tr("Zoom out"))};

    toolBar->addAction(back);
    toolBar->addAction(forward);
    toolBar->addAction(reload);
    toolBar->addAction(stop);
    toolBar->addAction(home);
    back->setShortcut(QKeySequence::Back);
    forward->setShortcut(QKeySequence::Forward);
    home->setShortcut(Qt::CTRL | Qt::Key_H);
    reload->setShortcuts(QKeySequence::Refresh);
    stop->setShortcut(QKeySequence::Cancel);
    connect(stop, &QAction::triggered, this, &MainWindow::done);
    connect(home, &QAction::triggered, this, [this] { displaySite(); });

    searchBox->setPlaceholderText(tr("search in page"));
    searchBox->setClearButtonEnabled(true);
    searchBox->setMaximumWidth(searchWidth);
    searchBox->setClearButtonEnabled(true);
    searchBox->addAction(QIcon::fromTheme("search", QIcon(":/icons/system-search.png")), QLineEdit::LeadingPosition);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::findForward);
    connect(searchBox, &QLineEdit::returnPressed, this, &MainWindow::findForward);
    addressBar = new AddressBar(this);
    addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addressBar->setClearButtonEnabled(true);
    addBookmark = addressBar->addAction(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/emblem-favorite.png")),
                                        QLineEdit::TrailingPosition);
    addBookmark->setToolTip(tr("Add bookmark"));
    connect(addressBar, &QLineEdit::returnPressed, this, [this] { displaySite(addressBar->text()); });
    toolBar->addWidget(addressBar);
    toolBar->addWidget(searchBox);
    toolBar->addAction(zoomout);
    toolBar->addAction(zoompercent);
    toolBar->addAction(zoomin);
    toolBar->addAction(menuButton
                       = new QAction(QIcon::fromTheme("open-menu", QIcon(":/icons/open-menu.png")), tr("Settings")));
    const auto step = 0.1;
    zoomin->setShortcuts({QKeySequence::ZoomIn, Qt::CTRL | Qt::Key_Equal});
    zoomout->setShortcut(QKeySequence::ZoomOut);
    zoompercent->setShortcut(Qt::CTRL | Qt::Key_0);

    connect(zoomout, &QAction::triggered, this, [this, step, zoompercent] {
        currentWebView()->setZoomFactor(currentWebView()->zoomFactor() - step);
        zoompercent->setText(QString::number(currentWebView()->zoomFactor() * 100) + "%");
    });
    connect(zoomin, &QAction::triggered, this, [this, step, zoompercent] {
        currentWebView()->setZoomFactor(currentWebView()->zoomFactor() + step);
        zoompercent->setText(QString::number(currentWebView()->zoomFactor() * 100) + "%");
    });
    connect(zoompercent, &QAction::triggered, this, [this, zoompercent] {
        currentWebView()->setZoomFactor(1);
        zoompercent->setText("100%");
    });
    menuButton->setShortcut(Qt::Key_F10);
    buildMenu();
    toolBar->show();
}

void MainWindow::openBrowseDialog()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select file to open"), QDir::homePath(),
                                                tr("Hypertext Files (*.htm *.html);;All Files (*.*)"));
    if (QFileInfo::exists(file)) {
        displaySite(file, file);
    }
}

// pop up a window and display website
void MainWindow::displaySite(QString url, const QString &title)
{
    if (url.isEmpty()) {
        if (tabWidget->currentIndex() == 0) {
            url = homeAddress;
        } else {
            return;
        }
    }
    disconnect(conn);
    conn = connect(currentWebView(), &QWebEngineView::loadFinished, [url](bool ok) {
        if (!ok) {
            qDebug() << "Error :" << url;
        }
    });
    if (QFile::exists(url)) {
        url = QFileInfo(url).absoluteFilePath();
    }
    QUrl qurl = QUrl::fromUserInput(url);
    currentWebView()->setUrl(qurl);
    currentWebView()->load(qurl);
    currentWebView()->show();
    showProgress ? loading() : progressBar->hide();
    setWindowTitle(title);
}

void MainWindow::loadBookmarks()
{
    int size = settings.beginReadArray("Bookmarks");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark
                             = new QAction(settings.value("icon").value<QIcon>(), settings.value("title").toString()));
        bookmark->setProperty("url", settings.value("url"));
        connectAddress(bookmark, bookmarks);
    }
    settings.endArray();
}

void MainWindow::loadHistory()
{
    int size = settings.beginReadArray("History");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *histItem {nullptr};
        QByteArray iconByteArray = settings.value("icon").toByteArray();
        QPixmap restoredIconPixmap;
        restoredIconPixmap.loadFromData(iconByteArray);
        QIcon restoredIcon;
        restoredIcon.addPixmap(restoredIconPixmap);
        history->addAction(histItem = new QAction(restoredIcon, settings.value("title").toString()));
        histItem->setProperty("url", settings.value("url"));
        connectAddress(histItem, history);
    }
    settings.endArray();
}

void MainWindow::loadSettings()
{
    // Load first from system .conf file and then overwrite with CLI switches where available
    websettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    websettings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QWebEngineProfile::defaultProfile()->setHttpAcceptLanguage(QLocale::system().name());

    homeAddress = settings.value("Home", "https://start.duckduckgo.com").toString();
    showProgress = settings.value("ShowProgressBar", false).toBool();

    websettings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled,
                              settings.value("SpatialNavigation", false).toBool());
    websettings->setAttribute(QWebEngineSettings::JavascriptEnabled, !settings.value("DisableJava", false).toBool());
    websettings->setAttribute(QWebEngineSettings::AutoLoadImages, settings.value("LoadImages", true).toBool());

    if (args && args->isSet("enable-spatial-navigation")) {
        websettings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, true);
    }
    if (args && args->isSet("disable-js")) {
        websettings->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    }
    if (args && args->isSet("disable-images")) {
        websettings->setAttribute(QWebEngineSettings::AutoLoadImages, false);
    }

    QSize size {defaultWidth, defaultHeight};
    resize(size);
    if (settings.contains("Geometry") && (!args || !args->isSet("full-screen"))) {
        restoreGeometry(settings.value("Geometry").toByteArray());
        if (isMaximized()) { // add option to resize if maximized
            resize(size);
            centerWindow();
        }
    } else {
        centerWindow();
    }
}

void MainWindow::centerWindow()
{
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
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
        settings.setValue("title", menu->actions().at(i)->text());
        settings.setValue("url", menu->actions().at(i)->property("url").toString());

        if (menu->objectName() == "Bookmarks") {
            settings.setValue("icon", menu->actions().at(i)->icon());
        } else {
            QPixmap iconPixmap = menu->actions().at(i)->icon().pixmap(QSize(16, 16));
            QByteArray iconByteArray;
            QBuffer buffer(&iconByteArray);
            buffer.open(QIODevice::WriteOnly);
            iconPixmap.save(&buffer, "PNG");
            settings.setValue("icon", iconByteArray);
        }
    }
    settings.endArray();
}

void MainWindow::setConnections()
{
    connect(currentWebView(), &QWebEngineView::loadStarted, toolBar, &QToolBar::show);
    connect(currentWebView(), &QWebEngineView::urlChanged, this, &MainWindow::updateUrl);
    connect(currentWebView(), &QWebEngineView::iconChanged, this,
            [this] { histIcons.insert(currentWebView()->url(), currentWebView()->icon()); });
    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, downloadWidget,
            &DownloadWidget::downloadRequested, Qt::UniqueConnection);
    if (showProgress) {
        connect(currentWebView(), &QWebEngineView::loadStarted, this, &MainWindow::loading);
    }
    connect(currentWebView(), &QWebEngineView::loadFinished, this, &MainWindow::done);
    connect(currentWebView()->page(), &QWebEnginePage::linkHovered, this, [this](const QString &url) {
        if (url.isEmpty()) {
            statusBar()->hide();
        } else {
            statusBar()->show();
            statusBar()->showMessage(url);
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
    label->setStyleSheet("padding: 15px; background-color:#787878; color:white");
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
    QTimer::singleShot(4s, this, [label, effect, end, start] {
        auto *a = new QPropertyAnimation(effect, "opacity");
        a->setDuration(duration_ms);
        a->setStartValue(end);
        a->setEndValue(start);
        a->setEasingCurve(QEasingCurve::OutBack);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        connect(a, &QPropertyAnimation::finished, label, &QLabel::deleteLater);
    });
}

void MainWindow::tabChanged()
{
    auto *back = pageAction(QWebEnginePage::Back);
    auto *forward = pageAction(QWebEnginePage::Forward);
    auto *reload = pageAction(QWebEnginePage::Reload);
    auto *stop = pageAction(QWebEnginePage::Stop);
    QMap<QString, QAction *> actionMap = {{"Back", back}, {"Forward", forward}, {"Reload", reload}, {"Stop", stop}};
    auto action_list = toolBar->actions();
    for (int i = 0; i < action_list.size() - 1; ++i) {
        auto *currentAction = action_list.at(i);
        auto *nextAction = action_list.at(i + 1);
        auto it = actionMap.find(currentAction->text());
        if (it != actionMap.end()) {
            QAction *replacementAction = it.value();
            toolBar->removeAction(currentAction);
            toolBar->insertAction(nextAction, replacementAction);
        }
    }
    addressBar->setText(currentWebView()->url().toString());
    if (addressBar->text().isEmpty()) {
        addressBar->setFocus();
    }
    setWindowTitle(currentWebView()->title());
}

// Show the address in the toolbar and also connect it to launch it
void MainWindow::connectAddress(const QAction *action, const QMenu *menu)
{
    connect(action, &QAction::hovered, this, [this, action] {
        QString url = action->property("url").toString();
        if (url.isEmpty()) {
            statusBar()->hide();
        } else {
            statusBar()->show();
            statusBar()->showMessage(url);
        }
    });
    connect(action, &QAction::triggered, this, [this, action] {
        QString url = action->property("url").toString();
        displaySite(url);
    });
    connect(menu, &QMenu::aboutToHide, statusBar(), &QStatusBar::hide);
}

void MainWindow::buildMenu()
{
    auto *menu = new QMenu(this);
    history = new QMenu(menu);
    bookmarks = new QMenu(menu);
    bookmarks->setObjectName("Bookmarks");
    history->setStyleSheet("QMenu { menu-scrollable: 1; }");
    history->setObjectName("History");
    bookmarks->setStyleSheet("QMenu { menu-scrollable: 1; }");
    QAction *newTab {nullptr};
    QAction *fullScreen {nullptr};
    QAction *about {nullptr};
    QAction *quit {nullptr};
    QAction *help {nullptr};
    QAction *downloadAction {nullptr};
    QAction *bookmarkAction {nullptr};
    QAction *historyAction {nullptr};
    menuButton->setMenu(menu);

    menu->addAction(newTab = new QAction(QIcon::fromTheme("tab-new"), tr("&New tab")));
    newTab->setShortcut(Qt::CTRL | Qt::Key_T);
    menu->addSeparator();
    menu->addAction(fullScreen = new QAction(QIcon::fromTheme("view-fullscreen"), tr("&Full screen")));
    menu->addSeparator();
    menu->addAction(historyAction = new QAction(QIcon::fromTheme("history"), tr("H&istory")));
    historyAction->setMenu(history);
    menu->addAction(downloadAction = new QAction(QIcon::fromTheme("folder-download"), tr("&Downloads")));
    downloadAction->setShortcut(Qt::CTRL | Qt::Key_J);
    menu->addAction(bookmarkAction = new QAction(QIcon::fromTheme("emblem-favorite"), tr("&Bookmarks")));
    bookmarkAction->setMenu(bookmarks);
    bookmarks->addAction(addBookmark);
    addBookmark->setText(tr("Bookmark current address"));
    addBookmark->setShortcut(Qt::CTRL | Qt::Key_D);
    bookmarks->addSeparator();
    menu->addSeparator();
    menu->addAction(help = new QAction(QIcon::fromTheme("help-contents"), tr("&Help")));
    menu->addAction(about = new QAction(QIcon::fromTheme("help-about"), tr("&About")));
    menu->addSeparator();
    menu->addAction(quit = new QAction(QIcon::fromTheme("window-close"), tr("&Exit")));

    loadBookmarks();
    addBookmarksSubmenu();
    addHistorySubmenu();

    connect(newTab, &QAction::triggered, this, [this] { addNewTab(); });

    connect(addBookmark, &QAction::triggered, this, [this] {
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark = new QAction(currentWebView()->icon(), currentWebView()->title()));
        bookmark->setProperty("url", currentWebView()->url());
        connectAddress(bookmark, bookmarks);
    });

    connect(menuButton, &QAction::triggered, this, [this, menu] {
        QPoint pos = mapToParent(toolBar->widgetForAction(menuButton)->pos());
        pos.setY(pos.y() + toolBar->widgetForAction(menuButton)->size().height());
        menu->popup(pos);
        listHistory();
    });

    connect(fullScreen, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    QApplication::processEvents();

    connect(downloadAction, &QAction::triggered, downloadWidget, &QWidget::show);
    connect(help, &QAction::triggered, this, &MainWindow::openQuickInfo);
    connect(quit, &QAction::triggered, this, &MainWindow::close);
    connect(about, &QAction::triggered, this, [this] {
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
    if (isFullScreen()) {
        showNormal();
        toolBar->show();
    } else {
        showFullScreen();
        toolBar->hide();
        showFullScreenNotification();
    }
}

void MainWindow::updateUrl()
{
    addressBar->show();
    addressBar->setText(currentWebView()->url().toDisplayString());
}

void MainWindow::findBackward()
{
    searchBox->setFocus();
    currentWebView()->findText(searchBox->text(), QWebEnginePage::FindBackward);
}

void MainWindow::findForward()
{
    searchBox->setFocus();
    currentWebView()->findText(searchBox->text());
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::FindNext) || event->matches(QKeySequence::Find) || event->key() == Qt::Key_Slash) {
        findForward();
    }
    if (event->matches(QKeySequence::FindPrevious)) {
        findBackward();
    } else if (event->matches(QKeySequence::Open)) {
        openBrowseDialog();
    } else if (event->matches(QKeySequence::HelpContents) || event->key() == Qt::Key_Question) {
        openQuickInfo();
    } else if (event->matches(QKeySequence::Cancel) && !searchBox->text().isEmpty() && searchBox->hasFocus()) {
        searchBox->clear();
    } else if (event->matches(QKeySequence::Cancel) && searchBox->text().isEmpty()) {
        currentWebView()->setFocus();
    } else if (event->key() == Qt::Key_L && event->modifiers() == Qt::ControlModifier) {
        addressBar->setFocus();
    }
}

// resize event
void MainWindow::resizeEvent(QResizeEvent * /*event*/)
{
    if (showProgress) {
        progressBar->move(geometry().width() / 2 - progressBar->width() / 2, geometry().height() - progBarVerticalAdj);
    }
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    downloadWidget->close();
}

QAction *MainWindow::pageAction(QWebEnginePage::WebAction webAction)
{
    return currentWebView()->pageAction(webAction);
}

WebView *MainWindow::currentWebView()
{
    return tabWidget->currentWebView();
}

// display progressbar while loading page
void MainWindow::loading()
{
    progressBar->setFixedHeight(progBarWidth);
    progressBar->setTextVisible(false);
    progressBar->move(geometry().width() / 2 - progressBar->width() / 2, geometry().height() - progBarVerticalAdj);
    progressBar->setFocus();
    progressBar->show();
    connect(timer, &QTimer::timeout, this, &MainWindow::procTime);
    timer->start(100ms);
}

// done loading
void MainWindow::done()
{
    timer->stop();
    timer->disconnect();
    currentWebView()->stop();
    currentWebView()->setFocus();
    searchBox->clear();
    progressBar->hide();
    tabWidget->setTabText(tabWidget->currentIndex(), currentWebView()->title());
    setWindowTitle(currentWebView()->title());
}

// advance progressbar
void MainWindow::procTime()
{
    const int step = 5;
    progressBar->setValue((progressBar->value() + step) % progressBar->maximum());
}

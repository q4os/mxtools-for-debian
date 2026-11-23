/*****************************************************************************
 * mainwindow.cpp
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
#include "findbar.h"

#include <QContextMenuEvent>
#include <QCompleter>
#include <QDrag>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QWebEngineProfile>
#include <QWebEngineView>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

QPointer<MainWindow> MainWindow::lastActiveWindow;

MainWindow::MainWindow(const QCommandLineParser &argParser, QWidget *parent)
    : QMainWindow(parent),
      downloadWidget {new DownloadWidget(this)},
      findBar {new FindBar(this)},
      progressBar {new QProgressBar(this)},
      toolBar {new QToolBar(this)},
      webProfile {sharedProfile()},
      tabWidget {new TabWidget(webProfile, this)},
      args {&argParser}
{
    commandLineOverrides = {argParser.isSet("enable-spatial-navigation"), argParser.isSet("disable-js"),
                            argParser.isSet("disable-images")};
    init();
    if (argParser.isSet("full-screen")) {
        showFullScreen();
        setToolbarsVisible(false);
    }
    QString url;
    QString title;
    if (args && !args->positionalArguments().isEmpty()) {
        url = args->positionalArguments().at(0);
        title = (args->positionalArguments().size() > 1) ? args->positionalArguments().at(1) : url;
    }
    if (!restoredTabs) {
        displaySite(url, title);
    } else if (!url.isEmpty()) {
        addNewTab(QUrl::fromUserInput(url), true);
    }
}

MainWindow::MainWindow(const QUrl &url, bool privateMode, bool restoreTabs, QWidget *parent)
    : QMainWindow(parent),
      downloadWidget {new DownloadWidget(this)},
      findBar {new FindBar(this)},
      progressBar {new QProgressBar(this)},
      toolBar {new QToolBar(this)},
      // A profile without a storage name is off-the-record: cookies, cache and permissions stay in memory.
      webProfile {privateMode ? new QWebEngineProfile : sharedProfile()},
      tabWidget {new TabWidget(webProfile, this)},
      args {nullptr}
{
    privateWindow = privateMode;
    restoreTabsOnOpen = restoreTabs;
    if (privateWindow) {
        // Parented only now, after the tabs, so the window deletes its pages before their profile.
        webProfile->setParent(this);
        connect(webProfile, &QWebEngineProfile::downloadRequested, this,
                [this](QWebEngineDownloadRequest *download) { routeDownload(webProfile, download); });
    }
    init();
    if (!restoredTabs) {
        displaySite(url.toString(), QString());
    }
}

void MainWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose);
    // Holding Esc leaves F11 full screen. Watched for the whole application, without taking the key:
    // it goes to the page, which keeps short presses for itself.
    escapeHoldTimer = new QTimer(this);
    escapeHoldTimer->setSingleShot(true);
    escapeHoldTimer->setInterval(escapeHoldMs);
    connect(escapeHoldTimer, &QTimer::timeout, this, [this] {
        if (isFullScreen() && !pageFullScreen) {
            toggleFullScreen();
        }
    });
    qApp->installEventFilter(this);
    toolBar->toggleViewAction()->setVisible(false);
    connect(tabWidget, &TabWidget::currentChanged, this, [this] { tabChanged(); });
    connect(tabWidget, &TabWidget::newTabButtonClicked, this, [this] { addNewTab(); });
    connect(tabWidget, &TabWidget::tabClosed, this, [this](const QUrl &url) {
        if (url.isValid()) {
            QIcon icon;
            if (auto *view = currentWebView()) {
                if (view->url() == url) {
                    icon = view->icon();
                }
            }
            if (icon.isNull()) {
                for (int i = 0; i < tabWidget->count(); ++i) {
                    if (auto *view = qobject_cast<WebView *>(tabWidget->widget(i))) {
                        if (view->url() == url) {
                            icon = view->icon();
                            break;
                        }
                    }
                }
            }
            // Each entry keeps an icon; only the most recent ones are offered again.
            constexpr qsizetype maxClosedTabs = 25;
            closedTabs.append({url, icon});
            if (closedTabs.size() > maxClosedTabs) {
                closedTabs.removeFirst();
            }
        }
    });
    // Set up every page as it is added, before it loads anything.
    connect(tabWidget, &TabWidget::viewAdded, this, [this](WebView *view) { applyPageSettings(view->settings()); });
    connect(tabWidget, &TabWidget::settingsSaveRequested, this, [this](const QUrl &url) { handleSettingsRequest(url); });
    loadSettings();
    addToolbar();
    setupBookmarkBar();
    addActions();
    setConnections();

    restoredTabs = !privateWindow && restoreTabsOnOpen && settings.value("SaveTabs", false).toBool() && restoreSavedTabs();
    if (privateWindow) {
        auto *label = new QLabel(tr("Private"), toolBar);
        label->setToolTip(tr("Private window: history, tabs and site data are not saved"));
        label->setContentsMargins(6, 0, 6, 0);
        toolBar->addWidget(label);
        auto markPrivate = [this](const QString &title) {
            const QString suffix = " \u2014 " + tr("Private");
            if (!title.endsWith(suffix)) {
                setWindowTitle(title + suffix);
            }
        };
        connect(this, &QWidget::windowTitleChanged, this, markPrivate);
        markPrivate(windowTitle());
    }

    auto *closeTabAction = new QAction(this);
    closeTabAction->setShortcut(QKeySequence::Close);
    connect(closeTabAction, &QAction::triggered, this, &MainWindow::closeCurrentTab);
    addAction(closeTabAction);
    auto *reopenTabAction = new QAction(this);
    reopenTabAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    connect(reopenTabAction, &QAction::triggered, this, &MainWindow::reopenClosedTab);
    addAction(reopenTabAction);
}

MainWindow::~MainWindow()
{
    // Top-level window without a parent, so it is not deleted with this one.
    delete downloadWidget;
    if (privateWindow) {
        return;
    }
    settings.setValue("Geometry", saveGeometry());
    saveBookmarks();
}

void MainWindow::addActions()
{
    auto *full = new QAction(tr("Full screen"), this);
    full->setShortcut(Qt::Key_F11);
    addAction(full);
    connect(full, &QAction::triggered, this, &MainWindow::toggleFullScreen);

    // Window-wide so they also work while the page has focus.
    auto *addressAction = new QAction(this);
    addressAction->setShortcuts({Qt::CTRL | Qt::Key_L, Qt::Key_F6, Qt::ALT | Qt::Key_D});
    addAction(addressAction);
    connect(addressAction, &QAction::triggered, this, &MainWindow::focusAddressBar);

    auto *nextTabAction = new QAction(this);
    nextTabAction->setShortcut(Qt::CTRL | Qt::Key_PageDown);
    addAction(nextTabAction);
    connect(nextTabAction, &QAction::triggered, this, [this] { cycleTab(1); });
    auto *previousTabAction = new QAction(this);
    previousTabAction->setShortcuts({Qt::CTRL | Qt::Key_PageUp, Qt::CTRL | Qt::SHIFT | Qt::Key_Backtab});
    addAction(previousTabAction);
    connect(previousTabAction, &QAction::triggered, this, [this] { cycleTab(-1); });

    // Also shown in the History menu; one action, so the shortcut is never registered twice.
    clearDataAction = new QAction(QIcon::fromTheme("edit-clear-history"), tr("Clear browsing data..."), this);
    clearDataAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete);
    addAction(clearDataAction);
    connect(clearDataAction, &QAction::triggered, this, &MainWindow::openClearDataDialog);

    // Only while a page is full screen: a shortcut is handled before the page sees the key, so the
    // page cannot keep Esc from getting the user out.
    exitPageFullScreenAction = new QAction(this);
    exitPageFullScreenAction->setShortcut(Qt::Key_Escape);
    exitPageFullScreenAction->setEnabled(false);
    addAction(exitPageFullScreenAction);
    connect(exitPageFullScreenAction, &QAction::triggered, this, &MainWindow::exitPageFullScreen);
}

void MainWindow::cycleTab(int step)
{
    const int count = tabWidget->count();
    if (count > 1) {
        tabWidget->setCurrentIndex((tabWidget->currentIndex() + step + count) % count);
    }
}

void MainWindow::updateSiteIcon()
{
    if (!siteIconAction) {
        return;
    }
    const WebView *view = currentWebView();
    static const QStringList draggableSchemes {"http", "https", "file", "ftp"};
    if (!view || !draggableSchemes.contains(view->url().scheme())) {
        siteIconAction->setVisible(false);
        return;
    }
    const QIcon icon = view->icon();
    siteIconAction->setIcon(icon.isNull() ? QIcon::fromTheme("text-html", style()->standardIcon(QStyle::SP_FileIcon))
                                          : icon);
    siteIconAction->setVisible(true);
}

void MainWindow::startAddressDrag()
{
    const WebView *view = currentWebView();
    if (!view) {
        return;
    }
    const QUrl url = view->url();
    auto *data = new QMimeData;
    data->setUrls({url});
    data->setText(url.toString());
    data->setHtml(QString("<a href=\"%1\">%2</a>").arg(url.toString().toHtmlEscaped(), view->title().toHtmlEscaped()));
    auto *drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(siteIconAction->icon().pixmap(16));
    drag->exec(Qt::CopyAction | Qt::LinkAction, Qt::CopyAction);
}

// Only the first press and the final release count; holding the key repeats both.
void MainWindow::trackEscapeHold(QObject *watched, const QKeyEvent *event)
{
    if (event->key() != Qt::Key_Escape || event->isAutoRepeat()) {
        return;
    }
    const auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget || widget->window() != this) {
        return;
    }
    if (event->type() == QEvent::KeyRelease) {
        escapeHoldTimer->stop();
    } else if (isFullScreen() && !pageFullScreen && !escapeHoldTimer->isActive()) {
        // A press passed on from a child to its parents is seen once per widget; the running timer
        // keeps it from starting over.
        escapeHoldTimer->start();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        trackEscapeHold(watched, static_cast<QKeyEvent *>(event));
    }
    if (watched == siteIconButton && siteIconButton) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                siteIconPressPos = mouseEvent->position().toPoint();
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if ((mouseEvent->buttons() & Qt::LeftButton)
                && (mouseEvent->position().toPoint() - siteIconPressPos).manhattanLength()
                       >= QApplication::startDragDistance()) {
                startAddressDrag();
                return true;
            }
        }
        return QMainWindow::eventFilter(watched, event);
    }
    if (watched != bookmarks) {
        return QMainWindow::eventFilter(watched, event);
    }
    if (event->type() == QEvent::ContextMenu) {
        // Keyboard only (the Menu key); a mouse right click is handled on release below.
        auto *menuEvent = static_cast<QContextMenuEvent *>(event);
        QAction *action = bookmarks->activeAction();
        if (menuEvent->reason() == QContextMenuEvent::Keyboard && action && action->property("url").isValid()) {
            showBookmarkMenu(action, bookmarks->mapToGlobal(bookmarks->actionGeometry(action).center()));
        }
        return true;
    }
    if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease) {
        return false;
    }
    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() != Qt::RightButton && mouseEvent->button() != Qt::MiddleButton) {
        return false;
    }
    QAction *action = bookmarks->actionAt(mouseEvent->position().toPoint());
    if (!action || !action->property("url").isValid()) {
        return false;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        if (mouseEvent->button() == Qt::RightButton) {
            showBookmarkMenu(action, mouseEvent->globalPosition().toPoint());
        } else {
            // Close the whole menu chain, as a normal click would.
            QWidget *top = bookmarks;
            while (qobject_cast<QMenu *>(top->parentWidget())) {
                top = top->parentWidget();
            }
            top->hide();
            openLinkInNewTab(action->property("url").toUrl());
        }
    }
    return true;
}

void MainWindow::addNavigationActions()
{
    backAction = pageAction(QWebEnginePage::Back);
    forwardAction = pageAction(QWebEnginePage::Forward);
    auto *reload = pageAction(QWebEnginePage::Reload);
    stopAction = pageAction(QWebEnginePage::Stop);
    toolBar->addAction(backAction);
    toolBar->addAction(forwardAction);
    reloadAction = new QAction(reload->icon(), reload->text(), this);
    toolBar->addAction(reloadAction);
    // On the window too, so the shortcut works with the toolbar hidden. It must stay window-local:
    // with several windows an application-wide shortcut would be ambiguous and fire nowhere.
    addAction(reloadAction);
    toolBar->addAction(stopAction);
    backAction->setShortcut(QKeySequence::Back);
    forwardAction->setShortcut(QKeySequence::Forward);
    reloadAction->setShortcuts(QKeySequence::Refresh);
    stopAction->setShortcut(QKeySequence::Cancel);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reloadCurrentView);
    connect(stopAction, &QAction::triggered, this, [this] { done(true); });
}

void MainWindow::addHomeAction()
{
    homeAction = new QAction(QIcon::fromTheme("go-home", QIcon(":/icons/go-home.svg")), tr("Home"), this);
    toolBar->addAction(homeAction);
    homeAction->setShortcut(Qt::ALT | Qt::Key_Home);
    connect(homeAction, &QAction::triggered, this, [this] { displaySite(); });
}

void MainWindow::setupAddressBar()
{
    addressBar = new AddressBar(this);
    addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addressBar->setClearButtonEnabled(true);
    historyCompletionModel = new QStringListModel(this);
    historyCompleter = new QCompleter(historyCompletionModel, this);
    historyCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    historyCompleter->setCompletionMode(QCompleter::PopupCompletion);
    historyCompleter->setFilterMode(Qt::MatchContains);
    addressBar->setCompleter(historyCompleter);
    connect(historyCompleter, QOverload<const QString &>::of(&QCompleter::activated), this,
            [this](const QString &text) {
                addressBar->setText(text);
                openFromAddressBarText(text);
            });
    connect(addressBar, &AddressBar::focused, this, [this] {
        lastAddressEditLength = addressBar->text().size();
        refreshHistoryCompleter();
    });
    connect(addressBar, &AddressBar::keyPressed, this, [this](int key) {
        lastAddressEditWasDeletion = (key == Qt::Key_Backspace || key == Qt::Key_Delete);
    });
    connect(addressBar, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (completingHistory) {
            return;
        }
        if (addressBar->cursorPosition() < text.size()) {
            return;
        }
        if (lastAddressEditWasDeletion) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty() || trimmed.contains(' ')) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        if (historyCompletionHosts.isEmpty()) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        QString prefix;
        QString hostInput = trimmed;
        const int schemeIndex = trimmed.indexOf("://");
        if (schemeIndex >= 0) {
            prefix = trimmed.left(schemeIndex + 3);
            hostInput = trimmed.mid(schemeIndex + 3);
        }
        const int pathIndex = hostInput.indexOf('/');
        if (pathIndex >= 0) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        QString match;
        for (const QString &entry : historyCompletionHosts) {
            if (entry.startsWith(hostInput, Qt::CaseInsensitive)) {
                match = entry;
                break;
            }
        }
        if (match.isEmpty() || match.compare(hostInput, Qt::CaseInsensitive) == 0) {
            lastAddressEditLength = text.size();
            lastAddressEditWasDeletion = false;
            return;
        }
        completingHistory = true;
        addressBar->setText(prefix + match);
        addressBar->setSelection(prefix.size() + hostInput.size(), match.size() - hostInput.size());
        lastAddressEditLength = addressBar->text().size();
        completingHistory = false;
        lastAddressEditWasDeletion = false;
    });
    refreshHistoryCompleter();
    addBookmark = addressBar->addAction(QIcon::fromTheme("emblem-favorite", QIcon(":/icons/emblem-favorite.png")),
                                        QLineEdit::TrailingPosition);
    addBookmark->setToolTip(tr("Add bookmark"));
    // The page icon, dragged to the bookmarks bar to bookmark the page.
    siteIconAction = addressBar->addAction(QIcon(), QLineEdit::LeadingPosition);
    siteIconAction->setToolTip(tr("Drag to the bookmarks bar to bookmark this page"));
    siteIconAction->setVisible(false);
    for (QObject *object : siteIconAction->associatedObjects()) {
        auto *widget = qobject_cast<QWidget *>(object);
        if (widget && widget != addressBar) {
            siteIconButton = widget;
            widget->installEventFilter(this);
        }
    }
    addressBar->setDragEnabled(true);
    connect(addressBar, &QLineEdit::returnPressed, this, &MainWindow::openFromAddressBar);
    toolBar->addWidget(addressBar);
}

void MainWindow::setupFindBar()
{
    connect(findBar, &FindBar::findRequested, this, [this](bool backward) {
        findInPage(backward ? QWebEnginePage::FindBackward : QWebEnginePage::FindFlags {});
    });
    connect(findBar, &FindBar::closed, this, &MainWindow::closeFindBar);
}

void MainWindow::addZoomActions()
{
    auto *zoomout {new QAction(QIcon::fromTheme("zoom-out", QIcon(":/icons/zoom-out.svg")), tr("Zoom out"), this)};
    zoomPercentAction = new QAction("100%", this);
    auto *zoomin {new QAction(QIcon::fromTheme("zoom-in", QIcon(":/icons/zoom-in.svg")), tr("Zoom In"), this)};
    toolBar->addAction(zoomout);
    toolBar->addAction(zoomPercentAction);
    toolBar->addAction(zoomin);
    zoomin->setShortcuts({QKeySequence::ZoomIn, Qt::CTRL | Qt::Key_Equal});
    zoomout->setShortcut(QKeySequence::ZoomOut);
    zoomPercentAction->setShortcut(Qt::CTRL | Qt::Key_0);
    zoomPercentAction->setToolTip(tr("Reset zoom for this site"));
    connect(zoomout, &QAction::triggered, this, [this] {
        setSiteZoom(currentZoomPercent() - 10);
    });
    connect(zoomin, &QAction::triggered, this, [this] {
        setSiteZoom(currentZoomPercent() + 10);
    });
    connect(zoomPercentAction, &QAction::triggered, this, [this] {
        setSiteZoom(zoomPercent);
    });
    applyZoom();
}

void MainWindow::setupMenuButton()
{
    menuButton = new QAction(QIcon::fromTheme("open-menu", QIcon(":/icons/open-menu.png")), tr("Settings"), this);
    toolBar->addAction(menuButton);
    menuButton->setShortcut(Qt::Key_F10);
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
    menuButton->setMenu(menu);

    addFileMenuActions(menu);
    addViewMenuActions(menu);
    addHelpMenuActions(menu);

    loadBookmarks();
    addBookmarksSubmenu();

    setupMenuConnections(menu);
}

void MainWindow::addFileMenuActions(QMenu *menu)
{
    QAction *newTab {nullptr};
    menu->addAction(newTab = new QAction(QIcon::fromTheme("tab-new"), tr("&New tab"), menu));
    newTab->setShortcut(Qt::CTRL | Qt::Key_T);
    connect(newTab, &QAction::triggered, this, [this] { addNewTab(); });

    auto *savePage = new QAction(QIcon::fromTheme("document-save-as"), tr("&Save page..."), this);
    savePage->setShortcut(QKeySequence::Save);
    menu->addAction(savePage);
    addAction(savePage);
    // Goes through downloadRequested, where the download widget asks for the file name.
    connect(savePage, &QAction::triggered, this, [this] {
        if (auto *view = currentWebView()) {
            view->triggerPageAction(QWebEnginePage::SavePage);
        }
    });

    auto *print = new QAction(QIcon::fromTheme("document-print"), tr("&Print..."), this);
    print->setShortcut(QKeySequence::Print);
    menu->addAction(print);
    addAction(print);
    connect(print, &QAction::triggered, this, [this] { printPage(currentWebView()); });

    auto *privateAction = new QAction(QIcon::fromTheme("view-private"), tr("New p&rivate window"), this);
    privateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    menu->addAction(privateAction);
    addAction(privateAction);
    connect(privateAction, &QAction::triggered, this, &MainWindow::openPrivateWindow);
}

void MainWindow::changeEvent(QEvent *event)
{
    // The release of a held Esc goes to whichever window is active by then.
    if (event->type() == QEvent::ActivationChange && !isActiveWindow() && escapeHoldTimer) {
        escapeHoldTimer->stop();
    }
    if (event->type() == QEvent::ActivationChange && isActiveWindow() && !privateWindow) {
        lastActiveWindow = this;
    }
    if (event->type() == QEvent::PaletteChange && menuButton) {
        adaptIcons();
    }
    if (event->type() == QEvent::WindowStateChange && titleBar) {
        updateTitleBar();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::addViewMenuActions(QMenu *menu)
{
    QAction *fullScreen {nullptr};
    QAction *devTools {nullptr};
    QAction *historyAction {nullptr};
    QAction *downloadAction {nullptr};
    QAction *bookmarkAction {nullptr};
    QAction *manageBookmarks {nullptr};
    menu->addAction(fullScreen = new QAction(QIcon::fromTheme("view-fullscreen"), tr("&Full screen"), menu));
    auto *readerAction = new QAction(QIcon::fromTheme("view-readermode"), tr("&Reader view"), this);
    readerAction->setShortcuts({Qt::Key_F9, Qt::CTRL | Qt::ALT | Qt::Key_R});
    menu->addAction(readerAction);
    addAction(readerAction);
    connect(readerAction, &QAction::triggered, this, &MainWindow::toggleReaderMode);
    bookmarkBarAction = new QAction(QIcon::fromTheme("bookmarks"), tr("Show bookmarks &bar"), this);
    bookmarkBarAction->setCheckable(true);
    bookmarkBarAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B));
    menu->addAction(bookmarkBarAction);
    addAction(bookmarkBarAction);
    // The same key leaves reader view, so the item names what it will do for this tab.
    connect(menu, &QMenu::aboutToShow, readerAction, [this, readerAction] {
        const WebView *view = currentWebView();
        readerAction->setText(view && view->isReaderMode() ? tr("Exit &reader view") : tr("&Reader view"));
    });
    menu->addSeparator();
    menu->addAction(devTools = new QAction(QIcon::fromTheme("applications-development"), tr("&Developer Tools"), menu));
    devTools->setShortcut(Qt::Key_F12);
    menu->addAction(historyAction = new QAction(QIcon::fromTheme("history"), tr("H&istory"), menu));
    historyAction->setMenu(history);
    menu->addAction(downloadAction = new QAction(QIcon::fromTheme("folder-download"), tr("&Downloads"), menu));
    downloadAction->setShortcut(Qt::CTRL | Qt::Key_J);
    menu->addAction(bookmarkAction = new QAction(QIcon::fromTheme("emblem-favorite"), tr("&Bookmarks"), menu));
    bookmarkAction->setMenu(bookmarks);
    // Its own action rather than the address bar's, whose icon is adapted to the address bar.
    QAction *bookmarkPage {nullptr};
    bookmarks->addAction(bookmarkPage = new QAction(addBookmark->icon(), tr("Bookmark current address"), this));
    bookmarkPage->setShortcut(Qt::CTRL | Qt::Key_D);
    connect(bookmarkPage, &QAction::triggered, addBookmark, &QAction::trigger);
    connect(addBookmark, &QAction::enabledChanged, bookmarkPage, &QAction::setEnabled);
    bookmarks->addAction(manageBookmarks = new QAction(QIcon::fromTheme("document-edit"), tr("Manage &bookmarks"), bookmarks));
    manageBookmarks->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    // Says what it does rather than a check mark; the shortcut is only shown, the window's action handles it.
    QAction *toggleBookmarkBar {nullptr};
    bookmarks->addAction(toggleBookmarkBar = new QAction(bookmarkBarAction->icon(), QString(), this));
    toggleBookmarkBar->setShortcut(bookmarkBarAction->shortcut());
    toggleBookmarkBar->setShortcutContext(Qt::WidgetShortcut);
    auto updateToggleText = [this, toggleBookmarkBar] {
        toggleBookmarkBar->setText(bookmarkBarAction->isChecked() ? tr("Hide bookmarks bar") : tr("Show bookmarks bar"));
    };
    updateToggleText();
    connect(bookmarkBarAction, &QAction::toggled, toggleBookmarkBar, updateToggleText);
    connect(toggleBookmarkBar, &QAction::triggered, bookmarkBarAction, &QAction::toggle);
    bookmarks->addSeparator();
    // Hovering a bookmark shows its address in the status bar until the menu closes.
    connect(bookmarks, &QMenu::aboutToHide, statusBar(), &QStatusBar::hide);
    connect(fullScreen, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    connect(devTools, &QAction::triggered, this, &MainWindow::openDevTools);
    connect(downloadAction, &QAction::triggered, downloadWidget, &QWidget::show);
    connect(manageBookmarks, &QAction::triggered, this, &MainWindow::openBookmarksEditor);
    // Bookmarks are saved when a window closes; a private window must not overwrite the regular list.
    if (privateWindow) {
        addBookmark->setEnabled(false);
        manageBookmarks->setEnabled(false);
    }
    connect(addBookmark, &QAction::triggered, this, [this] {
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark = new QAction(currentWebView()->icon(), QString(), bookmarks));
        setBookmarkTitle(bookmark, currentWebView()->title());
        bookmark->setProperty("url", currentWebView()->url());
        connectAddress(bookmark);
        bookmarksChanged();
    });
}

void MainWindow::addHelpMenuActions(QMenu *menu)
{
    QAction *settingsAction {nullptr};
    QAction *help {nullptr};
    QAction *about {nullptr};
    QAction *quit {nullptr};
    menu->addSeparator();
    menu->addAction(settingsAction = new QAction(QIcon::fromTheme("preferences-system"), tr("&Settings"), menu));
    settingsAction->setShortcuts({Qt::CTRL | Qt::Key_Comma, QKeySequence::Preferences});
    settingsAction->setEnabled(!privateWindow);
    if (privateWindow) {
        settingsAction->setToolTip(tr("Open a regular window to change settings"));
    }
    menu->addSeparator();
    menu->addAction(help = new QAction(QIcon::fromTheme("help-contents"), tr("&Keyboard shortcuts"), menu));
    help->setShortcut(QKeySequence::HelpContents);
    menu->addAction(about = new QAction(QIcon::fromTheme("help-about"), tr("&About"), menu));
    menu->addSeparator();
    menu->addAction(quit = new QAction(QIcon::fromTheme("window-close"), tr("&Exit"), menu));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    connect(help, &QAction::triggered, this, &MainWindow::openQuickInfo);
    connect(quit, &QAction::triggered, this, &MainWindow::close);
    connect(about, &QAction::triggered, this, &MainWindow::openAbout);
}

void MainWindow::setupMenuConnections(QMenu *menu)
{
    connect(menuButton, &QAction::triggered, this, [this, menu] {
        // Below the button, wherever the toolbar sits in the window (under the tabs in title bar mode).
        const QWidget *button = toolBar->widgetForAction(menuButton);
        menu->popup(button->mapToGlobal(QPoint(0, button->height())));
        listHistory();
    });
}

void MainWindow::openDevTools()
{
    if (!currentWebView()) {
        return;
    }
    if (!devToolsWindow) {
        devToolsWindow = new QMainWindow(this);
        devToolsWindow->setAttribute(Qt::WA_DeleteOnClose);
        devToolsWindow->setWindowTitle(tr("Developer Tools"));
        devToolsView = new QWebEngineView(devToolsWindow);
        devToolsWindow->setCentralWidget(devToolsView);
        devToolsWindow->resize(900, 700);
        connect(devToolsWindow.data(), &QObject::destroyed, this, [this] {
            devToolsWindow = nullptr;
            devToolsView = nullptr;
        });
    }
    currentWebView()->page()->setDevToolsPage(devToolsView->page());
    devToolsWindow->show();
    devToolsWindow->raise();
    devToolsWindow->activateWindow();
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_H && event->modifiers() == Qt::ControlModifier) {
        openHistoryPage();
        return;
    }
    if (event->matches(QKeySequence::Find) || event->key() == Qt::Key_Slash) {
        openFindBar();
        return;
    }
    if (event->matches(QKeySequence::FindNext)) {
        findForward();
        return;
    }
    if (event->matches(QKeySequence::FindPrevious)) {
        findBackward();
        return;
    }
    if (event->matches(QKeySequence::Open)) {
        openBrowseDialog();
        return;
    }
    if (event->matches(QKeySequence::HelpContents) || event->key() == Qt::Key_Question) {
        openQuickInfo();
        return;
    }
    if (event->matches(QKeySequence::Cancel) && findBar->isVisible()) {
        closeFindBar();
        return;
    }
    if (event->matches(QKeySequence::Cancel)) {
        if (auto *view = currentWebView()) {
            view->setFocus();
        }
        return;
    }
    if (event->key() == Qt::Key_R && event->modifiers() == Qt::ControlModifier) {
        reloadCurrentView();
        return;
    }
    if (event->key() == Qt::Key_Left && event->modifiers() == Qt::AltModifier) {
        if (auto *view = currentWebView()) {
            view->back();
        }
        return;
    }
    if (event->key() == Qt::Key_Right && event->modifiers() == Qt::AltModifier) {
        if (auto *view = currentWebView()) {
            view->forward();
        }
        return;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // The window's download list cancels the downloads it shows when it is deleted with the window.
    if (const int active = downloadWidget->activeDownloadCount(); active > 0 && !quitting) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle(tr("Downloads in progress"));
        box.setText(tr("%n download(s) still in progress. Closing this window will cancel them.", nullptr, active));
        QPushButton *closeButton = box.addButton(tr("Cancel downloads and close"), QMessageBox::DestructiveRole);
        QPushButton *keepButton = box.addButton(tr("Keep window open"), QMessageBox::RejectRole);
        box.setDefaultButton(keepButton);
        box.setEscapeButton(keepButton);
        box.exec();
        if (box.clickedButton() != closeButton) {
            event->ignore();
            downloadWidget->show();
            downloadWidget->raise();
            downloadWidget->activateWindow();
            return;
        }
    }
    downloadWidget->close();
    if (privateWindow) {
        // Its off-the-record profile goes with the window, cookies included.
        return;
    }
    // Regular windows share their cookies, so they are cleared only when the last one closes.
    if (clearCookiesAtExit && !otherRegularWindowOpen()) {
        webProfile->cookieStore()->deleteAllCookies();
    }
    settings.setValue("Geometry", saveGeometry());

    // Saved when the last regular window closes, or from every window when the program quits; a window
    // closed while others stay open takes its tabs with it.
    if (settings.value("SaveTabs", false).toBool() && (quitting || !otherRegularWindowOpen())) {
        if (!quitting) {
            sessionTabs.clear();
        }
        for (int i = 0; i < tabWidget->count(); ++i) {
            if (const auto *webView = qobject_cast<WebView *>(tabWidget->widget(i))) {
                sessionTabs.append({webView->url().toString(), tabWidget->isPinned(i)});
            }
        }
        // Removed first, so entries past the end of a shorter list are not left behind.
        settings.remove("SavedTabs");
        settings.beginWriteArray("SavedTabs");
        for (qsizetype i = 0; i < sessionTabs.size(); ++i) {
            settings.setArrayIndex(static_cast<int>(i));
            settings.setValue("url", sessionTabs.at(i).first);
            settings.setValue("pinned", sessionTabs.at(i).second);
        }
        settings.endArray();
    }
}

/*****************************************************************************
 * windowchrome.cpp
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

#include <QLabel>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

#include "resizegrip.h"
#include "tabmenubar.h"

void MainWindow::addToolbar()
{
    addToolBar(toolBar);
    setCentralWidget(tabWidget);
    // The menu bar is the only place above the toolbars, so the tabs go there in title bar mode.
    titleBar = new TabMenuBar(this);
    auto *titleLayout = new QVBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    setMenuBar(titleBar);
    for (const Qt::Edges edges : {Qt::Edges(Qt::TopEdge), Qt::Edges(Qt::BottomEdge), Qt::Edges(Qt::LeftEdge),
                                  Qt::Edges(Qt::RightEdge), Qt::TopEdge | Qt::LeftEdge, Qt::TopEdge | Qt::RightEdge,
                                  Qt::BottomEdge | Qt::LeftEdge, Qt::BottomEdge | Qt::RightEdge}) {
        resizeGrips.append(new ResizeGrip(edges, this));
    }
    updateTitleBar();
    addNavigationActions();
    addHomeAction();
    setupAddressBar();
    setupFindBar();
    addZoomActions();
    setupMenuButton();
    buildMenu();
    adaptIcons();
    toolBar->show();
}

// Run again whenever the palette or the tab's page actions change, since icons follow the panel.
void MainWindow::adaptIcons()
{
    const auto adapt = [](QWidget *widget, QPalette::ColorRole background, QPalette::ColorRole foreground) {
        const QPalette palette = widget->palette();
        for (QAction *action : widget->actions()) {
            action->setIcon(iconForBackground(action->icon(), palette.color(background), palette.color(foreground)));
        }
    };
    adapt(toolBar, QPalette::Window, QPalette::WindowText);
    adapt(addressBar, QPalette::Base, QPalette::Text);
    if (QMenu *menu = menuButton->menu()) {
        adaptMenuIcons(menu);
    }
    // The History menu is adapted each time it is rebuilt.
    adaptMenuIcons(bookmarks);
}

void MainWindow::centerWindow()
{
    const QScreen *target = screen() ? screen() : QGuiApplication::primaryScreen();
    QRect frame({}, size());
    frame.moveCenter(target->availableGeometry().center());
    move(frame.topLeft());
}

void MainWindow::showFullScreenNotification(const QString &text)
{
    constexpr int distanceTop = 100;
    constexpr int durationMs = 800;
    constexpr double start = 0;
    constexpr double end = 0.85;
    auto *label = new QLabel(this);
    auto *effect = new QGraphicsOpacityEffect;
    label->setGraphicsEffect(effect);
    label->setStyleSheet("padding: 15px; background-color:#787878; color:white");
    label->setText(text);
    label->adjustSize();
    // The full-screen resize may not have arrived yet; the window will cover its own screen.
    label->move((screen()->geometry().width() - label->width()) / 2, distanceTop);
    auto *a = new QPropertyAnimation(effect, "opacity");
    a->setDuration(durationMs);
    a->setStartValue(start);
    a->setEndValue(end);
    a->setEasingCurve(QEasingCurve::InBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    label->show();
    QTimer::singleShot(4000, this, [label, effect] {
        auto *a = new QPropertyAnimation(effect, "opacity");
        a->setDuration(durationMs);
        a->setStartValue(end);
        a->setEndValue(start);
        a->setEasingCurve(QEasingCurve::OutBack);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        connect(a, &QPropertyAnimation::finished, label, &QLabel::deleteLater);
    });
}

void MainWindow::handleFullScreenRequest(QWebEngineFullScreenRequest request, WebView *view)
{
    if (request.toggleOn()) {
        if (pageFullScreen || view != currentWebView()) {
            request.reject();
            return;
        }
        request.accept();
        pageFullScreen = true;
        pageFullScreenView = view;
        fullScreenBeforePage = isFullScreen();
        if (!fullScreenBeforePage) {
            normalGeometry = saveGeometry();
            showFullScreen();
        }
        setToolbarsVisible(false);
        // Auto-hide would show the tab bar again as soon as a tab is added in the background.
        // Turning it off shows the bar, so hide it afterwards.
        tabWidget->setTabBarAutoHide(false);
        tabWidget->tabBar()->hide();
        statusBar()->hide();
        // The page now covers the whole screen and could draw a fake address bar or dialog, so say how
        // to leave; Esc works as a window shortcut, ahead of the page's own key handling.
        exitPageFullScreenAction->setEnabled(true);
        showFullScreenNotification(tr("Press [Esc] to exit full screen"));
    } else {
        request.accept();
        restoreFromPageFullScreen();
    }
}

// Leave HTML5 fullscreen. The window is restored right away rather than waiting for the page's
// toggle-off request, which never arrives if the tab is being closed.
void MainWindow::exitPageFullScreen()
{
    if (!pageFullScreen) {
        return;
    }
    QPointer<WebView> view = pageFullScreenView;
    restoreFromPageFullScreen();
    if (view) {
        view->triggerPageAction(QWebEnginePage::ExitFullScreen);
    }
}

void MainWindow::restoreFromPageFullScreen()
{
    if (!pageFullScreen) {
        return;
    }
    pageFullScreen = false;
    pageFullScreenView = nullptr;
    exitPageFullScreenAction->setEnabled(false);
    // Turning auto-hide back on also shows the tab bar again when there is more than one tab.
    tabWidget->setTabBarAutoHide(true);
    if (!fullScreenBeforePage) {
        showNormal();
        if (!normalGeometry.isEmpty()) {
            restoreGeometry(normalGeometry);
        }
        setToolbarsVisible(true);
    }
}

void MainWindow::toggleFullScreen()
{
    if (pageFullScreen) {
        exitPageFullScreen();
        return;
    }
    if (isFullScreen()) {
        showNormal();
        if (!normalGeometry.isEmpty()) {
            restoreGeometry(normalGeometry);
        }
        setToolbarsVisible(true);
    } else {
        normalGeometry = saveGeometry();
        showFullScreen();
        setToolbarsVisible(false);
        showFullScreenNotification(tr("Press [F11] or hold [Esc] to exit full screen"));
    }
}

void MainWindow::updateTitleBar()
{
    if (windowFlags().testFlag(Qt::FramelessWindowHint) != tabsInTitleBar) {
        // Changing the flags hides a shown window, so show it again in the same state.
        const bool wasVisible = isVisible();
        const Qt::WindowStates state = windowState();
        setWindowFlag(Qt::FramelessWindowHint, tabsInTitleBar);
        if (wasVisible) {
            setWindowState(state);
            show();
        }
    }
    // Full screen has no title bar, so the tabs go back above the page as usual.
    const bool inTitleBar = tabsInTitleBar && !isFullScreen() && !pageFullScreen;
    if (inTitleBar) {
        titleBar->layout()->addWidget(tabWidget->tabStrip());
        tabWidget->tabStrip()->show();
        titleBar->show();
    } else {
        tabWidget->restoreTabStrip();
        titleBar->hide();
    }
    tabWidget->setTitleBarMode(inTitleBar);
    if (pageFullScreen) {
        tabWidget->setTabBarAutoHide(false);
        tabWidget->tabBar()->hide();
    }
    const bool grips = tabsInTitleBar && !isMaximized() && !isFullScreen();
    const int margin = grips ? gripSize : 0;
    setContentsMargins(margin, margin, margin, margin);
    for (auto *grip : std::as_const(resizeGrips)) {
        grip->setVisible(grips);
    }
    placeResizeGrips();
}

void MainWindow::placeResizeGrips()
{
    const QRect r = rect();
    constexpr int corner = gripSize * 3;
    for (auto *widget : std::as_const(resizeGrips)) {
        auto *grip = static_cast<ResizeGrip *>(widget);
        const Qt::Edges edges = grip->gripEdges();
        const int x = edges & Qt::LeftEdge ? 0 : (edges & Qt::RightEdge ? r.width() - gripSize : 0);
        const int y = edges & Qt::TopEdge ? 0 : (edges & Qt::BottomEdge ? r.height() - gripSize : 0);
        if (edges == Qt::TopEdge || edges == Qt::BottomEdge) {
            grip->setGeometry(0, y, r.width(), gripSize);
        } else if (edges == Qt::LeftEdge || edges == Qt::RightEdge) {
            grip->setGeometry(x, 0, gripSize, r.height());
        } else {
            // Corners reach a little into the window so they are easier to hit.
            grip->setGeometry(edges & Qt::LeftEdge ? 0 : r.width() - corner, edges & Qt::TopEdge ? 0 : r.height() - corner,
                              corner, corner);
        }
        grip->raise();
    }
}

void MainWindow::resizeEvent(QResizeEvent * /*event*/)
{
    placeResizeGrips();
    if (showProgress) {
        progressBar->move(geometry().width() / 2 - progressBar->width() / 2, geometry().height() - progBarVerticalAdj);
    }
}

void MainWindow::setToolbarsVisible(bool visible)
{
    toolbarsVisible = visible;
    toolBar->setVisible(visible);
    if (bookmarkBar) {
        bookmarkBar->setVisible(visible && bookmarkBarAction->isChecked());
    }
}

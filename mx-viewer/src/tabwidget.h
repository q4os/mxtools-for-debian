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

#include <QPointer>
#include <QTabBar>
#include <QWebEngineProfile>
#include <optional>
#include "webview.h"

class QPushButton;
class QStackedWidget;
class QToolButton;

// Tabs over a stack of pages, like QTabWidget, but with the tab bar in a separate strip that the
// window can move into its title bar. The strip then also holds the minimize, maximize and close buttons.
class TabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QWebEngineProfile *profile, QWidget *parent = nullptr);
    WebView *currentWebView();

    [[nodiscard]] int count() const;
    [[nodiscard]] int currentIndex() const;
    void setCurrentIndex(int index);
    [[nodiscard]] QWidget *currentWidget() const;
    [[nodiscard]] QWidget *widget(int index) const;
    [[nodiscard]] int indexOf(const QWidget *widget) const;
    [[nodiscard]] QTabBar *tabBar() const;
    void setTabBarAutoHide(bool enabled);
    // The row with the tab bar; it lives above the pages unless the window takes it for its title bar.
    [[nodiscard]] QWidget *tabStrip() const;
    // Puts the strip back above the pages.
    void restoreTabStrip();
    // In the title bar the strip shows the window buttons, the tabs even when there is only one, and
    // moves the window when dragged on an empty spot.
    void setTitleBarMode(bool enabled);
    [[nodiscard]] bool isTitleBarMode() const;
    // Room for the tabs in the strip, besides the "+" button and the window buttons.
    [[nodiscard]] int tabAreaWidth() const;

    WebView *createTab(bool makeCurrent = true);
    void addNewTab(WebView *webView, bool makeCurrent = true);
    // Moves a tab next to the tab whose page opened it, after the others that page opened.
    void placeAfterOpener(WebView *view, WebView *opener);
    void removeTab(int index);
    void setTabIcon(int index, const QIcon &icon);
    // Sets the tab tooltip, and the text unless the tab is pinned (pinned tabs show only the icon).
    void setTabTitle(int index, const QString &title);
    [[nodiscard]] bool isPinned(int index) const;
    void setPinned(int index, bool pinned);
    // The pinned tabs' indices in tab order, kept until tabs are added, removed, moved or (un)pinned.
    [[nodiscard]] const QList<int> &pinnedIndices() const;
    // Ctrl+W: closes the current tab, or selects an unpinned one if it is pinned.
    // Returns false when this is the last tab, so the caller can close the window.
    bool closeCurrentTabByShortcut();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void currentChanged(int index);
    void newTabButtonClicked();
    void tabClosed(const QUrl &url);
    // Every page added as a tab, before it starts loading.
    void viewAdded(WebView *view);
    // A settings page closed with "Save": the save request its form produced.
    void settingsSaveRequested(const QUrl &url);

private:
    QTabBar *bar {};
    QStackedWidget *stack {};
    QWidget *strip {};
    QWidget *windowButtons {};
    QToolButton *maximizeButton {};
    QPushButton *newTabButton {};
    QWebEngineProfile *profile {};
    QPoint dragStartPos;
    bool dragPending {};
    bool titleBarMode {};
    static constexpr int newTabButtonSpacing {2};
    mutable std::optional<QList<int>> pinnedTabs;
    int addTab(QWidget *widget, const QString &label);
    void moveStackWidget(int from, int to);
    void updateMaximizeButton();
    // Handles presses, drags and double-clicks on an empty part of the strip in title bar mode.
    bool handleTitleBarMouse(QWidget *source, QEvent *event);
    void handleCurrentChanged(int index);
    void finalizeRemoveTab(int index);
    void updateNewTabButton();
    void showTabMenu(const QPoint &pos);
    void duplicateTab(int index);
    void closeTabs(const QList<QPointer<QWidget>> &tabs);
    void updateAudioButton(WebView *webView);
    WebView *webViewAt(int index) const;
    [[nodiscard]] int pinnedCount() const;
    [[nodiscard]] QTabBar::ButtonPosition closeButtonSide() const;
    void normalizePinnedOrder();
    void updateTabIcon(int index);
};

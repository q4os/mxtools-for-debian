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
#include "tabwidget.h"

#include <algorithm>

#include <QApplication>
#include <QDataStream>
#include <QEvent>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSize>
#include <QStyle>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QWebEngineHistory>
#include <QAbstractButton>
#include <QPainter>
#include <QStyleOption>

namespace
{
// Same look as QTabBar's own close button, which can't be recreated once removed (used when unpinning).
class TabCloseButton : public QAbstractButton
{
public:
    explicit TabCloseButton(QWidget *parent)
        : QAbstractButton(parent)
    {
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::ArrowCursor);
        setToolTip(TabWidget::tr("Close Tab"));
        resize(sizeHint());
    }
    [[nodiscard]] QSize sizeHint() const override
    {
        ensurePolished();
        return {style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, this),
                style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, this)};
    }

protected:
    void enterEvent(QEnterEvent *event) override
    {
        update();
        QAbstractButton::enterEvent(event);
    }
    void leaveEvent(QEvent *event) override
    {
        update();
        QAbstractButton::leaveEvent(event);
    }
    void paintEvent(QPaintEvent * /*event*/) override
    {
        QPainter painter(this);
        QStyleOption opt;
        opt.initFrom(this);
        opt.state |= QStyle::State_AutoRaise;
        if (isEnabled() && underMouse() && !isDown()) {
            opt.state |= QStyle::State_Raised;
        }
        if (isDown()) {
            opt.state |= QStyle::State_Sunken;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &painter, this);
    }
};

// Tabs share one width whatever their titles, so a page changing its title does not resize its tab.
// They stay at the preferred width until the row is full, then shrink together down to the minimum,
// after which the bar scrolls. Pinned tabs keep their icon-only size.
class TabBar : public QTabBar
{
public:
    explicit TabBar(TabWidget *parent)
        : QTabBar(parent),
          tabWidget(parent)
    {
        setElideMode(Qt::ElideRight);
        useOwnPalette();
    }
    // Recomputes the tab sizes after the room for them changed.
    void relayout()
    {
        // Setting the elide mode is the only public way to make QTabBar lay out its tabs again.
        setElideMode(Qt::ElideRight);
        updateGeometry();
    }

protected:
    [[nodiscard]] QSize tabSizeHint(int index) const override
    {
        QSize size = QTabBar::tabSizeHint(index);
        if (tabWidget->isPinned(index)) {
            return size;
        }
        // Every layout asks for each tab, so the pinned tabs come from a list kept between changes
        // instead of a pass over all the tabs; their widths are asked fresh, as icons change them.
        const QList<int> &pinned = tabWidget->pinnedIndices();
        int pinnedWidth = 0;
        for (const int i : pinned) {
            if (i < count()) {
                pinnedWidth += QTabBar::tabSizeHint(i).width();
            }
        }
        const int unpinned = count() - static_cast<int>(pinned.size());
        const int available = tabWidget->tabAreaWidth() - pinnedWidth;
        const int width = unpinned > 0 ? available / unpinned : preferredWidth;
        size.setWidth(std::clamp(width, minimumWidth, preferredWidth));
        return size;
    }
    [[nodiscard]] QSize minimumTabSizeHint(int index) const override
    {
        return tabSizeHint(index);
    }
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::ApplicationPaletteChange) {
            useOwnPalette();
        }
        QTabBar::changeEvent(event);
    }

private:
    // In title bar mode the tabs sit in a menu bar, whose palette (light text on a dark bar with GTK themes)
    // would otherwise pass to the tabs, while the style paints the tabs themselves in the theme's tab colors.
    void useOwnPalette()
    {
        setPalette(QApplication::palette(this));
    }

    TabWidget *tabWidget;
    static constexpr int preferredWidth {220};
    static constexpr int minimumWidth {100};
};
} // namespace

TabWidget::TabWidget(QWebEngineProfile *profile, QWidget *parent)
    : QWidget(parent),
      bar(new TabBar(this)),
      stack(new QStackedWidget(this)),
      strip(new QWidget(this)),
      windowButtons(new QWidget(strip)),
      newTabButton(new QPushButton("+", strip)),
      profile(profile)
{
    bar->setAutoHide(true);
    bar->setExpanding(false);
    bar->setTabsClosable(true);
    bar->setMovable(true);
    bar->setUsesScrollButtons(true);
    // The bar is as wide as its tabs, so the "+" button follows the last one, and it shrinks and
    // scrolls when they do not fit.
    bar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Window buttons at the end of the strip, shown only in title bar mode.
    auto *buttonsLayout = new QHBoxLayout(windowButtons);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(0);
    auto addWindowButton = [this, buttonsLayout](const QString &name, const QString &themeIcon,
                                                 QStyle::StandardPixmap fallback, const QString &toolTip) {
        auto *button = new QToolButton(windowButtons);
        button->setObjectName(name);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIconSize(QSize(16, 16));
        button->setFixedSize(40, 30);
        button->setIcon(QIcon::fromTheme(themeIcon, style()->standardIcon(fallback)));
        button->setToolTip(toolTip);
        buttonsLayout->addWidget(button);
        return button;
    };
    auto *minimizeButton = addWindowButton("minimizeWindow", "window-minimize", QStyle::SP_TitleBarMinButton, tr("Minimize"));
    maximizeButton = addWindowButton("maximizeWindow", "window-maximize", QStyle::SP_TitleBarMaxButton, tr("Maximize"));
    auto *closeButton = addWindowButton("closeWindow", "window-close", QStyle::SP_TitleBarCloseButton, tr("Close"));
    closeButton->setStyleSheet("QToolButton#closeWindow:hover { background: #e81123; }");
    connect(minimizeButton, &QToolButton::clicked, this, [this] { window()->showMinimized(); });
    connect(maximizeButton, &QToolButton::clicked, this, [this] {
        window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
    });
    connect(closeButton, &QToolButton::clicked, this, [this] { window()->close(); });
    windowButtons->hide();

    auto *stripLayout = new QHBoxLayout(strip);
    stripLayout->setContentsMargins(0, 0, 0, 0);
    stripLayout->setSpacing(0);
    stripLayout->addWidget(bar);
    stripLayout->addSpacing(newTabButtonSpacing);
    stripLayout->addWidget(newTabButton);
    stripLayout->addStretch(1);
    stripLayout->addWidget(windowButtons, 0, Qt::AlignTop);
    strip->installEventFilter(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(strip);
    layout->addWidget(stack, 1);

    createTab();
    connect(bar, &QTabBar::tabCloseRequested, this, &TabWidget::removeTab);
    connect(bar, &QTabBar::currentChanged, this, [this](int index) {
        if (index >= 0 && index < stack->count()) {
            stack->setCurrentIndex(index);
        }
        handleCurrentChanged(index);
        emit currentChanged(index);
    });
    connect(bar, &QTabBar::tabMoved, this, &TabWidget::moveStackWidget);

    newTabButton->setFixedSize(30, 30);
    newTabButton->setToolTip(tr("New tab"));
    newTabButton->hide();
    connect(newTabButton, &QPushButton::clicked, this, &TabWidget::newTabButtonClicked);
    bar->installEventFilter(this);
    bar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bar, &QTabBar::customContextMenuRequested, this, &TabWidget::showTabMenu);
    updateNewTabButton();
}

int TabWidget::count() const
{
    return stack->count();
}

int TabWidget::currentIndex() const
{
    return bar->currentIndex();
}

void TabWidget::setCurrentIndex(int index)
{
    bar->setCurrentIndex(index);
}

QWidget *TabWidget::currentWidget() const
{
    return stack->currentWidget();
}

QWidget *TabWidget::widget(int index) const
{
    return stack->widget(index);
}

int TabWidget::indexOf(const QWidget *widget) const
{
    return stack->indexOf(widget);
}

QTabBar *TabWidget::tabBar() const
{
    return bar;
}

void TabWidget::setTabBarAutoHide(bool enabled)
{
    // In the title bar the tabs stay visible, as they are what the window is dragged by.
    bar->setAutoHide(enabled && !titleBarMode);
}

QWidget *TabWidget::tabStrip() const
{
    return strip;
}

void TabWidget::restoreTabStrip()
{
    if (strip->parentWidget() != this) {
        static_cast<QVBoxLayout *>(layout())->insertWidget(0, strip);
        strip->show();
    }
}

bool TabWidget::isTitleBarMode() const
{
    return titleBarMode;
}

void TabWidget::setTitleBarMode(bool enabled)
{
    titleBarMode = enabled;
    windowButtons->setVisible(enabled);
    bar->setAutoHide(!enabled);
    updateNewTabButton();
    if (enabled) {
        // Tabs sit flush with the top of the window, so leave a little room to grab the window above them.
        strip->setContentsMargins(0, 4, 0, 0);
        window()->installEventFilter(this);
        updateMaximizeButton();
    } else {
        strip->setContentsMargins(0, 0, 0, 0);
        window()->removeEventFilter(this);
    }
    static_cast<TabBar *>(bar)->relayout();
}

void TabWidget::updateMaximizeButton()
{
    const bool maximized = window()->isMaximized();
    maximizeButton->setIcon(maximized ? QIcon::fromTheme("window-restore", style()->standardIcon(QStyle::SP_TitleBarNormalButton))
                                      : QIcon::fromTheme("window-maximize", style()->standardIcon(QStyle::SP_TitleBarMaxButton)));
    maximizeButton->setToolTip(maximized ? tr("Restore") : tr("Maximize"));
}

bool TabWidget::handleTitleBarMouse(QWidget *source, QEvent *event)
{
    if (!titleBarMode) {
        return false;
    }
    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    const QPoint pos = mouseEvent->position().toPoint();
    // Only empty space moves the window; tabs keep their clicks and drags.
    if (source == bar && bar->tabAt(pos) != -1) {
        return false;
    }
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        if (mouseEvent->button() == Qt::LeftButton) {
            dragStartPos = mouseEvent->globalPosition().toPoint();
            dragPending = true;
        }
        return false;
    case QEvent::MouseMove:
        if (dragPending && (mouseEvent->buttons() & Qt::LeftButton)
            && (mouseEvent->globalPosition().toPoint() - dragStartPos).manhattanLength() >= QApplication::startDragDistance()) {
            dragPending = false;
            if (QWindow *handle = window()->windowHandle()) {
                handle->startSystemMove();
            }
            return true;
        }
        return false;
    case QEvent::MouseButtonRelease:
        dragPending = false;
        return false;
    case QEvent::MouseButtonDblClick:
        if (mouseEvent->button() == Qt::LeftButton) {
            dragPending = false;
            window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
            return true;
        }
        return false;
    default:
        return false;
    }
}

int TabWidget::addTab(QWidget *widget, const QString &label)
{
    const int index = stack->addWidget(widget);
    pinnedTabs.reset();
    bar->insertTab(index, label);
    return index;
}

// Keep the pages in the same order as the tabs after the user drags one.
void TabWidget::moveStackWidget(int from, int to)
{
    // The tab bar has moved the tab already, so the pinned tabs are at new places.
    pinnedTabs.reset();
    QWidget *page = stack->widget(from);
    if (!page) {
        return;
    }
    const QSignalBlocker blocker(stack);
    stack->removeWidget(page);
    stack->insertWidget(to, page);
    stack->setCurrentIndex(bar->currentIndex());
}

void TabWidget::setTabIcon(int index, const QIcon &icon)
{
    bar->setTabIcon(index, icon);
}

WebView *TabWidget::webViewAt(int index) const
{
    return qobject_cast<WebView *>(widget(index));
}

bool TabWidget::isPinned(int index) const
{
    const auto *w = widget(index);
    return w && w->property("pinned").toBool();
}

int TabWidget::pinnedCount() const
{
    return static_cast<int>(pinnedIndices().size());
}

const QList<int> &TabWidget::pinnedIndices() const
{
    if (!pinnedTabs) {
        pinnedTabs.emplace();
        for (int i = 0; i < count(); ++i) {
            if (isPinned(i)) {
                pinnedTabs->append(i);
            }
        }
    }
    return *pinnedTabs;
}

QTabBar::ButtonPosition TabWidget::closeButtonSide() const
{
    return static_cast<QTabBar::ButtonPosition>(
        style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, nullptr, tabBar()));
}

void TabWidget::setTabTitle(int index, const QString &title)
{
    if (index < 0) {
        return;
    }
    bar->setTabToolTip(index, title);
    bar->setTabText(index, isPinned(index) ? QString() : title);
}

// A pinned tab without a favicon would otherwise be an empty stub.
void TabWidget::updateTabIcon(int index)
{
    auto *view = webViewAt(index);
    if (!view) {
        return;
    }
    const QIcon icon = view->icon();
    setTabIcon(index, icon.isNull() && isPinned(index) ? style()->standardIcon(QStyle::SP_FileIcon) : icon);
}

void TabWidget::setPinned(int index, bool pinned)
{
    auto *view = webViewAt(index);
    if (!view || isPinned(index) == pinned) {
        return;
    }
    // Pinned tabs form a group on the left: pinning moves the tab to the end of that group,
    // unpinning to just after it.
    const int target = pinned ? pinnedCount() : pinnedCount() - 1;
    view->setProperty("pinned", pinned);
    pinnedTabs.reset();
    tabBar()->moveTab(index, target);
    const int i = indexOf(view);
    const auto side = closeButtonSide();
    if (pinned) {
        if (auto *button = tabBar()->tabButton(i, side)) {
            tabBar()->setTabButton(i, side, nullptr);
            button->deleteLater();
        }
    } else {
        auto *button = new TabCloseButton(tabBar());
        QPointer<WebView> guard = view;
        connect(button, &QAbstractButton::clicked, this, [this, guard] {
            if (guard) {
                removeTab(indexOf(guard));
            }
        });
        tabBar()->setTabButton(i, side, button);
    }
    setTabTitle(i, view->title());
    updateTabIcon(i);
}

// Keep pinned tabs in front after a drag or programmatic move, preserving relative order.
void TabWidget::normalizePinnedOrder()
{
    int target = 0;
    for (int i = 0; i < count(); ++i) {
        if (isPinned(i)) {
            if (i != target) {
                tabBar()->moveTab(i, target);
            }
            ++target;
        }
    }
}

bool TabWidget::closeCurrentTabByShortcut()
{
    if (isPinned(currentIndex())) {
        const int firstUnpinned = pinnedCount();
        if (firstUnpinned < count()) {
            setCurrentIndex(firstUnpinned);
        }
        return true;
    }
    if (count() > 1) {
        removeTab(currentIndex());
        return true;
    }
    return false;
}

void TabWidget::showTabMenu(const QPoint &pos)
{
    const int index = tabBar()->tabAt(pos);
    QMenu menu(this);
    menu.addAction(tr("New tab"), this, &TabWidget::newTabButtonClicked);
    QPointer<WebView> view = webViewAt(index);
    if (view) {
        menu.addAction(tr("Reload tab"), view.data(), &WebView::reloadPage);
        menu.addAction(tr("Duplicate tab"), this, [this, view] {
            if (view) {
                duplicateTab(indexOf(view));
            }
        });
        menu.addAction(isPinned(index) ? tr("Unpin tab") : tr("Pin tab"), this, [this, view] {
            if (view) {
                const int i = indexOf(view);
                setPinned(i, !isPinned(i));
            }
        });
        menu.addAction(view->page()->isAudioMuted() ? tr("Unmute tab") : tr("Mute tab"), this, [view] {
            if (view) {
                view->page()->setAudioMuted(!view->page()->isAudioMuted());
            }
        });
        menu.addSeparator();
        menu.addAction(tr("Close tab"), this, [this, view] {
            if (view) {
                removeTab(indexOf(view));
            }
        });
        // Guarded now: the menu runs its own event loop, in which a page can close its tab.
        QList<QPointer<QWidget>> others;
        QList<QPointer<QWidget>> right;
        // Pinned tabs are only closed one at a time, explicitly.
        for (int i = 0; i < count(); ++i) {
            if (isPinned(i)) {
                continue;
            }
            if (i != index) {
                others.append(widget(i));
            }
            if (i > index) {
                right.append(widget(i));
            }
        }
        menu.addAction(tr("Close other tabs"), this, [this, others] { closeTabs(others); })->setEnabled(!others.isEmpty());
        menu.addAction(tr("Close tabs to the right"), this, [this, right] { closeTabs(right); })
            ->setEnabled(!right.isEmpty());
    }
    menu.exec(tabBar()->mapToGlobal(pos));
}

void TabWidget::closeTabs(const QList<QPointer<QWidget>> &tabs)
{
    // Tabs may have been closed while the menu was open, so look each one up again.
    for (const auto &tab : tabs) {
        const int i = tab ? indexOf(tab) : -1;
        if (i >= 0 && count() > 1) {
            removeTab(i);
        }
    }
}

void TabWidget::duplicateTab(int index)
{
    auto *source = webViewAt(index);
    if (!source) {
        return;
    }
    auto *copy = new WebView(profile);
    addNewTab(copy, true);
    if (source->isGeneratedPage()) {
        // The history would reload the generated HTML as if it were the page itself.
        copy->setUrl(source->url());
    } else {
        // Copying the history restores back/forward and loads the current entry.
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << *source->history();
        QDataStream in(&data, QIODevice::ReadOnly);
        in >> *copy->history();
    }
    tabBar()->moveTab(indexOf(copy), index + 1);
    normalizePinnedOrder();
}

// Speaker button on the tab (opposite the close button) while the page plays sound or is muted;
// clicking it toggles mute.
void TabWidget::updateAudioButton(WebView *webView)
{
    const int i = indexOf(webView);
    if (i < 0) {
        return;
    }
    const auto side = closeButtonSide() == QTabBar::LeftSide ? QTabBar::RightSide : QTabBar::LeftSide;
    auto *button = qobject_cast<QToolButton *>(tabBar()->tabButton(i, side));
    const bool muted = webView->page()->isAudioMuted();
    if (!muted && !webView->page()->recentlyAudible()) {
        if (button) {
            tabBar()->setTabButton(i, side, nullptr);
            button->deleteLater();
        }
        return;
    }
    if (!button) {
        button = new QToolButton(tabBar());
        button->setAutoRaise(true);
        button->setIconSize(QSize(16, 16));
        QPointer<WebView> view = webView;
        connect(button, &QToolButton::clicked, this, [view] {
            if (view) {
                view->page()->setAudioMuted(!view->page()->isAudioMuted());
            }
        });
        tabBar()->setTabButton(i, side, button);
    }
    button->setIcon(muted ? QIcon::fromTheme("audio-volume-muted") : QIcon::fromTheme("audio-volume-high"));
    if (button->icon().isNull()) {
        button->setText(muted ? QStringLiteral("\U0001F507") : QStringLiteral("\U0001F50A"));
    }
    button->setToolTip(muted ? tr("Unmute tab") : tr("Mute tab"));
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == window() && event->type() == QEvent::WindowStateChange) {
        updateMaximizeButton();
    }
    if (obj == tabBar() && event->type() == QEvent::MouseButtonPress
        && static_cast<QMouseEvent *>(event)->button() == Qt::MiddleButton) {
        const int index = tabBar()->tabAt(static_cast<QMouseEvent *>(event)->position().toPoint());
        if (index != -1 && !isPinned(index)) {
            removeTab(index);
        }
        return true;
    }
    if ((obj == tabBar() || obj == strip)
        && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick)
        && handleTitleBarMouse(static_cast<QWidget *>(obj), event)) {
        return true;
    }
    // The "+" button is shown with the tabs, which hide themselves when there is a single tab.
    if (obj == tabBar() && (event->type() == QEvent::Show || event->type() == QEvent::Hide)) {
        updateNewTabButton();
    }
    if (obj == strip && event->type() == QEvent::Resize) {
        static_cast<TabBar *>(bar)->relayout();
    }
    // Reordering while QTabBar is still dragging would confuse it, so fix the order once the drag ends.
    if (obj == tabBar() && event->type() == QEvent::MouseButtonRelease) {
        QTimer::singleShot(0, this, &TabWidget::normalizePinnedOrder);
    }
    return QWidget::eventFilter(obj, event);
}

void TabWidget::handleCurrentChanged(int index)
{
    auto *webView = currentWebView();
    if (webView) {
        setTabTitle(index, webView->title());
    }
}

void TabWidget::removeTab(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    auto *w = widget(index);
    if (w) {
        auto *webView = qobject_cast<WebView *>(w);
        if (!webView || webView->url().scheme() != "mx-settings") {
            finalizeRemoveTab(index);
            return;
        }
        setCurrentIndex(index);
        QPointer<WebView> settingsView = webView;
        // Tabs can move or close while the page answers or the dialog is open, so find the tab again
        // each time instead of trusting index.
        auto closeSettingsTab = [this, settingsView] {
            if (settingsView) {
                finalizeRemoveTab(indexOf(settingsView));
            }
        };
        webView->page()->runJavaScript("window.mxSettingsDirty === true", [this, settingsView, closeSettingsTab](const QVariant &result) {
            if (!settingsView) {
                return;
            }
            if (!result.toBool()) {
                closeSettingsTab();
                return;
            }
            QMessageBox box(this);
            box.setIcon(QMessageBox::Question);
            box.setWindowTitle(tr("Unsaved settings"));
            box.setText(tr("Save changes before closing?"));
            box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Save);
            const auto choice = box.exec();
            if (!settingsView) {
                return;
            }
            if (choice == QMessageBox::Save) {
                // The page only builds the request; it is handled here before the tab goes, since a
                // navigation started by the page could still be pending when the tab is deleted.
                settingsView->page()->runJavaScript(
                    "window.mxSettingsSaveUrl ? window.mxSettingsSaveUrl() : ''",
                    [this, closeSettingsTab](const QVariant &result) {
                        const QUrl url(result.toString());
                        if (url.scheme() == "mx-settings" && url.host() == "save") {
                            emit settingsSaveRequested(url);
                        }
                        closeSettingsTab();
                    });
                return;
            }
            if (choice == QMessageBox::Discard) {
                closeSettingsTab();
            }
        });
        return;
    }
    updateNewTabButton();
}

void TabWidget::finalizeRemoveTab(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    auto *w = widget(index);
    if (!w) {
        updateNewTabButton();
        return;
    }
    // Closing the last tab closes the window, which keeps the tab if the user cancels.
    if (count() == 1) {
        window()->close();
        return;
    }
    if (auto *webView = qobject_cast<WebView *>(w)) {
        emit tabClosed(webView->url());
    }
    stack->removeWidget(w);
    pinnedTabs.reset();
    bar->removeTab(index);
    w->deleteLater();
    updateNewTabButton();
}

WebView *TabWidget::createTab(bool makeCurrent)
{
    QPointer<WebView> webView = new WebView(profile);
    addNewTab(webView, makeCurrent);
    return webView.data();
}

void TabWidget::addNewTab(WebView *webView, bool makeCurrent)
{
    auto tab = addTab(webView, tr("New Tab"));
    emit viewAdded(webView);
    if (makeCurrent) {
        setCurrentIndex(tab);
    }
    connect(webView, &WebView::titleChanged, this, [this, webView] {
        if (webView) {
            setTabTitle(indexOf(webView), webView->title());
        }
    });
    connect(webView->page(), &QWebEnginePage::recentlyAudibleChanged, this, [this, webView] { updateAudioButton(webView); });
    connect(webView->page(), &QWebEnginePage::audioMutedChanged, this, [this, webView] { updateAudioButton(webView); });
    connect(webView, &WebView::iconChanged, this, [this, webView] {
        if (webView) {
            updateTabIcon(indexOf(webView));
        }
    });
    connect(webView, &WebView::newWebView, this, [this, webView](WebView *view, bool makeCurrent) {
        addNewTab(view, false);
        placeAfterOpener(view, webView);
        if (makeCurrent) {
            setCurrentIndex(indexOf(view));
        }
    });
    // Popups opened as tabs (e.g. OAuth flows) close themselves with window.close()
    connect(webView->page(), &QWebEnginePage::windowCloseRequested, this, [this, webView] {
        const int i = indexOf(webView);
        if (i >= 0 && count() > 1) {
            removeTab(i);
        }
    });
    updateNewTabButton();
}

// Next to the opener, after the tabs it already opened there, so several links keep their order.
void TabWidget::placeAfterOpener(WebView *view, WebView *opener)
{
    const int last = indexOf(view);
    if (last < 0 || indexOf(opener) < 0) {
        return;
    }
    view->setProperty("opener", QVariant::fromValue(QPointer<WebView>(opener)));
    // Children of a pinned opener start after the pinned group.
    int target = std::max(indexOf(opener) + 1, pinnedCount());
    while (target < last) {
        const auto *next = webViewAt(target);
        if (!next || next->property("opener").value<QPointer<WebView>>() != opener) {
            break;
        }
        ++target;
    }
    tabBar()->moveTab(last, target);
}

void TabWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
            setCurrentIndex(event->key() - Qt::Key_1);
            return;
        }
        if (event->key() == Qt::Key_0) {
            setCurrentIndex(9);
            return;
        }
        if (event->key() == Qt::Key_Tab) {
            setCurrentIndex((currentIndex() + 1) % count());
            return;
        }
        if (event->key() == Qt::Key_W) {
            if (!closeCurrentTabByShortcut()) {
                window()->close();
            }
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

WebView *TabWidget::currentWebView()
{
    return qobject_cast<WebView *>(currentWidget());
}

void TabWidget::updateNewTabButton()
{
    newTabButton->setVisible(!bar->isHidden());
}

int TabWidget::tabAreaWidth() const
{
    int width = strip->contentsRect().width() - newTabButtonSpacing - newTabButton->width();
    if (!windowButtons->isHidden()) {
        width -= windowButtons->sizeHint().width();
    }
    return width;
}

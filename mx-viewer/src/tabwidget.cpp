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
#include "tabwidget.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QSize>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>

TabWidget::TabWidget(QWebEngineProfile *profile, QWidget *parent)
    : QTabWidget(parent),
      newTabButton(new QPushButton("+", this)),
      profile(profile)
{
    setTabBarAutoHide(true);
    setTabsClosable(true);
    setMovable(true);
    createTab();
    connect(this, &QTabWidget::tabCloseRequested, this, &TabWidget::removeTab);
    connect(this, &QTabWidget::currentChanged, this, &TabWidget::handleCurrentChanged);

    newTabButton->setMaximumSize(30, 30);
    newTabButton->setParent(this);
    newTabButton->setToolTip(tr("New tab"));
    newTabButton->hide();
    connect(newTabButton, &QPushButton::clicked, this, &TabWidget::newTabButtonClicked);
    tabBar()->installEventFilter(this);
    updateNewTabButton();
}

void TabWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        auto index = tabBar()->tabAt(event->pos());
        if (index != -1) {
            removeTab(index);
        }
    }
    QTabWidget::mousePressEvent(event);
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == tabBar() && (event->type() == QEvent::Resize || event->type() == QEvent::LayoutRequest || event->type() == QEvent::Show)) {
        QTimer::singleShot(0, this, &TabWidget::positionNewTabButton);
    }
    return QTabWidget::eventFilter(obj, event);
}

void TabWidget::handleCurrentChanged(int index)
{
    auto *webView = currentWebView();
    if (webView) {
        setTabText(index, webView->title());
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
        webView->page()->runJavaScript("window.mxSettingsDirty === true", [this, index, settingsView](const QVariant &result) {
            if (!settingsView || index < 0 || index >= count()) {
                return;
            }
            if (!result.toBool()) {
                finalizeRemoveTab(index);
                return;
            }
            QMessageBox box(this);
            box.setIcon(QMessageBox::Question);
            box.setWindowTitle(tr("Unsaved settings"));
            box.setText(tr("Save changes before closing?"));
            box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Save);
            const auto choice = box.exec();
            if (choice == QMessageBox::Save) {
                settingsView->page()->runJavaScript("document.getElementById('save').click();",
                                                    [this, index, settingsView](const QVariant &) {
                                                        if (!settingsView || index < 0 || index >= count()) {
                                                            return;
                                                        }
                                                        finalizeRemoveTab(index);
                                                    });
                return;
            }
            if (choice == QMessageBox::Discard) {
                finalizeRemoveTab(index);
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
    if (auto *webView = qobject_cast<WebView *>(w)) {
        emit tabClosed(webView->url());
    }
    QTabWidget::removeTab(index);
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
    if (makeCurrent) {
        setCurrentIndex(tab);
    }
    connect(webView, &WebView::titleChanged, this, [this, webView] {
        if (webView) {
            setTabText(indexOf(webView), webView->title());
        }
    });
    connect(webView, &WebView::iconChanged, this, [this, webView] {
        if (webView) {
            setTabIcon(indexOf(webView), webView->icon());
        }
    });
    connect(webView, &WebView::newWebView, this, [this](WebView *view, bool makeCurrent) {
        addNewTab(view, makeCurrent);
    });
    updateNewTabButton();
    QTimer::singleShot(0, this, &TabWidget::positionNewTabButton);
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
            count() == 1 ? QApplication::quit() : removeTab(currentIndex());
            return;
        }
    }
    QTabWidget::keyPressEvent(event);
}

WebView *TabWidget::currentWebView()
{
    return qobject_cast<WebView *>(currentWidget());
}

void TabWidget::updateNewTabButton()
{
    if (count() < 2) {
        newTabButton->hide();
        return;
    }
    newTabButton->show();
    positionNewTabButton();
    QTimer::singleShot(0, this, &TabWidget::positionNewTabButton);
}

void TabWidget::positionNewTabButton()
{
    if (count() < 2) {
        newTabButton->hide();
        return;
    }

    int lastIndex = count() - 1;
    QRect tabRect = tabBar()->tabRect(lastIndex);
    int reservedRight = 0;
    const QRect barRect = tabBar()->rect();
    const auto toolButtons = tabBar()->findChildren<QToolButton *>();
    for (const auto *button : toolButtons) {
        if (!button || !button->isVisible()) {
            continue;
        }
        const QRect buttonRect = button->geometry();
        if (buttonRect.right() >= barRect.right() - 2) {
            reservedRight += buttonRect.width() + 2;
        }
    }

    constexpr int scrollButtonPadding = 14;
    int minX = tabRect.right() + 2;
    int buttonX = minX;
    if (reservedRight > 0) {
        int maxX = barRect.right() - reservedRight - scrollButtonPadding - newTabButton->width();
        if (maxX < minX) {
            newTabButton->hide();
            return;
        }
        buttonX = qMin(buttonX, maxX);
    }
    int buttonY = tabRect.top() + (tabRect.height() - newTabButton->height()) / 2;

    newTabButton->move(buttonX, buttonY);
    newTabButton->show();
}

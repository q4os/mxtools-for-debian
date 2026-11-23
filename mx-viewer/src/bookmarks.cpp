/*****************************************************************************
 * bookmarks.cpp
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

#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

#include "mainwindowhelpers.h"

using namespace MainWindowHelpers;

namespace MainWindowHelpers {
// A bookmark keeps its title in the "title" property; its text is the title with "&" escaped,
// since menus would otherwise take the "&" for a mnemonic marker.
void setBookmarkTitle(QAction *bookmark, const QString &title)
{
    bookmark->setProperty("title", title);
    bookmark->setText(QString(title).replace('&', "&&"));
}

QString bookmarkTitle(const QAction *bookmark)
{
    return bookmark->property("title").toString();
}

} // namespace MainWindowHelpers

void MainWindow::addBookmarksSubmenu()
{
    // A menu takes any button release as a click, so right and middle clicks on bookmarks are handled
    // in eventFilter() before the menu sees them.
    bookmarks->installEventFilter(this);
}

void MainWindow::showBookmarkMenu(QAction *bookmark, QPoint globalPos, bool fromBar)
{
    const QList<QAction *> bookmarkActions = bookmarkList();
    const int index = bookmarkActions.indexOf(bookmark);
    if (index < 0) {
        return;
    }
    // Bookmarks are saved from regular windows only, so a private window leaves them as they are.
    const bool editable = !privateWindow;
    QMenu submenu;
    QAction *moveUp {nullptr};
    QAction *moveDown {nullptr};
    // The bar is reordered by dragging instead.
    if (!fromBar && editable) {
        if (index > 0) {
            moveUp = submenu.addAction(QIcon::fromTheme("arrow-up"), tr("Move up"));
        }
        if (index < bookmarkActions.count() - 1) {
            moveDown = submenu.addAction(QIcon::fromTheme("arrow-down"), tr("Move down"));
        }
    }
    QAction *openInTab = submenu.addAction(QIcon::fromTheme("tab-new"), tr("Open in new tab"));
    QAction *openInWindow = submenu.addAction(QIcon::fromTheme("window-new"), tr("Open in new window"));
    QAction *openInPrivate = submenu.addAction(QIcon::fromTheme("view-private"), tr("Open in new private window"));
    submenu.addSeparator();
    QAction *edit = submenu.addAction(QIcon::fromTheme("edit-symbolic"), tr("Edit..."));
    QAction *remove = submenu.addAction(QIcon::fromTheme("user-trash"), tr("Delete"));
    submenu.addSeparator();
    QAction *manage = submenu.addAction(QIcon::fromTheme("document-edit"), tr("Manage bookmarks"));
    edit->setEnabled(editable);
    remove->setEnabled(editable);
    manage->setEnabled(editable);
    adaptMenuIcons(&submenu);
    const QPointer<QAction> target(bookmark);
    QAction *chosen = submenu.exec(globalPos);
    if (!chosen || !target) {
        return;
    }
    // Close the Bookmarks menu chain before anything that opens a window or a dialog.
    auto closeMenus = [this, fromBar] {
        if (fromBar) {
            return;
        }
        QWidget *top = bookmarks;
        while (qobject_cast<QMenu *>(top->parentWidget())) {
            top = top->parentWidget();
        }
        top->hide();
    };
    const QUrl url = target->property("url").toUrl();
    if (chosen == moveUp) {
        insertBookmark(target, index - 1);
        bookmarksChanged();
    } else if (chosen == moveDown) {
        insertBookmark(target, index + 1);
        bookmarksChanged();
    } else if (chosen == openInTab) {
        openLinkInNewTab(url);
    } else if (chosen == openInWindow || chosen == openInPrivate) {
        closeMenus();
        openInNewWindow(url, chosen == openInPrivate);
    } else if (chosen == edit) {
        if (editBookmark(target)) {
            bookmarksChanged();
        }
    } else if (chosen == remove) {
        bookmarks->removeAction(target);
        target->deleteLater();
        bookmarksChanged();
    } else if (chosen == manage) {
        closeMenus();
        openBookmarksEditor();
    }
}

void MainWindow::showBookmarkBarMenu(QPoint globalPos)
{
    QMenu menu;
    QAction *add = menu.addAction(addBookmark->icon(), tr("Bookmark current address"));
    add->setEnabled(addBookmark->isEnabled() && currentWebView());
    QAction *manage = menu.addAction(QIcon::fromTheme("document-edit"), tr("Manage bookmarks"));
    manage->setEnabled(!privateWindow);
    menu.addSeparator();
    menu.addAction(bookmarkBarAction);
    adaptMenuIcons(&menu);
    QAction *chosen = menu.exec(globalPos);
    if (chosen == add) {
        addBookmark->trigger();
    } else if (chosen == manage) {
        openBookmarksEditor();
    }
}

void MainWindow::setupBookmarkBar()
{
    bookmarkBar = new BookmarkBar(this);
    addToolBarBreak();
    addToolBar(bookmarkBar);
    bookmarkBar->setEditable(!privateWindow);
    updateBookmarkBar();
    connect(bookmarkBar, &BookmarkBar::bookmarkMoved, this, &MainWindow::moveBookmark);
    connect(bookmarkBar, &BookmarkBar::urlDropped, this, &MainWindow::addDroppedBookmark);
    connect(bookmarkBar, &BookmarkBar::openInNewTabRequested, this,
            [this](QAction *bookmark) { openLinkInNewTab(bookmark->property("url").toUrl()); });
    connect(bookmarkBar, &BookmarkBar::bookmarkMenuRequested, this,
            [this](QAction *bookmark, QPoint globalPos) { showBookmarkMenu(bookmark, globalPos, true); });
    connect(bookmarkBar, &BookmarkBar::barMenuRequested, this, &MainWindow::showBookmarkBarMenu);

    bookmarkBarAction->setChecked(settings.value("BookmarkBar", true).toBool());
    bookmarkBar->setVisible(bookmarkBarAction->isChecked());
    connect(bookmarkBarAction, &QAction::toggled, this, [this](bool checked) {
        bookmarkBar->setVisible(checked && toolbarsVisible);
        // A private window changes only its own bar: it must not store the preference, which the
        // other windows would do when updated.
        if (privateWindow) {
            return;
        }
        settings.setValue("BookmarkBar", checked);
        // Shown or hidden in every window, as the setting applies to all of them.
        for (auto *widget : QApplication::topLevelWidgets()) {
            auto *window = qobject_cast<MainWindow *>(widget);
            if (window && window != this && window->bookmarkBarAction) {
                window->bookmarkBarAction->setChecked(checked);
            }
        }
    });
}

void MainWindow::updateBookmarkBar()
{
    if (bookmarkBar) {
        bookmarkBar->setBookmarks(bookmarkList());
    }
}

QList<QAction *> MainWindow::bookmarkList() const
{
    QList<QAction *> list;
    for (auto *action : bookmarks->actions()) {
        if (action->property("url").isValid()) {
            list.append(action);
        }
    }
    return list;
}

// Places a bookmark at the given position among the bookmarks, counted without the bookmark itself.
void MainWindow::insertBookmark(QAction *bookmark, int index)
{
    QList<QAction *> others = bookmarkList();
    others.removeAll(bookmark);
    bookmarks->insertAction(index >= 0 && index < others.count() ? others.at(index) : nullptr, bookmark);
}

// "to" is the drop position in the list as it was before the move.
void MainWindow::moveBookmark(int from, int to)
{
    QAction *bookmark = bookmarkList().value(from);
    if (!bookmark) {
        return;
    }
    insertBookmark(bookmark, to > from ? to - 1 : to);
    bookmarksChanged();
}

void MainWindow::addDroppedBookmark(const QUrl &url, const QString &title, int index)
{
    if (!url.isValid()) {
        return;
    }
    // An address already bookmarked is moved to where it was dropped instead of added twice.
    const QList<QAction *> list = bookmarkList();
    for (int i = 0; i < list.count(); ++i) {
        if (list.at(i)->property("url").toUrl() == url) {
            if (i != index && i + 1 != index) {
                moveBookmark(i, index);
            }
            return;
        }
    }
    // Title and icon from a tab showing the page, in any window.
    QString bookmarkTitle;
    QIcon icon;
    for (auto *widget : QApplication::topLevelWidgets()) {
        auto *window = qobject_cast<MainWindow *>(widget);
        if (!window) {
            continue;
        }
        for (int i = 0; i < window->tabWidget->count() && bookmarkTitle.isEmpty(); ++i) {
            auto *view = qobject_cast<WebView *>(window->tabWidget->widget(i));
            if (view && view->url() == url) {
                bookmarkTitle = view->title();
                icon = view->icon();
            }
        }
    }
    if (bookmarkTitle.isEmpty()) {
        bookmarkTitle = title;
    }
    if (bookmarkTitle.isEmpty()) {
        bookmarkTitle = url.host().isEmpty() ? url.toString() : url.host();
    }
    auto *bookmark = new QAction(icon, QString(), bookmarks);
    setBookmarkTitle(bookmark, bookmarkTitle);
    bookmark->setProperty("url", url);
    connectAddress(bookmark);
    insertBookmark(bookmark, index);
    bookmarksChanged();
}

bool MainWindow::editBookmark(QAction *bookmark)
{
    const QPointer<QAction> target(bookmark);
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Edit bookmark"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *formLayout = new QFormLayout;
    auto *titleEdit = new QLineEdit(bookmarkTitle(bookmark), &dialog);
    auto *urlEdit = new QLineEdit(bookmark->property("url").toString(), &dialog);
    titleEdit->setMinimumWidth(360);
    formLayout->addRow(tr("Title"), titleEdit);
    formLayout->addRow(tr("URL"), urlEdit);
    layout->addLayout(formLayout);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(urlEdit, &QLineEdit::textChanged, &dialog, [buttons](const QString &text) {
        buttons->button(QDialogButtonBox::Save)->setEnabled(!text.trimmed().isEmpty());
    });
    titleEdit->selectAll();
    if (dialog.exec() != QDialog::Accepted || !target) {
        return false;
    }
    const QUrl url = QUrl::fromUserInput(urlEdit->text().trimmed());
    if (!url.isValid()) {
        return false;
    }
    setBookmarkTitle(target, titleEdit->text().trimmed().isEmpty() ? url.toString() : titleEdit->text().trimmed());
    target->setProperty("url", url.toString());
    return true;
}

// Saves the bookmarks and shows the change in every window.
void MainWindow::bookmarksChanged()
{
    updateBookmarkBar();
    if (privateWindow) {
        return;
    }
    saveBookmarks();
    for (auto *widget : QApplication::topLevelWidgets()) {
        auto *window = qobject_cast<MainWindow *>(widget);
        if (window && window != this) {
            window->reloadBookmarks();
        }
    }
}

void MainWindow::reloadBookmarks()
{
    for (auto *bookmark : bookmarkList()) {
        bookmarks->removeAction(bookmark);
        bookmark->deleteLater();
    }
    loadBookmarks();
    adaptMenuIcons(bookmarks);
    updateBookmarkBar();
}

void MainWindow::loadBookmarks()
{
    int size = settings.beginReadArray("Bookmarks");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QAction *bookmark {nullptr};
        bookmarks->addAction(bookmark = new QAction(settings.value("icon").value<QIcon>(), QString(), bookmarks));
        setBookmarkTitle(bookmark, settings.value("title").toString());
        bookmark->setProperty("url", settings.value("url"));
        connectAddress(bookmark);
    }
    settings.endArray();
}

void MainWindow::saveBookmarks()
{
    // Removed first, so entries past the end of a shorter list are not left behind.
    settings.remove("Bookmarks");
    settings.beginWriteArray("Bookmarks");
    int index = 0;
    for (auto *action : bookmarks->actions()) {
        if (!action->property("url").isValid()) {
            continue;
        }
        settings.setArrayIndex(index++);
        settings.setValue("title", bookmarkTitle(action));
        settings.setValue("url", action->property("url").toString());
        settings.setValue("icon", action->icon());
    }
    settings.endArray();
}

// Show the hovered URL in the status bar and connect it to launch it.
void MainWindow::connectAddress(const QAction *action)
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
}

void MainWindow::openBookmarksEditor()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Manage bookmarks"));
    dialog.resize(520, 420);

    auto *layout = new QVBoxLayout(&dialog);
    auto *list = new QListWidget(&dialog);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(list);

    auto *formLayout = new QFormLayout;
    auto *titleEdit = new QLineEdit(&dialog);
    auto *urlEdit = new QLineEdit(&dialog);
    formLayout->addRow(tr("Title"), titleEdit);
    formLayout->addRow(tr("URL"), urlEdit);
    layout->addLayout(formLayout);

    auto *controlsLayout = new QHBoxLayout;
    auto *moveUpButton = new QPushButton(QIcon::fromTheme("arrow-up"), tr("Move up"), &dialog);
    auto *moveDownButton = new QPushButton(QIcon::fromTheme("arrow-down"), tr("Move down"), &dialog);
    auto *removeButton = new QPushButton(QIcon::fromTheme("user-trash"), tr("Remove"), &dialog);
    controlsLayout->addWidget(moveUpButton);
    controlsLayout->addWidget(moveDownButton);
    controlsLayout->addWidget(removeButton);
    controlsLayout->addStretch();
    layout->addLayout(controlsLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    for (auto *action : bookmarks->actions()) {
        if (!action->property("url").isValid()) {
            continue;
        }
        auto *item = new QListWidgetItem(action->icon(), bookmarkTitle(action), list);
        item->setData(Qt::UserRole, action->property("url").toString());
    }

    auto syncEditors = [list, titleEdit, urlEdit, moveUpButton, moveDownButton, removeButton] {
        auto *item = list->currentItem();
        const bool hasItem = (item != nullptr);
        titleEdit->setEnabled(hasItem);
        urlEdit->setEnabled(hasItem);
        moveUpButton->setEnabled(hasItem && list->currentRow() > 0);
        moveDownButton->setEnabled(hasItem && list->currentRow() < list->count() - 1);
        removeButton->setEnabled(hasItem);
        if (!hasItem) {
            titleEdit->clear();
            urlEdit->clear();
            return;
        }
        titleEdit->setText(item->text());
        urlEdit->setText(item->data(Qt::UserRole).toString());
    };

    connect(list, &QListWidget::currentRowChanged, &dialog, [syncEditors] { syncEditors(); });
    connect(titleEdit, &QLineEdit::textEdited, &dialog, [list](const QString &text) {
        if (auto *item = list->currentItem()) {
            item->setText(text);
        }
    });
    connect(urlEdit, &QLineEdit::textEdited, &dialog, [list](const QString &text) {
        if (auto *item = list->currentItem()) {
            item->setData(Qt::UserRole, text);
        }
    });
    connect(moveUpButton, &QPushButton::clicked, &dialog, [list] {
        const int row = list->currentRow();
        if (row > 0) {
            auto *item = list->takeItem(row);
            list->insertItem(row - 1, item);
            list->setCurrentItem(item);
        }
    });
    connect(moveDownButton, &QPushButton::clicked, &dialog, [list] {
        const int row = list->currentRow();
        if (row >= 0 && row < list->count() - 1) {
            auto *item = list->takeItem(row);
            list->insertItem(row + 1, item);
            list->setCurrentItem(item);
        }
    });
    connect(removeButton, &QPushButton::clicked, &dialog, [list, syncEditors] {
        const int row = list->currentRow();
        if (row >= 0) {
            delete list->takeItem(row);
            syncEditors();
        }
    });
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (list->count() > 0) {
        list->setCurrentRow(0);
    } else {
        syncEditors();
    }

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    auto actions = bookmarks->actions();
    for (auto *action : actions) {
        if (!action->property("url").isValid()) {
            continue;
        }
        bookmarks->removeAction(action);
        action->deleteLater();
    }
    for (int i = 0; i < list->count(); ++i) {
        auto *item = list->item(i);
        auto *bookmark = new QAction(item->icon(), QString(), bookmarks);
        setBookmarkTitle(bookmark, item->text());
        bookmark->setProperty("url", item->data(Qt::UserRole).toString());
        bookmarks->addAction(bookmark);
        connectAddress(bookmark);
    }
    bookmarksChanged();
}

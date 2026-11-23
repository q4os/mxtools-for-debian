/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2026 MX Authors
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
#include <QToolBar>

class QLabel;
class QMimeData;
class QToolButton;

// Bookmarks shown as buttons under the main toolbar. The bookmark actions stay owned by the Bookmarks
// menu, with the unescaped title in their "title" property; the bar only shows them and reports
// clicks, drops and context menu requests.
class BookmarkBar : public QToolBar
{
    Q_OBJECT
public:
    explicit BookmarkBar(QWidget *parent = nullptr);
    void setBookmarks(const QList<QAction *> &bookmarks);
    // A private window shows the bookmarks but must not change them.
    void setEditable(bool editable);
    // Web address carried by a drag from the address bar, a page link or another application.
    static QUrl urlFromMimeData(const QMimeData *data);

signals:
    void bookmarkMoved(int from, int to);
    void urlDropped(const QUrl &url, const QString &title, int index);
    void openInNewTabRequested(QAction *bookmark);
    void bookmarkMenuRequested(QAction *bookmark, QPoint globalPos);
    void barMenuRequested(QPoint globalPos);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QToolButton *> buttons;
    QList<QPointer<QAction>> bookmarkActions;
    QLabel *emptyHint {};
    QAction *emptyHintAction {};
    QWidget *dropIndicator {};
    QPoint pressPos;
    QPointer<QToolButton> pressedButton;
    bool editable {true};
    bool dragging {};
    bool updatePending {};
    QList<QPointer<QAction>> pendingBookmarks;

    [[nodiscard]] bool acceptsDrag(const QDropEvent *event) const;
    [[nodiscard]] int dropIndex(QPoint pos) const;
    void showDropIndicator(int index);
    void startDrag(QToolButton *button);
};

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
#include "bookmarkbar.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QStyle>
#include <QTextDocumentFragment>
#include <QToolButton>

namespace
{
// Marks a drag that started on a bookmark bar; the data is the bookmark's position.
const QString bookmarkMimeType = QStringLiteral("application/x-mx-viewer-bookmark");
constexpr int maxButtonTextWidth {160};
} // namespace

BookmarkBar::BookmarkBar(QWidget *parent)
    : QToolBar(parent),
      emptyHint {new QLabel(tr("Drag the address or a link here to bookmark it"), this)},
      dropIndicator {new QWidget(this)}
{
    setObjectName("BookmarkBar");
    setWindowTitle(tr("Bookmarks bar"));
    setMovable(false);
    setFloatable(false);
    setAcceptDrops(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setIconSize(QSize(16, 16));
    toggleViewAction()->setVisible(false);

    emptyHint->setContentsMargins(6, 3, 6, 3);
    QPalette hintPalette = emptyHint->palette();
    hintPalette.setColor(QPalette::WindowText, hintPalette.color(QPalette::PlaceholderText));
    emptyHint->setPalette(hintPalette);
    emptyHintAction = addWidget(emptyHint);

    dropIndicator->setAutoFillBackground(true);
    QPalette indicatorPalette = dropIndicator->palette();
    indicatorPalette.setColor(QPalette::Window, indicatorPalette.color(QPalette::Highlight));
    dropIndicator->setPalette(indicatorPalette);
    dropIndicator->hide();
}

void BookmarkBar::setBookmarks(const QList<QAction *> &bookmarks)
{
    if (dragging) {
        // A drop can change the bookmarks while the dragged button is still in use; rebuild afterwards.
        pendingBookmarks = QList<QPointer<QAction>>(bookmarks.cbegin(), bookmarks.cend());
        updatePending = true;
        return;
    }
    for (QAction *action : actions()) {
        if (action != emptyHintAction) {
            removeAction(action);
            // Deleting the action deletes its button, which may be the one being clicked right now.
            action->deleteLater();
        }
    }
    buttons.clear();
    bookmarkActions.clear();
    pressedButton = nullptr;
    for (QAction *bookmark : bookmarks) {
        auto *button = new QToolButton(this);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIcon(bookmark->icon().isNull() ? style()->standardIcon(QStyle::SP_FileIcon) : bookmark->icon());
        const QString url = bookmark->property("url").toString();
        const QString name = bookmark->property("title").toString();
        const QString title = name.isEmpty() ? url : name;
        // "&" would otherwise underline the next letter as a mnemonic.
        QString text = button->fontMetrics().elidedText(title, Qt::ElideRight, maxButtonTextWidth);
        button->setText(text.replace('&', "&&"));
        button->setToolTip(title == url ? url : title + '\n' + url);
        button->installEventFilter(this);
        const QPointer<QAction> target(bookmark);
        connect(button, &QToolButton::clicked, this, [target] {
            if (target) {
                target->trigger();
            }
        });
        addWidget(button);
        buttons.append(button);
        bookmarkActions.append(target);
    }
    emptyHintAction->setVisible(editable && bookmarks.isEmpty());
}

void BookmarkBar::setEditable(bool value)
{
    editable = value;
    emptyHintAction->setVisible(editable && buttons.isEmpty());
}

QUrl BookmarkBar::urlFromMimeData(const QMimeData *data)
{
    if (!data) {
        return {};
    }
    // A page can start a drag with any URL; only ordinary locations become bookmarks, never javascript:,
    // data: or internal pages.
    static const QStringList allowedSchemes {"http", "https", "file", "ftp"};
    if (data->hasUrls()) {
        const QList<QUrl> urls = data->urls();
        if (!urls.isEmpty() && urls.first().isValid() && allowedSchemes.contains(urls.first().scheme())) {
            return urls.first();
        }
    }
    if (!data->hasText()) {
        return {};
    }
    // Plain text only counts when it looks like an address, so dragging a word does not bookmark it.
    const QString text = data->text().trimmed();
    if (text.isEmpty() || text.contains(QRegularExpression("\\s"))
        || (!text.contains('.') && !text.contains("://") && !text.startsWith("localhost"))) {
        return {};
    }
    const QUrl url = QUrl::fromUserInput(text);
    return url.isValid() && allowedSchemes.contains(url.scheme()) ? url : QUrl();
}

bool BookmarkBar::acceptsDrag(const QDropEvent *event) const
{
    if (!editable) {
        return false;
    }
    if (qobject_cast<BookmarkBar *>(event->source()) && event->mimeData()->hasFormat(bookmarkMimeType)) {
        return true;
    }
    return urlFromMimeData(event->mimeData()).isValid();
}

int BookmarkBar::dropIndex(QPoint pos) const
{
    int index = 0;
    for (int i = 0; i < buttons.size(); ++i) {
        const QToolButton *button = buttons.at(i);
        if (!button->isVisible() || button->parentWidget() != this) {
            continue;
        }
        const QRect rect = button->geometry();
        const bool before = isRightToLeft() ? pos.x() > rect.center().x() : pos.x() < rect.center().x();
        if (before) {
            return i;
        }
        index = i + 1;
    }
    return index;
}

void BookmarkBar::showDropIndicator(int index)
{
    const QToolButton *reference {nullptr};
    bool atStart = true;
    if (index < buttons.size() && buttons.at(index)->isVisible()) {
        reference = buttons.at(index);
    } else {
        // After the last button still shown in the bar.
        for (int i = std::min<int>(index, buttons.size()) - 1; i >= 0; --i) {
            if (buttons.at(i)->isVisible() && buttons.at(i)->parentWidget() == this) {
                reference = buttons.at(i);
                atStart = false;
                break;
            }
        }
    }
    QRect rect = reference ? reference->geometry() : contentsRect();
    const bool leftEdge = (atStart != isRightToLeft());
    const int x = reference ? (leftEdge ? rect.left() : rect.right()) : (isRightToLeft() ? rect.right() - 2 : rect.left() + 2);
    dropIndicator->setGeometry(x - 1, rect.top(), 2, rect.height());
    dropIndicator->show();
    dropIndicator->raise();
}

void BookmarkBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (!acceptsDrag(event)) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
    showDropIndicator(dropIndex(event->position().toPoint()));
}

void BookmarkBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (!acceptsDrag(event)) {
        event->ignore();
        return;
    }
    const bool move = qobject_cast<BookmarkBar *>(event->source()) != nullptr;
    event->setDropAction(move ? Qt::MoveAction : Qt::CopyAction);
    event->accept();
    showDropIndicator(dropIndex(event->position().toPoint()));
}

void BookmarkBar::dragLeaveEvent(QDragLeaveEvent *event)
{
    dropIndicator->hide();
    QToolBar::dragLeaveEvent(event);
}

void BookmarkBar::dropEvent(QDropEvent *event)
{
    dropIndicator->hide();
    if (!acceptsDrag(event)) {
        event->ignore();
        return;
    }
    const int index = dropIndex(event->position().toPoint());
    const QMimeData *data = event->mimeData();
    // The bookmark bars of all windows show the same list, so a position from any of them applies here.
    if (qobject_cast<BookmarkBar *>(event->source()) && data->hasFormat(bookmarkMimeType)) {
        bool ok {false};
        const int from = data->data(bookmarkMimeType).toInt(&ok);
        event->setDropAction(Qt::MoveAction);
        event->accept();
        if (ok && from != index && from + 1 != index) {
            emit bookmarkMoved(from, index);
        }
        return;
    }
    const QUrl url = urlFromMimeData(data);
    QString title;
    if (data->hasHtml()) {
        title = QTextDocumentFragment::fromHtml(data->html()).toPlainText().simplified();
    }
    if (title == url.toString() || title == data->text().trimmed()) {
        title.clear();
    }
    event->setDropAction(Qt::CopyAction);
    event->accept();
    emit urlDropped(url, title, index);
}

void BookmarkBar::contextMenuEvent(QContextMenuEvent *event)
{
    event->accept();
    for (int i = 0; i < buttons.size(); ++i) {
        if (buttons.at(i)->isVisible() && buttons.at(i)->geometry().contains(event->pos())) {
            if (QAction *bookmark = bookmarkActions.at(i)) {
                emit bookmarkMenuRequested(bookmark, event->globalPos());
            }
            return;
        }
    }
    emit barMenuRequested(event->globalPos());
}

bool BookmarkBar::eventFilter(QObject *watched, QEvent *event)
{
    auto *button = qobject_cast<QToolButton *>(watched);
    const int index = button ? buttons.indexOf(button) : -1;
    if (index < 0) {
        return QToolBar::eventFilter(watched, event);
    }
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            pressPos = mouseEvent->position().toPoint();
            pressedButton = button;
        }
        return mouseEvent->button() == Qt::MiddleButton;
    }
    case QEvent::MouseMove: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (pressedButton == button && (mouseEvent->buttons() & Qt::LeftButton)
            && (mouseEvent->position().toPoint() - pressPos).manhattanLength() >= QApplication::startDragDistance()) {
            pressedButton = nullptr;
            startDrag(button);
            return true;
        }
        return false;
    }
    case QEvent::MouseButtonRelease: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::MiddleButton) {
            if (QAction *bookmark = bookmarkActions.at(index)) {
                emit openInNewTabRequested(bookmark);
            }
            return true;
        }
        pressedButton = nullptr;
        return false;
    }
    default:
        return QToolBar::eventFilter(watched, event);
    }
}

void BookmarkBar::startDrag(QToolButton *button)
{
    const int index = buttons.indexOf(button);
    QAction *bookmark = bookmarkActions.value(index);
    if (!bookmark) {
        return;
    }
    const QUrl url = bookmark->property("url").toUrl();
    auto *data = new QMimeData;
    data->setUrls({url});
    data->setText(url.toString());
    data->setHtml(QString("<a href=\"%1\">%2</a>")
                      .arg(url.toString().toHtmlEscaped(), bookmark->property("title").toString().toHtmlEscaped()));
    data->setData(bookmarkMimeType, QByteArray::number(index));
    auto *drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(button->grab());
    drag->setHotSpot(pressPos);
    // The press never gets its release, which goes to the drag instead.
    button->setDown(false);
    dragging = true;
    drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::MoveAction);
    dragging = false;
    dropIndicator->hide();
    if (updatePending) {
        updatePending = false;
        QList<QAction *> bookmarks;
        for (const auto &pending : std::as_const(pendingBookmarks)) {
            if (pending) {
                bookmarks.append(pending);
            }
        }
        pendingBookmarks.clear();
        setBookmarks(bookmarks);
    }
}

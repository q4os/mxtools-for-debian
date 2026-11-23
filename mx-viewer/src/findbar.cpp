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
#include "findbar.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStyle>
#include <QToolButton>

FindBar::FindBar(QWidget *parent)
    : QFrame(parent),
      field {new QLineEdit(this)},
      matches {new QLabel(this)},
      matchCase {new QToolButton(this)},
      previousButton {new QToolButton(this)},
      nextButton {new QToolButton(this)},
      closeButton {new QToolButton(this)}
{
    setObjectName("FindBar");
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);

    field->setPlaceholderText(tr("Find in page"));
    field->setClearButtonEnabled(true);
    field->setFixedWidth(fieldWidth);
    // The field is the only thing that takes focus, so the arrow buttons do not steal it while stepping.
    for (QToolButton *button : {matchCase, previousButton, nextButton, closeButton}) {
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
    }
    matchCase->setText(QStringLiteral("Aa"));
    matchCase->setToolTip(tr("Match case"));
    matchCase->setCheckable(true);
    previousButton->setIcon(QIcon::fromTheme("go-up", style()->standardIcon(QStyle::SP_ArrowUp)));
    previousButton->setToolTip(tr("Previous match (Shift+Enter)"));
    nextButton->setIcon(QIcon::fromTheme("go-down", style()->standardIcon(QStyle::SP_ArrowDown)));
    nextButton->setToolTip(tr("Next match (Enter)"));
    closeButton->setIcon(QIcon::fromTheme("window-close", style()->standardIcon(QStyle::SP_TitleBarCloseButton)));
    closeButton->setToolTip(tr("Close (Esc)"));
    QPalette matchesPalette = matches->palette();
    matchesPalette.setColor(QPalette::WindowText, matchesPalette.color(QPalette::PlaceholderText));
    matches->setPalette(matchesPalette);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 4, 4, 4);
    layout->setSpacing(2);
    layout->addWidget(field);
    layout->addSpacing(4);
    layout->addWidget(matches);
    layout->addSpacing(4);
    layout->addWidget(matchCase);
    layout->addWidget(previousButton);
    layout->addWidget(nextButton);
    layout->addWidget(closeButton);

    connect(field, &QLineEdit::textChanged, this, [this] { emit findRequested(false); });
    connect(matchCase, &QToolButton::toggled, this, [this] { emit findRequested(false); });
    connect(previousButton, &QToolButton::clicked, this, [this] { emit findRequested(true); });
    connect(nextButton, &QToolButton::clicked, this, [this] { emit findRequested(false); });
    connect(closeButton, &QToolButton::clicked, this, [this] {
        hide();
        emit closed();
    });
    field->installEventFilter(this);
    hide();
}

void FindBar::open(QWidget *newPage)
{
    setPage(newPage);
    show();
    raise();
    field->setFocus();
    field->selectAll();
}

void FindBar::setPage(QWidget *newPage)
{
    if (page == newPage) {
        return;
    }
    if (page) {
        page->removeEventFilter(this);
    }
    page = newPage;
    if (page) {
        page->installEventFilter(this);
    }
    reposition();
}

QString FindBar::text() const
{
    return field->text();
}

bool FindBar::isCaseSensitive() const
{
    return matchCase->isChecked();
}

void FindBar::setResult(int activeMatch, int numberOfMatches)
{
    matches->setText(numberOfMatches == 0 ? tr("No matches") : tr("%1 of %2").arg(activeMatch).arg(numberOfMatches));
    previousButton->setEnabled(numberOfMatches > 0);
    nextButton->setEnabled(numberOfMatches > 0);
    adjustSize();
    reposition();
}

void FindBar::clearResult()
{
    matches->clear();
    previousButton->setEnabled(true);
    nextButton->setEnabled(true);
    adjustSize();
    reposition();
}

void FindBar::reposition()
{
    if (!page || !parentWidget()) {
        return;
    }
    adjustSize();
    const QRect area(page->mapTo(parentWidget(), QPoint(0, 0)), page->size());
    const int x = isRightToLeft() ? area.left() + margin : area.right() - width() - margin + 1;
    move(std::max(area.left(), x), area.top() + margin / 2);
    raise();
}

bool FindBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == page && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        reposition();
    } else if (watched == field && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            emit findRequested(keyEvent->modifiers() & Qt::ShiftModifier);
            return true;
        }
        if (keyEvent->key() == Qt::Key_Escape) {
            hide();
            emit closed();
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void FindBar::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        emit closed();
        return;
    }
    QFrame::keyPressEvent(event);
}

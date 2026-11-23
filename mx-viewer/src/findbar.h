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

#include <QFrame>
#include <QPointer>

class QLabel;
class QLineEdit;
class QToolButton;

// Find-in-page panel that floats over the top right corner of the page while a search is open.
// It only collects the search and shows the result; the window runs the search on the page.
class FindBar : public QFrame
{
    Q_OBJECT
public:
    explicit FindBar(QWidget *parent = nullptr);
    // Shows the panel over the given page, with the search text selected for typing a new one.
    void open(QWidget *page);
    // Follows another page, e.g. when the current tab changes.
    void setPage(QWidget *page);
    [[nodiscard]] QString text() const;
    [[nodiscard]] bool isCaseSensitive() const;
    void setResult(int activeMatch, int numberOfMatches);
    void clearResult();

signals:
    void findRequested(bool backward);
    void closed();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QLineEdit *field {};
    QLabel *matches {};
    QToolButton *matchCase {};
    QToolButton *previousButton {};
    QToolButton *nextButton {};
    QToolButton *closeButton {};
    QPointer<QWidget> page;
    static constexpr int fieldWidth {200};
    static constexpr int margin {12};
    void reposition();
};

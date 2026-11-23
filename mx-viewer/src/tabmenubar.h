/*****************************************************************************
 * tabmenubar.h
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
#pragma once

#include <QMenuBar>
#include <QLayout>

// QMainWindow::menuBar() replaces and deletes a plain menu widget. Some styles call it while
// polishing the window, so keep the tab strip in a real menu bar and size it for its layout.
class TabMenuBar : public QMenuBar
{
public:
    explicit TabMenuBar(QWidget *parent) : QMenuBar(parent) { setNativeMenuBar(false); }

    QSize sizeHint() const override { return layout() ? layout()->sizeHint() : QMenuBar::sizeHint(); }
    QSize minimumSizeHint() const override { return layout() ? layout()->minimumSize() : QMenuBar::minimumSizeHint(); }
};

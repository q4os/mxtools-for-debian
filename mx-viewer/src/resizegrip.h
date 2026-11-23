/*****************************************************************************
 * resizegrip.h
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

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QWindow>

// Invisible handle along an edge or at a corner of a window without system decorations, which
// resizes the window when dragged.
class ResizeGrip : public QWidget
{
public:
    ResizeGrip(Qt::Edges edges, QWidget *parent)
        : QWidget(parent),
          edges(edges)
    {
        const bool horizontal = edges & (Qt::LeftEdge | Qt::RightEdge);
        const bool vertical = edges & (Qt::TopEdge | Qt::BottomEdge);
        if (horizontal && vertical) {
            const bool mainDiagonal = edges == (Qt::TopEdge | Qt::LeftEdge) || edges == (Qt::BottomEdge | Qt::RightEdge);
            setCursor(mainDiagonal ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
        } else {
            setCursor(horizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor);
        }
    }
    [[nodiscard]] Qt::Edges gripEdges() const
    {
        return edges;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && window()->windowHandle()) {
            window()->windowHandle()->startSystemResize(edges);
        }
    }
    void paintEvent(QPaintEvent * /*event*/) override
    {
        // The edges draw a thin outline, since the window has no frame of its own.
        if (edges == Qt::TopEdge || edges == Qt::BottomEdge || edges == Qt::LeftEdge || edges == Qt::RightEdge) {
            QPainter painter(this);
            painter.setPen(palette().color(QPalette::Mid));
            const QRect r = rect();
            if (edges == Qt::TopEdge) {
                painter.drawLine(r.topLeft(), r.topRight());
            } else if (edges == Qt::BottomEdge) {
                painter.drawLine(r.bottomLeft(), r.bottomRight());
            } else if (edges == Qt::LeftEdge) {
                painter.drawLine(r.topLeft(), r.bottomLeft());
            } else {
                painter.drawLine(r.topRight(), r.bottomRight());
            }
        }
    }

private:
    Qt::Edges edges;
};

/**********************************************************************
 *  Clock Face widget implementation.
 **********************************************************************
 *   Copyright (C) 2019 by AK-47
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of mx-datetime.
 **********************************************************************/

#include "clockface.h"
#include <QPainter>

ClockFace::ClockFace(QWidget *parent)
    : QWidget(parent)
{
}

void ClockFace::setTime(QTime newtime)
{
    time = newtime;
    update();
}

void ClockFace::paintEvent(QPaintEvent * /*event*/)
{
    // Polygon points for hands and marks.
    static const std::array<QPoint, 4> handHour {QPoint(7, 0), QPoint(0, 12), QPoint(-7, 0), QPoint(0, -60)};
    static const std::array<QPoint, 4> handMinute {QPoint(3, 0), QPoint(0, 6), QPoint(-3, 0), QPoint(0, -80)};
    static const std::array<QPoint, 4> handSecond {QPoint(1, 0), QPoint(0, 3), QPoint(-1, 0), QPoint(0, -95)};
    static const std::array<QPoint, 4> markMinute {QPoint(0, -94), QPoint(1, -97), QPoint(0, -98), QPoint(-1, -97)};
    static const std::array<QPoint, 4> markHour {QPoint(0, -91), QPoint(2, -96), QPoint(0, -98), QPoint(-2, -96)};
    // Colour calculations.
    QColor colNumbers = palette().text().color();
    colNumbers.setAlpha(150);
    QColor colHour = colNumbers;
    QColor colMinute = colHour;
    QColor colSecond = colMinute;
    colHour.setRed(255 - colHour.red());
    colMinute.setBlue(255 - colSecond.blue());
    colSecond.setGreen(255 - colSecond.green());

    // Paint the clock.
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(static_cast<int>(width() / 2), static_cast<int>(height() / 2));
    const int side = qMin(width(), height());
    painter.scale(side / 200.0, side / 200.0);
    // Numbers.
    painter.save();
    painter.setBrush(colNumbers);
    for (int ixi = 0; ixi < 12; ++ixi) {
        static const std::array<QString, 12> numerals {"I",   "II",   "III", "IIII", "V",  "VI",
                                                       "VII", "VIII", "IX",  "X",    "XI", "XII"};
        painter.rotate(30.0);
        painter.drawText(-12, -87, 24, 12, Qt::AlignCenter, numerals.at(ixi));
    }
    painter.restore();
    // Hour hand.
    painter.setBrush(colHour);
    painter.setPen(colHour.darker());
    painter.save();
    painter.rotate(30.0 * (time.hour() + (time.minute() / 60.0)));
    painter.drawConvexPolygon(handHour.data(), 4);
    painter.restore();
    // Minute hand.
    painter.setBrush(colMinute);
    painter.setPen(colMinute.darker());
    painter.save();
    painter.rotate(6.0 * (time.minute() + (time.second() / 60.0)));
    painter.drawConvexPolygon(handMinute.data(), 4);
    painter.restore();
    // Second hand.
    painter.setBrush(colSecond);
    painter.setPen(colSecond.darker());
    painter.save();
    painter.rotate(6.0 * time.second());
    painter.drawConvexPolygon(handSecond.data(), 4);
    colNumbers.setAlpha(255);
    painter.setBrush(colNumbers);
    painter.setPen(Qt::NoPen);
    if ((time.second() % 5) == 0) {
        painter.drawConvexPolygon(markHour.data(), 4);
    } else {
        painter.drawConvexPolygon(markMinute.data(), 4);
    }
    painter.restore();
    // Marks around the circumference.
    painter.setPen(colHour.darker());
    painter.setBrush(colHour);
    for (int ixi = 0; ixi < 12; ++ixi) {
        painter.drawConvexPolygon(markHour.data(), 4);
        painter.rotate(30.0);
    }
    painter.setPen(colMinute.darker());
    painter.setBrush(colMinute);
    for (int ixi = 0; ixi < 60; ++ixi) {
        if ((ixi % 5) != 0) {
            painter.drawConvexPolygon(markMinute.data(), 4);
        }
        painter.rotate(6.0);
    }
}

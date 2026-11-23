/*****************************************************************************
 * iconutils.cpp
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

#include <algorithm>
#include <cstdlib>

#include <QPainter>
#include <QStyleOptionMenuItem>

#include "mainwindowhelpers.h"
#include <QMenu>
#include <QAction>

using namespace MainWindowHelpers;

namespace MainWindowHelpers {
// Single-grey icons are drawn for one kind of panel (dark on light, like most themes and the bundled
// backups), so they vanish on the other; repaint those in the panel's text color. Colored icons are
// left alone.
QIcon iconForBackground(const QIcon &icon, const QColor &background, const QColor &foreground)
{
    if (icon.isNull()) {
        return icon;
    }
    const QImage sample = icon.pixmap(32).toImage().convertToFormat(QImage::Format_ARGB32);
    int darkest {255};
    int lightest {0};
    for (int y = 0; y < sample.height(); ++y) {
        for (int x = 0; x < sample.width(); ++x) {
            const QColor color = QColor::fromRgba(sample.pixel(x, y));
            if (color.alpha() < 64) {
                continue;
            }
            if (color.hsvSaturation() > 60 && color.value() > 40) {
                return icon;
            }
            darkest = std::min(darkest, color.lightness());
            lightest = std::max(lightest, color.lightness());
        }
    }
    // Nothing drawn, or a glyph over a filled shape of another grey, which a single color would erase.
    if (darkest > lightest || lightest - darkest > 80) {
        return icon;
    }
    const int mean = (darkest + lightest) / 2;
    if (std::abs(mean - background.lightness()) >= std::abs(mean - foreground.lightness())) {
        return icon;
    }
    QIcon tinted;
    for (const int size : {16, 22, 24, 32, 48, 64}) {
        QPixmap pixmap = icon.pixmap(size);
        if (pixmap.isNull()) {
            continue;
        }
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), foreground);
        painter.end();
        tinted.addPixmap(pixmap);
    }
    return tinted;
}

// Styles like qt6gtk2 paint menus from the desktop theme, which can be dark while the palette stays light,
// so ask the style what it paints behind an item and fall back to black or white text if the palette's
// text would not show on it.
std::pair<QColor, QColor> renderedMenuColors(QMenu *menu)
{
    menu->ensurePolished();
    QImage image(64, 32, QImage::Format_ARGB32_Premultiplied);
    image.fill(menu->palette().color(QPalette::Window));
    QPainter painter(&image);
    QStyleOption panel;
    panel.initFrom(menu);
    panel.rect = image.rect();
    menu->style()->drawPrimitive(QStyle::PE_PanelMenu, &panel, &painter, menu);
    QStyleOptionMenuItem item;
    item.initFrom(menu);
    item.state = QStyle::State_Enabled;
    item.menuItemType = QStyleOptionMenuItem::Normal;
    item.rect = image.rect();
    menu->style()->drawControl(QStyle::CE_MenuItem, &item, &painter, menu);
    painter.end();
    const QColor background = image.pixelColor(image.rect().center());
    QColor foreground = menu->palette().color(QPalette::WindowText);
    if (std::abs(foreground.lightness() - background.lightness()) < 100) {
        foreground = background.lightness() < 128 ? QColor(Qt::white) : QColor(Qt::black);
    }
    return {background, foreground};
}

// Bookmark icons are left alone: they are saved back to settings and must stay as fetched.
void adaptMenuIcons(QMenu *menu)
{
    const auto [background, foreground] = renderedMenuColors(menu);
    for (QAction *action : menu->actions()) {
        if (!action->property("url").isValid()) {
            action->setIcon(iconForBackground(action->icon(), background, foreground));
        }
    }
}
} // namespace MainWindowHelpers

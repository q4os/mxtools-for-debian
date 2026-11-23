/*****************************************************************************
 * mainwindowhelpers.h
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

#include <QColor>
#include <QIcon>
#include <QStringList>
#include <utility>

class QAction;
class QMenu;
class QUrlQuery;
class QWebEngineProfile;

// Shared implementation details of the MainWindow translation units.
namespace MainWindowHelpers {
void setBookmarkTitle(QAction *bookmark, const QString &title);
QString bookmarkTitle(const QAction *bookmark);
QIcon iconForBackground(const QIcon &icon, const QColor &background, const QColor &foreground);
std::pair<QColor, QColor> renderedMenuColors(QMenu *menu);
void adaptMenuIcons(QMenu *menu);
qint64 totalDirectorySize(const QStringList &paths);
QStringList collectCachePaths(const QWebEngineProfile *profile);
QString formValue(const QUrlQuery &query, const QString &name);
} // namespace MainWindowHelpers

/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
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

#include <QString>

#include <functional>

class QObject;

// Lets a plain "mx-viewer [URL]" launch (e.g. a link opened from another application) hand the URL
// to an already running browser instead of starting a second one.
namespace SingleInstance
{
// Returns true if a running instance accepted the request; the caller should then exit.
bool forward(const QString &argument);
// Starts accepting requests from later launches; handler receives the forwarded argument
// (an empty string asks for a new tab).
void listen(QObject *parent, const std::function<void(const QString &)> &handler);
} // namespace SingleInstance

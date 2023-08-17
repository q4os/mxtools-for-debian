/**********************************************************************
 *  lockfile.cpp
 **********************************************************************
 * Copyright (C) 2014 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of MX Package Installer.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-packageinstaller is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-packageinstaller.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "lockfile.h"

#include <QDebug>

#include <unistd.h>

// Checks if file is locked by another process (if locked by the same process returns false)
bool LockFile::isLocked()
{
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open lock file" << file.fileName() << "for reading:" << file.errorString();
        return false;
    }
    return (lockf(file.handle(), F_TEST, 0) != 0);
}

bool LockFile::lock()
{
    file.close(); // if openned by isLocked()
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Unable to open lock file" << file.fileName() << "for writing:" << file.errorString();
        return false;
    }
    return (lockf(file.handle(), F_LOCK, 0) == 0);
}

void LockFile::unlock()
{
    file.close();
}

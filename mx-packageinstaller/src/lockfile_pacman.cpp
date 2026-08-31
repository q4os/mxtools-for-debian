/**********************************************************************
 *  lockfile_pacman.cpp
 **********************************************************************
 * Copyright (C) 2014-2026 MX Authors
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

#include <QFile>
#include <QMessageBox>

#include "cmd.h"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

namespace {

const QString &pacmanLockPath()
{
    static const QString path = QStringLiteral("/var/lib/pacman/db.lck");
    return path;
}

// Pacman's lock is a plain lockfile (unlike dpkg's frontend-lock model), so a direct
// non-blocking lockf() probe tells us in-process whether another instance holds it,
// with no privilege elevation needed for the common "not locked" case.
bool pacmanDbLocked()
{
    if (!QFile::exists(pacmanLockPath())) {
        return false;
    }

    QFile lock {pacmanLockPath()};
    if (!lock.open(QIODevice::ReadWrite)) {
        // Can't even open it for a probe; assume another process holds it.
        return true;
    }

    const int result = lockf(lock.handle(), F_TLOCK, 0);
    lock.close();
    return (result == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
}

} // namespace

bool LockFile::isLocked()
{
    return pacmanDbLocked();
}

// Check if the file is locked and pop up a message
bool LockFile::isLockedGUI()
{
    if (Cmd::elevationDismissed()) {
        return true;
    }
    const QString proc = getLockingProcess();
    if (Cmd::elevationDismissed()) {
        // The lock check itself required elevation and the user dismissed it.
        // Cmd already showed the "Administrator Access Required" message; block
        // the operation without adding a misleading lock warning.
        return true;
    }
    if (!proc.isEmpty()) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("Pacman database is locked by another program: %1"
                                         "\nClose the program, or wait until it is done processing and try again.")
                                 .arg(proc));
        return true;
    }
    return false;
}

QString LockFile::getLockingProcess() const
{
    if (!pacmanDbLocked()) {
        return {};
    }
    return Cmd().lockingProcessAsRoot(pacmanLockPath(), Cmd::QuietMode::Yes);
}

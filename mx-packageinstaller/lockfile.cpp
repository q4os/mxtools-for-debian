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
#include <QFileInfo>
#include <QMessageBox>

#include <cmd.h>

#include <unistd.h>

LockFile::LockFile(const QString &file_name)
    : file(file_name)
{
}

bool LockFile::isLocked()
{
    return Cmd().runAsRoot("fuser " + fileName());
}

// Check if the file is locked and pop up a message
bool LockFile::isLockedGUI()
{
    QString proc = getLockingProcess();
    if (!proc.isEmpty()) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("Dpkg/apt database is locked by another program: %1"
                                         "\nClose the program, or wait until it is done processing and try again.")
                                 .arg(proc));
        return true;
    }
    return false;
}

bool LockFile::lock()
{
    file.close(); // if already opened by this process, this avoids messages that this process is locking the file
    if (isLockedGUI()) {
        return false;
    }
    Cmd().runAsRoot("chown $(logname): " + file.fileName(), true); // take ownership
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

QString LockFile::fileName()
{
    return file.fileName();
}

QString LockFile::getLockingProcess()
{
    return Cmd()
        .getOutAsRoot("pid=$(fuser " + fileName()
                      + " 2>/dev/null); [[ -n \"$pid\" ]] && ps --no-headers -o comm -p $pid")
        .trimmed();
}

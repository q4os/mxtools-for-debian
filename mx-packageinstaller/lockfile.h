/**********************************************************************
 *  lockfile.h
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

#ifndef LOCKFILE_H
#define LOCKFILE_H

#include <QFile>

class LockFile
{
public:
    LockFile(const QString &file_name)
        : file(file_name)
    {
    }
    ~LockFile() { unlock(); };

    bool isLocked();
    bool lock();
    void unlock();

private:
    QFile file;
};

#endif // LOCKFILE_H

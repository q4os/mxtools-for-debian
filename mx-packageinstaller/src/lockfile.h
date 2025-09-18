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
#pragma once

#include <QFile>

class LockFile
{
public:
    explicit LockFile(const QString &file_name);

    [[nodiscard]] QString fileName() const;
    [[nodiscard]] QString getLockingProcess() const;
    [[nodiscard]] bool isLocked();
    [[nodiscard]] bool isLockedGUI();

    [[nodiscard]] bool lock();
    void unlock();

private:
    QFile file;
};

/**********************************************************************
 *  lockfile.cpp
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

#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>

#include "cmd.h"

bool LockFile::isLocked()
{
    return !getLockingProcess().isEmpty();
}

// Check if the file is locked and pop up a message
bool LockFile::isLockedGUI()
{
    if (Cmd::elevationDismissed()) {
        return true;
    }
    QString proc = getLockingProcess();
    if (Cmd::elevationDismissed()) {
        // The lock check itself required elevation and the user dismissed it.
        // Cmd already showed the "Administrator Access Required" message; don't
        // pile on a confusing, misleading "locked by another program" box on top.
        return true;
    }
    if (!proc.isEmpty()) {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("Dpkg/apt database is locked by another program: %1"
                                         "\nClose the program, or wait until it is done processing and try again.")
                                 .arg(proc));
        return true;
    }
    return false;
}

QString LockFile::getLockingProcess() const
{
    // "apt-get check" acquires the dpkg *frontend* lock, which another apt-get/
    // aptitude/synaptic session holds for its whole run (including its download
    // phase, before dpkg is even invoked). It does not touch the lower-level
    // /var/lib/dpkg/lock, which dpkg itself (or a bare `dpkg -i`/`dpkg --configure
    // -a`) holds only while actually unpacking/configuring. Checking only one of
    // the two misses conflicts the other would catch, so check both.
    Cmd probe;
    QString output;
    const QHash<QString, QString> environment {
        {QStringLiteral("LANG"), QStringLiteral("C")},
        {QStringLiteral("LC_ALL"), QStringLiteral("C")},
    };
    const bool checkSucceeded = probe.procAsRootWithEnv(environment, QStringLiteral("apt-get"),
                                                         {QStringLiteral("check")}, &output, nullptr,
                                                         Cmd::QuietMode::Yes);
    if (Cmd::elevationDismissed()) {
        return {};
    }
    if (!checkSucceeded) {
        static const QRegularExpression lockError(
            QStringLiteral(R"((?:Unable to acquire|Could not get) the dpkg frontend lock)"));
        if (lockError.match(output).hasMatch()) {
            static const QRegularExpression ownerPattern(QStringLiteral(R"(held by process \d+ \(([^)]+)\))"));
            const QRegularExpressionMatch owner = ownerPattern.match(output);
            return owner.hasMatch() ? owner.captured(1) : QObject::tr("another package manager");
        }
    }

    return Cmd().lockingProcessAsRoot(QStringLiteral("/var/lib/dpkg/lock"), Cmd::QuietMode::Yes);
}

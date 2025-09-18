/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2025 MX Authors
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
#include "service.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

#include "cmd.h"

inline const QString initSystem {Service::getInit()};

Service::Service(QString name, bool running, bool enabled)
    : name {std::move(name)},
      running {running},
      enabled {enabled}
{
}

QString Service::getName() const
{
    return name;
}

QString Service::getInfo() const
{
    QString info;
    if (initSystem == "systemd") {
        info = Cmd().getOutAsRoot("/sbin/service " + name + " status", true).trimmed();
        if (!isEnabled()) {
            info.append("\nDescription: " + getDescription());
        }
    } else {
        info = getInfoFromFile(name);
    }
    return info;
}

bool Service::isEnabled(const QString &name)
{
    if (initSystem == QLatin1String("systemd")) {
        return QProcess::execute(QLatin1String("systemctl"),
                                 {QLatin1String("-q"), QLatin1String("is-enabled"), name + QLatin1String(".service")})
               == 0;
    } else {
        // Check both runlevel 5 (multi-user with GUI) and runlevel S (single-user/boot)
        QString command = QString(QLatin1String("[[ -e /etc/rc5.d/S*%1 || -e /etc/rcS.d/S*%1 ]]")).arg(name);
        return QProcess::execute(QLatin1String("/bin/bash"), {QLatin1String("-c"), command}) == 0;
    }
}

QString Service::getInit()
{
    QProcess proc;
    proc.start("cat", {"/proc/1/comm"});
    proc.waitForFinished();
    return proc.readAll().trimmed();
}

bool Service::isRunning() const
{
    return running;
}

QString Service::getDescription() const
{
    if (initSystem != "systemd") {
        QRegularExpression regex("\nShort-Description:([^\n]*)");
        QString info = getInfo();
        QRegularExpressionMatch match = regex.match(info);
        if (match.captured(1).isEmpty()) {
            regex.setPattern("\nDescription:\\s*(.*)\n");
            match = regex.match(info);
        }
        if (match.hasMatch()) {
            return match.captured(1);
        }
        return {};
    } else {
        // Try to get description from systemctl list-units first
        QString jsonOutput = Cmd()
                                .getOutAsRoot("systemctl list-units " + name + ".service -o json",
                                              true, true)
                                .trimmed();

        QString out;
        if (!jsonOutput.isEmpty()) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(jsonOutput.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError && doc.isArray()) {
                QJsonArray array = doc.array();
                if (!array.isEmpty()) {
                    QJsonObject obj = array[0].toObject();
                    out = obj["description"].toString();
                }
            }
        }

        // If that fails, try systemctl status
        if (out.isEmpty()) {
            QString statusOutput = Cmd()
                      .getOutAsRoot("systemctl status " + name, true, true)
                      .trimmed();

            // Parse the first line to extract description after " - "
            if (!statusOutput.isEmpty()) {
                QStringList lines = statusOutput.split('\n');
                if (!lines.isEmpty()) {
                    const QString firstLine = lines.first();
                    int dashPos = firstLine.indexOf(" - ");
                    if (dashPos != -1) {
                        out = firstLine.mid(dashPos + 3).trimmed();
                    }
                }
            }
        }

        // If still empty, try to get from init file
        if (out.isEmpty()) {
            QRegularExpression regex("\nShort-Description:([^\n]*)");
            QRegularExpressionMatch match = regex.match(getInfoFromFile(name));
            if (match.captured(1).isEmpty()) {
                regex.setPattern("\nDescription:\\s*(.*)\n");
                match = regex.match(getInfoFromFile(name));
            }
            out = match.hasMatch() ? match.captured(1) : QObject::tr("Could not find service description");
        }
        return out;
    }
}

bool Service::isEnabled() const noexcept
{
    return enabled;
}

bool Service::start()
{
    if (Cmd().runAsRoot("/sbin/service " + name + " start")) {
        setRunning(true);
        return true;
    }
    return false;
}

bool Service::stop()
{
    if (Cmd().runAsRoot("/sbin/service " + name + " stop")) {
        setRunning(false);
        return true;
    }
    return false;
}

void Service::setEnabled(bool enabled) noexcept
{
    this->enabled = enabled;
}

void Service::setRunning(bool running) noexcept
{
    this->running = running;
}

QString Service::getInfoFromFile(const QString &name)
{
    // Check for the service file in standard locations
    const QStringList possiblePaths = {"/etc/init.d/" + name, "/etc/init.d/" + name + ".sh"};

    QString filePath;
    for (const auto &path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            filePath = path;
            break;
        }
    }

    if (filePath.isEmpty()) {
        qDebug() << "Could not find unit file for" << name;
        return Cmd().getOutAsRoot("/sbin/service " + name + " status", false, true);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file" << filePath;
        return Cmd().getOutAsRoot("/sbin/service " + name + " status", false, true);
    }

    QString info;
    info.reserve(1024); // Pre-allocate memory to avoid reallocations
    bool info_header = false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("### BEGIN INIT INFO")) {
            info_header = true;
            continue;
        }

        if (line.startsWith("### END INIT INFO")) {
            break; // No need to read the rest of the file
        }

        if (info_header) {
            line.remove(0, 2);
            info.append(line + '\n');
        }
    }

    return info;
}

bool Service::enable()
{
    if (initSystem == "systemd") {
        // First unmask the service if it's masked
        Cmd().runAsRoot("systemctl unmask " + name);

        if (Cmd().runAsRoot("systemctl enable " + name)) {
            setEnabled(true);
            return true;
        }
    } else {
        // For SysV init, first set defaults then enable
        Cmd().runAsRoot("/sbin/update-rc.d " + name + " defaults");

        if (Cmd().runAsRoot("/sbin/update-rc.d " + name + " enable")) {
            setEnabled(true);
            return true;
        }
    }
    return false;
}

bool Service::disable()
{
    if (initSystem == "systemd") {
        if (Cmd().runAsRoot("systemctl disable " + name)) {
            // Mask the service to prevent it from being started indirectly
            Cmd().runAsRoot("systemctl mask " + name);
            setEnabled(false);
            return true;
        }
    } else {
        if (Cmd().runAsRoot("/sbin/update-rc.d " + name + " remove")) {
            setEnabled(false);
            return true;
        }
    }
    return false;
}

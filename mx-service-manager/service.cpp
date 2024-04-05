/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023 MX Authors
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
#include <QProcess>
#include <QRegularExpression>

#include "cmd.h"

inline const QString initSystem {Service::getInit()};

Service::Service(QString name, bool running)
    : name {std::move(name)},
      running {running}
{
}

QString Service::getName() const
{
    return name;
}

QString Service::getInfo() const
{
    if (initSystem == "systemd") {
        QString info = Cmd().getOutAsRoot("/sbin/service " + name + " status").trimmed();
        if (!isEnabled()) {
            info.append("\nDescription:" + getDescription());
        }
        return info;
    } else {
        return getInfoFromFile(name);
    }
}

bool Service::isEnabled(const QString &name)
{
    if (initSystem == "systemd") {
        return (QProcess::execute("systemctl", {"-q", "is-enabled", name}) == 0);
    } else {
        return (QProcess::execute("/bin/bash",
                                  {"-c", R"([[ -e /etc/rc5.d/S*"$file_name" || -e /etc/rcS.d/S*"$file_name" ]])"})
                == 0);
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
        QString out
            = Cmd()
                  .getOut("systemctl list-units " + name + ".service -o json-pretty | grep description | cut -d: -f2",
                          true, false, true)
                  .trimmed();
        out = out.mid(1, out.length() - 2);
        if (out.isEmpty()) {
            out = Cmd()
                      .getOut("systemctl status " + name + " | awk -F' - ' 'NR == 1 { print $2 } NR > 1 { exit }'",
                              true, false, true)
                      .trimmed();
        }
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

bool Service::isEnabled() const
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

void Service::setEnabled(bool enabled)
{
    this->enabled = enabled;
}

void Service::setRunning(bool running)
{
    this->running = running;
}

QString Service::getInfoFromFile(const QString &name)
{
    QFileInfo fileInfo("/etc/init.d/" + name);
    if (!fileInfo.isFile()) {
        fileInfo.setFile("/etc/init.d/" + name + ".sh");
        if (!fileInfo.isFile()) {
            qDebug() << "Could not find unit file" << name;
            return Cmd().getOut("/sbin/service " + name + " status", false, false, true);
        }
    }
    QFile file {fileInfo.canonicalFilePath()};
    if (!file.open(QIODevice::ReadOnly)) {
        return Cmd().getOut("/sbin/service " + name + " status", false, false, true);
    }
    QString info;
    bool info_header = false;
    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        if (line.startsWith("### END INIT INFO")) {
            info_header = false;
        }
        if (info_header) {
            line.remove(0, 2);
            info.append(line + "\n");
        }
        if (line.startsWith("### BEGIN INIT INFO")) {
            info_header = true;
        }
    }
    return info;
}

bool Service::enable()
{
    if (initSystem == "systemd") {
        Cmd().runAsRoot("systemctl unmask " + name);
        if (Cmd().runAsRoot("systemctl enable " + name)) {
            setEnabled(true);
            return true;
        }
    } else {
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

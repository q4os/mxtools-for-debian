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
#include "cmd.h"

#include <QDebug>
#include <QFile>

#include <utility>

Service::Service(QString name, bool running)
    : name {std::move(name)},
      running {running}
{
}

QString Service::getName() const
{
    return name;
}

QString Service::getInfo(const QString &name)
{
    Cmd cmd;
    if (getInit() == "systemd") {
        return cmd.getCmdOut("service " + name + " status");
    } else {
        return getInfoFromFile(name);
    }
}

bool Service::isEnabled(const QString &name)
{
    Cmd cmd;
    if (getInit() == "systemd") {
        return cmd.run("systemctl -q is-enabled " + name, true);
    } else {
        return cmd.run("[ -e /etc/rc5.d/S*" + name + " ] || [ -e /etc/rcS.d/S*" + name + " ]", true);
    }
    return false;
}

QString Service::getInit()
{
    Cmd cmd;
    return cmd.getCmdOut("cat /proc/1/comm", true).trimmed();
}

bool Service::isRunning() const
{
    return running;
}

bool Service::isEnabled() const
{
    return enabled;
}

bool Service::start()
{
    if (cmd.run("service " + name + " start")) {
        setRunning(true);
        return true;
    }
    return false;
}

bool Service::stop()
{
    if (cmd.run("service " + name + " stop")) {
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
    Cmd cmd;
    QFile file("/etc/init.d/" + name);
    if (!file.exists()) {
        qDebug() << "Could not find unit file";
        return cmd.getCmdOut("service " + name + " status");
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open unit file";
        return cmd.getCmdOut("service " + name + " status");
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
    if (getInit() == "systemd") {
        cmd.run("systemctl unmask " + name);
        if (cmd.run("systemctl enable " + name)) {
            setEnabled(true);
            return true;
        }
    } else {
        cmd.run("sudo update-rc.d " + name + " defaults");
        if (cmd.run("sudo update-rc.d " + name + " enable")) {
            setEnabled(true);
            return true;
        }
    }
    return false;
}

bool Service::disable()
{
    if (getInit() == "systemd") {
        if (cmd.run("systemctl disable " + name)) {
            cmd.run("systemctl mask " + name);
            setEnabled(false);
            return true;
        }
    } else {
        if (cmd.run("sudo update-rc.d " + name + " remove")) {
            setEnabled(false);
            return true;
        }
    }
    return false;
}

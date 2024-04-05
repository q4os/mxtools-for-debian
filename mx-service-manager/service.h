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
#pragma once

#include <QObject>
#include <QString>

class Service
{
    Q_DISABLE_COPY(Service)

public:
    Service() = default;
    explicit Service(QString name, bool running);
    [[nodiscard]] QString getDescription() const;
    [[nodiscard]] QString getInfo() const;
    [[nodiscard]] QString getName() const;
    [[nodiscard]] bool isEnabled() const;
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] static QString getInit();
    [[nodiscard]] static bool isEnabled(const QString &name);
    bool disable();
    bool enable();
    bool start();
    bool stop();
    void setEnabled(bool enabled);
    void setRunning(bool running);

private:
    QString name;
    bool running {};
    bool enabled {};
    static QString getInfoFromFile(const QString &name);
};

Q_DECLARE_METATYPE(Service *)

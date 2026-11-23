/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
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

#include <atomic>

class Service
{
    Q_DISABLE_COPY(Service)

public:
    enum class RunState { Unknown, Stopped, Running };

    Service() = default;
    explicit Service(QString name, RunState state = RunState::Stopped, bool enabled = false, bool isUserService = false);
    [[nodiscard]] QString getDescription() const;
    [[nodiscard]] QString getInfo() const;
    [[nodiscard]] QString getName() const;
    [[nodiscard]] bool isEnabled() const noexcept;
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] bool hasKnownRunState() const;
    [[nodiscard]] static bool isRunningSubState(const QString &subState);
    [[nodiscard]] bool isMasked() const;
    [[nodiscard]] bool isUserService() const noexcept;
    // True after enable() failed to restore a mask state it changed along the way (e.g. the
    // unit was unmasked but the enable attempt failed and re-masking it also failed) -- the
    // unit's real state may then differ from what a plain enable() failure implies.
    [[nodiscard]] bool hadUnrestoredMaskChange() const noexcept;
    [[nodiscard]] static QString getInit();
    [[nodiscard]] static bool isEnabled(const QString &name, bool isUserService = false);
    [[nodiscard]] static bool unitEnabled(const QString &name, const QString &state, bool isUserService);
    bool disable();
    bool enable();
    bool start();
    bool stop();
    void setEnabled(bool enabled) noexcept ;
    void setRunning(bool running) noexcept;

private:
    [[nodiscard]] static QString getUnitFileState(const QString &name, bool isUserService);
    QString name;
    // Written on the GUI thread by start()/stop()/enable()/disable(), read from a
    // background thread by getInfo()/getDescription() during async detail/tooltip
    // fetches (see MainWindow::fetchSelectionInfo/fetchTooltipDescription).
    std::atomic<RunState> runState = RunState::Stopped;
    std::atomic<bool> enabled = false;
    std::atomic<bool> unrestoredMaskChange = false;
    bool userService = false;
    static QString getInfoFromFile(const QString &name);
};

Q_DECLARE_METATYPE(Service *)

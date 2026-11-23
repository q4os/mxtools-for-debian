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
#define QT_USE_QSTRINGBUILDER
#include "service.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

#include <array>

#include "cmd.h"

inline const QString initSystem {Service::getInit()};

Service::Service(QString name, RunState state, bool enabled, bool isUserService)
    : name {std::move(name)},
      runState {state},
      enabled {enabled},
      userService {isUserService}
{
}

QString Service::getName() const
{
    return initSystem == QLatin1String("systemd") ? name.chopped(8) : name;
}

QString Service::getInfo() const
{
    QString info;
    if (initSystem == QLatin1String("systemd")) {
        if (userService) {
            Cmd cmd;
            info = cmd.getOut({"systemctl", "--user", "status", "--", name}, true).trimmed();
        } else {
            Cmd cmd;
            info = cmd.getOutAsRoot({"systemctl", "status", name}, true).trimmed();
            if (cmd.elevationUnavailable()) {
                return QObject::tr("Administrator access was unavailable for this request.");
            }
        }
        if (!isEnabled() && !info.isEmpty()) {
            const QString description = getDescription();
            if (!description.isEmpty()) {
                info.append("\nDescription: " % description);
            }
        }
    } else {
        info = getInfoFromFile(name);
    }
    return info;
}

namespace {
bool sysvEnabled(const QString &name)
{
    for (const QChar level : QStringLiteral("S2345")) {
        const QDir directory(QStringLiteral("/etc/rc%1.d").arg(level));
        const auto entries = directory.entryInfoList({QStringLiteral("S[0-9][0-9]*")}, QDir::Files | QDir::CaseSensitive);
        for (const QFileInfo &entry : entries) {
            if (entry.isSymLink() && entry.fileName().mid(3) == name) {
                return true;
            }
        }
    }
    return false;
}
}

bool Service::unitEnabled(const QString &name, const QString &state, bool isUserService)
{
    if (!isUserService && state == "generated") {
        QProcess process;
        process.start("systemctl", {"show", "-p", "SourcePath", "--value", "--", name});
        if (process.waitForFinished() && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            const QFileInfo source(QString::fromUtf8(process.readAllStandardOutput()).trimmed());
            return source.absolutePath() == "/etc/init.d" && source.isFile() && sysvEnabled(source.fileName());
        }
    }
    return state == "enabled";
}

bool Service::isEnabled(const QString &name, bool isUserService)
{
    return initSystem == "systemd" ? unitEnabled(name, getUnitFileState(name, isUserService), isUserService)
                                   : sysvEnabled(name);
}

QString Service::getUnitFileState(const QString &name, bool isUserService)
{
    QStringList args = {QLatin1String("is-enabled"), QLatin1String("--"), name};
    if (isUserService) [[unlikely]] {
        args.insert(0, QLatin1String("--user"));
    }
    QProcess process;
    process.start(QLatin1String("systemctl"), args);
    process.waitForFinished();
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

QString Service::getInit()
{
    QFile file(QStringLiteral("/proc/1/comm"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

bool Service::isMasked() const
{
    if (initSystem != "systemd") {
        return false;
    }
    const QString state = getUnitFileState(name, userService);
    return state == "masked" || state == "masked-runtime";
}

bool Service::isRunningSubState(const QString &subState)
{
    return subState == "running";
}

bool Service::isRunning() const
{
    return runState == RunState::Running;
}

bool Service::hasKnownRunState() const
{
    return runState != RunState::Unknown;
}

QString Service::getDescription() const
{
    static const QRegularExpression shortDescRegex("\nShort-Description:([^\n]*)");
    static const QRegularExpression descRegex("\nDescription:\\s*(.*)\n");

    if (initSystem != QLatin1String("systemd")) {
        QString info = getInfo();
        QRegularExpressionMatch match = shortDescRegex.match(info);
        if (match.captured(1).isEmpty()) {
            match = descRegex.match(info);
        }
        if (match.hasMatch()) {
            return match.captured(1);
        }
        return QStringLiteral("");
    } else {
        // Prefer systemctl show which works for inactive/disabled units
        Cmd cmd;
        QString out = userService
                          ? cmd.getOut({"systemctl", "--user", "show", "-p", "Description", "--value", "--", name}, true, true)
                          : cmd.getOutAsRoot({"systemctl", "show", "-p", "Description", "--value", name}, true, true);
        if (cmd.elevationUnavailable()) {
            return {};
        }
        out = out.trimmed();

        // Try list-units (covers active units; empty for inactive)
        if (out.isEmpty()) {
            const QString jsonOutput = userService
                                           ? cmd.getOut({"systemctl", "--user", "list-units", "-o", "json", "--", name}, true, true).trimmed()
                                           : cmd.getOutAsRoot({"systemctl", "list-units", name, "-o", "json"}, true, true).trimmed();

            if (cmd.elevationUnavailable()) {
                return {};
            }
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
        }

        // If that fails, try systemctl status
        if (out.isEmpty()) {
            const QString statusOutput = userService
                                             ? cmd.getOut({"systemctl", "--user", "status", "--", name}, true, true).trimmed()
                                             : cmd.getOutAsRoot({"systemctl", "status", name}, true, true).trimmed();

            if (cmd.elevationUnavailable()) {
                return {};
            }
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
            QRegularExpressionMatch match = shortDescRegex.match(getInfoFromFile(getName()));
            if (match.captured(1).isEmpty()) {
                match = descRegex.match(getInfoFromFile(getName()));
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

bool Service::isUserService() const noexcept
{
    return userService;
}

bool Service::start()
{
    if (initSystem == QLatin1String("systemd")) {
        Cmd cmd;
        if (userService ? cmd.run({"systemctl", "--user", "start", "--", name}) : Cmd().runAsRoot({"systemctl", "start", name})) {
            QStringList query {"systemctl", "show", "-p", "SubState", "--value", "--", name};
            if (userService) {
                query.insert(1, "--user");
            }
            const QString subState = cmd.getOut(query, true).trimmed();
            if (!subState.isEmpty()) {
                setRunning(isRunningSubState(subState));
            }
            return true;
        }
    } else {
        if (Cmd().runAsRoot({"service", name, "start"})) {
            setRunning(true);
            return true;
        }
    }
    return false;
}

bool Service::stop()
{
    if (initSystem == QLatin1String("systemd")) {
        Cmd cmd;
        if (userService ? cmd.run({"systemctl", "--user", "stop", "--", name}) : Cmd().runAsRoot({"systemctl", "stop", name})) {
            setRunning(false);
            return true;
        }
    } else {
        if (Cmd().runAsRoot({"service", name, "stop"})) {
            setRunning(false);
            return true;
        }
    }
    return false;
}

void Service::setEnabled(bool enabled) noexcept
{
    this->enabled = enabled;
}

void Service::setRunning(bool running) noexcept
{
    runState = running ? RunState::Running : RunState::Stopped;
}

QString Service::getInfoFromFile(const QString &name)
{
    // Check for the service file in standard locations
    const std::array possiblePaths = {QString("/etc/init.d/" % name), QString("/etc/init.d/" % name % ".sh")};

    QString filePath;
    for (const auto &path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            filePath = path;
            break;
        }
    }

    if (filePath.isEmpty()) {
        qDebug() << "Could not find unit file for" << name;
        return initSystem == QLatin1String("systemd") ? QString()
                                                      : Cmd().getOutAsRoot({"systemctl", "status", name}, false, true);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file" << filePath;
        return initSystem == QLatin1String("systemd") ? QString()
                                                      : Cmd().getOutAsRoot({"systemctl", "status", name}, false, true);
    }

    QString info;
    info.reserve(1024); // Pre-allocate memory to avoid reallocations
    bool info_header = false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith(QLatin1String("### BEGIN INIT INFO"))) {
            info_header = true;
            continue;
        }

        if (line.startsWith(QLatin1String("### END INIT INFO"))) {
            break; // No need to read the rest of the file
        }

        if (info_header) {
            line.remove(0, 2);
            info.append(line % '\n');
        }
    }

    return info;
}

bool Service::hadUnrestoredMaskChange() const noexcept
{
    return unrestoredMaskChange;
}

bool Service::enable()
{
    unrestoredMaskChange = false;
    if (initSystem == QLatin1String("systemd")) {
        Cmd cmd;
        // systemd reports two distinct mask variants via is-enabled: "masked" (persistent,
        // /etc symlink) and "masked-runtime" (temporary, /run symlink, cleared on reboot).
        // Both must be unmasked before enable can succeed, and rollback must restore the
        // same variant rather than always re-applying a persistent mask.
        const QString initialMaskState = getUnitFileState(name, userService);
        const bool wasRuntimeMasked = initialMaskState == QLatin1String("masked-runtime");
        const bool wasMasked = wasRuntimeMasked || initialMaskState == QLatin1String("masked");
        if (wasMasked) {
            (userService ? cmd.run({"systemctl", "--user", "unmask", "--", name}) : cmd.runAsRoot({"systemctl", "unmask", name}));
            if (cmd.elevationUnavailable()) {
                return false;
            }
        }

        (userService ? cmd.run({"systemctl", "--user", "enable", "--", name}) : Cmd().runAsRoot({"systemctl", "enable", name}));

        // Base the rollback decision on the verified unit-file state, not the enable
        // command's raw exit code: systemctl can exit 0 without the unit ending up
        // "enabled" (or the reverse), so trust reality over the return code.
        if (wasMasked && !isEnabled(name, userService)) {
            // Restore the previous masked state rather than leaving the unit unmasked
            // while still reporting that nothing was applied. Verify the rollback itself
            // actually landed -- it can fail too (e.g. a cancelled auth prompt).
            QStringList maskUserCmd {"systemctl", "--user", "mask"};
            if (wasRuntimeMasked) {
                maskUserCmd.append("--runtime");
            }
            maskUserCmd.append({"--", name});
            const bool remaskOk = (userService
                                        ? cmd.run(maskUserCmd)
                                        : Cmd().runAsRoot(wasRuntimeMasked
                                                               ? QStringList{"systemctl", "mask", "--runtime", name}
                                                               : QStringList{"systemctl", "mask", name}));
            const QString expectedMaskState = wasRuntimeMasked ? QLatin1String("masked-runtime") : QLatin1String("masked");
            if (!remaskOk || getUnitFileState(name, userService) != expectedMaskState) {
                unrestoredMaskChange = true;
                qWarning().noquote() << "Could not restore masked state for" << name
                                     << "after a failed enable attempt -- it may now be unmasked"
                                        " even though enabling it did not succeed";
            }
        }
    } else {
        // For SysV init, first set defaults then enable. `defaults` alone can already
        // enable the service (it creates the runlevel symlinks), so a failure of the
        // later `enable` step doesn't necessarily mean nothing was applied.
        Cmd cmd;
        cmd.runAsRoot({"update-rc.d", name, "defaults"});
        if (cmd.elevationUnavailable()) {
            return false;
        }

        Cmd().runAsRoot({"update-rc.d", name, "enable"});
    }
    // Re-check the real state instead of trusting the last sub-command's exit code: a
    // preceding step (unmask, or SysV defaults) can succeed and change the effective
    // enabled state even when the final command fails.
    const bool nowEnabled = isEnabled(name, userService);
    setEnabled(nowEnabled);
    return nowEnabled;
}

bool Service::disable()
{
    if (initSystem == QLatin1String("systemd")) {
        Cmd cmd;
        if (userService ? cmd.run({"systemctl", "--user", "disable", "--", name}) : Cmd().runAsRoot({"systemctl", "disable", name})) {
            const bool maskOk = userService ? cmd.run({"systemctl", "--user", "mask", "--", name})
                                            : Cmd().runAsRoot({"systemctl", "mask", name});
            const QString state = getUnitFileState(name, userService);
            if (!state.isEmpty()) {
                setEnabled(unitEnabled(name, state, userService));
            }
            return maskOk && state == "masked";
        }
    } else {
        Cmd().runAsRoot({"update-rc.d", name, "disable"});
        const bool nowEnabled = isEnabled(name);
        setEnabled(nowEnabled);
        return !nowEnabled;
    }
    return false;
}

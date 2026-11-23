/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2026 MX Authors
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

#include <QCoreApplication>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>

#include "serviceunit.h"

#include <cstdio>

namespace
{

struct ProcessResult
{
    bool started = false;
    int exitCode = 1;
    QProcess::ExitStatus exitStatus = QProcess::NormalExit;
    QByteArray standardOutput;
    QByteArray standardError;
};

void writeAndFlush(FILE *stream, const QByteArray &data)
{
    if (!data.isEmpty()) {
        std::fwrite(data.constData(), 1, static_cast<size_t>(data.size()), stream);
        std::fflush(stream);
    }
}

void printError(const QString &message)
{
    writeAndFlush(stderr, message.toUtf8() + '\n');
}

[[nodiscard]] const QHash<QString, QStringList> &allowedCommands()
{
    static const QHash<QString, QStringList> commands {
        {QStringLiteral("service"),
         {QStringLiteral("/usr/sbin/service"), QStringLiteral("/sbin/service"),
          QStringLiteral("/usr/bin/service")}},
        {QStringLiteral("systemctl"),
         {QStringLiteral("/usr/bin/systemctl"), QStringLiteral("/bin/systemctl")}},
        {QStringLiteral("true"),
         {QStringLiteral("/usr/bin/true"), QStringLiteral("/bin/true")}},
        {QStringLiteral("update-rc.d"),
         {QStringLiteral("/usr/sbin/update-rc.d"), QStringLiteral("/sbin/update-rc.d")}},
    };
    return commands;
}

[[nodiscard]] QString resolveBinary(const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isExecutable()) {
            return candidate;
        }
    }
    return {};
}

[[nodiscard]] bool validateArguments(const QString &command, QStringList &args)
{
    static const QRegularExpression scriptName(
        QRegularExpression::anchoredPattern(QStringLiteral(R"([A-Za-z0-9_][A-Za-z0-9_.@:+-]*)")));
    if (command == "true") {
        return args.isEmpty();
    }
    if (command == "service") {
        return args == QStringList{"--status-all"}
               || (args.size() == 2 && scriptName.match(args[0]).hasMatch()
                   && (args[1] == "start" || args[1] == "stop"));
    }
    if (command == "update-rc.d") {
        return args.size() == 2 && scriptName.match(args[0]).hasMatch()
               && (args[1] == "defaults" || args[1] == "enable" || args[1] == "disable");
    }
    if (command != "systemctl" || args.isEmpty()) {
        return false;
    }

    static const QStringList actions {"start", "stop", "status", "enable", "disable", "mask", "unmask"};
    qsizetype unitIndex;
    if (args.size() == 2 && actions.contains(args[0])) {
        unitIndex = 1;
    } else if (args.size() == 3 && args[0] == "mask" && args[1] == "--runtime") {
        unitIndex = 2;
    } else if (args.size() == 5 && args.mid(0, 4) == QStringList{"show", "-p", "Description", "--value"}) {
        unitIndex = 4;
    } else if (args.size() == 4 && args[0] == "list-units" && args[2] == "-o" && args[3] == "json") {
        unitIndex = 1;
    } else {
        return false;
    }

    QString unit = args[unitIndex];
    if (!unit.endsWith(".service")) {
        if (!scriptName.match(unit).hasMatch()) {
            return false;
        }
        unit += ".service";
    }
    if (!isServiceUnit(unit)) {
        return false;
    }
    args.removeAt(unitIndex);
    args.append({"--", unit});
    return true;
}

[[nodiscard]] ProcessResult runProcess(const QString &program, const QStringList &args)
{
    ProcessResult result;

    QProcess process;
    process.start(program, args, QIODevice::ReadWrite);
    if (!process.waitForStarted()) {
        result.standardError = QStringLiteral("Failed to start %1").arg(program).toUtf8();
        result.exitCode = 127;
        return result;
    }

    constexpr int kTimeoutMs = 120000;

    result.started = true;
    process.closeWriteChannel();
    if (!process.waitForFinished(kTimeoutMs)) {
        process.kill();
        process.waitForFinished(5000);
        result.standardError = QStringLiteral("Command timed out after %1ms: %2")
                                   .arg(kTimeoutMs)
                                   .arg(program)
                                   .toUtf8();
        result.exitCode = 124;
        return result;
    }

    result.exitStatus = process.exitStatus();
    result.exitCode = process.exitCode();
    result.standardOutput = process.readAllStandardOutput();
    result.standardError = process.readAllStandardError();
    return result;
}

[[nodiscard]] int relayResult(const ProcessResult &result)
{
    writeAndFlush(stdout, result.standardOutput);
    writeAndFlush(stderr, result.standardError);
    if (!result.started) {
        return result.exitCode;
    }
    return result.exitStatus == QProcess::NormalExit ? result.exitCode : 1;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments().mid(1);

    if (arguments.isEmpty()) {
        printError(QStringLiteral("Missing command"));
        return 1;
    }

    const QString command = arguments.takeFirst();

    const auto it = allowedCommands().constFind(command);
    if (it == allowedCommands().constEnd()) {
        printError(QStringLiteral("Command not allowed: %1").arg(command));
        return 127;
    }

    if (!validateArguments(command, arguments)) {
        printError(QStringLiteral("Invalid arguments for %1").arg(command));
        return 1;
    }

    const QString binary = resolveBinary(it.value());
    if (binary.isEmpty()) {
        printError(QStringLiteral("Command not available: %1").arg(command));
        return 127;
    }

    return relayResult(runProcess(binary, arguments));
}

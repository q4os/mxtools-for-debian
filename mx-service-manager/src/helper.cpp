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

// Allows flags (starting with -), service names, and option values.
// Covers: systemctl actions, service names (letters/digits/._@:-), flag values like
// "Description", "json", unit names like "ssh.service", "user@1000.service".
[[nodiscard]] bool isValidArg(const QString &arg)
{
    static const QRegularExpression safeRx(
        QRegularExpression::anchoredPattern(QStringLiteral(R"([-A-Za-z0-9_.@:/=]+)")));
    return !arg.isEmpty() && safeRx.match(arg).hasMatch();
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

    result.started = true;
    process.closeWriteChannel();
    process.waitForFinished(-1);

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

    for (const QString &arg : std::as_const(arguments)) {
        if (!isValidArg(arg)) {
            printError(QStringLiteral("Invalid argument: %1").arg(arg));
            return 1;
        }
    }

    const QString binary = resolveBinary(it.value());
    if (binary.isEmpty()) {
        printError(QStringLiteral("Command not available: %1").arg(command));
        return 127;
    }

    return relayResult(runProcess(binary, arguments));
}

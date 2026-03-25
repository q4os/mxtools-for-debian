/**********************************************************************
 *  helper.cpp
 **********************************************************************
 * Copyright (C) 2026 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *          OpenAI Codex
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
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QSet>

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

[[nodiscard]] QByteArray readHelperInput()
{
    QFile input;
    if (!input.open(stdin, QIODevice::ReadOnly)) {
        return {};
    }
    return input.readAll();
}

[[nodiscard]] const QHash<QString, QStringList> &allowedCommands()
{
    static const QHash<QString, QStringList> commands {
        {"apt-get", {"/usr/bin/apt-get"}},
        {"depmod", {"/usr/sbin/depmod", "/sbin/depmod", "/usr/bin/depmod"}},
        {"find", {"/usr/bin/find", "/bin/find"}},
        {"ip", {"/usr/sbin/ip", "/sbin/ip", "/usr/bin/ip", "/bin/ip"}},
        {"modprobe", {"/usr/sbin/modprobe", "/sbin/modprobe", "/usr/bin/modprobe"}},
        {"ndiswrapper", {"/usr/sbin/ndiswrapper", "/sbin/ndiswrapper", "/usr/bin/ndiswrapper"}},
        {"pkill", {"/usr/bin/pkill", "/bin/pkill"}},
        {"rfkill", {"/usr/sbin/rfkill", "/sbin/rfkill", "/usr/bin/rfkill"}},
        {"service", {"/usr/sbin/service", "/sbin/service", "/usr/bin/service", "/bin/service"}},
    };
    return commands;
}

[[nodiscard]] const QSet<QString> &allowedWritePaths()
{
    static const QSet<QString> paths {
        "/etc/modprobe.d/blacklist.conf",
        "/etc/modprobe.d/broadcom-sta-dkms.conf",
        "/etc/modules",
    };
    return paths;
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

[[nodiscard]] ProcessResult runProcess(const QString &program, const QStringList &args, const QByteArray &input = {})
{
    ProcessResult result;

    QProcess process;
    process.start(program, args, QIODevice::ReadWrite);
    if (!process.waitForStarted()) {
        result.standardError = QString("Failed to start %1").arg(program).toUtf8();
        result.exitCode = 127;
        return result;
    }

    result.started = true;
    if (!input.isEmpty()) {
        process.write(input);
    }
    process.closeWriteChannel();
    process.waitForFinished(-1);

    result.exitCode = process.exitCode();
    result.exitStatus = process.exitStatus();
    result.standardOutput = process.readAllStandardOutput();
    result.standardError = process.readAllStandardError();
    return result;
}

[[nodiscard]] int relayResult(const ProcessResult &result, bool relayStandardError = true)
{
    writeAndFlush(stdout, result.standardOutput);
    if (relayStandardError) {
        writeAndFlush(stderr, result.standardError);
    }
    if (!result.started) {
        return result.exitCode;
    }
    return result.exitStatus == QProcess::NormalExit ? result.exitCode : 1;
}

[[nodiscard]] bool isAllowedWritePath(const QString &path)
{
    return allowedWritePaths().contains(path);
}

[[nodiscard]] int handleExec(const QStringList &args)
{
    QStringList execArgs = args;
    bool quietStderr = false;
    if (!execArgs.isEmpty() && execArgs.constFirst() == QLatin1String("--quiet-stderr")) {
        quietStderr = true;
        execArgs.removeFirst();
    }

    if (execArgs.isEmpty()) {
        printError(QStringLiteral("exec requires a command name"));
        return 1;
    }

    const QString command = execArgs.constFirst();
    const auto commandIt = allowedCommands().constFind(command);
    if (commandIt == allowedCommands().constEnd()) {
        printError(QString("Command is not allowed: %1").arg(command));
        return 127;
    }

    const QString program = resolveBinary(commandIt.value());
    if (program.isEmpty()) {
        printError(QString("Command is not available: %1").arg(command));
        return 127;
    }

    return relayResult(runProcess(program, execArgs.mid(1), readHelperInput()), !quietStderr);
}

[[nodiscard]] int handleAppendLine(const QStringList &args)
{
    if (args.size() != 2) {
        printError(QStringLiteral("append-line requires a path and line"));
        return 1;
    }

    const QString &path = args.at(0);
    if (!isAllowedWritePath(path)) {
        printError(QString("append-line path is not allowed: %1").arg(path));
        return 1;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        printError(QString("Unable to open %1").arg(path));
        return 1;
    }

    QByteArray toAppend;
    if (file.size() > 0) {
        file.seek(file.size() - 1);
        if (file.read(1) != "\n") {
            toAppend.append('\n');
        }
    }
    file.seek(file.size());
    toAppend.append(args.at(1).toUtf8());
    toAppend.append('\n');
    if (file.write(toAppend) != toAppend.size()) {
        printError(QString("Unable to append to %1").arg(path));
        return 1;
    }

    return 0;
}

[[nodiscard]] int handleWriteFileFrom(const QStringList &args)
{
    if (args.size() != 2) {
        printError(QStringLiteral("write-file-from requires destination and source paths"));
        return 1;
    }

    const QString &destinationPath = args.at(0);
    const QString &sourcePath = args.at(1);
    if (!isAllowedWritePath(destinationPath)) {
        printError(QString("write-file-from destination path is not allowed: %1").arg(destinationPath));
        return 1;
    }

    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        printError(QString("Unable to read staged file %1").arg(sourcePath));
        return 1;
    }

    QFile destinationFile(destinationPath);
    if (!destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        printError(QString("Unable to write %1").arg(destinationPath));
        return 1;
    }

    const QByteArray content = sourceFile.readAll();
    if (!content.isEmpty() && destinationFile.write(content) != content.size()) {
        printError(QString("Unable to write %1").arg(destinationPath));
        return 1;
    }

    return 0;
}
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments().mid(1);
    if (args.isEmpty()) {
        printError(QStringLiteral("Missing helper action"));
        return 1;
    }

    const QString action = args.constFirst();
    const QStringList actionArgs = args.mid(1);

    if (action == QLatin1String("append-line")) {
        return handleAppendLine(actionArgs);
    }
    if (action == QLatin1String("exec")) {
        return handleExec(actionArgs);
    }
    if (action == QLatin1String("write-file-from")) {
        return handleWriteFileFrom(actionArgs);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

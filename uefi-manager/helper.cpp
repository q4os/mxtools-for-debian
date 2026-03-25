/**********************************************************************
 *  helper.cpp
 **********************************************************************
 * Copyright (C) 2026 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *          OpenAI Codex
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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
constexpr auto UEFI_MANAGER_LIB = "/usr/lib/uefi-manager/uefimanager-lib";

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
        {"blkid", {"/usr/sbin/blkid", "/sbin/blkid", "/usr/bin/blkid", "/bin/blkid"}},
        {"cp", {"/usr/bin/cp", "/bin/cp"}},
        {"cryptsetup", {"/usr/sbin/cryptsetup", "/sbin/cryptsetup", "/usr/bin/cryptsetup", "/bin/cryptsetup"}},
        {"efibootmgr", {"/usr/sbin/efibootmgr", "/sbin/efibootmgr", "/usr/bin/efibootmgr", "/bin/efibootmgr"}},
        {"findmnt", {"/usr/bin/findmnt", "/bin/findmnt"}},
        {"grep", {"/usr/bin/grep", "/bin/grep"}},
        {"lsblk", {"/usr/bin/lsblk", "/bin/lsblk"}},
        {"mkdir", {"/usr/bin/mkdir", "/bin/mkdir"}},
        {"mount", {"/usr/bin/mount", "/bin/mount"}},
        {"mountpoint", {"/usr/bin/mountpoint", "/bin/mountpoint"}},
        {"rm", {"/usr/bin/rm", "/bin/rm"}},
        {"sfdisk", {"/usr/sbin/sfdisk", "/sbin/sfdisk", "/usr/bin/sfdisk", "/bin/sfdisk"}},
    };
    return commands;
}

[[nodiscard]] const QSet<QString> &allowedLibSubcommands()
{
    static const QSet<QString> subcommands {
        QStringLiteral("cleanup_temp"),
        QStringLiteral("copy_log"),
        QStringLiteral("write_checkfile"),
    };
    return subcommands;
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

[[nodiscard]] int runAllowedCommand(const QString &command, const QStringList &args, const QByteArray &input = {})
{
    const auto commandIt = allowedCommands().constFind(command);
    if (commandIt == allowedCommands().constEnd()) {
        printError(QString("Command is not allowed: %1").arg(command));
        return 1;
    }

    const QString program = resolveBinary(commandIt.value());
    if (program.isEmpty()) {
        printError(QString("Command is not available: %1").arg(command));
        return 127;
    }

    return relayResult(runProcess(program, args, input));
}

[[nodiscard]] int handleExec(const QStringList &args)
{
    if (args.isEmpty()) {
        printError(QStringLiteral("exec requires a command name"));
        return 1;
    }
    return runAllowedCommand(args.constFirst(), args.mid(1), readHelperInput());
}

[[nodiscard]] int handleLib(const QStringList &args)
{
    if (args.isEmpty()) {
        printError(QStringLiteral("lib requires a subcommand"));
        return 1;
    }

    const QString subcommand = args.constFirst();
    if (!allowedLibSubcommands().contains(subcommand)) {
        printError(QString("lib subcommand is not allowed: %1").arg(subcommand));
        return 1;
    }

    const QFileInfo info(QString::fromUtf8(UEFI_MANAGER_LIB));
    if (!info.exists() || !info.isExecutable()) {
        printError(QStringLiteral("uefimanager-lib is not available"));
        return 127;
    }

    return relayResult(runProcess(info.absoluteFilePath(), args, readHelperInput()));
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
    const QStringList remainingArgs = args.mid(1);

    if (action == QLatin1String("exec")) {
        return handleExec(remainingArgs);
    }
    if (action == QLatin1String("lib")) {
        return handleLib(remainingArgs);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

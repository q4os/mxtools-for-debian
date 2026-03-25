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
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>

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
        {"addgroup", {"/usr/sbin/addgroup", "/sbin/addgroup", "/usr/bin/addgroup"}},
        {"adduser", {"/usr/sbin/adduser", "/sbin/adduser", "/usr/bin/adduser"}},
        {"chown", {"/usr/bin/chown", "/bin/chown"}},
        {"delgroup", {"/usr/sbin/delgroup", "/sbin/delgroup", "/usr/bin/delgroup"}},
        {"deluser", {"/usr/sbin/deluser", "/sbin/deluser", "/usr/bin/deluser"}},
        {"find", {"/usr/bin/find", "/bin/find"}},
        {"gpasswd", {"/usr/bin/gpasswd", "/sbin/gpasswd", "/bin/gpasswd"}},
        {"groupadd", {"/usr/sbin/groupadd", "/sbin/groupadd", "/usr/bin/groupadd"}},
        {"groupdel", {"/usr/sbin/groupdel", "/sbin/groupdel", "/usr/bin/groupdel"}},
        {"groupmod", {"/usr/sbin/groupmod", "/sbin/groupmod", "/usr/bin/groupmod"}},
        {"killall", {"/usr/bin/killall", "/bin/killall"}},
        {"passwd", {"/usr/bin/passwd", "/sbin/passwd", "/bin/passwd"}},
        {"rm", {"/usr/bin/rm", "/bin/rm"}},
        {"rsync", {"/usr/bin/rsync"}},
        {"sed", {"/usr/bin/sed", "/bin/sed"}},
        {"timeout", {"/usr/bin/timeout", "/bin/timeout"}},
        {"useradd", {"/usr/sbin/useradd", "/sbin/useradd", "/usr/bin/useradd"}},
        {"userdel", {"/usr/sbin/userdel", "/sbin/userdel", "/usr/bin/userdel"}},
        {"usermod", {"/usr/sbin/usermod", "/sbin/usermod", "/usr/bin/usermod"}},
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
        return 127;
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

[[nodiscard]] int handlePasswd(const QStringList &args)
{
    if (args.size() != 1) {
        printError(QStringLiteral("passwd requires exactly one username"));
        return 1;
    }

    static const QRegularExpression userNameRx(
        QRegularExpression::anchoredPattern(QStringLiteral(R"([A-Za-z_][A-Za-z0-9_-]*[$]?)")));
    const QString userName = args.constFirst();
    if (!userNameRx.match(userName).hasMatch()) {
        printError(QString("Invalid username: %1").arg(userName));
        return 1;
    }

    return runAllowedCommand(QStringLiteral("passwd"), {userName}, readHelperInput());
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
    if (action == QLatin1String("passwd")) {
        return handlePasswd(remainingArgs);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

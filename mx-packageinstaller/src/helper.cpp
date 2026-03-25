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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSocketNotifier>
#include <QRegularExpression>
#include <QSet>
#include <QtXml/QDomDocument>

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

constexpr auto TempSourceListPath = "/etc/apt/sources.list.d/mxpitemp.list";
constexpr auto PkgListDirPath = "/usr/share/mx-packageinstaller-pkglist";

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
        {"apt-get", {"/usr/bin/apt-get"}},
        {"apt-mark", {"/usr/bin/apt-mark"}},
        {"aptitude", {"/usr/bin/aptitude"}},
        {"chown", {"/usr/bin/chown", "/bin/chown"}},
        {"fuser", {"/usr/bin/fuser", "/bin/fuser"}},
        {"mxpi-lib", {"/usr/lib/mx-packageinstaller/mxpi-lib"}},
        {"ps", {"/usr/bin/ps", "/bin/ps"}},
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

[[nodiscard]] ProcessResult runProcess(const QString &program, const QStringList &args,
                                       const QHash<QString, QString> &environment = {})
{
    ProcessResult result;

    QProcess process;
    auto env = QProcessEnvironment::systemEnvironment();
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        env.insert(it.key(), it.value());
    }
    process.setProcessEnvironment(env);
    process.start(program, args, QIODevice::ReadWrite);
    if (!process.waitForStarted()) {
        result.standardError = QString("Failed to start %1").arg(program).toUtf8();
        result.exitCode = 127;
        return result;
    }

    result.started = true;

    QFile stdinFile;
    stdinFile.open(stdin, QIODevice::ReadOnly | QIODevice::Unbuffered);

    QSocketNotifier stdinNotifier(stdinFile.handle(), QSocketNotifier::Read);
    QObject::connect(&stdinNotifier, &QSocketNotifier::activated, [&](QSocketDescriptor) {
        const QByteArray data = stdinFile.read(4096);
        if (data.isEmpty()) {
            stdinNotifier.setEnabled(false);
            process.closeWriteChannel();
        } else {
            process.write(data);
        }
    });

    while (process.state() != QProcess::NotRunning) {
        process.waitForFinished(50);
        QCoreApplication::processEvents();

        const QByteArray stdoutChunk = process.readAllStandardOutput();
        if (!stdoutChunk.isEmpty()) {
            result.standardOutput += stdoutChunk;
            writeAndFlush(stdout, stdoutChunk);
        }

        const QByteArray stderrChunk = process.readAllStandardError();
        if (!stderrChunk.isEmpty()) {
            result.standardError += stderrChunk;
            writeAndFlush(stderr, stderrChunk);
        }
    }

    result.exitStatus = process.exitStatus();
    result.exitCode = process.exitCode();
    return result;
}

[[nodiscard]] int relayResult(const ProcessResult &result)
{
    if (!result.started) {
        return result.exitCode;
    }
    return result.exitStatus == QProcess::NormalExit ? result.exitCode : 1;
}

[[nodiscard]] bool isAllowedEnvironment(const QString &name)
{
    static const QSet<QString> allowedNames {QStringLiteral("DEBIAN_FRONTEND")};
    return allowedNames.contains(name);
}

[[nodiscard]] int runAllowedCommand(const QString &command, const QStringList &commandArgs,
                                    const QHash<QString, QString> &environment = {})
{
    const auto commandIt = allowedCommands().constFind(command);
    if (commandIt == allowedCommands().constEnd()) {
        printError(QString("Command is not allowed: %1").arg(command));
        return 127;
    }

    const QString resolvedCommand = resolveBinary(commandIt.value());
    if (resolvedCommand.isEmpty()) {
        printError(QString("Command is not available: %1").arg(command));
        return 127;
    }

    return relayResult(runProcess(resolvedCommand, commandArgs, environment));
}

[[nodiscard]] int handleExec(const QStringList &args)
{
    QStringList remainingArgs = args;
    QHash<QString, QString> environment;

    while (remainingArgs.size() >= 2 && remainingArgs.constFirst() == QLatin1String("--env")) {
        const QString assignment = remainingArgs.at(1);
        const int separatorIndex = assignment.indexOf('=');
        if (separatorIndex <= 0) {
            printError(QString("Invalid environment assignment: %1").arg(assignment));
            return 1;
        }

        const QString name = assignment.left(separatorIndex);
        const QString value = assignment.mid(separatorIndex + 1);
        if (!isAllowedEnvironment(name)) {
            printError(QString("Environment variable is not allowed: %1").arg(name));
            return 1;
        }
        environment.insert(name, value);
        remainingArgs = remainingArgs.mid(2);
    }

    if (remainingArgs.isEmpty()) {
        printError(QStringLiteral("exec requires a command name"));
        return 1;
    }

    return runAllowedCommand(remainingArgs.constFirst(), remainingArgs.mid(1), environment);
}

[[nodiscard]] int handleLockingProcess(const QStringList &args)
{
    if (args.size() != 1) {
        printError(QStringLiteral("locking-process requires exactly one path"));
        return 1;
    }

    const QString path = args.constFirst();
    if (!QFileInfo::exists(path)) {
        return 0;
    }

    const QString fuserBinary = resolveBinary(allowedCommands().value(QStringLiteral("fuser")));
    const QString psBinary = resolveBinary(allowedCommands().value(QStringLiteral("ps")));
    if (fuserBinary.isEmpty() || psBinary.isEmpty()) {
        printError(QStringLiteral("Required helper command is not available"));
        return 127;
    }

    const ProcessResult fuserResult = runProcess(fuserBinary, {path});
    const QString fuserOutput = QString::fromUtf8(fuserResult.standardOutput + fuserResult.standardError);
    const QRegularExpression pidRegex(QStringLiteral(R"((\d+))"));
    const QRegularExpressionMatch match = pidRegex.match(fuserOutput);
    if (!match.hasMatch()) {
        return 0;
    }

    const QString pid = match.captured(1);
    const ProcessResult psResult = runProcess(psBinary, {"--no-headers", "-o", "comm=", "-p", pid});
    if (!psResult.started || psResult.exitStatus != QProcess::NormalExit) {
        return 1;
    }

    writeAndFlush(stdout, psResult.standardOutput.trimmed());
    return 0;
}

[[nodiscard]] int handleWriteFile(const QStringList &args)
{
    if (args.size() != 2) {
        printError(QStringLiteral("write-file requires path and content"));
        return 1;
    }

    const QString path = args.at(0);
    if (path != QLatin1String(TempSourceListPath)) {
        printError(QString("write-file path is not allowed: %1").arg(path));
        return 1;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        printError(QString("Unable to write %1").arg(path));
        return 1;
    }

    file.write(args.at(1).toUtf8());
    file.close();
    return 0;
}

[[nodiscard]] QSet<QString> loadKnownHooks()
{
    QSet<QString> hooks;
    const QDir pkgListDir(QString::fromLatin1(PkgListDirPath));
    const QStringList pmFiles = pkgListDir.entryList({"*.pm"}, QDir::Files);
    static const QStringList hookTags {QStringLiteral("preinstall"), QStringLiteral("postinstall"),
                                       QStringLiteral("preuninstall"), QStringLiteral("postuninstall")};

    for (const QString &fileName : pmFiles) {
        QFile file(pkgListDir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QDomDocument doc;
        if (!doc.setContent(&file)) {
            continue;
        }

        const QDomElement root = doc.firstChildElement(QStringLiteral("app"));
        for (const QString &tagName : hookTags) {
            for (QDomElement element = root.firstChildElement(tagName); !element.isNull();
                 element = element.nextSiblingElement(tagName)) {
                const QString script = element.text().trimmed();
                if (!script.isEmpty()) {
                    hooks.insert(script);
                }
            }
        }
    }

    return hooks;
}

[[nodiscard]] int handleRunHook(const QStringList &args)
{
    if (args.size() != 1) {
        printError(QStringLiteral("run-hook requires exactly one script"));
        return 1;
    }

    const QString script = args.constFirst().trimmed();
    if (script.isEmpty()) {
        return 0;
    }

    static const QSet<QString> knownHooks = loadKnownHooks();
    if (!knownHooks.contains(script)) {
        printError(QStringLiteral("Hook is not recognized from installed package metadata"));
        return 1;
    }

    return relayResult(runProcess(QStringLiteral("/bin/bash"), {"-c", script}));
}
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QString markerPath = qEnvironmentVariable("MX_PKG_HELPER_MARKER");
    if (!markerPath.isEmpty()) {
        QFile markerFile(markerPath);
        if (markerFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            markerFile.close();
        }
    }
    const QStringList arguments = app.arguments().mid(1);
    if (arguments.isEmpty()) {
        printError(QStringLiteral("Missing helper action"));
        return 1;
    }

    QStringList remainingArgs = arguments;
    const QString action = remainingArgs.takeFirst();

    if (action == QLatin1String("exec")) {
        return handleExec(remainingArgs);
    }
    if (action == QLatin1String("locking-process")) {
        return handleLockingProcess(remainingArgs);
    }
    if (action == QLatin1String("run-hook")) {
        return handleRunHook(remainingArgs);
    }
    if (action == QLatin1String("write-file")) {
        return handleWriteFile(remainingArgs);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

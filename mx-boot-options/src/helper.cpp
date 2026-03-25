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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QThread>

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
        {"apt-get", {"/usr/bin/apt-get"}},
        {"chmod", {"/usr/bin/chmod", "/bin/chmod"}},
        {"chown", {"/usr/bin/chown", "/bin/chown"}},
        {"cp", {"/usr/bin/cp", "/bin/cp"}},
        {"cryptsetup", {"/usr/sbin/cryptsetup", "/sbin/cryptsetup", "/usr/bin/cryptsetup"}},
        {"dpkg-query", {"/usr/bin/dpkg-query"}},
        {"grub-editenv", {"/usr/bin/grub-editenv", "/sbin/grub-editenv", "/usr/sbin/grub-editenv"}},
        {"grub-mkconfig", {"/usr/sbin/grub-mkconfig", "/sbin/grub-mkconfig", "/usr/bin/grub-mkconfig"}},
        {"grub-reboot", {"/usr/sbin/grub-reboot", "/sbin/grub-reboot", "/usr/bin/grub-reboot"}},
        {"grub-set-default", {"/usr/sbin/grub-set-default", "/sbin/grub-set-default", "/usr/bin/grub-set-default"}},
        {"journalctl", {"/usr/bin/journalctl", "/bin/journalctl"}},
        {"live-grubsave", {"/usr/local/bin/live-grubsave", "/usr/bin/live-grubsave"}},
        {"lsblk", {"/usr/bin/lsblk", "/bin/lsblk"}},
        {"mkdir", {"/usr/bin/mkdir", "/bin/mkdir"}},
        {"mkinitcpio", {"/usr/bin/mkinitcpio"}},
        {"mount", {"/usr/bin/mount", "/bin/mount"}},
        {"mv", {"/usr/bin/mv", "/bin/mv"}},
        {"pacman", {"/usr/bin/pacman"}},
        {"plymouth", {"/usr/bin/plymouth", "/bin/plymouth"}},
        {"plymouth-set-default-theme",
         {"/usr/sbin/plymouth-set-default-theme", "/sbin/plymouth-set-default-theme", "/usr/bin/plymouth-set-default-theme"}},
        {"plymouthd", {"/usr/sbin/plymouthd", "/sbin/plymouthd", "/usr/bin/plymouthd"}},
        {"rm", {"/usr/bin/rm", "/bin/rm"}},
        {"rmdir", {"/usr/bin/rmdir", "/bin/rmdir"}},
        {"true", {"/usr/bin/true", "/bin/true"}},
        {"umount", {"/usr/bin/umount", "/bin/umount"}},
        {"update-grub", {"/usr/sbin/update-grub", "/sbin/update-grub", "/usr/bin/update-grub"}},
        {"update-initramfs", {"/usr/sbin/update-initramfs", "/sbin/update-initramfs", "/usr/bin/update-initramfs"}},
        {"update-rc.d", {"/usr/sbin/update-rc.d", "/sbin/update-rc.d", "/usr/bin/update-rc.d"}},
    };
    return commands;
}

[[nodiscard]] QString resolveHostBinary(const QStringList &candidates, const QString &rootPath = {})
{
    for (const QString &candidate : candidates) {
        const QString fullPath = rootPath.isEmpty() ? candidate : rootPath + candidate;
        const QFileInfo info(fullPath);
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

[[nodiscard]] bool isValidRootPath(const QString &rootPath)
{
    return rootPath.isEmpty() || (QDir::isAbsolutePath(rootPath) && QDir(rootPath).exists());
}

[[nodiscard]] QString pathInTarget(const QString &rootPath, const QString &path)
{
    if (rootPath.isEmpty()) {
        return path;
    }
    return rootPath + path;
}

[[nodiscard]] bool isAllowedReadFilePath(const QString &path)
{
    static const QSet<QString> allowedPaths {
        "/boot/grub/grub.cfg",
        "/run/rc.log",
        "/var/log/boot",
        "/var/log/boot.log",
    };
    return allowedPaths.contains(path);
}

[[nodiscard]] bool isAllowedAppendPath(const QString &path)
{
    return path == QLatin1String("/etc/default/rcS");
}

[[nodiscard]] bool resolvesWithinTarget(const QString &rootPath, const QString &requestedPath, const QString &resolvedPath)
{
    if (resolvedPath.isEmpty()) {
        return false;
    }

    if (rootPath.isEmpty()) {
        return resolvedPath == requestedPath;
    }

    const QString canonicalRoot = QDir(rootPath).canonicalPath();
    return !canonicalRoot.isEmpty() && (resolvedPath == canonicalRoot + requestedPath
                                        || resolvedPath.startsWith(canonicalRoot + requestedPath + '/'));
}

[[nodiscard]] QByteArray readHelperInput()
{
    QFile input;
    if (!input.open(stdin, QIODevice::ReadOnly)) {
        return {};
    }
    return input.readAll();
}

[[nodiscard]] QString resolveChrootBinary()
{
    static const QStringList chrootCandidates {"/usr/sbin/chroot", "/usr/bin/chroot", "/bin/chroot"};
    return resolveHostBinary(chrootCandidates);
}

[[nodiscard]] int runAllowedCommand(const QString &rootPath, const QString &command, const QStringList &commandArgs,
                                    const QByteArray &input = {})
{
    const auto commandIt = allowedCommands().constFind(command);
    if (commandIt == allowedCommands().constEnd()) {
        printError(QString("Command is not allowed: %1").arg(command));
        return 127;
    }

    const QString resolvedCommand = resolveHostBinary(commandIt.value(), rootPath);
    if (resolvedCommand.isEmpty()) {
        printError(QString("Command is not available: %1").arg(command));
        return 127;
    }

    if (rootPath.isEmpty()) {
        return relayResult(runProcess(resolvedCommand, commandArgs, input));
    }

    const QString chrootBinary = resolveChrootBinary();
    if (chrootBinary.isEmpty()) {
        printError(QStringLiteral("Unable to find chroot executable"));
        return 127;
    }

    QStringList chrootArgs {rootPath, resolvedCommand};
    chrootArgs += commandArgs;
    return relayResult(runProcess(chrootBinary, chrootArgs, input));
}

[[nodiscard]] int handleReadFile(const QString &rootPath, const QStringList &args)
{
    if (args.size() != 1) {
        printError(QStringLiteral("read-file requires exactly one path"));
        return 1;
    }

    const QString path = args.constFirst();
    if (!QDir::isAbsolutePath(path)) {
        printError(QStringLiteral("read-file path must be absolute"));
        return 1;
    }
    if (!isAllowedReadFilePath(path)) {
        printError(QString("read-file path is not allowed: %1").arg(path));
        return 1;
    }

    const QString fullPath = pathInTarget(rootPath, path);
    const QString canonicalPath = QFileInfo(fullPath).canonicalFilePath();
    if (!resolvesWithinTarget(rootPath, path, canonicalPath)) {
        printError(QString("read-file path resolves outside the allowed target: %1").arg(path));
        return 1;
    }

    QFile file(canonicalPath);
    if (!file.open(QIODevice::ReadOnly)) {
        printError(QString("Unable to read %1").arg(path));
        return 1;
    }

    writeAndFlush(stdout, file.readAll());
    return 0;
}

[[nodiscard]] int handlePackageInstalled(const QString &rootPath, const QStringList &args)
{
    if (args.size() != 2) {
        printError(QStringLiteral("package-installed requires manager and package"));
        return 1;
    }

    const QString manager = args.at(0);
    const QString package = args.at(1);
    static const QRegularExpression packageNameRx(
        QRegularExpression::anchoredPattern(QStringLiteral(R"([A-Za-z0-9.+:_-]+)")));
    if (!packageNameRx.match(package).hasMatch()) {
        printError(QString("Invalid package name: %1").arg(package));
        return 1;
    }

    ProcessResult result;
    if (manager == QLatin1String("apt")) {
        const QString executable = resolveHostBinary(allowedCommands().value("dpkg-query"), rootPath);
        if (executable.isEmpty()) {
            printError(QStringLiteral("dpkg-query is not available"));
            return 127;
        }
        if (rootPath.isEmpty()) {
            result = runProcess(executable, {"--show", "--showformat=${db:Status-Abbrev}", package});
        } else {
            const QString chrootBinary = resolveChrootBinary();
            if (chrootBinary.isEmpty()) {
                printError(QStringLiteral("Unable to find chroot executable"));
                return 127;
            }
            result = runProcess(chrootBinary,
                                {rootPath, executable, "--show", "--showformat=${db:Status-Abbrev}", package});
        }
        return result.started && result.exitStatus == QProcess::NormalExit && result.exitCode == 0
            && result.standardOutput.trimmed().startsWith("ii")
            ? 0
            : 1;
    } else if (manager == QLatin1String("pacman")) {
        const QString executable = resolveHostBinary(allowedCommands().value("pacman"), rootPath);
        if (executable.isEmpty()) {
            printError(QStringLiteral("pacman is not available"));
            return 127;
        }
        if (rootPath.isEmpty()) {
            result = runProcess(executable, {"-Q", package});
        } else {
            const QString chrootBinary = resolveChrootBinary();
            if (chrootBinary.isEmpty()) {
                printError(QStringLiteral("Unable to find chroot executable"));
                return 127;
            }
            result = runProcess(chrootBinary, {rootPath, executable, "-Q", package});
        }
    } else {
        printError(QString("Unsupported package manager: %1").arg(manager));
        return 1;
    }

    return result.started && result.exitStatus == QProcess::NormalExit ? result.exitCode : 1;
}

[[nodiscard]] int handleAppendIfMissing(const QString &rootPath, const QStringList &args)
{
    if (args.size() != 3) {
        printError(QStringLiteral("append-if-missing requires path, needle, and content"));
        return 1;
    }

    const QString path = args.at(0);
    if (!QDir::isAbsolutePath(path)) {
        printError(QStringLiteral("append-if-missing path must be absolute"));
        return 1;
    }
    if (!isAllowedAppendPath(path)) {
        printError(QString("append-if-missing path is not allowed: %1").arg(path));
        return 1;
    }

    const QString fullPath = pathInTarget(rootPath, path);
    const QString canonicalPath = QFileInfo(fullPath).canonicalFilePath();
    if (!resolvesWithinTarget(rootPath, path, canonicalPath)) {
        printError(QString("append-if-missing path resolves outside the allowed target: %1").arg(path));
        return 1;
    }

    QFile file(canonicalPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printError(QString("Unable to read %1").arg(path));
        return 1;
    }

    const QString existingContent = QString::fromUtf8(file.readAll());
    file.close();

    if (existingContent.contains(args.at(1))) {
        return 0;
    }

    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        printError(QString("Unable to append to %1").arg(path));
        return 1;
    }

    QByteArray appendedContent;
    if (!existingContent.isEmpty() && !existingContent.endsWith('\n')) {
        appendedContent.append('\n');
    }
    appendedContent.append(args.at(2).toUtf8());
    if (!appendedContent.endsWith('\n')) {
        appendedContent.append('\n');
    }
    file.write(appendedContent);
    file.close();
    return 0;
}

[[nodiscard]] int handlePreviewPlymouth()
{
    int exitCode = runAllowedCommand({}, "plymouthd", {});
    if (exitCode != 0) {
        return exitCode;
    }

    exitCode = runAllowedCommand({}, "plymouth", {"--show-splash"});
    if (exitCode != 0) {
        const int quitExitCode = runAllowedCommand({}, "plymouth", {"quit"});
        Q_UNUSED(quitExitCode);
        return exitCode;
    }

    for (int i = 0; i < 4; ++i) {
        exitCode = runAllowedCommand({}, "plymouth", {QString("--update=test%1").arg(i)});
        if (exitCode != 0) {
            break;
        }
        QThread::sleep(1);
    }

    const int quitExitCode = runAllowedCommand({}, "plymouth", {"quit"});
    return exitCode == 0 ? quitExitCode : exitCode;
}

[[nodiscard]] int handleExec(const QString &rootPath, const QStringList &args)
{
    if (args.isEmpty()) {
        printError(QStringLiteral("exec requires a command name"));
        return 1;
    }
    return runAllowedCommand(rootPath, args.constFirst(), args.mid(1), readHelperInput());
}
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList arguments = app.arguments().mid(1);
    if (arguments.isEmpty()) {
        printError(QStringLiteral("Missing helper action"));
        return 1;
    }

    QStringList remainingArgs = arguments;
    const QString action = remainingArgs.takeFirst();

    QString rootPath;
    if (!remainingArgs.isEmpty() && remainingArgs.constFirst() == QLatin1String("--root")) {
        if (remainingArgs.size() < 2) {
            printError(QStringLiteral("Missing value after --root"));
            return 1;
        }
        rootPath = remainingArgs.at(1);
        remainingArgs = remainingArgs.mid(2);
    }

    if (!isValidRootPath(rootPath)) {
        printError(QString("Invalid root path: %1").arg(rootPath));
        return 1;
    }

    if (action == QLatin1String("append-if-missing")) {
        return handleAppendIfMissing(rootPath, remainingArgs);
    }
    if (action == QLatin1String("exec")) {
        return handleExec(rootPath, remainingArgs);
    }
    if (action == QLatin1String("package-installed")) {
        return handlePackageInstalled(rootPath, remainingArgs);
    }
    if (action == QLatin1String("preview-plymouth")) {
        if (!rootPath.isEmpty()) {
            printError(QStringLiteral("preview-plymouth does not accept --root"));
            return 1;
        }
        return handlePreviewPlymouth();
    }
    if (action == QLatin1String("read-file")) {
        return handleReadFile(rootPath, remainingArgs);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

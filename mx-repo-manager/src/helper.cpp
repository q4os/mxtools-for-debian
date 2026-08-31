/**********************************************************************
 *  helper.cpp
 **********************************************************************
 * Copyright (C) 2015-2026 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *          OpenAI Codex
 *
 * This file is part of mx-repo-manager.
 *
 * mx-repo-manager is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 **********************************************************************/

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>
#include <QTemporaryFile>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
// Safety net against a genuinely hung privileged process. waitForFinished() returns as soon as
// the process exits, so this only bounds a stuck process; legitimate long operations such as
// netselect/netselect-apt mirror testing and apt-get update should finish well within it. Normal
// user-initiated interruption is handled GUI-side via MainWindow::cancelOperation().
constexpr int kProcessTimeoutMs = 300000; // 5 minutes

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
        {"apt-get", {"/usr/bin/apt-get", "/bin/apt-get"}},
        {"cp", {"/usr/bin/cp", "/bin/cp"}},
        {"mkdir", {"/usr/bin/mkdir", "/bin/mkdir"}},
        {"netselect", {"/usr/bin/netselect"}},
        {"netselect-apt", {"/usr/bin/netselect-apt"}},
        {"rm", {"/usr/bin/rm", "/bin/rm"}},
        {"true", {"/usr/bin/true", "/bin/true"}},
    };
    return commands;
}

[[nodiscard]] bool isCleanAbsolutePath(const QString &path)
{
    return path.startsWith(QLatin1Char('/')) && QDir::cleanPath(path) == path;
}

// The fixed, always-present Debian source files. rm should never be able to remove any of
// these -- it's only ever used to undo a just-created MX-specific restore file.
[[nodiscard]] bool isFixedSourceFile(const QString &path)
{
    static const QStringList fixedFiles {
        "/etc/apt/sources.list",
        "/etc/apt/sources.list.d/debian.list",
        "/etc/apt/sources.list.d/debian.sources",
        "/etc/apt/sources.list.d/debian-stable-updates.list",
        "/etc/apt/sources.list.d/debian-stable-updates.sources",
    };
    return fixedFiles.contains(path);
}

// The fixed set of APT source files this app manages, plus any *.list/*.sources file living
// directly inside sources.list.d (mirrors what the GUI enumerates) -- never in a subdirectory.
[[nodiscard]] bool isManagedSourceFile(const QString &path)
{
    static const QString sourcesListDir = QStringLiteral("/etc/apt/sources.list.d/");

    if (!isCleanAbsolutePath(path)) {
        return false;
    }
    if (isFixedSourceFile(path)) {
        return true;
    }
    if (!path.startsWith(sourcesListDir)) {
        return false;
    }
    const QString rest = path.mid(sourcesListDir.size());
    return !rest.isEmpty() && !rest.contains(QLatin1Char('/'))
        && (rest.endsWith(".list") || rest.endsWith(".sources"));
}

// A per-file backup living directly inside sources.list.d/backups.
[[nodiscard]] bool isBackupFile(const QString &path)
{
    static const QString backupDir = QStringLiteral("/etc/apt/sources.list.d/backups/");

    if (!isCleanAbsolutePath(path) || !path.startsWith(backupDir)) {
        return false;
    }
    const QString rest = path.mid(backupDir.size());
    return !rest.isEmpty() && !rest.contains(QLatin1Char('/'));
}

[[nodiscard]] QString processName(qint64 pid)
{
    QFile commFile(QString("/proc/%1/comm").arg(pid));
    if (!commFile.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromUtf8(commFile.readAll()).trimmed();
}

[[nodiscard]] qint64 parentPidOf(qint64 pid)
{
    QFile statFile(QString("/proc/%1/stat").arg(pid));
    if (!statFile.open(QIODevice::ReadOnly)) {
        return -1;
    }
    const QString stat = QString::fromUtf8(statFile.readAll());
    // Format: "pid (comm) state ppid ..." -- comm can itself contain spaces/parens, so skip past
    // the last ')' before splitting the remaining fields on spaces.
    const int closeParen = stat.lastIndexOf(QLatin1Char(')'));
    if (closeParen == -1) {
        return -1;
    }
    const QStringList fields = stat.mid(closeParen + 2).split(QLatin1Char(' '));
    if (fields.size() < 2) {
        return -1;
    }
    bool ok = false;
    const qint64 ppid = fields.at(1).toLongLong(&ok);
    return ok ? ppid : -1;
}

// The pid of the actual GUI/caller process that invoked us, walking past an intermediate pkexec
// hop if present (pkexec forks before exec'ing us, so our direct parent is pkexec itself, not the
// GUI). Derived from real process ancestry, not anything a caller supplies, so pidfile-based
// operation tracking can be scoped per app instance instead of one process ID being able to read
// or clobber a different instance's tracked operation via a shared, global file.
[[nodiscard]] qint64 callerPid()
{
    const qint64 directParent = static_cast<qint64>(getppid());
    if (processName(directParent) != QLatin1String("pkexec")) {
        return directParent;
    }
    const qint64 grandparent = parentPidOf(directParent);
    return grandparent > 0 ? grandparent : directParent;
}

[[nodiscard]] QString pidFilePath()
{
    return QString("/run/mx-repo-manager-%1.pid").arg(callerPid());
}

[[nodiscard]] qint64 readTrackedPid()
{
    QFile file(pidFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    bool ok = false;
    const qint64 pid = QString::fromUtf8(file.readAll()).trimmed().toLongLong(&ok);
    return ok ? pid : -1;
}

// Written via a temp file + atomic rename so a concurrent readTrackedPid() never sees a
// torn/partial write.
void writePidFile(qint64 pid)
{
    const QString tmpPath = pidFilePath() + QStringLiteral(".new");
    QFile file(tmpPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QByteArray::number(pid));
    file.close();
    if (std::rename(QFile::encodeName(tmpPath).constData(), QFile::encodeName(pidFilePath()).constData()) != 0) {
        const int savedErrno = errno;
        printError(
            QString("Could not update tracked pid file: %1").arg(QString::fromLocal8Bit(std::strerror(savedErrno))));
        QFile::remove(tmpPath);
        // Fail closed: whatever was at pidFilePath() before (nothing, or a previous, possibly
        // stale entry) must not survive as if it still described this operation. Otherwise a
        // later "kill" could read that stale pid and signal an unrelated allowed process instead
        // of just reporting no cancellable operation.
        QFile::remove(pidFilePath());
    }
}

// Only clears the tracked pid if it's still ours -- defense in depth on top of the per-caller
// pidfile path, in case of any overlapping tracked operations within the same instance.
void removePidFileIfMatches(qint64 pid)
{
    if (readTrackedPid() == pid) {
        QFile::remove(pidFilePath());
    }
}

// Extra guard against the tracked pid having exited and been reused for an unrelated process by
// the time "kill" runs.
[[nodiscard]] bool isTrackedProcessAllowed(qint64 pid)
{
    static const QStringList allowedNames {"apt-get", "netselect", "netselect-apt"};
    const QFileInfo exeLink(QString("/proc/%1/exe").arg(pid));
    return allowedNames.contains(QFileInfo(exeLink.symLinkTarget()).fileName());
}

// Validate that the requested arguments match one of the specific operations this app ever
// performs, so an authenticated pkexec session cannot be reused to run e.g.
// "cp <anything> <anywhere>" or kill an arbitrary process -- only the program name was checked
// before.
[[nodiscard]] bool validateArgs(const QString &command, const QStringList &args)
{
    if (command == "apt-get") {
        return args == QStringList {"update"};
    }
    if (command == "true") {
        return args.isEmpty();
    }
    if (command == "mkdir") {
        return args == QStringList {"-p", QStringLiteral("/etc/apt/sources.list.d/backups")};
    }
    if (command == "cp") {
        if (args.size() != 2) {
            return false;
        }
        const QString &src = args.at(0);
        const QString &dst = args.at(1);
        if (isManagedSourceFile(src) && isBackupFile(dst)) {
            return true; // taking a fresh, uniquely-named backup
        }
        // Rollback direction: only ever restores a file this app already knows about, never
        // conjures a brand-new source file from a backup's content.
        return isBackupFile(src) && isManagedSourceFile(dst) && QFileInfo::exists(dst);
    }
    if (command == "kill") {
        // The target pid is never taken from here -- see handleCancel(). This only validates the
        // signal-choice flag.
        return args.isEmpty() || args == QStringList {"-9"};
    }
    if (command == "netselect") {
        return args.size() >= 2 && args.at(0) == "-D" && args.at(1) == "-I";
    }
    if (command == "rm") {
        // Only ever used to undo a just-created MX-specific restore file on rollback. Never one of
        // the fixed Debian source files -- restore never creates those, so rm should never be able
        // to remove one even if invoked directly rather than via the actual restore/rollback flow.
        return args.size() == 2 && args.at(0) == "-f" && isManagedSourceFile(args.at(1))
            && !isFixedSourceFile(args.at(1));
    }
    return false;
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

[[nodiscard]] ProcessResult runProcess(const QString &program, const QStringList &args, const QByteArray &input,
                                       bool trackForCancel)
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
    if (trackForCancel) {
        writePidFile(process.processId());
    }

    if (!input.isEmpty()) {
        process.write(input);
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kProcessTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        result.standardOutput = process.readAllStandardOutput();
        result.standardError = process.readAllStandardError()
            + QString("\nCommand timed out after %1 seconds and was killed: %2\n")
                  .arg(kProcessTimeoutMs / 1000)
                  .arg(program)
                  .toUtf8();
        result.exitStatus = QProcess::CrashExit;
        result.exitCode = 124; // conventional timeout exit code
        if (trackForCancel) {
            removePidFileIfMatches(process.processId());
        }
        return result;
    }

    if (trackForCancel) {
        removePidFileIfMatches(process.processId());
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

// Signals only the pid this helper itself most recently spawned for a long-running, cancellable
// command, never a caller-supplied pid -- so invoking "kill" can, at worst, cancel the app's own
// in-flight operation.
[[nodiscard]] int handleCancel(const QStringList &args)
{
    const bool force = args.contains(QStringLiteral("-9"));
    const qint64 pid = readTrackedPid();
    if (pid <= 0) {
        printError(QStringLiteral("No cancellable operation is currently running"));
        return 1;
    }
    if (!isTrackedProcessAllowed(pid)) {
        printError(QStringLiteral("Tracked process is no longer valid"));
        return 1;
    }
    if (::kill(static_cast<pid_t>(pid), force ? SIGKILL : SIGTERM) != 0) {
        const int savedErrno = errno;
        printError(QString("Failed to signal process %1: %2").arg(pid).arg(QString::fromLocal8Bit(std::strerror(savedErrno))));
        return 1;
    }
    return 0;
}

// Runs netselect-apt with an output destination the helper creates and owns itself, never one
// supplied by the caller: a freshly mkstemp()-style created file can't be a pre-planted symlink,
// unlike a caller-supplied "-o" path that could be swapped for one between validation and use.
// The resulting content is relayed back over our own stdout instead.
[[nodiscard]] int handleNetselectApt(const QStringList &args)
{
    if (args.size() > 1) {
        printError(QStringLiteral("Arguments not allowed for command: netselect-apt"));
        return 127;
    }
    if (args.size() == 1) {
        static const QRegularExpression releaseName("^[A-Za-z0-9._-]+$");
        if (!releaseName.match(args.constFirst()).hasMatch()) {
            printError(QStringLiteral("Arguments not allowed for command: netselect-apt"));
            return 127;
        }
    }

    const QString program = resolveBinary(allowedCommands().value(QStringLiteral("netselect-apt")));
    if (program.isEmpty()) {
        printError(QStringLiteral("Command is not available: netselect-apt"));
        return 127;
    }

    QTemporaryFile outputFile;
    outputFile.setAutoRemove(false);
    if (!outputFile.open()) {
        printError(QStringLiteral("Could not create a temporary output file"));
        return 1;
    }
    const QString outputPath = outputFile.fileName();
    outputFile.close();

    QStringList fullArgs = args;
    fullArgs << "-o" << outputPath;

    ProcessResult result = runProcess(program, fullArgs, {}, /*trackForCancel=*/true);
    if (result.started && result.exitStatus == QProcess::NormalExit && result.exitCode == 0) {
        QFile output(outputPath);
        result.standardOutput = output.open(QIODevice::ReadOnly) ? output.readAll() : QByteArray();
        result.standardError.clear();
    }
    QFile::remove(outputPath);
    return relayResult(result);
}

[[nodiscard]] int runAllowedCommand(const QString &command, const QStringList &args, const QByteArray &input = {})
{
    if (command == "kill") {
        if (!validateArgs(command, args)) {
            printError(QStringLiteral("Arguments not allowed for command: kill"));
            return 127;
        }
        return handleCancel(args);
    }
    if (command == "netselect-apt") {
        return handleNetselectApt(args);
    }

    const auto commandIt = allowedCommands().constFind(command);
    if (commandIt == allowedCommands().constEnd()) {
        printError(QString("Command is not allowed: %1").arg(command));
        return 127;
    }

    if (!validateArgs(command, args)) {
        printError(QString("Arguments not allowed for command: %1").arg(command));
        return 127;
    }

    const QString program = resolveBinary(commandIt.value());
    if (program.isEmpty()) {
        printError(QString("Command is not available: %1").arg(command));
        return 127;
    }

    const bool trackForCancel = command == QLatin1String("apt-get") || command == QLatin1String("netselect");
    return relayResult(runProcess(program, args, input, trackForCancel));
}

[[nodiscard]] int handleExec(const QStringList &args)
{
    if (args.isEmpty()) {
        printError(QStringLiteral("exec requires a command name"));
        return 1;
    }
    return runAllowedCommand(args.constFirst(), args.mid(1), readHelperInput());
}

// Writes new content for a managed APT source file directly as root: no caller-supplied path is
// ever trusted as the source of file content, only the target location and the bytes received
// over our own stdin (which nothing but our direct parent process can feed).
//
// Known, accepted residual: --allow-create (needed by the restore flow to recreate a file the
// user deleted) is just another argument to a directly-callable action, so a caller invoking the
// helper itself during the pkexec auth-cache window -- not only the GUI -- can also use it to
// create a brand-new *.list/*.sources file under sources.list.d. This can't be closed by
// validating arguments more tightly: PolicyKit's auth_admin_keep authorizes running this binary
// for the session, not any specific calling process, so any gate added here (a flag, a token
// minted by an earlier call, a narrower filename allow-list) is just as directly invokable by
// that same caller. Fully closing this needs either dropping auth_admin_keep in favor of
// re-authenticating every privileged action (real UX cost), or replacing this pkexec+CLI-helper
// model with a D-Bus service that verifies the caller's actual binary/peer credentials (a
// significant rewrite of the whole elevation mechanism). Neither has been done; this is the same
// class of limitation as a caller being able to overwrite the *content* of an existing managed
// file, just extended to being able to create a new one.
[[nodiscard]] int handleInstall(const QStringList &args, const QByteArray &content)
{
    QStringList positional = args;
    const bool allowCreate = !positional.isEmpty() && positional.constFirst() == QLatin1String("--allow-create");
    if (allowCreate) {
        positional.removeFirst();
    }

    if (positional.size() != 1) {
        printError(QStringLiteral("install requires exactly one target path"));
        return 1;
    }

    const QString &targetPath = positional.constFirst();
    if (!isManagedSourceFile(targetPath)) {
        printError(QString("Refusing to install to disallowed path: %1").arg(targetPath));
        return 1;
    }
    // Every legitimate caller except the restore flow (which may recreate a file the user
    // deleted) only ever replaces a file that already exists; without this, any *.list/*.sources
    // name under sources.list.d would be accepted, letting a caller plant a brand-new,
    // attacker-controlled APT source file that was never one of the app's known files.
    if (!allowCreate && !QFileInfo::exists(targetPath)) {
        printError(QString("Refusing to create new file: %1").arg(targetPath));
        return 1;
    }

    const QString tmpPath = targetPath + QStringLiteral(".mxrm-new");
    {
        QFile tmpFile(tmpPath);
        if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            printError(QString("Could not create %1: %2").arg(tmpPath, tmpFile.errorString()));
            return 1;
        }
        if (tmpFile.write(content) != content.size()) {
            printError(QString("Could not write %1: %2").arg(tmpPath, tmpFile.errorString()));
            tmpFile.close();
            QFile::remove(tmpPath);
            return 1;
        }
    } // flush + close via scope exit

    const QByteArray tmpPathLocal = QFile::encodeName(tmpPath);
    if (::chmod(tmpPathLocal.constData(), 0644) != 0 || ::chown(tmpPathLocal.constData(), 0, 0) != 0) {
        const int savedErrno = errno;
        printError(QString("Could not set ownership/permissions on %1: %2")
                       .arg(tmpPath, QString::fromLocal8Bit(std::strerror(savedErrno))));
        QFile::remove(tmpPath);
        return 1;
    }

    const QByteArray targetPathLocal = QFile::encodeName(targetPath);
    if (std::rename(tmpPathLocal.constData(), targetPathLocal.constData()) != 0) {
        const int savedErrno = errno;
        printError(
            QString("Could not install %1: %2").arg(targetPath, QString::fromLocal8Bit(std::strerror(savedErrno))));
        QFile::remove(tmpPath);
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
    const QStringList remainingArgs = args.mid(1);

    if (action == QLatin1String("exec")) {
        return handleExec(remainingArgs);
    }
    if (action == QLatin1String("install")) {
        return handleInstall(remainingArgs, readHelperInput());
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

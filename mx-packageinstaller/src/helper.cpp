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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <pty.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

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
        {"ps", {"/usr/bin/ps", "/bin/ps"}},
        {"snap", {"/usr/bin/snap", "/snap/bin/snap"}},
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
                                       const QHash<QString, QString> &environment = {}, bool relayStdout = true,
                                       bool relayStderr = true)
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
            if (relayStdout) {
                writeAndFlush(stdout, stdoutChunk);
            }
        }

        const QByteArray stderrChunk = process.readAllStandardError();
        if (!stderrChunk.isEmpty()) {
            result.standardError += stderrChunk;
            if (relayStderr) {
                writeAndFlush(stderr, stderrChunk);
            }
        }
    }

    result.exitStatus = process.exitStatus();
    result.exitCode = process.exitCode();
    return result;
}

// Run a command attached to a pseudo-terminal so tools that gate their progress
// output on isatty() (notably `snap`) emit their live progress instead of staying
// silent until completion. Output is relayed to our stdout in real time and also
// captured. stdin is forwarded so the child can still read input if it needs to.
[[nodiscard]] ProcessResult runProcessOnPty(const QString &program, const QStringList &args,
                                            const QHash<QString, QString> &environment)
{
    ProcessResult result;

    // Materialize argv and the environment overrides before forking; the child must
    // avoid heap allocation between fork() and exec().
    const QByteArray programBytes = program.toLocal8Bit();
    QList<QByteArray> argStorage;
    argStorage.reserve(args.size());
    for (const QString &arg : args) {
        argStorage.append(arg.toLocal8Bit());
    }
    std::vector<char *> argv;
    argv.reserve(static_cast<size_t>(args.size()) + 2);
    argv.push_back(const_cast<char *>(programBytes.constData()));
    for (QByteArray &arg : argStorage) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    QList<QPair<QByteArray, QByteArray>> envOverrides;
    envOverrides.reserve(environment.size());
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        envOverrides.append({it.key().toLocal8Bit(), it.value().toLocal8Bit()});
    }

    // Hand the child a roomy terminal so progress bars are not truncated.
    struct winsize ws {};
    ws.ws_row = 24;
    ws.ws_col = 120;

    int masterFd = -1;
    const pid_t pid = forkpty(&masterFd, nullptr, nullptr, &ws);
    if (pid < 0) {
        result.standardError = QByteArrayLiteral("Failed to allocate pseudo-terminal");
        result.exitCode = 127;
        return result;
    }

    if (pid == 0) {
        // Child: the helper is single-threaded, so applying the environment with
        // setenv() before exec() is safe here.
        for (const auto &envVar : envOverrides) {
            setenv(envVar.first.constData(), envVar.second.constData(), 1);
        }
        // forkpty leaves the pty master open in the child; close it (and any other
        // inherited descriptors above stdio) so the privileged child only keeps the
        // terminal it needs. Best effort: ignore failure on kernels without close_range.
        close_range(STDERR_FILENO + 1, ~0U, 0);
        execv(programBytes.constData(), argv.data());
        _exit(127); // exec failed
    }

    result.started = true;

    const int stdinFd = fileno(stdin);
    bool stdinOpen = stdinFd >= 0;
    char buffer[4096];

    for (;;) {
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(masterFd, &readFds);
        int maxFd = masterFd;
        if (stdinOpen) {
            FD_SET(stdinFd, &readFds);
            maxFd = std::max(maxFd, stdinFd);
        }

        if (select(maxFd + 1, &readFds, nullptr, nullptr, nullptr) < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        if (FD_ISSET(masterFd, &readFds)) {
            const ssize_t count = read(masterFd, buffer, sizeof(buffer));
            if (count > 0) {
                const QByteArray chunk = QByteArray::fromRawData(buffer, static_cast<int>(count));
                result.standardOutput += chunk;
                writeAndFlush(stdout, chunk);
            } else if (count == 0) {
                break;
            } else if (errno != EINTR && errno != EAGAIN) {
                break; // EIO on Linux once the child has closed the slave side
            }
        }

        if (stdinOpen && FD_ISSET(stdinFd, &readFds)) {
            const ssize_t count = read(stdinFd, buffer, sizeof(buffer));
            if (count > 0) {
                ssize_t written = 0;
                while (written < count) {
                    const ssize_t bytes = write(masterFd, buffer + written, static_cast<size_t>(count - written));
                    if (bytes <= 0) {
                        break;
                    }
                    written += bytes;
                }
            } else if (count == 0) {
                // Our stdin hit EOF. In canonical mode the child only sees EOF
                // once we send the terminal's VEOF character; without this a
                // child that reads stdin to EOF would hang waiting for input
                // that never comes. (Skipped in raw mode, where VEOF would just
                // be delivered as a stray data byte.)
                struct termios tio {};
                if (tcgetattr(masterFd, &tio) == 0 && (tio.c_lflag & ICANON) != 0) {
                    const char eofByte = static_cast<char>(tio.c_cc[VEOF]);
                    if (write(masterFd, &eofByte, 1) < 0) {
                        // best effort: nothing useful to do if this fails
                    }
                }
                stdinOpen = false;
            } else if (errno != EINTR && errno != EAGAIN) {
                stdinOpen = false; // read error; stop forwarding
            }
        }
    }

    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    close(masterFd);

    if (WIFEXITED(status)) {
        result.exitStatus = QProcess::NormalExit;
        result.exitCode = WEXITSTATUS(status);
    } else {
        result.exitStatus = QProcess::CrashExit;
        result.exitCode = 1;
    }
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

    QHash<QString, QString> commandEnvironment = environment;
    // The snap client warns "/snap/bin is not in your $PATH" based on the PATH of
    // the process that invokes it. Running through this elevated helper, that PATH
    // is root's sanitized one, which never contains /snap/bin, so the warning shows
    // on every install regardless of the user's session (and a reboot won't help).
    // Ensure the snap process sees /snap/bin so the spurious warning is suppressed.
    if (command == QLatin1String("snap") && !commandEnvironment.contains(QStringLiteral("PATH"))) {
        QString path = qEnvironmentVariable("PATH");
        if (!path.split(QLatin1Char(':')).contains(QLatin1String("/snap/bin"))) {
            path = path.isEmpty() ? QStringLiteral("/snap/bin") : path + QStringLiteral(":/snap/bin");
        }
        commandEnvironment.insert(QStringLiteral("PATH"), path);
    }

    if (command == QLatin1String("snap")) {
        // snap stays silent unless it believes it is writing to a terminal, so run it
        // under a pseudo-terminal to surface its live progress in the Output tab.
        return relayResult(runProcessOnPty(resolvedCommand, commandArgs, commandEnvironment));
    }

    return relayResult(runProcess(resolvedCommand, commandArgs, commandEnvironment));
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

    const ProcessResult fuserResult = runProcess(fuserBinary, {path}, {}, false, false);
    const QString fuserOutput = QString::fromUtf8(fuserResult.standardOutput);
    const QRegularExpression pidRegex(QStringLiteral(R"((\d+))"));
    const QRegularExpressionMatch match = pidRegex.match(fuserOutput);
    if (!match.hasMatch()) {
        return 0;
    }

    const QString pid = match.captured(1);
    const ProcessResult psResult = runProcess(psBinary, {"--no-headers", "-o", "comm=", "-p", pid}, {}, false, false);
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

// The marker path comes from the (unprivileged) caller, and creating it truncates
// that path as root. Constrain it the way handleWriteFile constrains its target:
// it must be a "mx-pkg-helper-*.marker" file directly inside the caller's runtime
// dir (/run/user/<PKEXEC_UID>) or the temp dir, so a hostile path cannot truncate
// an arbitrary root-owned file.
[[nodiscard]] bool isValidMarkerPath(const QString &path)
{
    const QFileInfo info(path);
    const QString name = info.fileName();
    if (!name.startsWith(QLatin1String("mx-pkg-helper-")) || !name.endsWith(QLatin1String(".marker"))) {
        return false;
    }
    const QString dir = info.absolutePath();
    const QString pkexecUid = qEnvironmentVariable("PKEXEC_UID");
    static const QRegularExpression digits(QStringLiteral("^[0-9]+$"));
    if (digits.match(pkexecUid).hasMatch() && dir == QStringLiteral("/run/user/") + pkexecUid) {
        return true;
    }
    return dir == QDir::tempPath();
}
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments().mid(1);

    // The caller passes an auth-success marker path as a leading "--marker
    // <path>" argument (not via the environment, which pkexec strips). Creating
    // the file proves the helper actually ran, so the caller can tell a real
    // 126/127 from a dismissed authentication dialog.
    if (arguments.size() >= 2 && arguments.constFirst() == QLatin1String("--marker")) {
        const QString markerPath = arguments.at(1);
        arguments.removeFirst();
        arguments.removeFirst();
        if (isValidMarkerPath(markerPath)) {
            QFile markerFile(markerPath);
            if (markerFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                markerFile.close();
            }
        } else {
            printError(QString("Ignoring invalid marker path: %1").arg(markerPath));
        }
    }

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

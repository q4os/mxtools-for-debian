/**********************************************************************
 *  MX Date/Time helper.
 **********************************************************************
 *   Copyright (C) 2026 by MX Authors
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of mx-datetime.
 **********************************************************************/

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>

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

[[nodiscard]] bool isValidTimeZone(const QString &zone)
{
    static const QRegularExpression zoneRx(
        QRegularExpression::anchoredPattern(QStringLiteral(R"([A-Za-z0-9._+-]+(?:/[A-Za-z0-9._+-]+)*)")));
    return zoneRx.match(zone.trimmed()).hasMatch();
}

[[nodiscard]] bool isAllowedManagedDestination(const QString &path)
{
    if (!QDir::isAbsolutePath(path)) {
        return false;
    }

    static const QStringList exactPaths {
        QStringLiteral("/etc/chrony.conf"),
        QStringLiteral("/etc/chrony/chrony.conf"),
    };
    if (exactPaths.contains(path)) {
        return true;
    }

    const QFileInfo info(path);
    return info.fileName() == QLatin1String("mx-datetime.sources") && path.startsWith(QStringLiteral("/etc/"));
}

[[nodiscard]] int writeManagedFile(const QString &destinationPath, const QByteArray &content)
{
    const QFileInfo destinationInfo(destinationPath);
    QDir parentDir;
    if (!parentDir.mkpath(destinationInfo.absolutePath())) {
        printError(QString("Unable to create directory for %1").arg(destinationPath));
        return 1;
    }

    QSaveFile file(destinationPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        printError(QString("Unable to open %1 for writing").arg(destinationPath));
        return 1;
    }

    if (file.write(content) != content.size()) {
        printError(QString("Unable to write %1").arg(destinationPath));
        return 1;
    }
    if (!file.commit()) {
        printError(QString("Unable to commit %1").arg(destinationPath));
        return 1;
    }
    if (!QFile::setPermissions(destinationPath,
            QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther)) {
        printError(QString("Unable to set permissions on %1").arg(destinationPath));
        return 1;
    }

    return 0;
}

[[nodiscard]] const QHash<QString, QStringList> &allowedCommands()
{
    static const QHash<QString, QStringList> commands {
        {"chronyc", {"/usr/bin/chronyc", "/bin/chronyc"}},
        {"chronyd", {"/usr/sbin/chronyd", "/sbin/chronyd", "/usr/bin/chronyd"}},
        {"date", {"/usr/bin/date", "/bin/date"}},
        {"hwclock", {"/usr/sbin/hwclock", "/sbin/hwclock", "/usr/bin/hwclock"}},
        {"rc-update", {"/usr/sbin/rc-update", "/sbin/rc-update", "/usr/bin/rc-update"}},
        {"service", {"/usr/sbin/service", "/sbin/service", "/usr/bin/service"}},
        {"systemctl", {"/usr/bin/systemctl", "/bin/systemctl"}},
        {"timedatectl", {"/usr/bin/timedatectl", "/bin/timedatectl"}},
        {"update-rc.d", {"/usr/sbin/update-rc.d", "/sbin/update-rc.d", "/usr/bin/update-rc.d"}},
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

[[nodiscard]] ProcessResult runProcess(const QString &program, const QStringList &arguments)
{
    ProcessResult result;

    QProcess process;
    process.start(program, arguments, QIODevice::ReadOnly);
    if (!process.waitForStarted()) {
        result.standardError = QString("Failed to start %1").arg(program).toUtf8();
        result.exitCode = 127;
        return result;
    }

    result.started = true;
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

[[nodiscard]] int handleExec(const QStringList &arguments)
{
    if (arguments.isEmpty()) {
        printError(QStringLiteral("exec requires a command name"));
        return 1;
    }

    const QString command = arguments.constFirst();
    const auto commandIt = allowedCommands().constFind(command);
    if (commandIt == allowedCommands().constEnd()) {
        printError(QString("Command is not allowed: %1").arg(command));
        return 127;
    }

    const QString executable = resolveBinary(commandIt.value());
    if (executable.isEmpty()) {
        printError(QString("Command is not available: %1").arg(command));
        return 127;
    }

    return relayResult(runProcess(executable, arguments.mid(1)));
}

[[nodiscard]] int handleWriteTimezone(const QStringList &arguments)
{
    if (arguments.size() != 1) {
        printError(QStringLiteral("write-timezone requires exactly one value"));
        return 1;
    }

    const QString zone = arguments.constFirst().trimmed();
    if (!isValidTimeZone(zone)) {
        printError(QString("Invalid timezone value: %1").arg(zone));
        return 1;
    }
    return writeManagedFile(QStringLiteral("/etc/timezone"), zone.toUtf8() + '\n');
}

[[nodiscard]] int handleSetLocaltimeLink(const QStringList &arguments)
{
    if (arguments.size() != 1) {
        printError(QStringLiteral("set-localtime-link requires exactly one value"));
        return 1;
    }

    const QString zone = arguments.constFirst().trimmed();
    if (!isValidTimeZone(zone)) {
        printError(QString("Invalid timezone value: %1").arg(zone));
        return 1;
    }

    const QString targetPath = QStringLiteral("/usr/share/zoneinfo/") + zone;
    const QFileInfo targetInfo(targetPath);
    if (!targetInfo.exists() || !targetInfo.isFile()) {
        printError(QString("Timezone data is not available: %1").arg(zone));
        return 1;
    }

    const QFileInfo localtimeInfo(QStringLiteral("/etc/localtime"));
    if (localtimeInfo.exists()) {
        if (localtimeInfo.isDir() || !QFile::remove(localtimeInfo.filePath())) {
            printError(QStringLiteral("Unable to replace /etc/localtime"));
            return 1;
        }
    }
    if (!QFile::link(targetPath, QStringLiteral("/etc/localtime"))) {
        printError(QStringLiteral("Unable to create /etc/localtime symlink"));
        return 1;
    }
    return 0;
}

[[nodiscard]] int handleSetHwclockMode(const QStringList &arguments)
{
    if (arguments.size() != 1) {
        printError(QStringLiteral("set-hwclock-mode requires utc or local"));
        return 1;
    }

    const QString mode = arguments.constFirst().trimmed();
    QString replacement;
    if (mode == QLatin1String("utc")) {
        replacement = QStringLiteral("clock=\"UTC\"");
    } else if (mode == QLatin1String("local")) {
        replacement = QStringLiteral("clock=\"local\"");
    } else {
        printError(QString("Unsupported hwclock mode: %1").arg(mode));
        return 1;
    }

    const QString path = QStringLiteral("/etc/conf.d/hwclock");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printError(QStringLiteral("Unable to read /etc/conf.d/hwclock"));
        return 1;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    static const QRegularExpression clockLineRx(QStringLiteral(R"((?m)^clock=.*$)"));
    if (content.contains(clockLineRx)) {
        content.replace(clockLineRx, replacement);
    } else {
        if (!content.isEmpty() && !content.endsWith('\n')) {
            content.append('\n');
        }
        content.append(replacement);
        content.append('\n');
    }

    return writeManagedFile(path, content.toUtf8());
}

[[nodiscard]] int handleInstallManagedFile(const QStringList &arguments)
{
    if (arguments.size() != 2) {
        printError(QStringLiteral("install-managed-file requires source and destination"));
        return 1;
    }

    const QString sourcePath = arguments.at(0);
    const QString destinationPath = arguments.at(1);
    if (!QDir::isAbsolutePath(sourcePath)) {
        printError(QStringLiteral("install-managed-file source must be absolute"));
        return 1;
    }
    if (!isAllowedManagedDestination(destinationPath)) {
        printError(QString("install-managed-file destination is not allowed: %1").arg(destinationPath));
        return 1;
    }

    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        printError(QString("Unable to read %1").arg(sourcePath));
        return 1;
    }
    const QByteArray content = sourceFile.readAll();
    sourceFile.close();

    const int result = writeManagedFile(destinationPath, content);
    if (result == 0) {
        QFile::remove(sourcePath);
    }
    return result;
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

    const QString action = arguments.constFirst();
    const QStringList remainingArgs = arguments.mid(1);

    if (action == QStringLiteral("exec")) {
        return handleExec(remainingArgs);
    }
    if (action == QStringLiteral("install-managed-file")) {
        return handleInstallManagedFile(remainingArgs);
    }
    if (action == QStringLiteral("set-hwclock-mode")) {
        return handleSetHwclockMode(remainingArgs);
    }
    if (action == QStringLiteral("set-localtime-link")) {
        return handleSetLocaltimeLink(remainingArgs);
    }
    if (action == QStringLiteral("write-timezone")) {
        return handleWriteTimezone(remainingArgs);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

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

// Privileged helper for mx-user.
//
// Security model: this binary runs as root via pkexec. It exposes a fixed set
// of *named operations* (add-user, del-user, set-autologin, ...). Every command
// line it runs is built from internal constants; the only data accepted from the
// caller are scalar parameters (user/group names, a shell from the allow-list, a
// display-manager session token, and paths constrained to /home), each of which
// is validated before use. There is intentionally no generic "run this command"
// entry point, so a caller cannot inject paths, options, scripts, or commands.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QRegularExpression>
#include <QStringList>

#include <cstdio>
#include <pwd.h>

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

// --------------------------------------------------------------------------
// Parameter validation
// --------------------------------------------------------------------------

[[nodiscard]] bool matchesPattern(const QString &value, const QString &pattern)
{
    const QRegularExpression rx(QRegularExpression::anchoredPattern(pattern));
    return rx.match(value).hasMatch();
}

[[nodiscard]] bool isUserName(const QString &value)
{
    return matchesPattern(value, QStringLiteral(R"([A-Za-z_][A-Za-z0-9_-]*[$]?)"));
}

[[nodiscard]] bool isGroupName(const QString &value)
{
    return matchesPattern(value, QStringLiteral(R"([A-Za-z0-9_][A-Za-z0-9_.-]*[$]?)"));
}

[[nodiscard]] bool isGroupList(const QString &value)
{
    if (value.isEmpty()) {
        return true; // empty list clears supplementary groups
    }
    const QStringList groups = value.split(',');
    for (const QString &group : groups) {
        if (!isGroupName(group)) {
            return false;
        }
    }
    return true;
}

// A display-manager session identifier such as "plasma", "plasmax11" or
// "plasma.desktop"; empty means "do not touch the session".
[[nodiscard]] bool isSessionToken(const QString &value)
{
    return value.isEmpty() || matchesPattern(value, QStringLiteral(R"([A-Za-z0-9._-]+)"));
}

[[nodiscard]] bool hasNoTraversal(const QString &path)
{
    if (path.contains(QChar(u'\0')) || path.contains('\n') || path.contains('\r')) {
        return false;
    }
    const QStringList parts = path.split('/');
    return !parts.contains(QStringLiteral(".."));
}

[[nodiscard]] bool isAbsoluteSafePath(const QString &path)
{
    if (!path.startsWith('/') || !hasNoTraversal(path)) {
        return false;
    }
    for (const QChar c : path) {
        if (c.unicode() < 0x20) {
            return false;
        }
    }
    return true;
}

// Resolve `requested` against `user`'s home (/home/<user>), following symlinks,
// and confirm the result is the home itself or strictly inside it. On success,
// *safe receives the canonical, symlink-resolved path (no trailing slash).
//
// This binds every file operation to the named user's own home, so a caller
// cannot reach another user's home (cross-user) or escape /home through a symlink
// (the canonical path of e.g. ~/link -> /etc lands outside the home and is
// rejected). Because containment is checked against the resolved home directory
// rather than a regex over the path, usernames are matched verbatim, including
// machine accounts ending in '$'.
[[nodiscard]] bool resolveWithinHome(const QString &user, const QString &requested, QString *safe)
{
    if (!isUserName(user) || requested.contains(QChar(u'\0')) || requested.contains('\n')
        || requested.contains('\r')) {
        return false;
    }
    const QString homeReal = QFileInfo(QStringLiteral("/home/%1").arg(user)).canonicalFilePath();
    if (homeReal.isEmpty()) {
        return false; // the user's home must exist
    }
    const QString clean = QDir::cleanPath(requested);
    if (!clean.startsWith('/')) {
        return false;
    }

    const auto contained = [&homeReal](const QString &p) {
        return p == homeReal || p.startsWith(homeReal + '/');
    };

    QString resolved = QFileInfo(clean).canonicalFilePath();
    if (resolved.isEmpty()) {
        // Target does not exist yet (e.g. ~/.recently-used): validate its parent,
        // which must exist and resolve inside the home, then re-attach the name.
        const QString name = QFileInfo(clean).fileName();
        if (name.isEmpty() || name == QLatin1String(".") || name == QLatin1String("..")) {
            return false;
        }
        const QString parentReal = QFileInfo(QFileInfo(clean).path()).canonicalFilePath();
        if (parentReal.isEmpty() || !contained(parentReal)) {
            return false;
        }
        resolved = parentReal + '/' + name;
    } else if (!contained(resolved)) {
        return false;
    }

    if (safe) {
        *safe = resolved;
    }
    return true;
}

// rsync destinations are restricted to the only places the GUI is meant to copy
// into: anywhere under /home (another user's profile) or under a removable-media
// mount root. This is an allowlist, not a denylist — a root-run rsync (including
// the destructive sync/--delete-after mode) therefore can never target an
// unexpected top-level location such as /opt, /srv, /tmp, /run or a bare mount root.
[[nodiscard]] bool isAllowedCopyDestination(const QString &path)
{
    static const QStringList roots {"/home", "/media", "/mnt", "/run/media"};
    for (const QString &root : roots) {
        if (path.startsWith(root + '/')) {
            return true;
        }
    }
    return false;
}

// Validate an rsync destination: absolute, traversal-free, and within an allowed
// destination root (checked on the symlink-resolved path so a symlink into, say,
// /etc cannot slip through).
[[nodiscard]] bool resolveCopyDestination(const QString &requested, QString *safe)
{
    if (!isAbsoluteSafePath(requested)) {
        return false;
    }
    const QString clean = QDir::cleanPath(requested);
    QString resolved = QFileInfo(clean).canonicalFilePath();
    if (resolved.isEmpty()) {
        // rsync creates the final directory if its parent exists.
        const QString name = QFileInfo(clean).fileName();
        if (name.isEmpty() || name == QLatin1String(".") || name == QLatin1String("..")) {
            return false;
        }
        const QString parentReal = QFileInfo(QFileInfo(clean).path()).canonicalFilePath();
        if (parentReal.isEmpty()) {
            return false;
        }
        resolved = parentReal + '/' + name;
    }
    if (!isAllowedCopyDestination(resolved)) {
        return false;
    }
    if (safe) {
        *safe = resolved;
    }
    return true;
}

[[nodiscard]] bool isAllowedShell(const QString &path)
{
    if (!matchesPattern(path, QStringLiteral(R"(/[A-Za-z0-9._/-]+)")) || !hasNoTraversal(path)) {
        return false;
    }
    const QFileInfo info(path);
    if (!info.exists() || !info.isExecutable()) {
        return false;
    }
    // Prefer membership in /etc/shells; if it is unreadable, accept any existing
    // executable that already passed the character/traversal checks above.
    QFile shells(QStringLiteral("/etc/shells"));
    if (shells.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!shells.atEnd()) {
            const QString line = QString::fromUtf8(shells.readLine()).trimmed();
            if (line == path) {
                return true;
            }
        }
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------
// Tool resolution and execution
// --------------------------------------------------------------------------

[[nodiscard]] const QHash<QString, QStringList> &toolPaths()
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

[[nodiscard]] QString resolveTool(const QString &key)
{
    return resolveBinary(toolPaths().value(key));
}

[[nodiscard]] bool toolAvailable(const QString &key)
{
    return !resolveTool(key).isEmpty();
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

// Run an allow-listed tool with a fixed argument list and relay its output.
int runTool(const QString &key, const QStringList &args, const QByteArray &input = {})
{
    const QString program = resolveTool(key);
    if (program.isEmpty()) {
        printError(QString("Command is not available: %1").arg(key));
        return 127;
    }
    return relayResult(runProcess(program, args, input));
}

// --------------------------------------------------------------------------
// Display-manager autologin configuration
// --------------------------------------------------------------------------

const QString LightdmConf = QStringLiteral("/etc/lightdm/lightdm.conf");
const QString SddmKdeConf = QStringLiteral("/etc/sddm.conf.d/kde_settings.conf");
const QString SddmConf = QStringLiteral("/etc/sddm.conf");
const QString PlasmaConf = QStringLiteral("/etc/plasmalogin.conf.d/autologin.conf");

// Returns the active sddm configuration file (kde variant preferred), or empty.
[[nodiscard]] QString sddmConfFile()
{
    if (QFileInfo::exists(SddmKdeConf)) {
        return SddmKdeConf;
    }
    if (QFileInfo::exists(SddmConf)) {
        return SddmConf;
    }
    return {};
}

// --------------------------------------------------------------------------
// Operations
// --------------------------------------------------------------------------

[[nodiscard]] int opAddUser(const QStringList &args)
{
    // <username> <shell> [<comma-separated groups>]
    if (args.size() < 2 || args.size() > 3) {
        printError(QStringLiteral("add-user requires <username> <shell> [groups]"));
        return 1;
    }
    const QString user = args.at(0);
    const QString shell = args.at(1);
    const QString groups = args.size() == 3 ? args.at(2) : QString();
    if (!isUserName(user) || !isAllowedShell(shell) || !isGroupList(groups)) {
        printError(QStringLiteral("add-user: invalid parameters"));
        return 1;
    }

    if (toolAvailable(QStringLiteral("adduser"))) {
        // Detect the option spelling supported by this adduser implementation.
        const ProcessResult help = runProcess(resolveTool(QStringLiteral("adduser")), {QStringLiteral("--help")});
        const QString helpText = QString::fromUtf8(help.standardOutput + help.standardError);
        const QString commentOption
            = helpText.contains(QLatin1String("--comment")) ? QStringLiteral("--comment") : QStringLiteral("--gecos");
        const QString allowBadNames = helpText.contains(QLatin1String("--allow-bad-names"))
            ? QStringLiteral("--allow-bad-names")
            : QStringLiteral("--force-badname");
        return runTool(QStringLiteral("adduser"),
                       {"--disabled-login", allowBadNames, "--shell", shell, commentOption, user, user});
    }
    if (toolAvailable(QStringLiteral("useradd"))) {
        QStringList useraddArgs {"-m", "-s", shell, "-c", user};
        if (!groups.isEmpty()) {
            useraddArgs << "-G" << groups;
        }
        useraddArgs << user;
        return runTool(QStringLiteral("useradd"), useraddArgs);
    }
    printError(QStringLiteral("No suitable user creation tool found (adduser or useradd)"));
    return 127;
}

[[nodiscard]] int opSetPassword(const QStringList &args)
{
    if (args.size() != 1 || !isUserName(args.constFirst())) {
        printError(QStringLiteral("set-password requires exactly one valid username"));
        return 1;
    }
    return runTool(QStringLiteral("passwd"), {args.constFirst()}, readHelperInput());
}

[[nodiscard]] int opGroupMember(const QStringList &args, bool add)
{
    // <username> <group>
    if (args.size() != 2 || !isUserName(args.at(0)) || !isGroupName(args.at(1))) {
        printError(QStringLiteral("group membership change requires <username> <group>"));
        return 1;
    }
    return runTool(QStringLiteral("gpasswd"), {add ? "-a" : "-d", args.at(0), args.at(1)});
}

[[nodiscard]] int opSetGroups(const QStringList &args)
{
    // <username> <comma-separated groups (may be empty)>
    if (args.isEmpty() || args.size() > 2 || !isUserName(args.at(0))) {
        printError(QStringLiteral("set-groups requires <username> [groups]"));
        return 1;
    }
    const QString groups = args.size() == 2 ? args.at(1) : QString();
    if (!isGroupList(groups)) {
        printError(QStringLiteral("set-groups: invalid group list"));
        return 1;
    }
    return runTool(QStringLiteral("usermod"), {"-G", groups, args.at(0)});
}

[[nodiscard]] int opDelUser(const QStringList &args)
{
    // <username> [--remove-home]
    if (args.isEmpty() || !isUserName(args.constFirst())) {
        printError(QStringLiteral("del-user requires a valid username"));
        return 1;
    }
    const QString user = args.constFirst();
    const bool removeHome = args.contains(QStringLiteral("--remove-home"));
    if (toolAvailable(QStringLiteral("deluser"))) {
        return runTool(QStringLiteral("deluser"),
                       removeHome ? QStringList {"--remove-home", user} : QStringList {user});
    }
    if (toolAvailable(QStringLiteral("userdel"))) {
        return runTool(QStringLiteral("userdel"), removeHome ? QStringList {"-r", user} : QStringList {user});
    }
    printError(QStringLiteral("No suitable user removal tool found (deluser or userdel)"));
    return 127;
}

[[nodiscard]] int opKillUser(const QStringList &args)
{
    if (args.size() != 1 || !isUserName(args.constFirst())) {
        printError(QStringLiteral("kill-user requires exactly one valid username"));
        return 1;
    }
    const QString user = args.constFirst();
    // Best effort: terminate, then force-kill any remaining processes.
    runTool(QStringLiteral("timeout"), {"5s", "killall", "-w", "-u", user});
    runTool(QStringLiteral("timeout"), {"5s", "killall", "-9", "-w", "-u", user});
    return 0;
}

[[nodiscard]] int opAddGroup(const QStringList &args)
{
    // <groupname> <system|user>
    if (args.size() != 2 || !isGroupName(args.at(0))) {
        printError(QStringLiteral("add-group requires <groupname> <system|user>"));
        return 1;
    }
    const QString group = args.at(0);
    const bool systemGroup = args.at(1) == QLatin1String("system");
    if (!systemGroup && args.at(1) != QLatin1String("user")) {
        printError(QStringLiteral("add-group: scope must be 'system' or 'user'"));
        return 1;
    }
    if (toolAvailable(QStringLiteral("addgroup"))) {
        return runTool(QStringLiteral("addgroup"),
                       systemGroup ? QStringList {group, "--system"} : QStringList {group, "--quiet"});
    }
    if (toolAvailable(QStringLiteral("groupadd"))) {
        return runTool(QStringLiteral("groupadd"),
                       systemGroup ? QStringList {"--system", group} : QStringList {group});
    }
    printError(QStringLiteral("No suitable group creation tool found (addgroup or groupadd)"));
    return 127;
}

[[nodiscard]] int opDelGroup(const QStringList &args)
{
    if (args.size() != 1 || !isGroupName(args.constFirst())) {
        printError(QStringLiteral("del-group requires exactly one valid group name"));
        return 1;
    }
    const QString group = args.constFirst();
    if (toolAvailable(QStringLiteral("delgroup"))) {
        return runTool(QStringLiteral("delgroup"), {group});
    }
    if (toolAvailable(QStringLiteral("groupdel"))) {
        return runTool(QStringLiteral("groupdel"), {group});
    }
    printError(QStringLiteral("No suitable group removal tool found (delgroup or groupdel)"));
    return 127;
}

int opRewriteHomePaths(const QStringList &args)
{
    // <owner-user> <dir under owner's home> <old-user> <new-user>:
    // rewrite "home/<old>" -> "home/<new>" in files under <dir>.
    QString dir;
    if (args.size() != 4 || !resolveWithinHome(args.at(0), args.at(1), &dir) || !isUserName(args.at(2))
        || !isUserName(args.at(3))) {
        printError(QStringLiteral("rewrite-home-paths requires <owner-user> <dir under that home> <old-user> <new-user>"));
        return 1;
    }
    // Only rewrite text files that actually contain the old path. The `grep -I`
    // test makes find skip binary files (SQLite databases, caches, browser
    // profiles, images, ...), which `sed -i` would otherwise corrupt by treating
    // them as text. find runs sed (batched via +) only on files the grep test
    // selected.
    const QString pattern = QStringLiteral("home/%1").arg(args.at(2));
    const QString script = QString("s|home/%1|home/%2|g").arg(args.at(2), args.at(3));
    return runTool(QStringLiteral("find"),
                   {dir, "-type", "f", "-exec", "grep", "-IqF", "-e", pattern, "{}", ";", "-exec", "sed", "-i",
                    script, "{}", "+"});
}

[[nodiscard]] int opRenameUser(const QStringList &args)
{
    // <old-user> <new-user>
    if (args.size() != 2 || !isUserName(args.at(0)) || !isUserName(args.at(1))) {
        printError(QStringLiteral("rename-user requires <old-user> <new-user>"));
        return 1;
    }
    const QString oldName = args.at(0);
    const QString newName = args.at(1);

    const int renameCode = runTool(
        QStringLiteral("usermod"),
        {"--login", newName, "--move-home", "--home", QStringLiteral("/home/%1").arg(newName), oldName});
    if (renameCode != 0) {
        return renameCode; // critical failure; do not attempt the cosmetic follow-ups
    }

    // Update the GECOS/full-name field if it referenced the old name (whole word).
    if (const passwd *pw = getpwnam(newName.toLocal8Bit().constData())) {
        const QString gecos = QString::fromLocal8Bit(pw->pw_gecos);
        const QRegularExpression boundary(
            QString(R"((?<![A-Za-z0-9_-])%1(?![A-Za-z0-9_-]))").arg(QRegularExpression::escape(oldName)));
        QString updated = gecos;
        updated.replace(boundary, newName);
        if (updated != gecos) {
            runTool(QStringLiteral("usermod"), {"-c", updated, newName});
        }
    }

    // Rename the user's personal group if it shared the old name (best effort).
    runTool(QStringLiteral("groupmod"), {"--new-name", newName, oldName});

    // Fix "home/<old>" references inside the (now relocated) home directory.
    opRewriteHomePaths({newName, QStringLiteral("/home/%1").arg(newName), oldName, newName});

    return 0;
}

[[nodiscard]] int opRemoveHomePath(const QStringList &args)
{
    // <user> <path under that user's home>
    QString safe;
    if (args.size() != 2 || !resolveWithinHome(args.at(0), args.at(1), &safe)) {
        printError(QStringLiteral("remove-home-path requires <user> <path under that user's home>"));
        return 1;
    }
    return runTool(QStringLiteral("rm"), {"-rf", safe});
}

[[nodiscard]] int opChownHome(const QStringList &args)
{
    // <user> <path under that user's home>
    QString safe;
    if (args.size() != 2 || !resolveWithinHome(args.at(0), args.at(1), &safe)) {
        printError(QStringLiteral("chown-home requires <user> <path under that user's home>"));
        return 1;
    }
    return runTool(QStringLiteral("chown"), {"-R", QString("%1:%1").arg(args.at(0)), safe});
}

[[nodiscard]] int opCleanOpenOfficeLocks(const QStringList &args)
{
    // <user> <home dir>
    QString safe;
    if (args.size() != 2 || !resolveWithinHome(args.at(0), args.at(1), &safe)) {
        printError(QStringLiteral("clean-openoffice-locks requires <user> <home dir>"));
        return 1;
    }
    return runTool(QStringLiteral("find"),
                   {safe + "/.openoffice.org", "-type", "f", "-name", ".lock", "-delete"});
}

[[nodiscard]] int opRsyncCopy(const QStringList &args)
{
    // <from-user> <from-dir under that home> <to-dir> <sync|copy>
    QString fromSafe;
    QString toSafe;
    if (args.size() != 4 || !resolveWithinHome(args.at(0), args.at(1), &fromSafe)
        || !resolveCopyDestination(args.at(2), &toSafe)) {
        printError(QStringLiteral("rsync-copy requires <from-user> <from-dir under that home> <to-dir> <sync|copy>"));
        return 1;
    }
    const bool sync = args.at(3) == QLatin1String("sync");
    if (!sync && args.at(3) != QLatin1String("copy")) {
        printError(QStringLiteral("rsync-copy: mode must be 'sync' or 'copy'"));
        return 1;
    }
    QStringList rsyncArgs {"-a", "--info=progress2"};
    if (sync) {
        rsyncArgs << "--delete-after";
    }
    // Preserve rsync's "copy directory contents" semantics: the source always
    // carried a trailing slash in the original call.
    rsyncArgs << (fromSafe + '/') << toSafe;
    return runTool(QStringLiteral("rsync"), rsyncArgs);
}

[[nodiscard]] int opAutologinEnable(const QStringList &args)
{
    // <user> <kde-session> <default-session>
    if (args.size() != 3 || !isUserName(args.at(0)) || !isSessionToken(args.at(1)) || !isSessionToken(args.at(2))) {
        printError(QStringLiteral("autologin-enable requires <user> <kde-session> <default-session>"));
        return 1;
    }
    const QString user = args.at(0);
    const QString kdeSession = args.at(1);
    const QString defaultSession = args.at(2);

    if (QFileInfo::exists(LightdmConf)) {
        runTool(QStringLiteral("sed"),
                {"-i", "-E", "-e",
                 QStringLiteral(R"(/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=/d;})"), "-e",
                 QString(R"(/^[[]Seat(Defaults|:[*])[]]/aautologin-user=%1)").arg(user), LightdmConf});
    }
    const QString sddm = sddmConfFile();
    if (!sddm.isEmpty()) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=.*/User=%1/").arg(user), sddm});
        const QString session = sddm == SddmKdeConf ? kdeSession : defaultSession;
        if (!session.isEmpty()) {
            runTool(QStringLiteral("sed"), {"-i", QString("s/^Session=.*/Session=%1/").arg(session), sddm});
        }
    }
    if (QFileInfo::exists(PlasmaConf)) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=.*/User=%1/").arg(user), PlasmaConf});
    }
    return 0;
}

[[nodiscard]] int opAutologinDisable(const QStringList &args)
{
    if (args.size() != 1 || !isUserName(args.constFirst())) {
        printError(QStringLiteral("autologin-disable requires one valid username"));
        return 1;
    }
    const QString user = args.constFirst();
    if (QFileInfo::exists(LightdmConf)) {
        runTool(QStringLiteral("sed"),
                {"-i", "-E",
                 QStringLiteral(R"(/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=/d;})"), LightdmConf});
    }
    const QString sddm = sddmConfFile();
    if (!sddm.isEmpty()) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=%1/User=/").arg(user), sddm});
    }
    if (QFileInfo::exists(PlasmaConf)) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=%1/User=/").arg(user), PlasmaConf});
    }
    return 0;
}

[[nodiscard]] int opAutologinRemoveUser(const QStringList &args)
{
    if (args.size() != 1 || !isUserName(args.constFirst())) {
        printError(QStringLiteral("autologin-remove-user requires one valid username"));
        return 1;
    }
    const QString user = args.constFirst();
    if (QFileInfo::exists(LightdmConf)) {
        runTool(QStringLiteral("sed"),
                {"-i", "-E",
                 QString(R"(/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=%1$/d;})").arg(user),
                 LightdmConf});
    }
    const QString sddm = sddmConfFile();
    if (!sddm.isEmpty()) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=%1$/User=/").arg(user), sddm});
    }
    if (QFileInfo::exists(PlasmaConf)) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=%1$/User=/").arg(user), PlasmaConf});
    }
    return 0;
}

[[nodiscard]] int opAutologinRename(const QStringList &args)
{
    if (args.size() != 2 || !isUserName(args.at(0)) || !isUserName(args.at(1))) {
        printError(QStringLiteral("autologin-rename requires <old-user> <new-user>"));
        return 1;
    }
    const QString oldName = args.at(0);
    const QString newName = args.at(1);
    if (QFileInfo::exists(LightdmConf)) {
        runTool(QStringLiteral("sed"),
                {"-i", QString("s/autologin-user=%1/autologin-user=%2/g").arg(oldName, newName), LightdmConf});
    }
    const QString sddm = sddmConfFile();
    if (!sddm.isEmpty()) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=%1$/User=%2/g").arg(oldName, newName), sddm});
    }
    if (QFileInfo::exists(PlasmaConf)) {
        runTool(QStringLiteral("sed"), {"-i", QString("s/^User=%1$/User=%2/g").arg(oldName, newName), PlasmaConf});
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
    const QStringList rest = args.mid(1);

    if (action == QLatin1String("add-user")) {
        return opAddUser(rest);
    }
    if (action == QLatin1String("set-password")) {
        return opSetPassword(rest);
    }
    if (action == QLatin1String("group-add-member")) {
        return opGroupMember(rest, true);
    }
    if (action == QLatin1String("group-remove-member")) {
        return opGroupMember(rest, false);
    }
    if (action == QLatin1String("set-groups")) {
        return opSetGroups(rest);
    }
    if (action == QLatin1String("del-user")) {
        return opDelUser(rest);
    }
    if (action == QLatin1String("kill-user")) {
        return opKillUser(rest);
    }
    if (action == QLatin1String("add-group")) {
        return opAddGroup(rest);
    }
    if (action == QLatin1String("del-group")) {
        return opDelGroup(rest);
    }
    if (action == QLatin1String("rename-user")) {
        return opRenameUser(rest);
    }
    if (action == QLatin1String("remove-home-path")) {
        return opRemoveHomePath(rest);
    }
    if (action == QLatin1String("chown-home")) {
        return opChownHome(rest);
    }
    if (action == QLatin1String("rewrite-home-paths")) {
        return opRewriteHomePaths(rest);
    }
    if (action == QLatin1String("clean-openoffice-locks")) {
        return opCleanOpenOfficeLocks(rest);
    }
    if (action == QLatin1String("rsync-copy")) {
        return opRsyncCopy(rest);
    }
    if (action == QLatin1String("autologin-enable")) {
        return opAutologinEnable(rest);
    }
    if (action == QLatin1String("autologin-disable")) {
        return opAutologinDisable(rest);
    }
    if (action == QLatin1String("autologin-remove-user")) {
        return opAutologinRemoveUser(rest);
    }
    if (action == QLatin1String("autologin-rename")) {
        return opAutologinRename(rest);
    }

    printError(QString("Unsupported helper action: %1").arg(action));
    return 1;
}

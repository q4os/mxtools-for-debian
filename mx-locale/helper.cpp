#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringList>
#include <QTextStream>

#include "common.h"

#include <algorithm>
#include <unistd.h>

namespace {
using ExecutableResolution::resolveExecutable;
using LocaleValidation::isSafeLocaleGenLine;
using LocaleValidation::isSafeLocaleToken;

bool fail(const QString &message)
{
    QTextStream(stderr) << message << Qt::endl;
    return false;
}

bool isSafeLocaleKey(const QString &value)
{
    static const QRegularExpression regex(R"(^[A-Z_]+$)");
    return regex.match(value).hasMatch();
}

bool isSafePackageName(const QString &value)
{
    static const QRegularExpression regex(R"(^[a-z0-9][a-z0-9+.-]*$)");
    return regex.match(value).hasMatch();
}

bool parseLocaleAssignment(const QString &line, QString &key, QString &value)
{
    static const QRegularExpression regex(
        R"re(^\s*(?:export\s+)?([A-Z_]+)\s*=\s*(?:"([^"]*)"|'([^']*)'|([^#\s]+))\s*(?:#.*)?$)re");
    const auto match = regex.match(line);
    if (!match.hasMatch()) {
        return false;
    }
    key = match.captured(1);
    value = match.captured(2);
    if (value.isEmpty()) {
        value = match.captured(3);
    }
    if (value.isEmpty()) {
        value = match.captured(4);
    }
    return isSafeLocaleToken(value);
}

QString resolveWritablePath(const QString &path)
{
    QFileInfo info(path);
    if (info.isSymLink()) {
        return info.symLinkTarget();
    }
    return path;
}

bool readLines(const QString &path, QStringList &lines, bool missingOk = false)
{
    lines.clear();
    const QString resolvedPath = resolveWritablePath(path);
    QFile file(resolvedPath);
    if (!file.exists()) {
        return missingOk ? true : fail(QObject::tr("Missing file: %1").arg(resolvedPath));
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return fail(QObject::tr("Could not open %1").arg(resolvedPath));
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return true;
}

bool writeFileAtomic(const QString &path, const QByteArray &data)
{
    const QString resolvedPath = resolveWritablePath(path);
    QSaveFile file(resolvedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return fail(QObject::tr("Could not open %1 for writing").arg(resolvedPath));
    }
    if (file.write(data) != data.size()) {
        return fail(QObject::tr("Could not write %1").arg(resolvedPath));
    }
    if (!file.commit()) {
        return fail(QObject::tr("Could not commit %1").arg(resolvedPath));
    }
    QFile::setPermissions(resolvedPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup
                                            | QFileDevice::ReadOther);
    return true;
}

bool writeLinesAtomic(const QString &path, const QStringList &lines)
{
    QByteArray data;
    for (const QString &line : lines) {
        data += line.toUtf8();
        data += '\n';
    }
    return writeFileAtomic(path, data);
}

bool runExternal(const QString &program, const QStringList &arguments)
{
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C.UTF-8");
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start(resolveExecutable(program), arguments);
    if (!process.waitForStarted()) {
        return fail(QObject::tr("Could not start %1").arg(program));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(-1)) {
        return fail(QObject::tr("Could not finish %1").arg(program));
    }
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool ensureDefaultLocaleSymlink()
{
    const QString localeConf = QStringLiteral("/etc/locale.conf");
    const QString relativeTarget = QDir(QFileInfo(Paths::defaultLocale).path()).relativeFilePath(localeConf);
    QFileInfo info(Paths::defaultLocale);
    if (info.isSymLink() && info.symLinkTarget() == localeConf) {
        return true;
    }

    QFile::remove(Paths::defaultLocale);
    if (::symlink(relativeTarget.toLocal8Bit().constData(), Paths::defaultLocale.toLocal8Bit().constData()) != 0) {
        return fail(QObject::tr("Could not recreate %1").arg(Paths::defaultLocale));
    }
    return true;
}

bool setLocaleValues(const QStringList &arguments)
{
    if (arguments.size() != 2 || !isSafeLocaleKey(arguments.at(0)) || !isSafeLocaleToken(arguments.at(1))) {
        return fail(QObject::tr("Invalid locale assignment"));
    }

    const QString assignment = arguments.at(0) + "=" + arguments.at(1);
#ifdef MX_LOCALE_ARCH
    return runExternal("localectl", {"set-locale", assignment});
#else
    return runExternal("update-locale", {assignment});
#endif
}

bool resetSubvariables(const QStringList &arguments)
{
    if (arguments.size() != 1 || !isSafeLocaleToken(arguments.at(0))) {
        return fail(QObject::tr("Invalid locale value"));
    }

#ifdef MX_LOCALE_ARCH
    QStringList localeArguments {"set-locale", "LANG=" + arguments.at(0)};
    const QStringList categories {"LC_CTYPE", "LC_NUMERIC", "LC_TIME", "LC_COLLATE", "LC_MONETARY", "LC_MESSAGES",
                                  "LC_PAPER", "LC_NAME", "LC_ADDRESS", "LC_TELEPHONE", "LC_MEASUREMENT",
                                  "LC_IDENTIFICATION"};
    for (const QString &category : categories) {
        localeArguments.append(category + '=' + arguments.at(0));
    }
    return runExternal("localectl", localeArguments);
#else
    const QString localeConf = QStringLiteral("/etc/locale.conf");
    const QString targetPath = QFile::exists(localeConf) ? localeConf : Paths::defaultLocale;
    if (!writeLinesAtomic(targetPath, {"LANG=" + arguments.at(0)})) {
        return false;
    }
    if (targetPath == localeConf) {
        return ensureDefaultLocaleSymlink();
    }
    return true;
#endif
}

bool filterLocaleGen(const QStringList &arguments)
{
    if (arguments.isEmpty() || arguments.size() > 2 || !isSafeLocaleToken(arguments.at(0))
        || (arguments.size() == 2 && !isSafeLocaleToken(arguments.at(1)))) {
        return fail(QObject::tr("Invalid locale value"));
    }

    QStringList lines;
    if (!readLines(Paths::localeGen, lines)) {
        return false;
    }

    QStringList localeSettings;
    if (!readLines(Paths::defaultLocale, localeSettings, true)) {
        return false;
    }
    QStringList protectedLocales = arguments;
    for (const QString &line : std::as_const(localeSettings)) {
        QString key;
        QString value;
        if (parseLocaleAssignment(line, key, value) && key.startsWith("LC_")) {
            protectedLocales.append(value);
        }
    }

    QStringList updatedLines;
    bool changed = false;

    for (const QString &line : std::as_const(lines)) {
        const QString trimmed = line.trimmed();
        const bool commented = trimmed.startsWith('#');
        const QString body = commented ? trimmed.mid(1).trimmed() : trimmed;
        if (trimmed.isEmpty() || commented
            || std::any_of(protectedLocales.cbegin(), protectedLocales.cend(), [&body](const QString &locale) {
                   return LocaleValidation::localeMatchesGenLine(locale, body);
               })) {
            updatedLines.append(line);
            continue;
        }

        const QString commentedLine = "# " + trimmed;
        updatedLines.append(commentedLine);
        changed |= line != commentedLine;
    }

    return !changed || writeLinesAtomic(Paths::localeGen, updatedLines);
}

bool enableLocale(const QStringList &arguments)
{
    if (arguments.size() != 1 || !isSafeLocaleGenLine(arguments.at(0))) {
        return fail(QObject::tr("Invalid locale value"));
    }

    QStringList lines;
    if (!readLines(Paths::localeGen, lines)) {
        return false;
    }

    const QString entry = arguments.at(0).simplified();
    QStringList updatedLines;
    bool found = false;
    bool changed = false;

    for (const QString &line : std::as_const(lines)) {
        const QString trimmed = line.trimmed();
        const bool commented = trimmed.startsWith('#');
        const QString body = commented ? trimmed.mid(1).trimmed() : trimmed;
        if (body.simplified() == entry) {
            if (!found) {
                updatedLines.append(entry);
                found = true;
                changed |= line != entry;
            } else {
                changed = true;
            }
            continue;
        }
        updatedLines.append(line);
    }

    if (!found) {
        updatedLines.append(entry);
        changed = true;
    }

    return !changed || writeLinesAtomic(Paths::localeGen, updatedLines);
}

bool disableLocale(const QStringList &arguments)
{
    if (arguments.size() != 1 || !isSafeLocaleGenLine(arguments.at(0))) {
        return fail(QObject::tr("Invalid locale value"));
    }

    const QString entry = arguments.at(0).simplified();
    QStringList localeGenLines;
    if (!readLines(Paths::localeGen, localeGenLines)) {
        return false;
    }

    QStringList updatedLocaleGen;
    bool handled = false;
    bool localeGenChanged = false;
    for (const QString &line : std::as_const(localeGenLines)) {
        const QString trimmed = line.trimmed();
        const bool commented = trimmed.startsWith('#');
        const QString body = commented ? trimmed.mid(1).trimmed() : trimmed;
        if (body.simplified() == entry) {
            if (!handled) {
                const QString commentedLine = "# " + entry;
                updatedLocaleGen.append(commentedLine);
                handled = true;
                localeGenChanged |= line != commentedLine;
            } else {
                localeGenChanged = true;
            }
            continue;
        }
        updatedLocaleGen.append(line);
    }

    if (localeGenChanged && !writeLinesAtomic(Paths::localeGen, updatedLocaleGen)) {
        return false;
    }

    QStringList localeSettings;
    if (!readLines(Paths::defaultLocale, localeSettings, true)) {
        return false;
    }

    QStringList updatedSettings;
    bool settingsChanged = false;
    for (const QString &line : std::as_const(localeSettings)) {
        QString key;
        QString value;
        if (parseLocaleAssignment(line, key, value) && LocaleValidation::localeMatchesGenLine(value, entry)) {
            settingsChanged = true;
            continue;
        }
        updatedSettings.append(line);
    }

    return !settingsChanged || writeLinesAtomic(Paths::defaultLocale, updatedSettings);
}

bool runLocaleGen(const QStringList &arguments)
{
    if (!arguments.isEmpty()) {
        return fail(QObject::tr("run-locale-gen does not accept arguments"));
    }
    return runExternal("locale-gen", {});
}

bool purgePackages(const QStringList &arguments)
{
    if (arguments.isEmpty()) {
        return true;
    }
    for (const QString &packageName : arguments) {
        if (!isSafePackageName(packageName)) {
            return fail(QObject::tr("Invalid package name"));
        }
    }

    QStringList purgeArguments {"purge", "-y"};
    purgeArguments += arguments;
    return runExternal("apt-get", purgeArguments);
}

bool resetLocaleGen(const QStringList &arguments)
{
    if (!arguments.isEmpty()) {
        return fail(QObject::tr("reset-locale-gen does not accept arguments"));
    }

    QFile sourceFile(QDir(Paths::mxLocaleLib).filePath("locale.gen"));
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return fail(QObject::tr("Could not open %1").arg(sourceFile.fileName()));
    }
    return writeFileAtomic(Paths::localeGen, sourceFile.readAll());
}

bool noop(const QStringList &arguments)
{
    if (!arguments.isEmpty()) {
        return fail(QObject::tr("noop does not accept arguments"));
    }
    return true;
}
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QStringList arguments = app.arguments().mid(1);

    if (getuid() != 0) {
        fail(QObject::tr("This helper must run as root"));
        return EXIT_FAILURE;
    }
    if (arguments.isEmpty()) {
        fail(QObject::tr("Missing command"));
        return EXIT_FAILURE;
    }

    const QString command = arguments.first();
    const QStringList commandArguments = arguments.mid(1);

    const bool ok = [&]() {
        if (command == "set-locale") {
            return setLocaleValues(commandArguments);
        }
        if (command == "reset-subvariables") {
            return resetSubvariables(commandArguments);
        }
        if (command == "filter-locale-gen") {
            return filterLocaleGen(commandArguments);
        }
        if (command == "enable-locale") {
            return enableLocale(commandArguments);
        }
        if (command == "disable-locale") {
            return disableLocale(commandArguments);
        }
        if (command == "run-locale-gen") {
            return runLocaleGen(commandArguments);
        }
        if (command == "purge-packages") {
            return purgePackages(commandArguments);
        }
        if (command == "reset-locale-gen") {
            return resetLocaleGen(commandArguments);
        }
        if (command == "noop") {
            return noop(commandArguments);
        }
        return fail(QObject::tr("Unknown command: %1").arg(command));
    }();

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include "aptcache.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>

#include "versionnumber.h"

AptCache::AptCache()
    : arch(getArch())
{
    if (!isDirValid()) {
        qWarning() << "APT cache directory is not valid:" << dir.path();
        return;
    }

    loadCacheFiles();
    parseContent();
}

void AptCache::loadCacheFiles()
{
    // Regex expressions to match package files
    static const QRegularExpression allBinaryArchRegex(QString(R"(^.*binary-%1_Packages$)").arg(arch));
    static const QRegularExpression allBinaryAnyRegex(R"(^.*binary-[a-z0-9]+_Packages$)");
    static const QRegularExpression allRegex(R"(^.*_Packages$)");

    // Exclusion patterns for Debian backports and MX testrepo and temp repositories
    static const QStringList exclusionPatterns
        = {R"(debian_.*-backports_.*_Packages)", R"(mx_testrepo.*_test_.*_Packages)", R"(mx_repo.*_temp_.*_Packages)"};
    static const QRegularExpression excludeRegex(exclusionPatterns.join('|'));

    QDirIterator it(dir.path(), QDir::Files);
    while (it.hasNext()) {
        const QString fileName = it.next();

        // Filter files by extension first to reduce unnecessary regex matches
        if (!fileName.endsWith("_Packages")) {
            continue;
        }

        const bool isBinaryArchMatch = allBinaryArchRegex.match(fileName).hasMatch();
        const bool isBinaryAnyMismatch = !allBinaryAnyRegex.match(fileName).hasMatch();
        const bool isAllMatch = allRegex.match(fileName).hasMatch();
        const bool isExcluded = excludeRegex.match(fileName).hasMatch();

        if ((isBinaryArchMatch || (isBinaryAnyMismatch && isAllMatch)) && !isExcluded) {
            if (!readFile(fileName)) {
                qWarning() << "Error reading cache file:" << fileName << "-"
                           << QFile(dir.absoluteFilePath(fileName)).errorString();
            }
        }
    }
}

QMap<QString, PackageInfo> AptCache::getCandidates() const
{
    return candidates;
}

// Return DEB_BUILD_ARCH format which differs from what 'arch' or currentCpuArchitecture return
QString AptCache::getArch()
{
    return arch_names.value(QSysInfo::currentCpuArchitecture(), QStringLiteral("unknown"));
}

bool AptCache::isDirValid() const
{
    return dir.exists() && dir.isReadable();
}

void AptCache::parseContent()
{
    if (filesContent.isEmpty()) {
        return;
    }

    // Create a regex to match the current architecture or 'all'
    const QRegularExpression reArch(QStringLiteral(".*(%1|all).*").arg(arch));

    QTextStream stream(&filesContent);
    QString line;
    QString package;
    QString version;
    QString description;
    QString architecture;
    bool isArchMatched = false;

    constexpr QLatin1String packageStr("Package:");
    constexpr QLatin1String archStr("Architecture:");
    constexpr QLatin1String versionStr("Version:");
    constexpr QLatin1String descStr("Description:");
    constexpr int packageSize = packageStr.size();
    constexpr int archSize = archStr.size();
    constexpr int versionSize = versionStr.size();
    constexpr int descSize = descStr.size();

    // Assumes the "Description:" line is the last field in the package data block
    while (stream.readLineInto(&line)) {
        if (line.startsWith(packageStr)) {
            package = line.mid(packageSize).trimmed();
            // Reset state for new package
            version.clear();
            description.clear();
            isArchMatched = false;
        } else if (line.startsWith(archStr)) {
            architecture = line.mid(archSize).trimmed();
            isArchMatched = reArch.match(architecture).hasMatch();
        } else if (line.startsWith(versionStr)) {
            version = line.mid(versionSize).trimmed();
        } else if (line.startsWith(descStr)) {
            description = line.mid(descSize).trimmed();
            if (isArchMatched && !package.isEmpty() && !version.isEmpty()) {
                updateCandidate(package, version, description);
            }
        }
    }
    filesContent.clear();
}

void AptCache::updateCandidate(const QString &package, const QString &version, const QString &description)
{
    auto it = candidates.find(package);
    if (it == candidates.end()) {
        candidates.insert(package, {version, description});
    } else {
        const VersionNumber currentVersion(it->version);
        const VersionNumber newVersion(version);
        if (currentVersion < newVersion) {
            it->version = version;
            it->description = description;
        }
    }
}

bool AptCache::readFile(const QString &fileName)
{
    QFile file(dir.absoluteFilePath(fileName));
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Could not open file:" << file.fileName() << "-" << file.errorString();
        return false;
    }

    QByteArray fileContent = file.readAll();
    if (!fileContent.isEmpty()) {
        if (!filesContent.isEmpty()) {
            filesContent.append('\n');
        }
        filesContent.append(QString::fromUtf8(fileContent));
    }
    file.close();
    return true;
}

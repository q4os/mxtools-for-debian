#include "aptcache.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStringView>

#include "versionnumber.h"

AptCache::AptCache()
    : arch(getArch())
{
    if (!isDirValid()) {
        qWarning() << "APT cache directory is not valid:" << dir.path();
        return;
    }

    // Pre-allocate map capacity for expected ~70,000 packages to reduce rehashing
    candidates.reserve(70000);

    loadCacheFiles();
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

const QHash<QString, PackageInfo>& AptCache::getCandidates() const
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

    QString content;
    const qint64 fileSize = file.size();

    // Use memory mapping for files larger than 10MB to avoid large memory copies
    if (fileSize > 10 * 1024 * 1024) {
        uchar* mappedData = file.map(0, fileSize);
        if (mappedData) {
            content = QString::fromUtf8(reinterpret_cast<const char*>(mappedData), fileSize);
            file.unmap(mappedData);
        } else {
            // Fall back to regular read if mapping fails
            QByteArray fileContent = file.readAll();
            content = QString::fromUtf8(fileContent);
        }
    } else {
        QByteArray fileContent = file.readAll();
        content = QString::fromUtf8(fileContent);
    }

    if (!content.isEmpty()) {
        parseFileContent(content);
    }
    file.close();
    return true;
}

void AptCache::parseFileContent(const QString &content)
{

    QTextStream stream(const_cast<QString*>(&content));
    QString line;
    QString package;
    QString version;
    QString description;
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
        QStringView lineView(line);
        if (lineView.startsWith(packageStr)) {
            QStringView packageView = lineView.mid(packageSize).trimmed();
            package = packageView.toString();
            // Reset state for new package
            version.clear();
            description.clear();
            isArchMatched = false;
        } else if (lineView.startsWith(archStr)) {
            QStringView archView = lineView.mid(archSize).trimmed();
            isArchMatched = (archView == arch || archView == QLatin1String("all"));
        } else if (lineView.startsWith(versionStr)) {
            QStringView versionView = lineView.mid(versionSize).trimmed();
            version = versionView.toString();
        } else if (lineView.startsWith(descStr)) {
            QStringView descView = lineView.mid(descSize).trimmed();
            description = descView.toString();
            if (isArchMatched && !package.isEmpty() && !version.isEmpty()) {
                updateCandidate(package, version, description);
            }
        }
    }
}

#include "aptcache.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>

#include "versionnumber.h"

AptCache::AptCache()
{
    loadCacheFiles();
    parseContent();
}

void AptCache::loadCacheFiles()
{
    // Exclude Debian backports and MX testrepo and temp repos
    const QString arch = getArch();

    // Define include and exclude regex patterns
    const QRegularExpression allBinaryArchRegex(QString(R"(^.*binary-%1_Packages$)").arg(arch));
    const QRegularExpression allBinaryAnyRegex(R"(^.*binary-[a-z0-9]+_Packages$)");
    const QRegularExpression allRegex(R"(^.*_Packages$)");

    const QRegularExpression excludeRegex(
        R"((debian_.*-backports_.*_Packages)|(mx_testrepo.*_test_.*_Packages)|(mx_repo.*_temp_.*_Packages))");

    QDirIterator it(dir.path(), QDir::Files);
    while (it.hasNext()) {
        const QString fileName = it.next();
        if ((allBinaryArchRegex.match(fileName).hasMatch() ||
            (!allBinaryAnyRegex.match(fileName).hasMatch() && allRegex.match(fileName).hasMatch()))
            && !excludeRegex.match(fileName).hasMatch()) {
            if (!readFile(fileName)) {
                qWarning() << "Error reading cache file:" << fileName;
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
    static const QString arch = arch_names.value(QSysInfo::currentCpuArchitecture());
    return arch;
}

void AptCache::parseContent()
{
    QTextStream stream(&files_content);
    QString line;
    QString package;
    QString version;
    QString description;
    QString architecture;

    const QString arch = getArch();
    static const QRegularExpression re_arch(QStringLiteral(".*(%1|all).*").arg(arch));
    bool match_arch = false;

    static const QLatin1String packageStr("Package:");
    static const QLatin1String archStr("Architecture:");
    static const QLatin1String versionStr("Version:");
    static const QLatin1String descStr("Description:");
    static constexpr int packageSize = packageStr.size();
    static constexpr int archSize = archStr.size();
    static constexpr int versionSize = versionStr.size();
    static constexpr int descSize = descStr.size();

    // Code assumes Description: is the last matched line
    while (stream.readLineInto(&line)) {
        if (line.startsWith(packageStr)) {
            package = line.mid(packageSize).trimmed();
            // Reset state for new package
            version.clear();
            description.clear();
            match_arch = false;
        } else if (line.startsWith(archStr)) {
            architecture = line.mid(archSize).trimmed();
            match_arch = re_arch.match(architecture).hasMatch();
        } else if (line.startsWith(versionStr)) {
            version = line.mid(versionSize).trimmed();
        } else if (line.startsWith(descStr)) {
            description = line.mid(descSize).trimmed();
            if (match_arch && !package.isEmpty() && !version.isEmpty()) {
                auto it = candidates.find(package);
                if (it == candidates.end()) {
                    candidates.insert(package, {version, description});
                } else if (VersionNumber(it->version) < VersionNumber(version)) {
                    it->version = version;
                    it->description = description;
                }
            }
        }
    }
    files_content.clear();
}

bool AptCache::readFile(const QString &file_name)
{
    QFile file(dir.absoluteFilePath(file_name));
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Could not open file: " << file.fileName();
        return false;
    }
    files_content += QLatin1String("\n") + file.readAll();
    file.close();
    return true;
}

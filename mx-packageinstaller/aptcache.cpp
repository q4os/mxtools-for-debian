#include "aptcache.h"

#include <QDebug>
#include <QDirIterator>
#include <QRegularExpression>

#include "versionnumber.h"

AptCache::AptCache()
{
    loadCacheFiles();
}

void AptCache::loadCacheFiles()
{
    // Exclude Debian backports and MX testrepo and temp repos
    const QRegularExpression packagesFilter("(.*binary-" + getArch()
                                            + "_Packages)|"
                                              "(.*binary-.*_Packages(?!.*debian_.*-backports_.*_Packages)"
                                              "(?!.*mx_testrepo.*_test_.*_Packages)"
                                              "(?!.*mx_repo.*_temp_.*_Packages))");
    QStringList matchingFiles;
    const QStringList files = QDir(dir).entryList(QDir::Files);
    for (const QString &fileName : qAsConst(files)) {
        if (packagesFilter.match(fileName).hasMatch()) {
            matchingFiles.append(fileName);
        }
    }
    for (const QString &fileName : qAsConst(matchingFiles)) {
        if (!readFile(fileName)) {
            qDebug() << "error reading a cache file";
        }
    }
    parseContent();
}

QMap<QString, PackageInfo> AptCache::getCandidates() const
{
    return candidates;
}

// Return DEB_BUILD_ARCH format which differs from what 'arch' or currentCpuArchitecture return
QString AptCache::getArch()
{
    return arch_names.value(QSysInfo::currentCpuArchitecture());
}

void AptCache::parseContent()
{
    const QStringList list = files_content.split('\n');

    QStringRef package;
    QStringRef version;
    QStringRef description;
    QStringRef architecture;

    const QRegularExpression re_arch(".*(" + getArch() + "|all).*");
    bool match_arch = false;

    // Code assumes Description: is the last matched line
    for (const QString &line : list) {
        if (line.startsWith(QLatin1String("Package:"))) {
            package = line.midRef(9);
        } else if (line.startsWith(QLatin1String("Architecture:"))) {
            architecture = line.midRef(14).trimmed();
            match_arch = re_arch.match(architecture).hasMatch();
        } else if (line.startsWith(QLatin1String("Version:"))) {
            version = line.midRef(9);
        } else if (line.startsWith(QLatin1String("Description:"))) {
            description = line.midRef(13).trimmed();
            if (match_arch) {
                if (candidates.constFind(package.toString()) == candidates.constEnd()
                    || VersionNumber(candidates.value(package.toString()).version)
                           < VersionNumber(version.toString())) {
                    candidates.insert(package.toString(), {version.toString(), description.toString()});
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
    files_content += file.readAll();
    file.close();
    return true;
}

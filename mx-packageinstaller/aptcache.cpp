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
    QTextStream stream(&files_content);
    QString line;

    QString package;
    QString version;
    QString description;
    QString architecture;

    const QRegularExpression re_arch(".*(" + getArch() + "|all).*");
    bool match_arch = false;

    // Code assumes Description: is the last matched line
    while (stream.readLineInto(&line)) {
        if (line.startsWith(QLatin1String("Package:"))) {
            package = line.mid(9);
        } else if (line.startsWith(QLatin1String("Architecture:"))) {
            architecture = line.mid(14).trimmed();
            match_arch = re_arch.match(architecture).hasMatch();
        } else if (line.startsWith(QLatin1String("Version:"))) {
            version = line.mid(9);
        } else if (line.startsWith(QLatin1String("Description:"))) {
            description = line.mid(13).trimmed();
            if (match_arch) {
                auto it = candidates.constFind(package);
                if (it == candidates.constEnd() || VersionNumber(it.value().version) < VersionNumber(version)) {
                    candidates.insert(package, {version, description});
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

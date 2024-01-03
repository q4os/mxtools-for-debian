#pragma once

#include <QDir>
#include <QMap>

// Pair of arch names returned by QSysInfo::currentCpuArchitecture() and corresponding DEB_BUILD_ARCH formats
static const QMap<QString, QString> arch_names {
    {"x86_64", "amd64"}, {"i386", "i386"}, {"arm", "armhf"}, {"arm64", "arm64"}};

struct PackageInfo {
    QString version;
    QString description;
};

class AptCache
{
public:
    AptCache();

    QMap<QString, PackageInfo> getCandidates() const;
    static QString getArch();

private:
    QMap<QString, PackageInfo> candidates;
    QString files_content;
    const QDir dir {"/var/lib/apt/lists/"};

    bool readFile(const QString &file_name);
    void loadCacheFiles();
    void parseContent();
};

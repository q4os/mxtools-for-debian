#pragma once

#include <QDir>
#include <QMap>
#include <QString>

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
    QString arch;
    QString filesContent;
    const QDir dir {"/var/lib/apt/lists/"};

    bool isDirValid() const;
    bool readFile(const QString &fileName);
    void loadCacheFiles();
    void parseContent();
    void updateCandidate(const QString &package, const QString &version, const QString &description);
};

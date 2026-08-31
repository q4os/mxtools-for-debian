#pragma once

#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

#include "aptcache.h"

namespace PackageListParser
{
// Debian policy 5.6.1: lowercase alphanumerics plus '+', '-', '.', at least two
// characters, starting with an alphanumeric. Names from downloaded package lists
// are interpolated into shell commands and apt-get argv, so reject anything else.
inline bool isValidPackageName(const QString &name)
{
    static const QRegularExpression validName(QStringLiteral("^[a-z0-9][a-z0-9+.-]+$"));
    return validName.match(name).hasMatch();
}

inline QHash<QString, PackageInfo> parse(const QString &content)
{
    QHash<QString, PackageInfo> packages;
    QString input = content;
    QTextStream stream(&input);
    QString line, package, version, description;
    const auto finalize = [&] {
        if (!package.isEmpty() && !version.isEmpty()) {
            packages.insert(package, {version, description});
        }
    };

    while (stream.readLineInto(&line)) {
        if (line.startsWith(QLatin1String("Package: "))) {
            finalize();
            package = line.section(' ', 1);
            if (!isValidPackageName(package)) {
                package.clear();
            }
            version.clear();
            description.clear();
        } else if (line.startsWith(QLatin1String("Version: "))) {
            version = line.section(' ', 1);
        } else if (line.startsWith(QLatin1String("Description: "))) {
            description = line.section(' ', 1);
        }
    }
    finalize();
    return packages;
}
} // namespace PackageListParser

#pragma once

#include <QStringList>

namespace utils
{

struct KernelFiles {
    QString vmlinuz;
    QString initrd;
    QString amdUcode;
    QString intelUcode;
};

KernelFiles resolveKernelFiles(const QString &sourceDir, const QString &kernelVersion, bool isFrugal);
QStringList sortKernelVersions(const QStringList &kernelFiles, bool reverse = true);
QString extractDiskFromPartition(const QString &partition);

} // namespace utils

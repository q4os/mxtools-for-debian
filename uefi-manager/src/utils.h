#pragma once

#include <QStringList>

namespace utils
{

QStringList sortKernelVersions(const QStringList &kernelFiles, bool reverse = true);
QString extractDiskFromPartition(const QString &partition);

} // namespace utils

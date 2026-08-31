#include "utils.h"

#include <QFile>
#include <QRegularExpression>
#include <algorithm>

namespace utils
{

KernelFiles resolveKernelFiles(const QString &sourceDir, const QString &kernelVersion, bool isFrugal)
{
    KernelFiles result;

    // vmlinuz: try versioned first, fall back to Arch-style vmlinuz-linux
    result.vmlinuz = QString("%1/vmlinuz%2").arg(sourceDir, isFrugal ? "" : "-" + kernelVersion);
    if (!QFile::exists(result.vmlinuz)) {
        result.vmlinuz = QString("%1/vmlinuz-linux").arg(sourceDir);
    }

    // initrd: try initrd pattern first, then initramfs, then Arch fallback
    const QString initrdPath = QString("%1/initrd%2").arg(sourceDir, isFrugal ? ".gz" : ".img-" + kernelVersion);
    const QString initramfsPath = QString("%1/initramfs-%2").arg(sourceDir, isFrugal ? "" : kernelVersion + ".img");

    if (QFile::exists(initrdPath)) {
        result.initrd = initrdPath;
    } else if (QFile::exists(initramfsPath)) {
        result.initrd = initramfsPath;
    } else {
        const QString archFallback = QString("%1/initramfs-linux.img").arg(sourceDir);
        result.initrd = QFile::exists(archFallback) ? archFallback : initramfsPath;
    }

    // microcode images (may not exist)
    result.amdUcode = QString("%1/amd-ucode.img").arg(sourceDir);
    result.intelUcode = QString("%1/intel-ucode.img").arg(sourceDir);

    return result;
}

QStringList sortKernelVersions(const QStringList &kernelFiles, bool reverse)
{
    static const QRegularExpression regex(R"((\d+)\.(\d+)(?:\.(\d+))?(-([a-z0-9]+[^-]*)?)?(-.*)?)");

    auto versionCompare = [reverse](const QString &a, const QString &b) {
        QRegularExpressionMatch matchA = regex.match(a);
        QRegularExpressionMatch matchB = regex.match(b);

        if (!matchA.hasMatch() && !matchB.hasMatch()) {
            return reverse ? a > b : a < b;
        }
        if (!matchA.hasMatch()) {
            return reverse;
        }
        if (!matchB.hasMatch()) {
            return !reverse;
        }

        int majorA = matchA.captured(1).toInt();
        int minorA = matchA.captured(2).toInt();
        int patchA = matchA.captured(3).isEmpty() ? 0 : matchA.captured(3).toInt();

        int majorB = matchB.captured(1).toInt();
        int minorB = matchB.captured(2).toInt();
        int patchB = matchB.captured(3).isEmpty() ? 0 : matchB.captured(3).toInt();

        if (majorA != majorB) {
            return reverse ? majorA > majorB : majorA < majorB;
        }
        if (minorA != minorB) {
            return reverse ? minorA > minorB : minorA < minorB;
        }
        if (patchA != patchB) {
            return reverse ? patchA > patchB : patchA < patchB;
        }

        return reverse ? matchA.captured(4) > matchB.captured(4) : matchA.captured(4) < matchB.captured(4);
    };

    QStringList sortedList = kernelFiles;
    std::sort(sortedList.begin(), sortedList.end(), versionCompare);
    return sortedList;
}

QString extractDiskFromPartition(const QString &partition)
{
    // NVMe/MMC partitions have the form nvme0n1p2 / mmcblk0p1 — strip the p\d+ suffix
    static const QRegularExpression nvmeRegex("^((?:nvme|mmcblk).+)p\\d+$");
    QRegularExpressionMatch nvmeMatch = nvmeRegex.match(partition);
    if (nvmeMatch.hasMatch()) {
        return nvmeMatch.captured(1);
    }

    // If it's an nvme/mmcblk name without a partition suffix, return as-is
    if (partition.startsWith("nvme") || partition.startsWith("mmcblk")) {
        return partition;
    }

    // Standard drives (sda1, vda3, xvda1) — strip trailing digits
    static const QRegularExpression regex("^(.*?)(\\d+)$");
    QRegularExpressionMatch match = regex.match(partition);
    return match.hasMatch() ? match.captured(1) : partition;
}

} // namespace utils

#include <QTest>
#include "utils.h"

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    void sortKernelVersions_descending();
    void sortKernelVersions_ascending();
    void sortKernelVersions_mixedFormats();
    void sortKernelVersions_unmatchedFallback();
    void sortKernelVersions_singleElement();
    void sortKernelVersions_empty();

    void extractDisk_sata();
    void extractDisk_nvme();
    void extractDisk_mmc();
    void extractDisk_virtio();
    void extractDisk_wholeDisk();
};

void TestUtils::sortKernelVersions_descending()
{
    QStringList input = {"vmlinuz-5.10.0", "vmlinuz-6.1.0", "vmlinuz-5.15.0"};
    QStringList result = utils::sortKernelVersions(input, true);
    QCOMPARE(result, QStringList({"vmlinuz-6.1.0", "vmlinuz-5.15.0", "vmlinuz-5.10.0"}));
}

void TestUtils::sortKernelVersions_ascending()
{
    QStringList input = {"vmlinuz-5.10.0", "vmlinuz-6.1.0", "vmlinuz-5.15.0"};
    QStringList result = utils::sortKernelVersions(input, false);
    QCOMPARE(result, QStringList({"vmlinuz-5.10.0", "vmlinuz-5.15.0", "vmlinuz-6.1.0"}));
}

void TestUtils::sortKernelVersions_mixedFormats()
{
    QStringList input = {"vmlinuz-6.6.87.2-microsoft-standard-WSL2", "vmlinuz-6.1.0-2-amd64", "vmlinuz-5.10.0"};
    QStringList result = utils::sortKernelVersions(input, true);
    // 6.6.87 > 6.1.0 > 5.10.0
    QCOMPARE(result.first(), QString("vmlinuz-6.6.87.2-microsoft-standard-WSL2"));
    QCOMPARE(result.last(), QString("vmlinuz-5.10.0"));
}

void TestUtils::sortKernelVersions_unmatchedFallback()
{
    QStringList input = {"zzz-noversion", "aaa-noversion"};
    QStringList result = utils::sortKernelVersions(input, true);
    QCOMPARE(result, QStringList({"zzz-noversion", "aaa-noversion"}));

    result = utils::sortKernelVersions(input, false);
    QCOMPARE(result, QStringList({"aaa-noversion", "zzz-noversion"}));
}

void TestUtils::sortKernelVersions_singleElement()
{
    QStringList input = {"vmlinuz-6.1.0"};
    QCOMPARE(utils::sortKernelVersions(input), input);
}

void TestUtils::sortKernelVersions_empty()
{
    QStringList input;
    QCOMPARE(utils::sortKernelVersions(input), QStringList());
}

void TestUtils::extractDisk_sata()
{
    QCOMPARE(utils::extractDiskFromPartition("sda1"), QString("sda"));
    QCOMPARE(utils::extractDiskFromPartition("sdb3"), QString("sdb"));
}

void TestUtils::extractDisk_nvme()
{
    QCOMPARE(utils::extractDiskFromPartition("nvme0n1p2"), QString("nvme0n1"));
    QCOMPARE(utils::extractDiskFromPartition("nvme1n1p1"), QString("nvme1n1"));
}

void TestUtils::extractDisk_mmc()
{
    QCOMPARE(utils::extractDiskFromPartition("mmcblk0p1"), QString("mmcblk0"));
}

void TestUtils::extractDisk_virtio()
{
    QCOMPARE(utils::extractDiskFromPartition("vda3"), QString("vda"));
    QCOMPARE(utils::extractDiskFromPartition("xvda1"), QString("xvda"));
}

void TestUtils::extractDisk_wholeDisk()
{
    QCOMPARE(utils::extractDiskFromPartition("sda"), QString("sda"));
    QCOMPARE(utils::extractDiskFromPartition("nvme0n1"), QString("nvme0n1"));
    QCOMPARE(utils::extractDiskFromPartition("mmcblk0"), QString("mmcblk0"));
}

QTEST_MAIN(TestUtils)
#include "test_utils.moc"

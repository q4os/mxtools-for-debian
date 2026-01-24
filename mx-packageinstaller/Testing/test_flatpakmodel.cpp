#include <QtTest>
#include <QSignalSpy>
#include "../src/models/flatpakmodel.h"

// Use the same Status values as flatpakmodel.cpp
namespace FPStatus
{
enum { Installed = 1, Upgradable, NotInstalled, Autoremovable };
}

class TestFlatpakModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Basic model tests
    void testEmptyModel();
    void testSetFlatpakData();
    void testAddFlatpak();
    void testRowColumnCount();

    // Data retrieval tests
    void testDataDisplayRole();
    void testDataCheckStateRole();
    void testDataUserRole();

    // Check state tests
    void testCheckStateChange();
    void testCheckStateSignal();
    void testSetAllChecked();
    void testCheckedPackages();
    void testSetCheckedForVisible();

    // Flatpak lookup tests
    void testFindFlatpakRow();
    void testFlatpakAt();
    void testFlatpakAtInvalidRow();

    // Duplicate handling tests
    void testMarkDuplicates();
    void testNoDuplicates();

    // Status tests
    void testUpdateInstalledStatus();
    void testSetInstalledSizes();

    // Model reset tests
    void testClear();

    // Flags tests
    void testItemFlags();

    // Header tests
    void testHeaderData();

private:
    QVector<FlatpakData> createTestFlatpaks();
};

void TestFlatpakModel::initTestCase()
{
    qDebug() << "Starting FlatpakModel tests";
}

void TestFlatpakModel::cleanupTestCase()
{
    qDebug() << "Finished FlatpakModel tests";
}

QVector<FlatpakData> TestFlatpakModel::createTestFlatpaks()
{
    QVector<FlatpakData> flatpaks;

    FlatpakData fp1;
    fp1.shortName = "GIMP";
    fp1.longName = "org.gimp.GIMP";
    fp1.version = "2.10.34";
    fp1.branch = "stable";
    fp1.size = "100 MB";
    fp1.fullName = "org.gimp.GIMP/x86_64/stable";
    fp1.canonicalRef = "app/org.gimp.GIMP/x86_64/stable";
    fp1.status = FPStatus::NotInstalled;
    flatpaks.append(fp1);

    FlatpakData fp2;
    fp2.shortName = "Firefox";
    fp2.longName = "org.mozilla.firefox";
    fp2.version = "120.0";
    fp2.branch = "stable";
    fp2.size = "200 MB";
    fp2.fullName = "org.mozilla.firefox/x86_64/stable";
    fp2.canonicalRef = "app/org.mozilla.firefox/x86_64/stable";
    fp2.status = FPStatus::Installed;
    flatpaks.append(fp2);

    FlatpakData fp3;
    fp3.shortName = "VLC";
    fp3.longName = "org.videolan.VLC";
    fp3.version = "3.0.18";
    fp3.branch = "stable";
    fp3.size = "150 MB";
    fp3.fullName = "org.videolan.VLC/x86_64/stable";
    fp3.canonicalRef = "app/org.videolan.VLC/x86_64/stable";
    fp3.status = FPStatus::NotInstalled;
    flatpaks.append(fp3);

    return flatpaks;
}

void TestFlatpakModel::testEmptyModel()
{
    FlatpakModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), FlatCol::FullName + 1);
    QVERIFY(model.checkedPackages().isEmpty());
}

void TestFlatpakModel::testSetFlatpakData()
{
    FlatpakModel model;
    QVector<FlatpakData> flatpaks = createTestFlatpaks();

    model.setFlatpakData(flatpaks);

    QCOMPARE(model.rowCount(), 3);
}

void TestFlatpakModel::testAddFlatpak()
{
    FlatpakModel model;

    QCOMPARE(model.rowCount(), 0);

    FlatpakData fp;
    fp.shortName = "Test";
    fp.longName = "org.test.Test";
    fp.canonicalRef = "app/org.test.Test/x86_64/stable";
    fp.fullName = "org.test.Test/x86_64/stable";

    model.addFlatpak(fp);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.findFlatpakRow("app/org.test.Test/x86_64/stable"), 0);
}

void TestFlatpakModel::testRowColumnCount()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.columnCount(), FlatCol::FullName + 1);

    // Parent should return 0 for table models
    QModelIndex parent = model.index(0, 0);
    QCOMPARE(model.rowCount(parent), 0);
    QCOMPARE(model.columnCount(parent), 0);
}

void TestFlatpakModel::testDataDisplayRole()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Test name column
    QModelIndex nameIndex = model.index(0, FlatCol::Name);
    QCOMPARE(model.data(nameIndex, Qt::DisplayRole).toString(), QString("GIMP"));

    // Test long name column
    QModelIndex longNameIndex = model.index(0, FlatCol::LongName);
    QCOMPARE(model.data(longNameIndex, Qt::DisplayRole).toString(), QString("org.gimp.GIMP"));

    // Test version column
    QModelIndex versionIndex = model.index(0, FlatCol::Version);
    QCOMPARE(model.data(versionIndex, Qt::DisplayRole).toString(), QString("2.10.34"));

    // Test branch column
    QModelIndex branchIndex = model.index(0, FlatCol::Branch);
    QCOMPARE(model.data(branchIndex, Qt::DisplayRole).toString(), QString("stable"));

    // Test size column
    QModelIndex sizeIndex = model.index(0, FlatCol::Size);
    QCOMPARE(model.data(sizeIndex, Qt::DisplayRole).toString(), QString("100 MB"));
}

void TestFlatpakModel::testDataCheckStateRole()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QModelIndex checkIndex = model.index(0, FlatCol::Check);
    QCOMPARE(model.data(checkIndex, Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Unchecked));

    // Non-check column should return empty
    QModelIndex nameIndex = model.index(0, FlatCol::Name);
    QVERIFY(!model.data(nameIndex, Qt::CheckStateRole).isValid());
}

void TestFlatpakModel::testDataUserRole()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Status column
    QModelIndex statusIndex = model.index(1, FlatCol::Status);
    QCOMPARE(model.data(statusIndex, Qt::UserRole).toInt(), FPStatus::Installed);

    // Duplicate column
    QModelIndex dupIndex = model.index(0, FlatCol::Duplicate);
    QCOMPARE(model.data(dupIndex, Qt::UserRole).toBool(), false);

    // FullName column
    QModelIndex fullNameIndex = model.index(0, FlatCol::FullName);
    QCOMPARE(model.data(fullNameIndex, Qt::UserRole).toString(),
             QString("org.gimp.GIMP/x86_64/stable"));

    // CanonicalRef via UserRole+1
    QCOMPARE(model.data(fullNameIndex, Qt::UserRole + 1).toString(),
             QString("app/org.gimp.GIMP/x86_64/stable"));
}

void TestFlatpakModel::testCheckStateChange()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QModelIndex checkIndex = model.index(0, FlatCol::Check);

    // Initially unchecked
    QCOMPARE(model.data(checkIndex, Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Unchecked));

    // Set to checked
    bool result = model.setData(checkIndex, Qt::Checked, Qt::CheckStateRole);
    QVERIFY(result);
    QCOMPARE(model.data(checkIndex, Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Checked));

    // Set back to unchecked
    result = model.setData(checkIndex, Qt::Unchecked, Qt::CheckStateRole);
    QVERIFY(result);
    QCOMPARE(model.data(checkIndex, Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Unchecked));
}

void TestFlatpakModel::testCheckStateSignal()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QSignalSpy spy(&model, &FlatpakModel::checkStateChanged);
    QVERIFY(spy.isValid());

    QModelIndex checkIndex = model.index(0, FlatCol::Check);
    model.setData(checkIndex, Qt::Checked, Qt::CheckStateRole);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("org.gimp.GIMP/x86_64/stable"));
    QCOMPARE(args.at(1).value<Qt::CheckState>(), Qt::Checked);
    QCOMPARE(args.at(2).toInt(), FPStatus::NotInstalled);
}

void TestFlatpakModel::testSetAllChecked()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Check all
    model.setAllChecked(true);

    for (int i = 0; i < model.rowCount(); ++i) {
        QModelIndex idx = model.index(i, FlatCol::Check);
        QCOMPARE(model.data(idx, Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Checked));
    }

    // Uncheck all
    model.setAllChecked(false);

    for (int i = 0; i < model.rowCount(); ++i) {
        QModelIndex idx = model.index(i, FlatCol::Check);
        QCOMPARE(model.data(idx, Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Unchecked));
    }
}

void TestFlatpakModel::testCheckedPackages()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Initially empty
    QVERIFY(model.checkedPackages().isEmpty());

    // Check first and third flatpaks
    model.setData(model.index(0, FlatCol::Check), Qt::Checked, Qt::CheckStateRole);
    model.setData(model.index(2, FlatCol::Check), Qt::Checked, Qt::CheckStateRole);

    QStringList checked = model.checkedPackages();
    QCOMPARE(checked.size(), 2);
    QVERIFY(checked.contains("org.gimp.GIMP/x86_64/stable"));
    QVERIFY(checked.contains("org.videolan.VLC/x86_64/stable"));
    QVERIFY(!checked.contains("org.mozilla.firefox/x86_64/stable"));
}

void TestFlatpakModel::testSetCheckedForVisible()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Check only rows 0 and 2
    QVector<int> visibleRows = {0, 2};
    model.setCheckedForVisible(visibleRows, true);

    QCOMPARE(model.data(model.index(0, FlatCol::Check), Qt::CheckStateRole).toInt(),
             static_cast<int>(Qt::Checked));
    QCOMPARE(model.data(model.index(1, FlatCol::Check), Qt::CheckStateRole).toInt(),
             static_cast<int>(Qt::Unchecked));
    QCOMPARE(model.data(model.index(2, FlatCol::Check), Qt::CheckStateRole).toInt(),
             static_cast<int>(Qt::Checked));
}

void TestFlatpakModel::testFindFlatpakRow()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QCOMPARE(model.findFlatpakRow("app/org.gimp.GIMP/x86_64/stable"), 0);
    QCOMPARE(model.findFlatpakRow("app/org.mozilla.firefox/x86_64/stable"), 1);
    QCOMPARE(model.findFlatpakRow("app/org.videolan.VLC/x86_64/stable"), 2);
    QCOMPARE(model.findFlatpakRow("nonexistent"), -1);
}

void TestFlatpakModel::testFlatpakAt()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    const FlatpakData *fp = model.flatpakAt(0);
    QVERIFY(fp != nullptr);
    QCOMPARE(fp->shortName, QString("GIMP"));
    QCOMPARE(fp->longName, QString("org.gimp.GIMP"));

    const FlatpakData *fp2 = model.flatpakAt(2);
    QVERIFY(fp2 != nullptr);
    QCOMPARE(fp2->shortName, QString("VLC"));
}

void TestFlatpakModel::testFlatpakAtInvalidRow()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QVERIFY(model.flatpakAt(-1) == nullptr);
    QVERIFY(model.flatpakAt(100) == nullptr);
}

void TestFlatpakModel::testMarkDuplicates()
{
    FlatpakModel model;

    QVector<FlatpakData> flatpaks;

    // Same canonical ref - should be marked as duplicates
    FlatpakData fp1;
    fp1.shortName = "GIMP";
    fp1.canonicalRef = "app/org.gimp.GIMP/x86_64/stable";
    fp1.fullName = "org.gimp.GIMP/x86_64/stable (flathub)";
    flatpaks.append(fp1);

    FlatpakData fp2;
    fp2.shortName = "GIMP";
    fp2.canonicalRef = "app/org.gimp.GIMP/x86_64/stable";  // Same ref
    fp2.fullName = "org.gimp.GIMP/x86_64/stable (other)";
    flatpaks.append(fp2);

    FlatpakData fp3;
    fp3.shortName = "Firefox";
    fp3.canonicalRef = "app/org.mozilla.firefox/x86_64/stable";  // Different ref
    fp3.fullName = "org.mozilla.firefox/x86_64/stable";
    flatpaks.append(fp3);

    model.setFlatpakData(flatpaks);
    model.markDuplicates();

    // First two should be duplicates
    QVERIFY(model.flatpakAt(0)->isDuplicate);
    QVERIFY(model.flatpakAt(1)->isDuplicate);

    // Third should not be a duplicate
    QVERIFY(!model.flatpakAt(2)->isDuplicate);
}

void TestFlatpakModel::testNoDuplicates()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());
    model.markDuplicates();

    // None should be duplicates in the standard test data
    for (int i = 0; i < model.rowCount(); ++i) {
        QVERIFY(!model.flatpakAt(i)->isDuplicate);
    }
}

void TestFlatpakModel::testUpdateInstalledStatus()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Initially: GIMP not installed, Firefox installed, VLC not installed
    QCOMPARE(model.flatpakAt(0)->status, FPStatus::NotInstalled);
    QCOMPARE(model.flatpakAt(1)->status, FPStatus::Installed);
    QCOMPARE(model.flatpakAt(2)->status, FPStatus::NotInstalled);

    // Update: GIMP now installed, Firefox uninstalled
    QStringList installedRefs = {"app/org.gimp.GIMP/x86_64/stable", "app/org.videolan.VLC/x86_64/stable"};
    model.updateInstalledStatus(installedRefs);

    QCOMPARE(model.flatpakAt(0)->status, FPStatus::Installed);  // Now installed
    QCOMPARE(model.flatpakAt(1)->status, FPStatus::NotInstalled);  // Now not installed
    QCOMPARE(model.flatpakAt(2)->status, FPStatus::Installed);  // Now installed
}

void TestFlatpakModel::testSetInstalledSizes()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Update sizes
    QHash<QString, QString> sizeMap;
    sizeMap["app/org.gimp.GIMP/x86_64/stable"] = "250 MB";
    sizeMap["app/org.mozilla.firefox/x86_64/stable"] = "180 MB";

    model.setInstalledSizes(sizeMap);

    QCOMPARE(model.flatpakAt(0)->size, QString("250 MB"));
    QCOMPARE(model.flatpakAt(1)->size, QString("180 MB"));
    QCOMPARE(model.flatpakAt(2)->size, QString("150 MB"));  // Unchanged
}

void TestFlatpakModel::testClear()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    QCOMPARE(model.rowCount(), 3);

    model.clear();

    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.checkedPackages().isEmpty());
    QCOMPARE(model.findFlatpakRow("app/org.gimp.GIMP/x86_64/stable"), -1);
}

void TestFlatpakModel::testItemFlags()
{
    FlatpakModel model;
    model.setFlatpakData(createTestFlatpaks());

    // Check column should be checkable
    QModelIndex checkIndex = model.index(0, FlatCol::Check);
    Qt::ItemFlags checkFlags = model.flags(checkIndex);
    QVERIFY(checkFlags & Qt::ItemIsEnabled);
    QVERIFY(checkFlags & Qt::ItemIsSelectable);
    QVERIFY(checkFlags & Qt::ItemIsUserCheckable);

    // Name column should not be checkable
    QModelIndex nameIndex = model.index(0, FlatCol::Name);
    Qt::ItemFlags nameFlags = model.flags(nameIndex);
    QVERIFY(nameFlags & Qt::ItemIsEnabled);
    QVERIFY(nameFlags & Qt::ItemIsSelectable);
    QVERIFY(!(nameFlags & Qt::ItemIsUserCheckable));

    // Invalid index should have no flags
    QModelIndex invalidIndex;
    QCOMPARE(model.flags(invalidIndex), Qt::NoItemFlags);
}

void TestFlatpakModel::testHeaderData()
{
    FlatpakModel model;

    // Check header labels
    QCOMPARE(model.headerData(FlatCol::Name, Qt::Horizontal, Qt::DisplayRole).toString(),
             QString("Package"));
    QCOMPARE(model.headerData(FlatCol::LongName, Qt::Horizontal, Qt::DisplayRole).toString(),
             QString("Full Name"));
    QCOMPARE(model.headerData(FlatCol::Version, Qt::Horizontal, Qt::DisplayRole).toString(),
             QString("Version"));
    QCOMPARE(model.headerData(FlatCol::Branch, Qt::Horizontal, Qt::DisplayRole).toString(),
             QString("Branch"));
    QCOMPARE(model.headerData(FlatCol::Size, Qt::Horizontal, Qt::DisplayRole).toString(),
             QString("Size"));

    // Check column should have empty header
    QVERIFY(model.headerData(FlatCol::Check, Qt::Horizontal, Qt::DisplayRole).toString().isEmpty());

    // Vertical orientation should return empty
    QVERIFY(!model.headerData(0, Qt::Vertical, Qt::DisplayRole).isValid());
}

QTEST_MAIN(TestFlatpakModel)
#include "test_flatpakmodel.moc"

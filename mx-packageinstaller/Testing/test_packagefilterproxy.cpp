#include <QtTest>
#include "../src/models/packagemodel.h"
#include "../src/models/packagefilterproxy.h"

class TestPackageFilterProxy : public QObject
{
    Q_OBJECT

private slots:
    void testHideLibraries();
    void testStatusFilter();
    void testSearchText();
    void testVisibleSourceRows();
    void testSortByCheckColumn();

private:
    QVector<PackageData> createPackages() const;
};

QVector<PackageData> TestPackageFilterProxy::createPackages() const
{
    QVector<PackageData> packages;

    PackageData pkg1;
    pkg1.name = "libfoo";
    pkg1.description = "Library package";
    pkg1.status = Status::Installed;
    packages.append(pkg1);

    PackageData pkg2;
    pkg2.name = "bar-dev";
    pkg2.description = "Development headers";
    pkg2.status = Status::NotInstalled;
    packages.append(pkg2);

    PackageData pkg3;
    pkg3.name = "vim";
    pkg3.description = "Vi IMproved editor";
    pkg3.status = Status::Upgradable;
    packages.append(pkg3);

    PackageData pkg4;
    pkg4.name = "firefox";
    pkg4.description = "Web browser";
    pkg4.status = Status::Installed;
    packages.append(pkg4);

    PackageData pkg5;
    pkg5.name = "librecad";
    pkg5.description = "Computer-aided design application";
    pkg5.status = Status::NotInstalled;
    packages.append(pkg5);

    PackageData pkg6;
    pkg6.name = "librecad-dev";
    pkg6.description = "LibreCAD development files";
    pkg6.status = Status::NotInstalled;
    packages.append(pkg6);

    return packages;
}

void TestPackageFilterProxy::testHideLibraries()
{
    PackageModel model;
    model.setPackageData(createPackages());

    PackageFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideLibraries(true);

#ifdef PACKAGE_BACKEND_PACMAN
    // isLibraryPackage() is a no-op on Arch (no lib*/-dev/-dbg*/-libs split-package
    // convention to filter on yet), so "hide libraries" hides nothing.
    QCOMPARE(proxy.rowCount(), 6);
#else
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.index(0, TreeCol::Name).data().toString(), QString("vim"));
    QCOMPARE(proxy.index(1, TreeCol::Name).data().toString(), QString("firefox"));
    QCOMPARE(proxy.index(2, TreeCol::Name).data().toString(), QString("librecad"));
#endif
}

void TestPackageFilterProxy::testStatusFilter()
{
    PackageModel model;
    model.setPackageData(createPackages());

    PackageFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideLibraries(false);
    proxy.setStatusFilter(Status::Upgradable);

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, TreeCol::Name).data().toString(), QString("vim"));
}

void TestPackageFilterProxy::testSearchText()
{
    PackageModel model;
    model.setPackageData(createPackages());

    PackageFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideLibraries(false);

    proxy.setSearchText("editor");
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, TreeCol::Name).data().toString(), QString("vim"));

    proxy.setSearchText("fox");
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, TreeCol::Name).data().toString(), QString("firefox"));
}

void TestPackageFilterProxy::testVisibleSourceRows()
{
    PackageModel model;
    model.setPackageData(createPackages());

    PackageFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideLibraries(true);

    QVector<int> rows = proxy.visibleSourceRows();
#ifdef PACKAGE_BACKEND_PACMAN
    // isLibraryPackage() is a no-op on Arch, so nothing gets hidden; see testHideLibraries().
    QCOMPARE(rows.size(), 6);
#else
    QCOMPARE(rows.size(), 3);
    QCOMPARE(rows.at(0), 2);
    QCOMPARE(rows.at(1), 3);
    QCOMPARE(rows.at(2), 4);
#endif
}

void TestPackageFilterProxy::testSortByCheckColumn()
{
    PackageModel model;
    model.setPackageData(createPackages());

    PackageFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideLibraries(false);
    proxy.sort(TreeCol::Check, Qt::AscendingOrder);

    // Status enum order is Installed(1) < Upgradable(2) < NotInstalled(3) < Autoremovable(4);
    // sorting the icon column should group rows by that status, not leave them unordered.
    QCOMPARE(proxy.rowCount(), 6);
    for (int i = 0; i < proxy.rowCount() - 1; ++i) {
        const int leftStatus = model.packageAt(proxy.mapToSource(proxy.index(i, TreeCol::Check)).row())->status;
        const int rightStatus = model.packageAt(proxy.mapToSource(proxy.index(i + 1, TreeCol::Check)).row())->status;
        QVERIFY(leftStatus <= rightStatus);
    }
    QCOMPARE(proxy.index(0, TreeCol::Name).data().toString(), QString("libfoo"));
    QCOMPARE(proxy.index(1, TreeCol::Name).data().toString(), QString("firefox"));
    QCOMPARE(proxy.index(2, TreeCol::Name).data().toString(), QString("vim"));
}

QTEST_MAIN(TestPackageFilterProxy)
#include "test_packagefilterproxy.moc"

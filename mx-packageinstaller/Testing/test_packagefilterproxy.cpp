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

    return packages;
}

void TestPackageFilterProxy::testHideLibraries()
{
    PackageModel model;
    model.setPackageData(createPackages());

    PackageFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideLibraries(true);

    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.index(0, TreeCol::Name).data().toString(), QString("vim"));
    QCOMPARE(proxy.index(1, TreeCol::Name).data().toString(), QString("firefox"));
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
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows.at(0), 2);
    QCOMPARE(rows.at(1), 3);
}

QTEST_MAIN(TestPackageFilterProxy)
#include "test_packagefilterproxy.moc"

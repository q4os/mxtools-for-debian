#include <QtTest>
#include "../src/models/flatpakmodel.h"
#include "../src/models/flatpakfilterproxy.h"

namespace FPStatus
{
enum { Installed = 1, Upgradable, NotInstalled, Autoremovable };
}

class TestFlatpakFilterProxy : public QObject
{
    Q_OBJECT

private slots:
    void testHideDuplicates();
    void testAllowedRefs();
    void testStatusFilter();
    void testSearchText();

private:
    QVector<FlatpakData> createFlatpaks() const;
};

QVector<FlatpakData> TestFlatpakFilterProxy::createFlatpaks() const
{
    QVector<FlatpakData> flatpaks;

    FlatpakData fp1;
    fp1.shortName = "GIMP";
    fp1.longName = "org.gimp.GIMP";
    fp1.canonicalRef = "app/org.gimp.GIMP/x86_64/stable";
    fp1.fullName = "org.gimp.GIMP/x86_64/stable";
    fp1.status = FPStatus::Installed;
    flatpaks.append(fp1);

    FlatpakData fp2;
    fp2.shortName = "GIMP";
    fp2.longName = "org.gimp.GIMP";
    fp2.canonicalRef = "app/org.gimp.GIMP/x86_64/stable";
    fp2.fullName = "org.gimp.GIMP/x86_64/stable";
    fp2.status = FPStatus::Installed;
    flatpaks.append(fp2);

    FlatpakData fp3;
    fp3.shortName = "Firefox";
    fp3.longName = "org.mozilla.firefox";
    fp3.canonicalRef = "app/org.mozilla.firefox/x86_64/stable";
    fp3.fullName = "org.mozilla.firefox/x86_64/stable";
    fp3.status = FPStatus::NotInstalled;
    flatpaks.append(fp3);

    return flatpaks;
}

void TestFlatpakFilterProxy::testHideDuplicates()
{
    FlatpakModel model;
    model.setFlatpakData(createFlatpaks());
    model.markDuplicates();

    FlatpakFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setHideDuplicates(true);

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, FlatCol::Name).data().toString(), QString("Firefox"));
}

void TestFlatpakFilterProxy::testAllowedRefs()
{
    FlatpakModel model;
    model.setFlatpakData(createFlatpaks());

    FlatpakFilterProxy proxy;
    proxy.setSourceModel(&model);

    QSet<QString> allowed {"app/org.mozilla.firefox/x86_64/stable"};
    proxy.setAllowedRefs(allowed);

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, FlatCol::Name).data().toString(), QString("Firefox"));
}

void TestFlatpakFilterProxy::testStatusFilter()
{
    FlatpakModel model;
    model.setFlatpakData(createFlatpaks());

    FlatpakFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setStatusFilter(FPStatus::Installed);

    QCOMPARE(proxy.rowCount(), 2);
}

void TestFlatpakFilterProxy::testSearchText()
{
    FlatpakModel model;
    model.setFlatpakData(createFlatpaks());

    FlatpakFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("mozilla");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, FlatCol::Name).data().toString(), QString("Firefox"));
}

QTEST_MAIN(TestFlatpakFilterProxy)
#include "test_flatpakfilterproxy.moc"

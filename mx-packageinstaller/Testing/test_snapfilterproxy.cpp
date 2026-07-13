#include <QtTest>
#include "../src/packagestatus.h"
#include "../src/models/snapmodel.h"
#include "../src/models/snapfilterproxy.h"

class TestSnapFilterProxy : public QObject
{
    Q_OBJECT

private slots:
    void testNoSourceModel();
    void testNoFilter();
    void testStatusFilter();
    void testSearchByName();
    void testSearchByPublisher();
    void testSearchByDescription();
    void testCaseInsensitiveSearch();
    void testSearchAndStatusCombined();
    void testClearingFilters();
    void testDynamicStatusFilterUpdates();

private:
    QVector<SnapData> createSnaps() const;
};

QVector<SnapData> TestSnapFilterProxy::createSnaps() const
{
    QVector<SnapData> snaps;

    SnapData s1;
    s1.name = "core";
    s1.publisher = "canonical";
    s1.description = "snapd runtime environment";
    s1.status = Status::Installed;
    snaps.append(s1);

    SnapData s2;
    s2.name = "code";
    s2.publisher = "vscode";
    s2.description = "Code editing. Redefined.";
    s2.status = Status::NotInstalled;
    snaps.append(s2);

    SnapData s3;
    s3.name = "vlc";
    s3.publisher = "videolan";
    s3.description = "The ultimate media player";
    s3.status = Status::NotInstalled;
    snaps.append(s3);

    return snaps;
}

void TestSnapFilterProxy::testNoSourceModel()
{
    SnapFilterProxy proxy;
    QCOMPARE(proxy.rowCount(), 0);

    proxy.setStatusFilter(Status::Installed);
    proxy.setSearchText("core");
    QCOMPARE(proxy.rowCount(), 0);
}

void TestSnapFilterProxy::testNoFilter()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);

    QCOMPARE(proxy.rowCount(), 3);
}

void TestSnapFilterProxy::testStatusFilter()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);

    proxy.setStatusFilter(Status::Installed);
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("core"));

    proxy.setStatusFilter(Status::NotInstalled);
    QCOMPARE(proxy.rowCount(), 2);

    proxy.setStatusFilter(0); // 0 = show all
    QCOMPARE(proxy.rowCount(), 3);
}

void TestSnapFilterProxy::testSearchByName()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("vlc");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("vlc"));
}

void TestSnapFilterProxy::testSearchByPublisher()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("videolan");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("vlc"));
}

void TestSnapFilterProxy::testSearchByDescription()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("editing");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("code"));
}

void TestSnapFilterProxy::testCaseInsensitiveSearch()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("CODE EDITING");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("code"));
}

void TestSnapFilterProxy::testSearchAndStatusCombined()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);

    // Not-installed snaps whose fields contain "o": code (vscode), vlc (videolan)
    proxy.setStatusFilter(Status::NotInstalled);
    proxy.setSearchText("video");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("vlc"));

    // A term that only matches an installed snap should yield nothing under NotInstalled
    proxy.setSearchText("canonical");
    QCOMPARE(proxy.rowCount(), 0);
}

void TestSnapFilterProxy::testClearingFilters()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setStatusFilter(Status::Installed);
    proxy.setSearchText("canonical");

    QCOMPARE(proxy.rowCount(), 1);

    proxy.setSearchText(QString());
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setStatusFilter(0);
    QCOMPARE(proxy.rowCount(), 3);
}

void TestSnapFilterProxy::testDynamicStatusFilterUpdates()
{
    SnapModel model;
    model.setSnapData(createSnaps());

    SnapFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setStatusFilter(Status::Installed);

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("core"));

    model.updateInstalledStatus({"core", "vlc"});
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("core"));
    QCOMPARE(proxy.index(1, SnapCol::Name).data().toString(), QString("vlc"));

    model.updateInstalledStatus({"vlc"});
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, SnapCol::Name).data().toString(), QString("vlc"));
}

QTEST_MAIN(TestSnapFilterProxy)
#include "test_snapfilterproxy.moc"

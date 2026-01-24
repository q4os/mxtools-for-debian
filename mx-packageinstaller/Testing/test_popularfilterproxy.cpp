#include <QtTest>
#include "../src/models/popularmodel.h"
#include "../src/models/popularfilterproxy.h"

class TestPopularFilterProxy : public QObject
{
    Q_OBJECT

private slots:
    void testEmptySearchShowsAllCategories();
    void testSearchFiltersCategories();
    void testSearchFiltersChildren();

private:
    QList<PopularAppData> createApps() const;
};

QList<PopularAppData> TestPopularFilterProxy::createApps() const
{
    QList<PopularAppData> apps;

    PopularAppData app1;
    app1.category = "Internet";
    app1.name = "Firefox";
    app1.description = "Web browser";
    apps.append(app1);

    PopularAppData app2;
    app2.category = "Internet";
    app2.name = "Chromium";
    app2.description = "Open source browser";
    apps.append(app2);

    PopularAppData app3;
    app3.category = "Graphics";
    app3.name = "GIMP";
    app3.description = "Image editor";
    apps.append(app3);

    return apps;
}

void TestPopularFilterProxy::testEmptySearchShowsAllCategories()
{
    PopularModel model;
    QIcon icon(QPixmap(1, 1));
    model.setIcons(icon, icon, icon);
    model.setPopularApps(createApps());

    PopularFilterProxy proxy;
    proxy.setSourceModel(&model);

    QCOMPARE(proxy.rowCount(), 2);
}

void TestPopularFilterProxy::testSearchFiltersCategories()
{
    PopularModel model;
    QIcon icon(QPixmap(1, 1));
    model.setIcons(icon, icon, icon);
    model.setPopularApps(createApps());

    PopularFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("browser");

    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.index(0, PopCol::Category).data().toString(), QString("Internet"));
}

void TestPopularFilterProxy::testSearchFiltersChildren()
{
    PopularModel model;
    QIcon icon(QPixmap(1, 1));
    model.setIcons(icon, icon, icon);
    model.setPopularApps(createApps());

    PopularFilterProxy proxy;
    proxy.setSourceModel(&model);
    proxy.setSearchText("Chromium");

    QModelIndex categoryIndex = proxy.index(0, PopCol::Category);
    QVERIFY(categoryIndex.isValid());
    QCOMPARE(proxy.rowCount(categoryIndex), 1);

    QModelIndex childIndex = proxy.index(0, PopCol::Name, categoryIndex);
    QCOMPARE(childIndex.data().toString(), QString("Chromium"));
}

QTEST_MAIN(TestPopularFilterProxy)
#include "test_popularfilterproxy.moc"

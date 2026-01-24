#include <QtTest>
#include "../src/models/popularmodel.h"

class TestPopularModel : public QObject
{
    Q_OBJECT

private slots:
    void testSetPopularApps();
    void testCheckedItems();
    void testUncheckAll();
    void testFindItemByName();

private:
    QList<PopularAppData> createApps() const;
};

QList<PopularAppData> TestPopularModel::createApps() const
{
    QList<PopularAppData> apps;

    PopularAppData app1;
    app1.category = "Internet";
    app1.name = "Firefox";
    app1.description = "Web browser";
    app1.installNames = "firefox";
    app1.uninstallNames = "firefox";
    app1.isInstalled = true;
    apps.append(app1);

    PopularAppData app2;
    app2.category = "Internet";
    app2.name = "Chromium";
    app2.description = "Open source browser";
    app2.installNames = "chromium";
    app2.uninstallNames = "chromium";
    app2.isInstalled = false;
    apps.append(app2);

    PopularAppData app3;
    app3.category = "Graphics";
    app3.name = "GIMP";
    app3.description = "Image editor";
    app3.installNames = "gimp";
    app3.uninstallNames = "gimp";
    app3.isInstalled = false;
    apps.append(app3);

    return apps;
}

void TestPopularModel::testSetPopularApps()
{
    PopularModel model;
    QIcon installedIcon(QPixmap(1, 1));
    QIcon folderIcon(QPixmap(1, 1));
    QIcon infoIcon(QPixmap(1, 1));
    model.setIcons(installedIcon, folderIcon, infoIcon);

    model.setPopularApps(createApps());

    QCOMPARE(model.rowCount(), 2);

    QModelIndex categoryIndex = model.index(0, PopCol::Category);
    QVERIFY(categoryIndex.isValid());
    QVERIFY(model.rowCount(categoryIndex) > 0);

    QModelIndex checkIndex = model.index(0, PopCol::Check, categoryIndex);
    QVERIFY(checkIndex.isValid());
    QVERIFY(model.flags(checkIndex).testFlag(Qt::ItemIsUserCheckable));
    QCOMPARE(checkIndex.data(Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Unchecked));

    QIcon installedItemIcon = qvariant_cast<QIcon>(checkIndex.data(Qt::DecorationRole));
    QVERIFY(!installedItemIcon.isNull());
}

void TestPopularModel::testCheckedItems()
{
    PopularModel model;
    QIcon installedIcon(QPixmap(1, 1));
    QIcon folderIcon(QPixmap(1, 1));
    QIcon infoIcon(QPixmap(1, 1));
    model.setIcons(installedIcon, folderIcon, infoIcon);

    model.setPopularApps(createApps());

    QModelIndex categoryIndex = model.index(0, PopCol::Category);
    QVERIFY(categoryIndex.isValid());

    QModelIndex checkIndex = model.index(1, PopCol::Check, categoryIndex);
    QVERIFY(checkIndex.isValid());
    QVERIFY(model.setData(checkIndex, Qt::Checked, Qt::CheckStateRole));

    QModelIndexList checkedItems = model.checkedItems();
    QCOMPARE(checkedItems.size(), 1);

    QStringList checkedNames = model.checkedPackageNames();
    QCOMPARE(checkedNames.size(), 1);
    QCOMPARE(checkedNames.at(0), QString("Chromium"));
}

void TestPopularModel::testUncheckAll()
{
    PopularModel model;
    QIcon installedIcon(QPixmap(1, 1));
    QIcon folderIcon(QPixmap(1, 1));
    QIcon infoIcon(QPixmap(1, 1));
    model.setIcons(installedIcon, folderIcon, infoIcon);

    model.setPopularApps(createApps());

    QModelIndex categoryIndex = model.index(0, PopCol::Category);
    QVERIFY(categoryIndex.isValid());

    QModelIndex checkIndex = model.index(0, PopCol::Check, categoryIndex);
    QVERIFY(checkIndex.isValid());
    QVERIFY(model.setData(checkIndex, Qt::Checked, Qt::CheckStateRole));

    model.uncheckAll();
    QCOMPARE(checkIndex.data(Qt::CheckStateRole).toInt(), static_cast<int>(Qt::Unchecked));
}

void TestPopularModel::testFindItemByName()
{
    PopularModel model;
    QIcon installedIcon(QPixmap(1, 1));
    QIcon folderIcon(QPixmap(1, 1));
    QIcon infoIcon(QPixmap(1, 1));
    model.setIcons(installedIcon, folderIcon, infoIcon);

    model.setPopularApps(createApps());

    QModelIndex checkIndex = model.findItemByName("GIMP");
    QVERIFY(checkIndex.isValid());
    QCOMPARE(checkIndex.column(), PopCol::Check);
}

QTEST_MAIN(TestPopularModel)
#include "test_popularmodel.moc"

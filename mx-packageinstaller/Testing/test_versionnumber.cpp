#include <QtTest>
#include <QDebug>
#include "src/versionnumber.h"

class TestVersionNumber : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic functionality tests
    void testConstructor();
    void testAssignment();
    void testToString();
    
    // Comparison tests
    void testBasicComparisons();
    void testEpochComparisons();
    void testDebianRevisionComparisons();
    void testComplexVersions();
    
    // Edge cases
    void testEmptyVersions();
    void testSpecialCharacters();
    void testTildeHandling();
    
    // Real-world Debian version examples
    void testRealDebianVersions();
    
    // Sorting tests
    void testVersionSorting();

private:
    void compareVersions(const QString &v1, const QString &v2, bool v1ShouldBeLess);
};

void TestVersionNumber::initTestCase()
{
    qDebug() << "Starting VersionNumber tests";
}

void TestVersionNumber::cleanupTestCase()
{
    qDebug() << "Finished VersionNumber tests";
}

void TestVersionNumber::testConstructor()
{
    VersionNumber v1;
    QCOMPARE(v1.toString(), QString());
    
    VersionNumber v2("1.0.0");
    QCOMPARE(v2.toString(), QString("1.0.0"));
    
    VersionNumber v3(v2);
    QCOMPARE(v3.toString(), QString("1.0.0"));
}

void TestVersionNumber::testAssignment()
{
    VersionNumber v1;
    v1 = QString("2.5.1");
    QCOMPARE(v1.toString(), QString("2.5.1"));
    
    VersionNumber v2;
    v2 = v1;
    QCOMPARE(v2.toString(), QString("2.5.1"));
}

void TestVersionNumber::testToString()
{
    VersionNumber v("1:2.3.4-5ubuntu1");
    QCOMPARE(v.toString(), QString("1:2.3.4-5ubuntu1"));
}

void TestVersionNumber::compareVersions(const QString &v1, const QString &v2, bool v1ShouldBeLess)
{
    VersionNumber ver1(v1);
    VersionNumber ver2(v2);
    
    if (v1ShouldBeLess) {
        QVERIFY2(ver1 < ver2, qPrintable(QString("%1 should be < %2").arg(v1, v2)));
        QVERIFY2(ver1 <= ver2, qPrintable(QString("%1 should be <= %2").arg(v1, v2)));
        QVERIFY2(!(ver1 > ver2), qPrintable(QString("%1 should not be > %2").arg(v1, v2)));
        QVERIFY2(!(ver1 >= ver2), qPrintable(QString("%1 should not be >= %2").arg(v1, v2)));
        QVERIFY2(ver1 != ver2, qPrintable(QString("%1 should != %2").arg(v1, v2)));
        QVERIFY2(!(ver1 == ver2), qPrintable(QString("%1 should not == %2").arg(v1, v2)));
    } else {
        QVERIFY2(!(ver1 < ver2), qPrintable(QString("%1 should not be < %2").arg(v1, v2)));
        QVERIFY2(!(ver1 <= ver2), qPrintable(QString("%1 should not be <= %2").arg(v1, v2)));
        QVERIFY2(ver1 > ver2, qPrintable(QString("%1 should be > %2").arg(v1, v2)));
        QVERIFY2(ver1 >= ver2, qPrintable(QString("%1 should be >= %2").arg(v1, v2)));
        QVERIFY2(ver1 != ver2, qPrintable(QString("%1 should != %2").arg(v1, v2)));
        QVERIFY2(!(ver1 == ver2), qPrintable(QString("%1 should not == %2").arg(v1, v2)));
    }
}

void TestVersionNumber::testBasicComparisons()
{
    // Simple numeric comparisons
    compareVersions("1.0", "2.0", true);
    compareVersions("1.1", "1.2", true);
    compareVersions("1.0.1", "1.0.2", true);
    compareVersions("2.0", "1.0", false);
    
    // Equal versions
    VersionNumber v1("1.0.0");
    VersionNumber v2("1.0.0");
    QVERIFY(v1 == v2);
    QVERIFY(v1 <= v2);
    QVERIFY(v1 >= v2);
    QVERIFY(!(v1 != v2));
    QVERIFY(!(v1 < v2));
    QVERIFY(!(v1 > v2));
}

void TestVersionNumber::testEpochComparisons()
{
    // Epoch tests - higher epoch always wins
    compareVersions("1:1.0", "2:0.1", true);
    compareVersions("0:2.0", "1:1.0", true);
    compareVersions("1.0", "1:0.1", true); // no epoch = epoch 0
    compareVersions("2:1.0", "1:2.0", false);
}

void TestVersionNumber::testDebianRevisionComparisons()
{
    // Debian revision tests
    compareVersions("1.0-1", "1.0-2", true);
    compareVersions("1.0", "1.0-1", true); // no revision < with revision
    compareVersions("1.0-2", "1.0-1", false);
    compareVersions("1.0-1ubuntu1", "1.0-1ubuntu2", true);
}

void TestVersionNumber::testComplexVersions()
{
    // Complex real-world examples
    compareVersions("1:2.3.4-5ubuntu1", "1:2.3.4-5ubuntu2", true);
    compareVersions("1:2.3.4-5ubuntu1", "2:1.0.0-1", true);
    compareVersions("1.0.0+dfsg-1", "1.0.0+dfsg-2", true);
    compareVersions("1.0~rc1-1", "1.0-1", true); // ~ sorts before everything
}

void TestVersionNumber::testEmptyVersions()
{
    VersionNumber empty1;
    VersionNumber empty2;
    VersionNumber nonEmpty("1.0");
    
    QVERIFY(empty1 == empty2);
    QVERIFY(empty1 < nonEmpty);
    QVERIFY(!(nonEmpty < empty1));
}

void TestVersionNumber::testSpecialCharacters()
{
    // Test various special characters
    compareVersions("1.0+dfsg", "1.0+dfsg.1", true);
    compareVersions("1.0.git20230101", "1.0.git20230102", true);
    compareVersions("1.0~alpha", "1.0~beta", true);
    compareVersions("1.0~alpha", "1.0", true);
}

void TestVersionNumber::testTildeHandling()
{
    // Tilde (~) has special meaning - it sorts before everything else
    compareVersions("1.0~rc1", "1.0~rc2", true);
    compareVersions("1.0~rc1", "1.0", true);
    compareVersions("1.0~", "1.0", true);
    compareVersions("1.0~a", "1.0~b", true);
}

void TestVersionNumber::testRealDebianVersions()
{
    // Real examples from Debian packages
    compareVersions("2.6.0-2", "2.6.0-10", true);
    compareVersions("1:7.4.052-1ubuntu3", "1:7.4.052-1ubuntu3.1", true);
    compareVersions("1.2.3-4+deb10u1", "1.2.3-4+deb10u2", true);
    compareVersions("5.4.0-42.46", "5.4.0-42.46+1", true);
    compareVersions("2.0.0~git20200101.abcdef-1", "2.0.0-1", true);
}

void TestVersionNumber::testVersionSorting()
{
    QList<QString> unsorted = {
        "1.0",
        "1:0.5",
        "2.0",
        "1.0-1",
        "1.0~rc1",
        "1.1",
        "1.0-2",
        "0.9"
    };
    
    QList<QString> expectedSorted = {
        "0.9",
        "1.0~rc1",
        "1.0",
        "1.0-1", 
        "1.0-2",
        "1.1",
        "2.0",
        "1:0.5"  // epoch 1 comes after epoch 0
    };
    
    // Convert to VersionNumber and sort
    QList<VersionNumber> versions;
    for (const QString &v : unsorted) {
        versions.append(VersionNumber(v));
    }
    
    std::sort(versions.begin(), versions.end());
    
    // Check if sorted correctly
    for (int i = 0; i < expectedSorted.size(); ++i) {
        QCOMPARE(versions[i].toString(), expectedSorted[i]);
    }
}

QTEST_MAIN(TestVersionNumber)
#include "test_versionnumber.moc"
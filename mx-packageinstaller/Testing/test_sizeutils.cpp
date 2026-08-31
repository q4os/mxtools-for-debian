#include <QtTest>

#include "../src/sizeutils.h"

class TestSizeUtils : public QObject
{
    Q_OBJECT

private slots:
    void testSizeStringToBytes();
    void testSortOrderAcrossUnits();
    void testUnrecognizedUnitFailsToParse();
    void testOverflowFailsToParse();
};

void TestSizeUtils::testSizeStringToBytes()
{
    QCOMPARE(SizeUtils::sizeStringToBytes("500 kB"), 500ULL * 1000ULL);
    QCOMPARE(SizeUtils::sizeStringToBytes("1,0 MB"), 1000ULL * 1000ULL);
    QCOMPARE(SizeUtils::sizeStringToBytes(QString("1,0") + QChar(0x00a0) + "GB"),
             1000ULL * 1000ULL * 1000ULL);
    QCOMPARE(SizeUtils::sizeStringToBytes("1.1 GB"),
             static_cast<quint64>(1.1 * 1000.0 * 1000.0 * 1000.0));
    QCOMPARE(SizeUtils::sizeStringToBytes("1 MiB"), 1024ULL * 1024ULL);
    QCOMPARE(SizeUtils::sizeStringToBytes("42 bytes"), 42ULL);
    QCOMPARE(SizeUtils::sizeStringToBytes(""), 0ULL);
    QCOMPARE(SizeUtils::sizeStringToBytes("?"), 0ULL);

    // A currently-supported unit should parse successfully and report ok == true.
    bool ok = false;
    QCOMPARE(SizeUtils::sizeStringToBytes("1 MiB", &ok), 1024ULL * 1024ULL);
    QVERIFY(ok);
}

void TestSizeUtils::testSortOrderAcrossUnits()
{
    const QStringList ascending {"500 kB", "1,0 MB", "100,0 MB", "1,0 GB", "1,1 GB"};
    for (int i = 1; i < ascending.size(); ++i) {
        QVERIFY2(SizeUtils::sizeStringToBytes(ascending.at(i - 1))
                     < SizeUtils::sizeStringToBytes(ascending.at(i)),
                 qPrintable(ascending.at(i - 1) + " should be smaller than " + ascending.at(i)));
    }
}

void TestSizeUtils::testUnrecognizedUnitFailsToParse()
{
    // "PB" is not a unit this function knows about. It must fail to parse
    // rather than silently falling back to a multiplier of 1 (which would
    // previously have turned "2 PB" into "2 bytes").
    bool ok = true;
    const quint64 bytes = SizeUtils::sizeStringToBytes("2 PB", &ok);
    QVERIFY(!ok);
    QCOMPARE(bytes, 0ULL);
}

void TestSizeUtils::testOverflowFailsToParse()
{
    // A value large enough that value * multiplier overflows quint64's range
    // must fail to parse rather than silently wrapping/truncating.
    bool ok = true;
    const quint64 bytes = SizeUtils::sizeStringToBytes("20000000 TiB", &ok);
    QVERIFY(!ok);
    QCOMPARE(bytes, 0ULL);
}

QTEST_MAIN(TestSizeUtils)
#include "test_sizeutils.moc"

#include <QtTest>

#include "src/packagelistparser.h"

class PackageListParserTest : public QObject
{
    Q_OBJECT

private slots:
    void missingVersionDoesNotBleedIntoNextStanza()
    {
        const auto packages = PackageListParser::parse(
            "Package: first\nVersion: 1\nDescription: old\n\n"
            "Package: missing-version\nDescription: skipped\n\n"
            "Package: second\nVersion: 2\nDescription: new\n");
        QCOMPARE(packages.value("first").version, QString("1"));
        QVERIFY(!packages.contains("missing-version"));
        QCOMPARE(packages.value("second").version, QString("2"));
    }

    void finalStanzaIsKeptWithoutDescription()
    {
        const auto packages = PackageListParser::parse("Package: final\nVersion: 3\n");
        QCOMPARE(packages.value("final").version, QString("3"));
    }

    void invalidPackageNamesAreSkipped()
    {
        const auto packages = PackageListParser::parse("Package: BAD!\nVersion: 1\nDescription: bad\n");
        QVERIFY(packages.isEmpty());
    }
};

QTEST_GUILESS_MAIN(PackageListParserTest)
#include "test_packagelistparser.moc"

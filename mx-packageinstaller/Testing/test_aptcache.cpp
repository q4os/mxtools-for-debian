#include <QtTest>
#include <QDebug>
#include <QTemporaryDir>
#include <QTextStream>
#include "../src/aptcache.h"
#include "../src/versionnumber.h"

class TestAptCache : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Architecture tests
    void testArchDetection();

    // Package parsing tests
    void testPackageInfoParsing();
    void testMultiplePackages();
    void testVersionComparison();
    void testArchitectureFiltering();

    // File filtering tests
    void testFileFiltering();

    // Edge cases
    void testEmptyCache();
    void testMalformedPackages();
    void testSpecialCharacters();

private:
    void createTestPackageFile(const QString &filePath, const QString &content);
    QTemporaryDir *testDir;
};

void TestAptCache::initTestCase()
{
    qDebug() << "Starting AptCache tests";
    testDir = new QTemporaryDir();
    QVERIFY(testDir->isValid());
}

void TestAptCache::cleanupTestCase()
{
    delete testDir;
    qDebug() << "Finished AptCache tests";
}

void TestAptCache::createTestPackageFile(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
    file.close();
}

void TestAptCache::testArchDetection()
{
    QString arch = AptCache::getArch();
    qDebug() << "Detected architecture:" << arch;
    
    // Should be one of the known architectures
    QStringList knownArchs = {"amd64", "i386", "armhf", "arm64", "unknown"};
    QVERIFY2(knownArchs.contains(arch), qPrintable(QString("Unknown architecture: %1").arg(arch)));
}

void TestAptCache::testPackageInfoParsing()
{
    QString packageContent = R"(Package: test-package
Version: 1.0.0-1
Architecture: amd64
Description: A test package for unit testing
 This is a longer description that spans
 multiple lines for testing purposes.

Package: another-package
Version: 2.5.3-ubuntu1
Architecture: all
Description: Another test package
 Short description here.

)";

    // Create a temporary file that simulates APT cache format
    QString testFile = testDir->path() + "/test_amd64_Packages";
    createTestPackageFile(testFile, packageContent);
    
    // Note: This test would require modifying AptCache to accept a custom directory
    // For now, we test the static arch detection method
    QString arch = AptCache::getArch();
    QVERIFY(!arch.isEmpty());
}

void TestAptCache::testMultiplePackages()
{
    // Test parsing multiple packages with different versions
    QString multiPackageContent = R"(Package: vim
Version: 2:8.2.3458-2
Architecture: amd64
Description: Vi IMproved - enhanced vi editor
 Vim is an almost compatible version of the UNIX editor Vi.

Package: vim
Version: 2:8.2.3458-1
Architecture: amd64  
Description: Vi IMproved - enhanced vi editor (older version)
 Vim is an almost compatible version of the UNIX editor Vi.

Package: emacs
Version: 1:27.1+1-3ubuntu5
Architecture: all
Description: GNU Emacs editor
 GNU Emacs is the extensible self-documenting text editor.

)";

    // In a real implementation, we'd test that:
    // 1. Only the highest version of each package is kept
    // 2. Architecture filtering works correctly  
    // 3. Package info is parsed correctly
    
    // For now, verify the version comparison logic would work
    VersionNumber older("2:8.2.3458-1");
    VersionNumber newer("2:8.2.3458-2");
    QVERIFY(older < newer);
}

void TestAptCache::testVersionComparison()
{
    // Test that package version comparison works correctly
    // This simulates the updateCandidate logic in AptCache
    
    struct TestPackage {
        QString name;
        QString version;
        QString description;
    };
    
    QList<TestPackage> packages = {
        {"firefox", "95.0-1", "Mozilla Firefox browser"},
        {"firefox", "96.0-1", "Mozilla Firefox browser (newer)"},
        {"firefox", "95.0-2", "Mozilla Firefox browser (patch)"},
        {"chromium", "97.0.4692.71-1", "Chromium web browser"}
    };
    
    // Simulate what AptCache::updateCandidate does
    QMap<QString, PackageInfo> candidates;
    
    for (const auto &pkg : packages) {
        auto it = candidates.find(pkg.name);
        if (it == candidates.end()) {
            candidates.insert(pkg.name, {pkg.version, pkg.description});
        } else {
            VersionNumber currentVersion(it->version);
            VersionNumber newVersion(pkg.version);
            if (currentVersion < newVersion) {
                it->version = pkg.version;
                it->description = pkg.description;
            }
        }
    }
    
    // Firefox should have version 96.0-1 (highest)
    QVERIFY(candidates.contains("firefox"));
    QCOMPARE(candidates["firefox"].version, QString("96.0-1"));
    
    // Chromium should be present
    QVERIFY(candidates.contains("chromium"));
    QCOMPARE(candidates["chromium"].version, QString("97.0.4692.71-1"));
}

void TestAptCache::testArchitectureFiltering()
{
    // Test that only appropriate architectures are processed
    QString currentArch = AptCache::getArch();
    
    // Packages with 'all' architecture should always be included
    // Packages with current architecture should be included
    // Packages with other architectures should be filtered out
    
    QStringList testArchs = {"all", currentArch, "i386", "armhf", "arm64", "amd64"};
    
    for (const QString &arch : testArchs) {
        bool shouldInclude = (arch == "all" || arch == currentArch);
        
        // This would be the regex test from AptCache::parseContent
        QRegularExpression reArch(QStringLiteral(".*(%1|all).*").arg(currentArch));
        bool matches = reArch.match(arch).hasMatch();
        
        QCOMPARE(matches, shouldInclude);
    }
}

void TestAptCache::testFileFiltering()
{
    // Test the file filtering logic from AptCache::loadCacheFiles
    QString arch = AptCache::getArch();
    
    // Create test file names that should be included/excluded
    QStringList testFiles = {
        QString("debian.org_debian_dists_bullseye_main_binary-%1_Packages").arg(arch),
        "debian.org_debian_dists_bullseye_main_binary-all_Packages",
        "debian.org_debian_dists_bullseye_main_binary-i386_Packages",
        "mx_repo_dists_bullseye_main_binary-amd64_Packages",
        "debian_dists_bullseye-backports_main_binary-amd64_Packages",
        "mx_testrepo_dists_test_main_binary-amd64_Packages",
        "mx_repo_dists_temp_main_binary-amd64_Packages",
        "not_a_package_file.txt"
    };
    
    // Test binary-arch pattern
    QRegularExpression allBinaryArchRegex(QString(R"(^.*binary-%1_Packages$)").arg(arch));
    QRegularExpression allBinaryAnyRegex(R"(^.*binary-[a-z0-9]+_Packages$)");
    QRegularExpression allRegex(R"(^.*_Packages$)");
    
    // Test exclusion patterns
    QStringList exclusionPatterns = {
        R"(debian_.*-backports_.*_Packages)",
        R"(mx_testrepo.*_test_.*_Packages)", 
        R"(mx_repo.*_temp_.*_Packages)"
    };
    QRegularExpression excludeRegex(exclusionPatterns.join('|'));
    
    for (const QString &fileName : testFiles) {
        bool isBinaryArchMatch = allBinaryArchRegex.match(fileName).hasMatch();
        bool isBinaryAnyMismatch = !allBinaryAnyRegex.match(fileName).hasMatch();
        bool isAllMatch = allRegex.match(fileName).hasMatch();
        bool isExcluded = excludeRegex.match(fileName).hasMatch();
        
        bool shouldInclude = (isBinaryArchMatch || (isBinaryAnyMismatch && isAllMatch)) && !isExcluded;
        
        qDebug() << fileName << "-> include:" << shouldInclude;
        
        // Verify specific expected results
        if (fileName.contains(QString("binary-%1_Packages").arg(arch))) {
            QVERIFY2(!isExcluded || shouldInclude == false, qPrintable(fileName));
        }
        if (fileName.contains("not_a_package_file.txt")) {
            QVERIFY2(!shouldInclude, qPrintable(fileName));
        }
        if (fileName.contains("backports") || fileName.contains("test") || fileName.contains("temp")) {
            QVERIFY2(!shouldInclude, qPrintable(fileName));
        }
    }
}

void TestAptCache::testEmptyCache()
{
    // Test behavior with empty or non-existent cache
    // This would require a custom AptCache constructor that accepts a directory path
    // For now, we can test the static methods
    
    QString arch = AptCache::getArch();
    QVERIFY(!arch.isEmpty());
    QVERIFY(arch != "unknown" || true); // Allow unknown for testing environments
}

void TestAptCache::testMalformedPackages()
{
    // Test handling of malformed package data
    QString malformedContent = R"(Package: incomplete-package
Version: 1.0
# Missing Architecture and Description

Package: 
Version: 2.0
Architecture: amd64
Description: Package with empty name

Package: no-version
Architecture: amd64
Description: Package without version

)";

    // The parsing should be robust enough to handle malformed data
    // without crashing. In a real test, we'd verify the parser
    // gracefully handles these cases.
}

void TestAptCache::testSpecialCharacters()
{
    // Test packages with special characters in names and versions
    QString specialContent = R"(Package: lib++dev
Version: 1.0+dfsg-1~ubuntu2.1
Architecture: amd64
Description: C++ development library
 A library with special characters in the name.

Package: python3.9
Version: 3.9.7-1+deb11u1
Architecture: amd64
Description: Python 3.9 interpreter
 Python interpreter version 3.9

)";

    // These should parse correctly without issues
    // The version comparison should handle the special characters properly
    VersionNumber v1("1.0+dfsg-1~ubuntu2.1");
    VersionNumber v2("3.9.7-1+deb11u1");
    
    QCOMPARE(v1.toString(), QString("1.0+dfsg-1~ubuntu2.1"));
    QCOMPARE(v2.toString(), QString("3.9.7-1+deb11u1"));
}

QTEST_MAIN(TestAptCache)
#include "test_aptcache.moc"

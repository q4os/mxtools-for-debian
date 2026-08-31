#include <QtTest>
#include <QDebug>

#include <dlfcn.h>

#include "../src/packagebackend.h"

// PackageBackend::compareVersions() on the pacman backend resolves libalpm's
// alpm_pkg_vercmp() via dlopen/dlsym at runtime (see packagebackend_pacman.cpp's
// doc comment for why: a per-comparison `vercmp` subprocess is far too slow for
// a full-repo sort, and there's no libalpm-dev/pkg-config file to build against
// directly in every environment this project is built in -- but the shared
// library itself is always present wherever pacman is, since pacman links
// against it). These tests exercise the real library directly rather than
// asserting against a hand-rolled port of its algorithm -- and since this
// sandbox/CI environment may not have libalpm at all, every test here skips
// (not fails) when it can't be resolved, via the same dlopen probe
// packagebackend_pacman.cpp itself uses.
class TestPackageBackendPacman : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testPlainNumeric();
    void testEpoch();
    void testPkgrel();
    void testUnmarkedSuffixVsTilde();

private:
    bool m_libalpmAvailable = false;
};

void TestPackageBackendPacman::initTestCase()
{
    void *handle = dlopen("libalpm.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        handle = dlopen("libalpm.so", RTLD_NOW);
    }
    if (!handle) {
        handle = dlopen("libalpm.so.15", RTLD_NOW);
    }
    m_libalpmAvailable = handle != nullptr && dlsym(handle, "alpm_pkg_vercmp") != nullptr;
    if (!m_libalpmAvailable) {
        qWarning("libalpm (or its alpm_pkg_vercmp symbol) not found -- all tests in this file "
                 "will skip. It's always present wherever pacman itself is, since pacman links "
                 "against it; this only affects environments (like some CI sandboxes) that don't "
                 "have it installed at all.");
    }
}

void TestPackageBackendPacman::testPlainNumeric()
{
    if (!m_libalpmAvailable) {
        QSKIP("libalpm not available");
    }

    QVERIFY(PackageBackend::compareVersions("1.0", "2.0") < 0);
    QVERIFY(PackageBackend::compareVersions("2.0", "1.0") > 0);
    QCOMPARE(PackageBackend::compareVersions("1.0", "1.0"), 0);
    QVERIFY(PackageBackend::compareVersions("1.0.0", "1.0") > 0); // extra component: newer
    QVERIFY(PackageBackend::compareVersions("1.9", "1.10") < 0);  // numeric, not lexical
    QCOMPARE(PackageBackend::compareVersions("06", "6"), 0);      // leading zeros don't matter
}

void TestPackageBackendPacman::testEpoch()
{
    if (!m_libalpmAvailable) {
        QSKIP("libalpm not available");
    }

    QVERIFY(PackageBackend::compareVersions("1:1.0", "2:0.1") < 0); // higher epoch always wins
    QVERIFY(PackageBackend::compareVersions("1:1.0", "1.0") > 0);   // no epoch means epoch 0
    QCOMPARE(PackageBackend::compareVersions("0:1.0", "1.0"), 0);   // explicit epoch 0 == no epoch
}

void TestPackageBackendPacman::testPkgrel()
{
    if (!m_libalpmAvailable) {
        QSKIP("libalpm not available");
    }

    QVERIFY(PackageBackend::compareVersions("1.0-1", "1.0-2") < 0);
    // pkgrel is only compared when BOTH sides have one.
    QCOMPARE(PackageBackend::compareVersions("1.0-1", "1.0"), 0);
    QVERIFY(PackageBackend::compareVersions("1.5.0-2", "1.5.0-10") < 0); // numeric pkgrel compare
}

void TestPackageBackendPacman::testUnmarkedSuffixVsTilde()
{
    if (!m_libalpmAvailable) {
        QSKIP("libalpm not available");
    }

    // The two headline cases this whole fix exists for:
    //   vercmp 1.0rc1 1.0   -> older  (unmarked suffix, like dpkg)
    //   vercmp 1.0~rc1 1.0  -> newer  (tilde-marked -- the opposite of dpkg's
    //                                  "~" sorts before everything)
    QVERIFY(PackageBackend::compareVersions("1.0rc1", "1.0") < 0);
    QVERIFY(PackageBackend::compareVersions("1.0~rc1", "1.0") > 0);
}

QTEST_MAIN(TestPackageBackendPacman)
#include "test_packagebackend_pacman.moc"

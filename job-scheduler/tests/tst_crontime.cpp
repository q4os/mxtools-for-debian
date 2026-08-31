#include "CronTime.h"

#include <QTest>

class TstCronTime : public QObject
{
    Q_OBJECT

private slots:
    void emptyStringIsInvalid();
    void outOfRangeFieldsAreInvalid_data();
    void outOfRangeFieldsAreInvalid();
    void validFieldsAreAccepted_data();
    void validFieldsAreAccepted();
    void namedSchedulesExpandCorrectly_data();
    void namedSchedulesExpandCorrectly();
    void dayOfMonthAndDayOfWeekMatchWithOr();
    void dayOfMonthOnlyMatchesWhenDayOfWeekIsWild();
};

void TstCronTime::emptyStringIsInvalid()
{
    CronTime t{QString()};
    QVERIFY(!t.isValid());
}

void TstCronTime::outOfRangeFieldsAreInvalid_data()
{
    QTest::addColumn<QString>("expr");

    QTest::newRow("minute too high") << "99 * * * *";
    QTest::newRow("hour too high") << "0 99 * * *";
    QTest::newRow("day-of-month too high") << "0 0 99 * *";
    QTest::newRow("month too high") << "0 0 1 99 *";
    QTest::newRow("day-of-week too high") << "0 0 * * 99";
    QTest::newRow("wrong field count") << "0 0 * *";
    QTest::newRow("range end below range start") << "10-5 * * * *";
    QTest::newRow("non-numeric field") << "abc * * * *";
}

void TstCronTime::outOfRangeFieldsAreInvalid()
{
    QFETCH(QString, expr);

    CronTime t(expr);
    QVERIFY(!t.isValid());
}

void TstCronTime::validFieldsAreAccepted_data()
{
    QTest::addColumn<QString>("expr");

    QTest::newRow("all wildcards") << "* * * * *";
    QTest::newRow("fixed fields") << "5 4 1 1 0";
    QTest::newRow("step values") << "*/15 * * * *";
    QTest::newRow("range") << "0 9-17 * * 1-5";
    QTest::newRow("list") << "0,30 * * * *";
    QTest::newRow("month name") << "0 0 1 jan *";
    QTest::newRow("day-of-week name") << "0 0 * * mon";
}

void TstCronTime::validFieldsAreAccepted()
{
    QFETCH(QString, expr);

    CronTime t(expr);
    QVERIFY(t.isValid());
}

void TstCronTime::namedSchedulesExpandCorrectly_data()
{
    QTest::addColumn<QString>("expr");
    QTest::addColumn<QString>("expected");

    QTest::newRow("@hourly") << "@hourly" << "0 * * * *";
    QTest::newRow("@daily") << "@daily" << "0 0 * * *";
    QTest::newRow("@midnight") << "@midnight" << "0 0 * * *";
    QTest::newRow("@weekly") << "@weekly" << "0 0 * * 0";
    QTest::newRow("@monthly") << "@monthly" << "0 0 1 * *";
    QTest::newRow("@yearly") << "@yearly" << "0 0 1 1 *";
    QTest::newRow("@annually") << "@annually" << "0 0 1 1 *";
}

void TstCronTime::namedSchedulesExpandCorrectly()
{
    QFETCH(QString, expr);
    QFETCH(QString, expected);

    CronTime t(expr);
    QVERIFY(t.isValid());
    QCOMPARE(t.toString(), expected);
}

void TstCronTime::dayOfMonthAndDayOfWeekMatchWithOr()
{
    // When both day-of-month and day-of-week are restricted, cron fires
    // when EITHER field matches. 2026-07-25 is a Saturday (dayOfWeek 6);
    // "1" for day-of-month never matches that date, so only the
    // day-of-week restriction ("sat") should cause a hit.
    CronTime t(QStringLiteral("0 0 1 * sat"));
    QVERIFY(t.isValid());

    QDateTime start(QDate(2026, 7, 24), QTime(0, 1));
    QDateTime next = t.getNextTime(start);

    QVERIFY(next.isValid());
    QCOMPARE(next.date(), QDate(2026, 7, 25));
    QCOMPARE(next.time(), QTime(0, 0));
}

void TstCronTime::dayOfMonthOnlyMatchesWhenDayOfWeekIsWild()
{
    CronTime t(QStringLiteral("0 0 15 * *"));
    QVERIFY(t.isValid());

    QDateTime start(QDate(2026, 7, 1), QTime(0, 1));
    QDateTime next = t.getNextTime(start);

    QVERIFY(next.isValid());
    QCOMPARE(next.date(), QDate(2026, 7, 15));
    QCOMPARE(next.time(), QTime(0, 0));
}

QTEST_GUILESS_MAIN(TstCronTime)
#include "tst_crontime.moc"

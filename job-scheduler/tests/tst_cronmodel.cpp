#include "CronModel.h"
#include "Crontab.h"

#include <QSignalSpy>
#include <QTest>

#include <memory>
#include <vector>

namespace {

TCommand *addCommand(Crontab &crontab, const QString &time, const QString &command)
{
    auto cmnd = std::make_unique<TCommand>(time, crontab.getCronOwner(), command, QString(), &crontab);
    TCommand *raw = cmnd.get();
    crontab.getTCommands().push_back(std::move(cmnd));
    return raw;
}

} // namespace

class TstCronModel : public QObject
{
    Q_OBJECT

private slots:
    void oneUserIndexRejectsNegativeRow();
    void oneUserIndexRejectsOutOfRangeRow();
    void multiUserChildIndexRejectsNegativeRow();
    void insertAddsRowAtRequestedPosition();
    void removeShiftsToNextRowOrReturnsInvalid();
    void dropMimeDataMovesCommandAndReturnsTrue();
};

void TstCronModel::oneUserIndexRejectsNegativeRow()
{
    std::vector<std::unique_ptr<Crontab>> crontabs;
    crontabs.push_back(std::make_unique<Crontab>());
    addCommand(*crontabs[0], "0 0 * * *", "echo one");

    CronModel model(&crontabs);

    QVERIFY(!model.index(-1, 0, QModelIndex()).isValid());
    QVERIFY(model.index(0, 0, QModelIndex()).isValid());
}

void TstCronModel::oneUserIndexRejectsOutOfRangeRow()
{
    std::vector<std::unique_ptr<Crontab>> crontabs;
    crontabs.push_back(std::make_unique<Crontab>());
    addCommand(*crontabs[0], "0 0 * * *", "echo one");

    CronModel model(&crontabs);

    QVERIFY(!model.index(1, 0, QModelIndex()).isValid());
}

void TstCronModel::multiUserChildIndexRejectsNegativeRow()
{
    std::vector<std::unique_ptr<Crontab>> crontabs;
    crontabs.push_back(std::make_unique<Crontab>());
    crontabs.push_back(std::make_unique<Crontab>());
    addCommand(*crontabs[0], "0 0 * * *", "echo one");

    CronModel model(&crontabs);

    QModelIndex cronIdx = model.index(0, 0, QModelIndex());
    QVERIFY(cronIdx.isValid());
    QVERIFY(!model.index(-1, 0, cronIdx).isValid());
    QVERIFY(model.index(0, 0, cronIdx).isValid());
}

void TstCronModel::insertAddsRowAtRequestedPosition()
{
    std::vector<std::unique_ptr<Crontab>> crontabs;
    crontabs.push_back(std::make_unique<Crontab>());
    addCommand(*crontabs[0], "0 0 * * *", "echo one");

    CronModel model(&crontabs);

    QModelIndex first = model.index(0, 0, QModelIndex());
    auto *inserted = new TCommand("0 1 * * *", QString(), "echo two", QString(), crontabs[0].get());
    QModelIndex insertedIdx = model.insertTCommand(first, inserted);

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(insertedIdx.row(), 1);
    QCOMPARE(model.getTCommand(insertedIdx), inserted);
}

void TstCronModel::removeShiftsToNextRowOrReturnsInvalid()
{
    std::vector<std::unique_ptr<Crontab>> crontabs;
    crontabs.push_back(std::make_unique<Crontab>());
    addCommand(*crontabs[0], "0 0 * * *", "echo one");
    addCommand(*crontabs[0], "0 1 * * *", "echo two");

    CronModel model(&crontabs);

    QModelIndex first = model.index(0, 0, QModelIndex());
    QModelIndex next = model.removeCommand(first);
    QCOMPARE(model.rowCount(QModelIndex()), 1);
    QVERIFY(next.isValid());
    QCOMPARE(model.data(model.index(next.row(), CronModel::Command, QModelIndex()), Qt::DisplayRole).toString(),
             QStringLiteral("echo two"));

    QModelIndex last = model.index(0, 0, QModelIndex());
    QModelIndex afterLast = model.removeCommand(last);
    QCOMPARE(model.rowCount(QModelIndex()), 0);
    QVERIFY(!afterLast.isValid());
}

void TstCronModel::dropMimeDataMovesCommandAndReturnsTrue()
{
    std::vector<std::unique_ptr<Crontab>> crontabs;
    crontabs.push_back(std::make_unique<Crontab>());
    TCommand *cmndA = addCommand(*crontabs[0], "0 0 * * *", "echo a");
    addCommand(*crontabs[0], "0 1 * * *", "echo b");

    CronModel model(&crontabs);

    QModelIndex dragIdx = model.searchTCommand(cmndA);
    QVERIFY(dragIdx.isValid());
    model.dragTCommand(dragIdx);

    QSignalSpy moveSpy(&model, &CronModel::moveTCommand);

    // Drop at the end of the list (row == rowCount), moving "echo a" after "echo b".
    bool dropped = model.dropMimeData(nullptr, Qt::MoveAction, model.rowCount(QModelIndex()), 0, QModelIndex());

    QVERIFY(dropped);
    QCOMPARE(moveSpy.count(), 1);
    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.data(model.index(1, CronModel::Command, QModelIndex()), Qt::DisplayRole).toString(),
             QStringLiteral("echo a"));
}

QTEST_GUILESS_MAIN(TstCronModel)
#include "tst_cronmodel.moc"

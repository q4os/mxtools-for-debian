/**********************************************************************
 *  MX Date/Time application.
 **********************************************************************
 *   Copyright (C) 2019 by AK-47
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of mx-datetime.
 **********************************************************************/

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextCharFormat>
#include <QTimeZone>

#include "about.h"
#include "datetime.h"
#include <unistd.h>

using namespace Qt::StringLiterals;

MXDateTime::MXDateTime(QWidget *parent)
    : QDialog(parent),
      updater(this)
{
    setupUi(this);
    setClockLock(true);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    QTextCharFormat tcfmt;
    tcfmt.setFontPointSize(calendar->font().pointSizeF() * 0.75);
    calendar->setHeaderTextFormat(tcfmt);
    connect(pushClose, &QPushButton::clicked, this, &MXDateTime::close);
    connect(tabsDateTime, &QTabWidget::currentChanged, this, &MXDateTime::loadTab);
    // This runs the slow startup tasks after the GUI is displayed.
    QTimer::singleShot(0, this, &MXDateTime::startup);
}

void MXDateTime::startup()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    timeEdit->setTimeZone(QTimeZone::systemTimeZone());  // TODO: Implement time zone differences properly.
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    timeEdit->setTimeSpec(Qt::LocalTime);                // TODO: Implement time zone differences properly.
    #pragma GCC diagnostic pop
#endif
    timeEdit->setDateTime(QDateTime::currentDateTime()); // Curtail the sudden jump.
    connect(pushReadHardware, &QPushButton::clicked, this, &MXDateTime::readHardwareClock);

    // Make the NTP server table columns the right proportions.
    int colSizes[3];
    addServerRow(true, QString(), QString(), QString());
    tableServers->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    for (int ixi = 0; ixi < 3; ++ixi) {
        colSizes[ixi] = tableServers->columnWidth(ixi);
    }
    tableServers->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    for (int ixi = 0; ixi < 3; ++ixi) {
        tableServers->setColumnWidth(ixi, colSizes[ixi]);
    }
    tableServers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableServers->removeRow(0);

    // Server page slots
    connect(pushServerMoveUp, &QPushButton::clicked, pushServerMoveUp, [this]() { moveServerRow(-1); });
    connect(pushServerMoveDown, &QPushButton::clicked, pushServerMoveDown, [this]() { moveServerRow(1); });
    connect(tableServers, &QTableWidget::itemChanged, this, &MXDateTime::serverRowChanged);

    // Used to decide the type of commands to run on this system.
    if (QFile::exists(u"/run/openrc"_s)) {
        sysInit = OpenRC;
    } else {
        QFile proc1comm(u"/proc/1/comm"_s);
        if (proc1comm.open(QIODevice::ReadOnly)) {
            if (proc1comm.readAll().contains("systemd")) {
                sysInit = SystemD;
            }
            proc1comm.close();
        }
    }
    static const char *sysInitNames[] = {"SystemV", "OpenRC", "SystemD"};
    qDebug() << "Init system:" << sysInitNames[sysInit];

    // Time zone areas.
    QByteArray zoneOut;
    execute(u"find"_s,
        {u"-L"_s, u"/usr/share/zoneinfo"_s, u"-mindepth"_s, u"2"_s, u"!"_s,
            u"-path"_s, u"*/posix/*"_s, u"!"_s, u"-path"_s, u"*/right/*"_s,
            u"-type"_s, u"f"_s, u"-printf"_s, u"%P\n"_s},
        &zoneOut);
    zones = zoneOut.trimmed().split('\n');
    comboTimeZone->blockSignals(true); // Keep blocked until loadSysTimeConfig().
    comboTimeArea->clear();
    for (const QByteArray &zone : std::as_const(zones)) {
        const QString &area = QString(zone).section('/', 0, 0);
        if (comboTimeArea->findData(area) < 0) {
            QString text(area);
            if (area == "Indian"_L1 || area == "Pacific"_L1 || area == "Atlantic"_L1 || area == "Arctic"_L1) {
                text.append(" Ocean");
            }
            comboTimeArea->addItem(text, area);
        }
    }
    comboTimeArea->model()->sort(0);

    // Prepare the GUI.
    connect(pushAbout, &QPushButton::clicked, this, &MXDateTime::aboutClicked);
    connect(pushHelp, &QPushButton::clicked, this, &MXDateTime::helpClicked);
    connect(pushApply, &QPushButton::clicked, this, &MXDateTime::applyClicked);
    setClockLock(false);
    // Setup the display update timer.
    connect(&updater, &QTimer::timeout, this, QOverload<>::of(&MXDateTime::update));
    updater.start(0);
}
void MXDateTime::setClockLock(bool locked)
{
    if (clockLock != locked) {
        clockLock = locked;
        if (locked) {
            qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
        } else {
            loadTab(tabsDateTime->currentIndex());
        }
        tabsDateTime->blockSignals(locked);
        tabDateTime->setDisabled(locked);
        tabHardware->setDisabled(locked);
        tabNetwork->setDisabled(locked);
        pushApply->setDisabled(locked);
        pushClose->setDisabled(locked);
        if (!locked) {
            qApp->restoreOverrideCursor();
        }
    }
}
bool MXDateTime::shell(const QString &cmd, QByteArray *output, bool elevate)
{
    qDebug() << "Shell:" << cmd;
    return execute(u"bash"_s, {u"-c"_s, cmd}, output, nullptr, elevate);
}
bool MXDateTime::execute(const QString &program, const QStringList &arguments, QByteArray *output, QByteArray *error,
                         bool elevate)
{
    qDebug() << "Exec:" << program << arguments;
    QProcess proc(this);
    QEventLoop eloop;
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &eloop, &QEventLoop::quit);
    if (elevate && getuid() != 0) {
        QString runAsRoot = QFile::exists(u"/usr/bin/pkexec"_s) ? u"/usr/bin/pkexec"_s : u"/usr/bin/gksu"_s;
        proc.start(runAsRoot, (QStringList() << u"/usr/lib/mx-datetime/helper"_s << program << arguments));
    } else {
        proc.start(program, arguments);
    }
    if (!output) {
        proc.closeReadChannel(QProcess::StandardOutput);
    }
    proc.closeWriteChannel();
    eloop.exec();
    disconnect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
    const QByteArray &sout = proc.readAllStandardOutput();
    if (output) {
        output->append(sout);
    } else if (!sout.isEmpty()) {
        qDebug() << "SOut:" << proc.readAllStandardOutput();
    }
    const QByteArray &serr = proc.readAllStandardError();
    if (error) {
        error->append(serr);
    } else if (!serr.isEmpty()) {
        qDebug() << "SErr:" << serr;
    }
    qDebug() << "Exit:" << proc.exitCode() << proc.exitStatus();
    return (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0);
}

bool MXDateTime::executeAsRoot(const QString &program, const QStringList &arguments, QByteArray *output,
                               QByteArray *error)
{
    return execute(program, arguments, output, error, true);
}

void MXDateTime::loadTab(int index)
{
    const unsigned int loaded = 1 << index;
    if ((loadedTabs & loaded) == 0) {
        loadedTabs |= loaded;
        setClockLock(true);
        switch (index) {
        case 0:
            connect(comboTimeArea, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MXDateTime::timeAreaIndexChanged);
            connect(comboTimeZone, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MXDateTime::timeZoneIndexChanged);
            connect(calendar, &QCalendarWidget::selectionChanged, this, &MXDateTime::calendarSelectionChanged);
            connect(timeEdit, &MTimeEdit::dateTimeChanged, this, &MXDateTime::timeEditDateTimeChanged);
            loadDateTime();
            break; // Date & Time.
        case 1:
            connect(pushHardwareAdjust, &QPushButton::clicked, this, &MXDateTime::hardwareAdjustClicked);
            connect(pushSystemToHardware, &QPushButton::clicked, this, &MXDateTime::systemToHardwareClicked);
            connect(pushHardwareToSystem, &QPushButton::clicked, this, &MXDateTime::hardwareToSystemClicked);
            readHardwareClock();
            break; // Hardware Clock.
        case 2:
            connect(pushSyncNow, &QPushButton::clicked, this, &MXDateTime::syncNowClicked);
            connect(tableServers, &QTableWidget::itemSelectionChanged,
                this, &MXDateTime::serversItemSelectionChanged);
            connect(pushServerAdd, &QPushButton::clicked, this, &MXDateTime::serverAddClicked);
            connect(pushServerRemove, &QPushButton::clicked, this, &MXDateTime::serverRemoveClicked);
            loadNetworkTime();
            break; // Network Time.
        }
        setClockLock(false);
    }
}

// DATE & TIME

void MXDateTime::timeAreaIndexChanged(int index)
{
    if (index < 0 || index >= comboTimeArea->count()) {
        return;
    }
    const QByteArray &area = comboTimeArea->itemData(index).toByteArray();
    comboTimeZone->clear();
    for (const QByteArray &zone : std::as_const(zones)) {
        if (zone.startsWith(area)) {
            QString text(QString(zone).section('/', 1));
            text.replace('_', ' ');
            comboTimeZone->addItem(text, QVariant(zone));
        }
    }
    comboTimeZone->model()->sort(0);
}
void MXDateTime::timeZoneIndexChanged(int index)
{
    if (index < 0 || index >= comboTimeZone->count()) {
        return;
    }
    // Calculate and store the difference between current and newly selected time zones.
    const QDateTime &current = QDateTime::currentDateTime();
    zoneDelta = QTimeZone(comboTimeZone->itemData(index).toByteArray()).offsetFromUtc(current)
                - QTimeZone::systemTimeZone().offsetFromUtc(current); // Delta = new - old
    // Check IANA-zone id differ
    const QByteArray &currentTimeZone = QTimeZone::systemTimeZoneId();
    const QByteArray &selectedTimeZone = QTimeZone(comboTimeZone->itemData(index).toByteArray()).id();
    zoneIdChanged = (currentTimeZone != selectedTimeZone);
    // Make the change immediately visible
    update();
}
void MXDateTime::calendarSelectionChanged()
{
    dateDelta = static_cast<int>(timeEdit->date().daysTo(calendar->selectedDate()));
}
void MXDateTime::timeEditDateTimeChanged(const QDateTime &dateTime)
{
    clock->setTime(dateTime.time());
    if (!updating) {
        timeDelta = QDateTime::currentDateTime().secsTo(dateTime) - zoneDelta;
        if (abs(dateDelta * 86400 + timeDelta) == 316800) {
            setWindowTitle(u"88 MILES PER HOUR"_s);
        }
    }
}

void MXDateTime::update()
{
    updating = true;
    timeEdit->updateDateTime(QDateTime::currentDateTime().addSecs(timeDelta + zoneDelta));
    updater.setInterval(1000 - QTime::currentTime().msec());
    if (timeEdit->date().daysTo(calendar->selectedDate()) != dateDelta) {
        calendar->setSelectedDate(timeEdit->date().addDays(dateDelta));
    }
    updating = false;
}

void MXDateTime::loadDateTime()
{
    // Time zone.
    comboTimeZone->blockSignals(true);
    const QByteArray &zone = QTimeZone::systemTimeZoneId();
    int index = comboTimeArea->findData(QString(zone).section('/', 0, 0));
    comboTimeArea->setCurrentIndex(index);
    qApp->processEvents();
    index = comboTimeZone->findData(QVariant(zone));
    comboTimeZone->setCurrentIndex(index);
    zoneDelta = 0;
    zoneIdChanged = false;
    comboTimeZone->blockSignals(false);
}
void MXDateTime::saveDateTime(const QDateTime &driftStart)
{
    // Stop display updates while setting the system clock.
    if (zoneDelta || dateDelta || timeDelta || zoneIdChanged) {
        updater.stop();
    }

    // Set the time zone (if changed) before setting the time.
    if (zoneDelta || zoneIdChanged) {
        const QString newzone(comboTimeZone->currentData().toByteArray());
        if (sysInit == SystemD) {
            executeAsRoot(u"timedatectl"_s, {u"set-timezone"_s, newzone});
        } else {
            executeAsRoot(u"ln"_s, {u"-nfs"_s, "/usr/share/zoneinfo/"_L1 + newzone, u"/etc/localtime"_s});
        }
        shell("echo "_L1 + newzone + " >/etc/timezone"_L1, nullptr, true);
        zoneDelta = 0;
        zoneIdChanged = false;
    }

    // Set the date and time if their controls have been altered.
    if (dateDelta || timeDelta) {
        static const QString dtFormat(u"yyyy-MM-ddTHH:mm:ss.zzz"_s);
        QDateTime newTime(calendar->selectedDate(), timeEdit->time());
        updater.stop();
        QString param;
        if (timeDelta) {
            const qint64 drift = driftStart.msecsTo(QDateTime::currentDateTimeUtc());
            param = newTime.addMSecs(drift).toString(dtFormat);
        } else {
            newTime.setTime(QTime::currentTime());
            param = newTime.toString(dtFormat);
        }
        if (sysInit != SystemD) {
            executeAsRoot(u"date"_s, {u"-s"_s, param});
        } else {
            executeAsRoot(u"timedatectl"_s, {u"set-time"_s, param});
        }
        dateDelta = 0;
        timeDelta = 0;
    }

    // Kick the display timer back in action.
    if (!updater.isActive()) {
        updater.start(0);
    }
}

// HARDWARE CLOCK

void MXDateTime::readHardwareClock()
{
    setClockLock(true);
    const QString btext = pushReadHardware->text();
    pushReadHardware->setText(tr("Reading..."));
    QByteArray rtcout;

    // For systemd, use timedatectl to check RTC setting consistently
    if (sysInit == SystemD) {
        QByteArray timedatectl_out;
        execute(u"timedatectl"_s, {u"show"_s, u"--property=LocalRTC"_s}, &timedatectl_out);
        // LocalRTC=no means UTC, LocalRTC=yes means local time
        isHardwareUTC = timedatectl_out.contains("LocalRTC=no");

        // Still show hwclock output for user reference
        executeAsRoot(u"hwclock"_s, {u"--verbose"_s}, &rtcout);
    } else {
        // For non-systemd systems, use hwclock output
        executeAsRoot(u"hwclock"_s, {u"--verbose"_s}, &rtcout);
        isHardwareUTC = rtcout.contains("\nHardware clock is on UTC time\n");
    }

    if (isHardwareUTC) {
        radioHardwareUTC->setChecked(true);
    } else {
        radioHardwareLocal->setChecked(true);
    }
    textHardwareClock->setPlainText(QString(rtcout.trimmed()));
    pushReadHardware->setText(btext);
    setClockLock(false);
}
void MXDateTime::hardwareAdjustClicked()
{
    setClockLock(true);
    const QString btext = pushHardwareAdjust->text();
    pushHardwareAdjust->setText(tr("Adjusting..."));
    QByteArray rtcout;
    executeAsRoot(u"hwclock"_s, {u"--adjust"_s}, &rtcout);
    textHardwareClock->setPlainText(QString(rtcout.trimmed()));
    pushHardwareAdjust->setText(btext);
    setClockLock(false);
}
void MXDateTime::systemToHardwareClicked()
{
    setClockLock(true);
    QStringList params("--systohc");
    if (checkDriftUpdate->isChecked()) {
        params.append(u"--update-drift"_s);
    }
    transferTime(params, tr("System Clock"), tr("Hardware Clock"));
    checkDriftUpdate->setCheckState(Qt::Unchecked);
    setClockLock(false);
}
void MXDateTime::hardwareToSystemClicked()
{
    setClockLock(true);
    transferTime({"--hctosys"}, tr("Hardware Clock"), tr("System Clock"));
    setClockLock(false);
}
void MXDateTime::transferTime(const QStringList &params, const QString &from, const QString &to)
{
    if (executeAsRoot(u"hwclock"_s, params)) {
        const QString &msg = tr("The %1 time was transferred to the %2.");
        QMessageBox::information(this, windowTitle(), msg.arg(from, to));
    } else {
        const QString &msg = tr("The %1 time could not be transferred to the %2.");
        QMessageBox::warning(this, windowTitle(), msg.arg(from, to));
    }
}

void MXDateTime::saveHardwareClock()
{
    const bool rtcUTC = radioHardwareUTC->isChecked();
    if (rtcUTC != isHardwareUTC) {
        if (sysInit == SystemD) {
            executeAsRoot(u"timedatectl"_s, {u"set-local-rtc"_s, rtcUTC ? u"0"_s : u"1"_s});
        } else {
            if (sysInit == OpenRC && QFile::exists(u"/etc/conf.d/hwclock"_s)) {
                executeAsRoot(u"sed"_s, {u"-i"_s,
                    rtcUTC ? u"(s/clock=.*/clock=\"UTC\"/)"_s : u"(s/clock=.*/clock=\"local\"/)"_s,
                    u"/etc/conf.d/hwclock"_s});
            }
            executeAsRoot(u"hwclock"_s, {u"--systohc"_s, rtcUTC ? u"--utc"_s : u"--localtime"_s});
        }
    }
}

// NETWORK TIME

void MXDateTime::syncNowClicked()
{
    if (!validateServerList()) {
        return;
    }
    setClockLock(true);

    saveNetworkTime();
    QByteArray output;
    bool rexit = false;
    if (enabledNTP) {
        // All commands can be passed via stdin, but chronyc may return 0 even if one command fails.
        rexit = executeAsRoot(u"chronyc"_s, {u"burst 4/4"_s}, &output);
        if (rexit) {
            rexit = execute(u"chronyc"_s, {u"waitsync 10"_s}, &output);
        }
        if (rexit) {
            rexit = executeAsRoot(u"chronyc"_s, {u"makestep"_s}, &output);
        }
    } else {
        rexit = executeAsRoot(u"chronyd"_s, {u"-q"_s}, nullptr, &output);
    }

    setClockLock(false);
    dateDelta = 0;
    timeDelta = 0;
    updater.setInterval(0);

    QMessageBox msgbox(this);
    if (rexit) {
        msgbox.setIcon(QMessageBox::Information);
        msgbox.setText(tr("The system clock was updated successfully."));
    } else {
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("The system clock could not be updated."));
    }
    msgbox.setDetailedText(output.trimmed());
    msgbox.exec();
}
void MXDateTime::serversItemSelectionChanged()
{
    const QList<QTableWidgetSelectionRange> &ranges = tableServers->selectedRanges();
    bool remove = false;
    bool up = false;
    bool down = false;
    if (ranges.count() == 1) {
        const QTableWidgetSelectionRange &range = ranges.at(0);
        remove = true;
        if (range.topRow() > 0) {
            up = true;
        }
        if (range.bottomRow() < (tableServers->rowCount() - 1)) {
            down = true;
        }
    }
    pushServerRemove->setEnabled(remove);
    pushServerMoveUp->setEnabled(up);
    pushServerMoveDown->setEnabled(down);
}
void MXDateTime::serverAddClicked()
{
    QTableWidgetItem *item = addServerRow(true, u"server"_s, QString(), QString());
    tableServers->setCurrentItem(item);
    tableServers->editItem(item);
}
void MXDateTime::serverRemoveClicked()
{
    const QList<QTableWidgetSelectionRange> &ranges = tableServers->selectedRanges();
    for (int ixi = ranges.count() - 1; ixi >= 0; --ixi) {
        const int top = ranges.at(ixi).topRow();
        for (int row = ranges.at(ixi).bottomRow(); row >= top; --row) {
            tableServers->removeRow(row);
        }
    }
    changedServers = true;
}

QTableWidgetItem *MXDateTime::addServerRow(bool enabled, const QString &type, const QString &address,
                                           const QString &options)
{
    auto *itemComboType = new QComboBox(tableServers);
    auto *item = new QTableWidgetItem(address);
    auto *itemOptions = new QTableWidgetItem(options);
    itemComboType->addItem(u"Pool"_s, QVariant("pool"));
    itemComboType->addItem(u"Server"_s, QVariant("server"));
    itemComboType->addItem(u"Peer"_s, QVariant("peer"));
    itemComboType->setCurrentIndex(itemComboType->findData(QVariant(type)));
    connect(itemComboType, &QComboBox::currentTextChanged, this, &MXDateTime::serverRowChanged);
    item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
    item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    const int newRow = tableServers->rowCount();
    tableServers->insertRow(newRow);
    tableServers->setCellWidget(newRow, 0, itemComboType);
    tableServers->setItem(newRow, 1, item);
    tableServers->setItem(newRow, 2, itemOptions);
    return item;
}
void MXDateTime::moveServerRow(int movement)
{
    const QList<QTableWidgetSelectionRange> &ranges = tableServers->selectedRanges();
    if (ranges.count() == 1) {
        const QTableWidgetSelectionRange &range = ranges.at(0);
        int end;
        int row;
        if (movement < 0) {
            row = range.topRow();
            end = range.bottomRow();
        } else {
            row = range.bottomRow();
            end = range.topRow();
        }
        row += movement;
        // Save the original row contents.
        int targetType = qobject_cast<QComboBox *>(tableServers->cellWidget(row, 0))->currentIndex();
        QTableWidgetItem *targetItemAddress = tableServers->takeItem(row, 1);
        QTableWidgetItem *targetItemOptions = tableServers->takeItem(row, 2);
        // Update the list selection.
        const QTableWidgetSelectionRange targetRange(row, range.leftColumn(), end + movement, range.rightColumn());
        tableServers->setCurrentItem(nullptr);
        tableServers->setRangeSelected(targetRange, true);
        // Move items one by one.
        do {
            row -= movement;
            int type = qobject_cast<QComboBox *>(tableServers->cellWidget(row, 0))->currentIndex();
            QTableWidgetItem *itemAddress = tableServers->takeItem(row, 1);
            QTableWidgetItem *itemOptions = tableServers->takeItem(row, 2);
            const int step = row + movement;
            qobject_cast<QComboBox *>(tableServers->cellWidget(step, 0))->setCurrentIndex(type);
            tableServers->setItem(step, 1, itemAddress);
            tableServers->setItem(step, 2, itemOptions);
        } while (row != end);
        // Move the target where the range originally finished.
        qobject_cast<QComboBox *>(tableServers->cellWidget(end, 0))->setCurrentIndex(targetType);
        tableServers->setItem(end, 1, targetItemAddress);
        tableServers->setItem(end, 2, targetItemOptions);
    }
}
bool MXDateTime::validateServerList()
{
    bool allValid = true;
    const int serverCount = tableServers->rowCount();
    for (int ixi = 0; ixi < serverCount; ++ixi) {
        QTableWidgetItem *item = tableServers->item(ixi, 1);
        const QString &address = item->text().trimmed();
        if (address.isEmpty()) {
            allValid = false;
        }
    }
    const char *msg = nullptr;
    if (serverCount <= 0) {
        msg = QT_TR_NOOP("There are no NTP servers on the list.");
    } else if (!allValid) {
        msg = QT_TR_NOOP("There are invalid entries on the NTP server list.");
    }
    if (msg) {
        QMessageBox::critical(this, windowTitle(), tr(msg));
        return false;
    }
    return true;
}

void MXDateTime::loadNetworkTime()
{
    while (tableServers->rowCount() > 0) {
        tableServers->removeRow(0);
    }
    loadSources(u"/etc/chrony/sources.d/mx-datetime.sources"_s);
    const bool move = loadSources(u"/etc/chrony/chrony.conf"_s);

    // Stray change signals may have caused changedServers to be set.
    QApplication::processEvents();
    changedServers = move;

    if (sysInit != SystemD) {
        enabledNTP = shell(u"ls /etc/rc*.d | grep chrony | grep '^S'"_s);
    } else {
        enabledNTP = shell(u"LANG=C systemctl status chrony | grep Loaded"_s
            u"| grep service |cut -d';' -f2 |grep -q enabled"_s);
    }
    checkAutoSync->setChecked(enabledNTP);
}
void MXDateTime::saveNetworkTime()
{
    // Enable/disable NTP
    const bool ntp = checkAutoSync->isChecked();
    if (ntp != enabledNTP) {
        const QString enable_disable(ntp ? u"enable"_s : u"disable"_s);
        const QString start_stop(ntp ? u"start"_s : u"stop"_s);
        if (sysInit == SystemD) {
            executeAsRoot(u"systemctl"_s, {enable_disable, u"chrony"_s});
            executeAsRoot(u"systemctl"_s, {start_stop, u"chrony"_s});
        } else if (sysInit == OpenRC) {
            if (QFile::exists(u"/etc/init.d/chronyd"_s)) {
                executeAsRoot(u"rc-update"_s, {ntp ? u"add"_s : u"del"_s, u"chronyd"_s});
            }
        } else {
            executeAsRoot(u"update-rc.d"_s, {u"chrony"_s, enable_disable});
            executeAsRoot(u"service"_s, {u"chrony"_s, start_stop});
        }
        enabledNTP = ntp;
    }

    // Generate chrony sources file.
    if (changedServers) {
        QTemporaryFile file;
        if (file.open()) {
            file.write("# Generated by MX Date & Time - ");
            file.write(QDateTime::currentDateTime().toString(u"yyyy-MM-dd H:mm:ss t"_s).toUtf8());
            file.write("\n");
            for (int ixi = 0; ixi < tableServers->rowCount(); ++ixi) {
                auto *comboType = qobject_cast<QComboBox *>(tableServers->cellWidget(ixi, 0));
                QTableWidgetItem *item = tableServers->item(ixi, 1);
                if (item->checkState() != Qt::Checked) {
                    file.write("#");
                }
                file.write(comboType->currentData().toByteArray());
                file.write(" ");
                file.write(item->text().trimmed().toUtf8());
                const QString &options = tableServers->item(ixi, 2)->text().simplified();
                if (!options.isEmpty()) {
                    file.write(" ");
                    file.write(options.toUtf8());
                }
                file.write("\n");
            }
            file.close();
            executeAsRoot(u"mv"_s, {file.fileName(), u"/etc/chrony/sources.d/mx-datetime.sources"_s});
            executeAsRoot(u"chown"_s, {u"root:"_s, u"/etc/chrony/sources.d/mx-datetime.sources"_s});
            executeAsRoot(u"chmod"_s, {u"+r"_s, u"/etc/chrony/sources.d/mx-datetime.sources"_s});
        }
        if (ntp) {
            if (!clearSources(u"/etc/chrony/chrony.conf"_s)) {
                executeAsRoot(u"chronyc"_s, {u"reload"_s, u"sources"_s});
            } else {
                if (sysInit == SystemD) {
                    executeAsRoot(u"systemctl"_s, {u"restart"_s, u"chrony"_s});
                } else {
                    executeAsRoot(u"service"_s, {u"chrony"_s, u"restart"_s});
                }
            }
        }
    }
}

bool MXDateTime::loadSources(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }
    bool hassrc = false;
    while (!file.atEnd()) {
        const QByteArray &bline = file.readLine();
        const QString line(bline.simplified());
        static const QRegularExpression tregex(u"^#?(pool|server|peer)\\s"_s);
        if (line.contains(tregex)) {
            QStringList args = line.split(' ', Qt::SkipEmptyParts);
            QString curarg = args.at(0);
            bool enabled = true;
            if (curarg.startsWith('#')) {
                enabled = false;
                curarg = curarg.remove(0, 1);
            }
            QString options;
            for (int ixi = 2; ixi < args.count(); ++ixi) {
                options.append(' ');
                options.append(args.at(ixi));
            }
            addServerRow(enabled, curarg, args.at(1), options.trimmed());
            hassrc = true;
        }
    }
    return hassrc;
}

// Remove all source NTP lines from a file. Make a backup before writing.
// Return the number of sources removed on success, -1 on failure.
bool MXDateTime::clearSources(const QString &filename)
{
    bool changed = false;
    QFile file(filename);
    QByteArray confdata;
    // Read config and skip sources.
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }
    while (!file.atEnd()) {
        const QByteArray &bline = file.readLine();
        static const QRegularExpression tregex(u"^\\s?(pool|server|peer)\\s"_s);
        if (QString(bline).contains(tregex)) {
            confdata.append("##");
            changed = true;
        }
        confdata.append(bline);
    }
    file.close();
    // Write cleared config.
    if (changed) {
        QTemporaryFile tmpFile;
        if (!tmpFile.open()) {
            return false;
        }
        tmpFile.write(confdata);
        tmpFile.close();
        executeAsRoot(u"mv"_s, {tmpFile.fileName(), file.fileName()});
        executeAsRoot(u"chown"_s, {u"root:"_s, file.fileName()});
        executeAsRoot(u"chmod"_s, {u"+r"_s, file.fileName()});
    }
    return changed;
}

void MXDateTime::serverRowChanged()
{
    changedServers = true;
}

// ACTION BUTTONS

void MXDateTime::applyClicked()
{
    // Compensation for the execution time of this section.
    QDateTime driftStart = QDateTime::currentDateTimeUtc();
    setClockLock(true);

    // Validate all data before trying to save settings.
    if ((loadedTabs & 4) && !validateServerList()) {
        setClockLock(false);
        return;
    }

    // Save the settings.
    saveDateTime(driftStart);
    if (loadedTabs & 2) {
        saveHardwareClock();
    }
    if (loadedTabs & 4) {
        saveNetworkTime();
    }

    // Refresh the UI (especially the current tab) with newly set values.
    loadedTabs = 0;
    setClockLock(false);
}

// MX Standard User Interface
void MXDateTime::aboutClicked()
{
    displayAboutMsgBox(tr("About MX Date & Time"),
        "<p align=\"center\"><b><h2>"_L1 + tr("MX Date & Time") + "</h2></b></p><p align=\"center\">"_L1
        + tr("Version: ") + qApp->applicationVersion() + "</p><p align=\"center\"><h3>"_L1
        + tr("GUI program for setting the time and date in MX Linux")
        + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br/>"_L1
            "</p><p align=\"center\">"_L1
        + tr("Copyright (c) MX Linux") + "<br /><br /></p>"_L1,
        u"/usr/share/doc/mx-datetime/license.html"_s, tr("%1 License").arg(this->windowTitle()));
}
void MXDateTime::helpClicked()
{
    displayDoc(u"/usr/share/doc/mx-datetime/mx-datetime.html"_s, tr("MX Date & Time Help"));
}

// SUBCLASSING FOR QTimeEdit THAT FIXES CURSOR AND SELECTION JUMPING EVERY SECOND

void MTimeEdit::updateDateTime(const QDateTime &dateTime)
{
    QLineEdit *ledit = lineEdit();
    // Original cursor position and selections
    int select = ledit->selectionStart();
    int cursor = ledit->cursorPosition();
    // Calculation for backward selections.
    if (select >= 0 && select >= cursor) {
        const int cslength = ledit->selectedText().length();
        if (cslength > 0) {
            select = cursor + cslength;
        }
    }
    // Set the date/time as normal.
    setDateTime(dateTime);
    // Restore cursor and selection.
    if (select >= 0) {
        ledit->setSelection(select, cursor - select);
    } else {
        ledit->setCursorPosition(cursor);
    }
}

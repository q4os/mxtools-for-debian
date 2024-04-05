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
#include "version.h"
#include <unistd.h>

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
    timeEdit->setTimeSpec(Qt::LocalTime);                // TODO: Implement time zone differences properly.
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
    if (QFile::exists("/run/openrc")) {
        sysInit = OpenRC;
    } else if (QFile::exists("/usr/bin/timedatectl")) {
        QByteArray test;
        if (execute("ps", {"-hp1"}, &test) && test.contains("systemd")) {
            sysInit = SystemD;
        }
    }
    static const char *sysInitNames[] = {"SystemV", "OpenRC", "SystemD"};
    qDebug() << "Init system:" << sysInitNames[sysInit];

    // Time zone areas.
    QByteArray zoneOut;
    execute("find",
            {"-L", "/usr/share/zoneinfo", "-mindepth", "2", "!", "-path", "*/posix/*", "!", "-path", "*/right/*",
             "-type", "f", "-printf", "%P\n"},
            &zoneOut);
    zones = zoneOut.trimmed().split('\n');
    comboTimeZone->blockSignals(true); // Keep blocked until loadSysTimeConfig().
    comboTimeArea->clear();
    for (const QByteArray &zone : qAsConst(zones)) {
        const QString &area = QString(zone).section('/', 0, 0);
        if (comboTimeArea->findData(area) < 0) {
            QString text(area);
            if (area == QLatin1String("Indian") || area == QLatin1String("Pacific") || area == QLatin1String("Atlantic")
                || area == QLatin1String("Arctic")) {
                text.append(" Ocean");
            }
            comboTimeArea->addItem(text, area);
        }
    }
    comboTimeArea->model()->sort(0);

    // Prepare the GUI.
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
    return execute("bash", {"-c", cmd}, output, nullptr, elevate);
}
bool MXDateTime::execute(const QString &program, const QStringList &arguments, QByteArray *output, QByteArray *error,
                         bool elevate)
{
    qDebug() << "Exec:" << program << arguments;
    QProcess proc(this);
    QEventLoop eloop;
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &eloop, &QEventLoop::quit);
    if (elevate && getuid() != 0) {
        QString runAsRoot = QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu";
        QString helper {"/usr/lib/mx-datetime/helper"};
        proc.start(runAsRoot, {QStringList() << helper << program << arguments});
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
            loadDateTime();
            break; // Date & Time.
        case 1:
            readHardwareClock();
            break; // Hardware Clock.
        case 2:
            loadNetworkTime();
            break; // Network Time.
        }
        setClockLock(false);
    }
}

// DATE & TIME

void MXDateTime::on_comboTimeArea_currentIndexChanged(int index)
{
    if (index < 0 || index >= comboTimeArea->count()) {
        return;
    }
    const QByteArray &area = comboTimeArea->itemData(index).toByteArray();
    comboTimeZone->clear();
    for (const QByteArray &zone : qAsConst(zones)) {
        if (zone.startsWith(area)) {
            QString text(QString(zone).section('/', 1));
            text.replace('_', ' ');
            comboTimeZone->addItem(text, QVariant(zone));
        }
    }
    comboTimeZone->model()->sort(0);
}
void MXDateTime::on_comboTimeZone_currentIndexChanged(int index)
{
    if (index < 0 || index >= comboTimeZone->count()) {
        return;
    }
    // Calculate and store the difference between current and newly selected time zones.
    const QDateTime &current = QDateTime::currentDateTime();
    zoneDelta = QTimeZone(comboTimeZone->itemData(index).toByteArray()).offsetFromUtc(current)
                - QTimeZone::systemTimeZone().offsetFromUtc(current); // Delta = new - old
    update();                                                         // Make the change immediately visible
}
void MXDateTime::on_calendar_selectionChanged()
{
    dateDelta = static_cast<int>(timeEdit->date().daysTo(calendar->selectedDate()));
}
void MXDateTime::on_timeEdit_dateTimeChanged(const QDateTime &dateTime)
{
    clock->setTime(dateTime.time());
    if (!updating) {
        timeDelta = QDateTime::currentDateTime().secsTo(dateTime) - zoneDelta;
        if (abs(dateDelta * 86400 + timeDelta) == 316800) {
            setWindowTitle("88 MILES PER HOUR");
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
    comboTimeZone->blockSignals(false);
}
void MXDateTime::saveDateTime(const QDateTime &driftStart)
{
    // Stop display updates while setting the system clock.
    if (zoneDelta || dateDelta || timeDelta) {
        updater.stop();
    }

    // Set the time zone (if changed) before setting the time.
    if (zoneDelta) {
        const QString newzone(comboTimeZone->currentData().toByteArray());
        if (sysInit == SystemD) {
            executeAsRoot("timedatectl", {"set-timezone", newzone});
        } else {
            executeAsRoot("ln", {"-nfs", "/usr/share/zoneinfo/" + newzone, "/etc/localtime"});
            QFile file("/etc/timezone");
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(newzone.toUtf8());
                file.close();
            }
        }
        zoneDelta = 0;
    }

    // Set the date and time if their controls have been altered.
    if (dateDelta || timeDelta) {
        static const QString dtFormat("yyyy-MM-ddTHH:mm:ss.zzz");
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
            executeAsRoot("date", {"-s", param});
        } else {
            executeAsRoot("timedatectl", {"set-time", param});
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
    executeAsRoot("hwclock", {"--verbose"}, &rtcout);
    isHardwareUTC = rtcout.contains("\nHardware clock is on UTC time\n");
    if (isHardwareUTC) {
        radioHardwareUTC->setChecked(true);
    } else {
        radioHardwareLocal->setChecked(true);
    }
    textHardwareClock->setPlainText(QString(rtcout.trimmed()));
    pushReadHardware->setText(btext);
    setClockLock(false);
}
void MXDateTime::on_pushHardwareAdjust_clicked()
{
    setClockLock(true);
    const QString btext = pushHardwareAdjust->text();
    pushHardwareAdjust->setText(tr("Adjusting..."));
    QByteArray rtcout;
    executeAsRoot("hwclock", {"--adjust"}, &rtcout);
    textHardwareClock->setPlainText(QString(rtcout.trimmed()));
    pushHardwareAdjust->setText(btext);
    setClockLock(false);
}
void MXDateTime::on_pushSystemToHardware_clicked()
{
    setClockLock(true);
    QStringList params("--systohc");
    if (checkDriftUpdate->isChecked()) {
        params.append("--update-drift");
    }
    transferTime(params, tr("System Clock"), tr("Hardware Clock"));
    checkDriftUpdate->setCheckState(Qt::Unchecked);
    setClockLock(false);
}
void MXDateTime::on_pushHardwareToSystem_clicked()
{
    setClockLock(true);
    transferTime({"--hctosys"}, tr("Hardware Clock"), tr("System Clock"));
    setClockLock(false);
}
void MXDateTime::transferTime(const QStringList &params, const QString &from, const QString &to)
{
    if (executeAsRoot("hwclock", params)) {
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
            executeAsRoot("timedatectl", {"set-local-rtc", rtcUTC ? "0" : "1"});
        } else {
            if (sysInit == OpenRC && QFile::exists("/etc/conf.d/hwclock")) {
                const char *sed = rtcUTC ? R"(s/clock=.*/clock=\"UTC\"/)" : R"(s/clock=.*/clock=\"local\"/)";
                executeAsRoot("sed", {"-i", sed, "/etc/conf.d/hwclock"});
            }
            executeAsRoot("hwclock", {"--systohc", rtcUTC ? "--utc" : "--localtime"});
        }
    }
}

// NETWORK TIME

void MXDateTime::on_pushSyncNow_clicked()
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
        rexit = executeAsRoot("chronyc", {"burst 4/4"}, &output);
        if (rexit) {
            rexit = execute("chronyc", {"waitsync 10"}, &output);
        }
        if (rexit) {
            rexit = executeAsRoot("chronyc", {"makestep"}, &output);
        }
    } else {
        rexit = executeAsRoot("chronyd", {"-q"}, nullptr, &output);
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
void MXDateTime::on_tableServers_itemSelectionChanged()
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
void MXDateTime::on_pushServerAdd_clicked()
{
    QTableWidgetItem *item = addServerRow(true, "server", QString(), QString());
    tableServers->setCurrentItem(item);
    tableServers->editItem(item);
}
void MXDateTime::on_pushServerRemove_clicked()
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
    itemComboType->addItem("Pool", QVariant("pool"));
    itemComboType->addItem("Server", QVariant("server"));
    itemComboType->addItem("Peer", QVariant("peer"));
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
        msg = "There are no NTP servers on the list.";
    } else if (!allValid) {
        msg = "There are invalid entries on the NTP server list.";
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
    loadSources("/etc/chrony/sources.d/mx-datetime.sources");
    const bool move = loadSources("/etc/chrony/chrony.conf");

    // Stray change signals may have caused changedServers to be set.
    QApplication::processEvents();
    changedServers = move;

    if (sysInit != SystemD) {
        enabledNTP = shell("ls /etc/rc*.d | grep chrony | grep '^S'");
    } else {
        enabledNTP = shell("LANG=C systemctl status chrony | grep Loaded"
                           "| grep service |cut -d';' -f2 |grep -q enabled");
    }
    checkAutoSync->setChecked(enabledNTP);
}
void MXDateTime::saveNetworkTime()
{
    // Enable/disable NTP
    const bool ntp = checkAutoSync->isChecked();
    if (ntp != enabledNTP) {
        const QString enable_disable(ntp ? "enable" : "disable");
        const QString start_stop(ntp ? "start" : "stop");
        if (sysInit == SystemD) {
            executeAsRoot("systemctl", {enable_disable, "chrony"});
            executeAsRoot("systemctl", {start_stop, "chrony"});
        } else if (sysInit == OpenRC) {
            if (QFile::exists("/etc/init.d/chronyd")) {
                executeAsRoot("rc-update", {ntp ? "add" : "del", "chronyd"});
            }
        } else {
            executeAsRoot("update-rc.d", {"chrony", enable_disable});
            executeAsRoot("service", {"chrony", start_stop});
        }
        enabledNTP = ntp;
    }

    // Generate chrony sources file.
    if (changedServers) {
        QTemporaryFile file;
        if (file.open()) {
            file.write("# Generated by MX Date & Time - ");
            file.write(QDateTime::currentDateTime().toString("yyyy-MM-dd H:mm:ss t").toUtf8());
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
            executeAsRoot("mv", {file.fileName(), "/etc/chrony/sources.d/mx-datetime.sources"});
            executeAsRoot("chown root: /etc/chrony/sources.d/mx-datetime.sources");
            executeAsRoot("chmod +r /etc/chrony/sources.d/mx-datetime.sources");
        }
        if (ntp) {
            if (!clearSources("/etc/chrony/chrony.conf")) {
                executeAsRoot("chronyc", {"reload", "sources"});
            } else {
                if (sysInit == SystemD) {
                    executeAsRoot("systemctl", {"restart", "chrony"});
                } else {
                    executeAsRoot("service", {"chrony", "restart"});
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
        static const QRegularExpression tregex("^#?(pool|server|peer)\\s");
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
        static const QRegularExpression tregex("^\\s?(pool|server|peer)\\s");
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
        executeAsRoot("mv", {tmpFile.fileName(), file.fileName()});
        executeAsRoot("chown root: " + file.fileName());
        executeAsRoot("chmod +r " + file.fileName());
    }
    return changed;
}

// ACTION BUTTONS

void MXDateTime::on_pushApply_clicked()
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

// Slots

void MXDateTime::serverRowChanged()
{
    changedServers = true;
}

// MX Standard User Interface
void MXDateTime::on_pushAbout_clicked()
{
    displayAboutMsgBox(tr("About MX Date & Time"),
                       "<p align=\"center\"><b><h2>" + tr("MX Date & Time") + "</h2></b></p><p align=\"center\">"
                           + tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>"
                           + tr("GUI program for setting the time and date in MX Linux")
                           + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br "
                             "/></p><p align=\"center\">"
                           + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                       "/usr/share/doc/mx-datetime/license.html", tr("%1 License").arg(this->windowTitle()));
}
void MXDateTime::on_pushHelp_clicked()
{
    displayDoc("/usr/share/doc/mx-datetime/mx-datetime.html", tr("MX Date & Time Help").toUtf8());
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

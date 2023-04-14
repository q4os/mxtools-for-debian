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
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QTextCharFormat>
#include <QTimeZone>

#include "about.h"
#include "datetime.h"
#include "version.h"
#include <unistd.h>

MXDateTime::MXDateTime(QWidget *parent) :
    QDialog(parent), updater(this)
{
    setupUi(this);
    setClockLock(true);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    QTextCharFormat tcfmt;
    tcfmt.setFontPointSize(calendar->font().pointSizeF() * 0.75);
    calendar->setHeaderTextFormat(tcfmt);
    // Operate with reduced functionality if not running as root.
    userRoot = (getuid() == 0);
    if (!userRoot) {
        pushAbout->hide();
        pushHelp->hide();
        labelLogo->hide();
        pushApply->hide();
        pushClose->hide();
        tabsDateTime->tabBar()->hide();
        tabsDateTime->setDocumentMode(true);
        gridWindow->setMargin(0);
        gridDateTime->setMargin(0);
        gridDateTime->setSpacing(1);
    }
    // This runs the slow startup tasks after the GUI is displayed.
    QTimer::singleShot(0, this, &MXDateTime::startup);
}

void MXDateTime::startup()
{
    timeEdit->setDateTime(QDateTime::currentDateTime()); // Curtail the sudden jump.
    if (userRoot) {
        // Make the NTP server table columns the right proportions.
        int colSizes[3];
        addServerRow(true, QString(), QString(), QString());
        tableServers->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        for (int ixi = 0; ixi < 3; ++ixi) colSizes[ixi] = tableServers->columnWidth(ixi);
        tableServers->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        for (int ixi = 0; ixi < 3; ++ixi) tableServers->setColumnWidth(ixi, colSizes[ixi]);
        tableServers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        tableServers->removeRow(0);

        // Used to decide the type of commands to run on this system.
        QByteArray testSystemD;
        if (QFileInfo::exists(QStringLiteral("/run/openrc"))) sysInit = OpenRC;
        else if (QFileInfo(QStringLiteral("/usr/bin/timedatectl")).isExecutable()
                && execute(QStringLiteral("ps -hp1"), &testSystemD) && testSystemD.contains("systemd")) {
            sysInit = SystemD;
        } else sysInit = SystemV;
        static const char *sysInitNames[] = {"SystemV", "OpenRC", "SystemD"};
        qDebug() << "Init system:" << sysInitNames[sysInit];
    }

    // Time zone areas.
    QByteArray zoneOut;
    execute(QStringLiteral("find -L /usr/share/zoneinfo -mindepth 2 ! -path */posix/* ! -path */right/* -type f -printf %P\\n"), &zoneOut);
    zones = zoneOut.trimmed().split('\n');
    comboTimeZone->blockSignals(true); // Keep blocked until loadSysTimeConfig().
    comboTimeArea->clear();
    for (const QByteArray &zone : qAsConst(zones)) {
        const QString &area = QString(zone).section('/', 0, 0);
        if (comboTimeArea->findData(area) < 0) {
            QString text(area);
            if (area == QLatin1String("Indian") || area == QLatin1String("Pacific")
                || area == QLatin1String("Atlantic") || area == QLatin1String("Arctic")) text.append(" Ocean");
            comboTimeArea->addItem(text, QVariant(area.toUtf8()));
        }
    }
    comboTimeArea->model()->sort(0);

    // Prepare the GUI.
    setClockLock(false);
    if (!userRoot) calendar->setFocus();
    // Setup the display update timer.
    connect(&updater, &QTimer::timeout, this, QOverload<>::of(&MXDateTime::update));
    updater.start(0);
}
void MXDateTime::setClockLock(bool locked)
{
    if (clockLock != locked) {
        clockLock = locked;
        if (locked) qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
        else on_tabsDateTime_currentChanged(tabsDateTime->currentIndex());
        tabsDateTime->blockSignals(locked);
        tabDateTime->setDisabled(locked);
        tabHardware->setDisabled(locked);
        tabNetwork->setDisabled(locked);
        pushApply->setDisabled(locked);
        pushClose->setDisabled(locked);
        if (!locked) qApp->restoreOverrideCursor();
    }
}
bool MXDateTime::execute(const QString &cmd, QByteArray *output)
{
    qDebug() << "Exec:" << cmd;
    QProcess proc(this);
    QEventLoop eloop;
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &eloop, &QEventLoop::quit);
    proc.start(cmd);
    if (!output) proc.closeReadChannel(QProcess::StandardOutput);
    proc.closeWriteChannel();
    eloop.exec();
    disconnect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
    const QByteArray &sout = proc.readAllStandardOutput();
    if (output) *output = sout;
    else if (!sout.isEmpty()) qDebug() << "SOut:" << proc.readAllStandardOutput();
    const QByteArray &serr = proc.readAllStandardError();
    if (!serr.isEmpty()) qDebug() << "SErr:" << serr;
    qDebug() << "Exit:" << proc.exitCode() << proc.exitStatus();
    return (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0);
}

void MXDateTime::on_tabsDateTime_currentChanged(int index)
{
    const unsigned int loaded = 1 << index;
    if ((loadedTabs & loaded) == 0) {
        loadedTabs |= loaded;
        setClockLock(true);
        switch(index) {
        case 0: loadDateTime(); break; // Date & Time.
        case 1: on_pushReadHardware_clicked(); break; // Hardware Clock.
        case 2: loadNetworkTime(); break; // Network Time.
        }
        setClockLock(false);
    }
}

// DATE & TIME

void MXDateTime::on_comboTimeArea_currentIndexChanged(int index)
{
    if (index < 0 || index >= comboTimeArea->count()) return;
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
    if (index < 0 || index >= comboTimeZone->count()) return;
    // Calculate and store the difference between current and newly selected time zones.
    const QDateTime &current = QDateTime::currentDateTime();
    zoneDelta = QTimeZone(comboTimeZone->itemData(index).toByteArray()).offsetFromUtc(current)
              - QTimeZone::systemTimeZone().offsetFromUtc(current); // Delta = new - old
    update(); // Make the change immediately visible
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
        if (abs(dateDelta*86400 + timeDelta) == 316800) setWindowTitle(QStringLiteral("88 MILES PER HOUR"));
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
    int index = comboTimeArea->findData(QVariant(QString(zone).section('/', 0, 0).toUtf8()));
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
    if (zoneDelta || dateDelta || timeDelta) updater.stop();

    // Set the time zone (if changed) before setting the time.
    if (zoneDelta) {
        const QString newzone(comboTimeZone->currentData().toByteArray());
        if (sysInit == SystemD) execute("timedatectl set-timezone " + newzone);
        else {
            execute("ln -nfs /usr/share/zoneinfo/" + newzone + " /etc/localtime");
            QFile file(QStringLiteral("/etc/timezone"));
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(newzone.toUtf8());
                file.close();
            }
        }
        zoneDelta = 0;
    }

    // Set the date and time if their controls have been altered.
    if (dateDelta || timeDelta) {
        QString cmd;
        if (sysInit == SystemD) cmd = QStringLiteral("timedatectl set-time ");
        else cmd = QStringLiteral("date -s ");
        static const QString dtFormat(QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"));
        QDateTime newTime(calendar->selectedDate(),
                          timeEdit->time());
        updater.stop();
        if (timeDelta) {
            const qint64 drift = driftStart.msecsTo(QDateTime::currentDateTimeUtc());
            execute(cmd + newTime.addMSecs(drift).toString(dtFormat));
        } else {
            newTime.setTime(QTime::currentTime());
            execute(cmd + newTime.toString(dtFormat));
        }
        dateDelta = 0;
        timeDelta = 0;
    }

    // Kick the display timer back in action.
    if (!updater.isActive()) updater.start(0);
}

// HARDWARE CLOCK

void MXDateTime::on_pushReadHardware_clicked()
{
    setClockLock(true);
    const QString btext = pushReadHardware->text();
    pushReadHardware->setText(tr("Reading..."));
    QByteArray rtcout;
    execute(QStringLiteral("hwclock --verbose"), &rtcout);
    isHardwareUTC = rtcout.contains("\nHardware clock is on UTC time\n");
    if (isHardwareUTC) radioHardwareUTC->setChecked(true);
    else radioHardwareLocal->setChecked(true);
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
    execute(QStringLiteral("hwclock --adjust"), &rtcout);
    textHardwareClock->setPlainText(QString(rtcout.trimmed()));
    pushHardwareAdjust->setText(btext);
    setClockLock(false);
}
void MXDateTime::on_pushSystemToHardware_clicked()
{
    setClockLock(true);
    QString cmd(QStringLiteral("hwclock --systohc"));
    if (checkDriftUpdate->isChecked()) cmd.append(" --update-drift");
    transferTime(cmd, tr("System Clock"), tr("Hardware Clock"));
    checkDriftUpdate->setCheckState(Qt::Unchecked);
    setClockLock(false);
}
void MXDateTime::on_pushHardwareToSystem_clicked()
{
    setClockLock(true);
    transferTime(QStringLiteral("hwclock --hctosys"), tr("Hardware Clock"), tr("System Clock"));
    setClockLock(false);
}
void MXDateTime::transferTime(const QString &cmd, const QString &from, const QString &to)
{
    if (execute(cmd)) {
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
            execute("timedatectl set-local-rtc " + QString(rtcUTC?"0":"1"));
        } else if (sysInit == OpenRC) {
            if (QFile::exists(QStringLiteral("/etc/conf.d/hwclock"))) {
                execute(QStringLiteral("sed -i \"s/clock=.*/clock=\\\"UTC\\\"/\" /etc/conf.d/hwclock"));
            }
        }
        execute("hwclock --systohc --" + QString(rtcUTC?"utc":"localtime"));
    }
}

// NETWORK TIME

void MXDateTime::on_pushSyncNow_clicked()
{
    if (!validateServerList()) return;
    setClockLock(true);

    // Command preparation.
    const int serverCount = tableServers->rowCount();
    bool checked = false;
    bool rexit = false;

    // Run ntpdate one server at a time and break at first succesful update
    const QString cmd = QFile::exists(QStringLiteral("/usr/bin/ntpdig")) ? "ntpdig -Ss --steplimit=500 " : "ntpdate -u ";
    for (int ixi = 0; ixi < serverCount; ++ixi) {
        QTableWidgetItem *item = tableServers->item(ixi, 1);
        const QString &address = item->text().trimmed();
        if (item->checkState() == Qt::Checked) {
            checked = true;
            QString btext = pushSyncNow->text();
            pushSyncNow->setText(tr("Updating..."));
            rexit = execute(cmd + address);
            pushSyncNow->setText(btext);
        }
        if (rexit) break;
    }

    // Finishing touches.
    setClockLock(false);
    dateDelta = 0;
    timeDelta = 0;
    updater.setInterval(0);
    if (rexit) {
        QMessageBox::information(this, windowTitle(), tr("The system clock was updated successfully."));
    } else if (checked) {
        QMessageBox::warning(this, windowTitle(), tr("The system clock could not be updated."));
    } else {
        QMessageBox::critical(this, windowTitle(), tr("None of the NTP servers on the list are currently enabled."));
    }
}
void MXDateTime::on_tableServers_itemSelectionChanged()
{
    const QList<QTableWidgetSelectionRange> &ranges = tableServers->selectedRanges();
    bool remove = false, up = false, down = false;
    if (ranges.count() == 1) {
        const QTableWidgetSelectionRange &range = ranges.at(0);
        remove = true;
        if (range.topRow() > 0) up = true;
        if (range.bottomRow() < (tableServers->rowCount() - 1)) down = true;
    }
    pushServerRemove->setEnabled(remove);
    pushServerMoveUp->setEnabled(up);
    pushServerMoveDown->setEnabled(down);
}
void MXDateTime::on_pushServerAdd_clicked()
{
    QTableWidgetItem *item = addServerRow(true, QStringLiteral("server"), QString(), QString());
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
}
void MXDateTime::on_pushServerMoveUp_clicked()
{
    moveServerRow(-1);
}
void MXDateTime::on_pushServerMoveDown_clicked()
{
    moveServerRow(1);
}

QTableWidgetItem *MXDateTime::addServerRow(bool enabled, const QString &type, const QString &address, const QString &options)
{
    QComboBox *itemComboType = new QComboBox(tableServers);
    QTableWidgetItem *item = new QTableWidgetItem(address);
    QTableWidgetItem *itemOptions = new QTableWidgetItem(options);
    itemComboType->addItem(QStringLiteral("Pool"), QVariant("pool"));
    itemComboType->addItem(QStringLiteral("Server"), QVariant("server"));
    itemComboType->addItem(QStringLiteral("Peer"), QVariant("peer"));
    itemComboType->setCurrentIndex(itemComboType->findData(QVariant(type)));
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
        int end, row;
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
        if (address.isEmpty()) allValid = false;
    }
    const char *msg = nullptr;
    if (serverCount <= 0) msg = "There are no NTP servers on the list.";
    else if (!allValid) msg = "There are invalid entries on the NTP server list.";
    if (msg) {
        QMessageBox::critical(this, windowTitle(), tr(msg));
        return false;
    }
    return true;
}

void MXDateTime::loadNetworkTime()
{
    QFile file(QStringLiteral("/etc/ntpsec/ntp.conf"));
    if (!file.exists()) file.setFileName(QStringLiteral("/etc/ntp.conf"));

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray conf;
        while (tableServers->rowCount() > 0) tableServers->removeRow(0);
        confServers.clear();
        while (!file.atEnd()) {
            const QByteArray &bline = file.readLine();
            const QString line(bline.trimmed());
            const QRegularExpression tregex(QStringLiteral("^#?(pool|server|peer)\\s"));
            if (!line.contains(tregex)) conf.append(bline);
            else {
                QStringList args = line.split(QRegularExpression(QStringLiteral("\\s")), QString::SkipEmptyParts);
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
                confServers.append('\n');
                confServers.append(line);
            }
        }
        confBaseNTP = conf.trimmed();
        file.close();
    }
    if (sysInit == SystemD) enabledNTP = execute(QStringLiteral("bash -c \"LANG=C systemctl status ntp* |grep Loaded | grep service |cut -d';' -f2 |grep -q enabled\""));
    else enabledNTP = execute(QStringLiteral("bash -c \"ls /etc/rc*.d | grep ntp | grep '^S'"));
    checkAutoSync->setChecked(enabledNTP);
}
void MXDateTime::saveNetworkTime()
{
    const bool ntp = checkAutoSync->isChecked();
    if (ntp != enabledNTP) {
        if (sysInit == SystemD) {
            execute("timedatectl set-ntp " + QString(ntp?"1":"0"));
        } else if (sysInit == OpenRC) {
            if (QFile::exists(QStringLiteral("/etc/init.d/ntpd"))) {
                execute("rc-update " + QString(ntp?"add":"del") + " ntpd");
            }
        } else if (ntp){
            execute(QStringLiteral("update-rc.d ntp enable"));
            execute(QStringLiteral("service ntp start"));
        } else {
            execute(QStringLiteral("service ntp stop"));
            execute(QStringLiteral("update-rc.d ntp disable"));
        }
    }
    QByteArray confServersNew;
    for (int ixi = 0; ixi < tableServers->rowCount(); ++ixi) {
        QComboBox *comboType = qobject_cast<QComboBox *>(tableServers->cellWidget(ixi, 0));
        QTableWidgetItem *item = tableServers->item(ixi, 1);
        confServersNew.append('\n');
        if (item->checkState() != Qt::Checked) confServersNew.append('#');
        confServersNew.append(comboType->currentData().toString());
        confServersNew.append(' ');
        confServersNew.append(item->text().trimmed());
        const QString &options = tableServers->item(ixi, 2)->text().trimmed();
        if (!options.isEmpty()) {
            confServersNew.append(' ');
            confServersNew.append(options);
        }
    }
    if (confServersNew != confServers) {
        QFile file(QStringLiteral("/etc/ntpsec/ntp.conf"));
        if (!file.exists()) file.setFileName(QStringLiteral("/etc/ntp.conf"));
        file.copy(file.fileName() + ".bak");
        if (file.open(QFile::WriteOnly | QFile::Text)){
            file.write(confBaseNTP);
            file.write("\n\n# Generated by MX Date & Time");
            file.write(confServersNew);
            file.close();
        }
    }
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
    if (loadedTabs & 2) saveHardwareClock();
    if (loadedTabs & 4) saveNetworkTime();

    // Refresh the UI (especially the current tab) with newly set values.
    loadedTabs = 0;
    setClockLock(false);
}
void MXDateTime::on_pushClose_clicked()
{
    qApp->exit(0);
}
// MX Standard User Interface
void MXDateTime::on_pushAbout_clicked()
{
    displayAboutMsgBox(tr("About MX Date & Time"), "<p align=\"center\"><b><h2>" + tr("MX Date & Time") +
                       "</h2></b></p><p align=\"center\">" + tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>" +
                       tr("GUI program for setting the time and date in MX Linux") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                       QStringLiteral("/usr/share/doc/mx-datetime/license.html"),
                       tr("%1 License").arg(this->windowTitle()));
}

void MXDateTime::on_pushHelp_clicked()
{
    QString url = QStringLiteral("/usr/share/doc/mx-datetime/mx-datetime.html");
    displayDoc(url, tr("MX Date & Time Help").toUtf8());
}

// SUBCLASSING FOR QTimeEdit THAT FIXES CURSOR AND SELECTION JUMPING EVERY SECOND

MTimeEdit::MTimeEdit(QWidget *parent) : QTimeEdit(parent)
{
    // Ensure the widget is not too wide.
    QString fmt(displayFormat());
    if (fmt.section(':', 0, 0).length() < 2) fmt.insert(0, QChar('X'));
    setMaximumWidth(fontMetrics().boundingRect(fmt).width() + (width() - lineEdit()->width()));
}
void MTimeEdit::updateDateTime(const QDateTime &dateTime)
{
    QLineEdit *ledit = lineEdit();
    // Original cursor position and selections
    int select = ledit->selectionStart();
    int cursor = ledit->cursorPosition();
    // Calculation for backward selections.
    if (select >= 0 && select >= cursor) {
        const int cslength = ledit->selectedText().length();
        if (cslength > 0) select = cursor + cslength;
    }
    // Set the date/time as normal.
    setDateTime(dateTime);
    // Restore cursor and selection.
    if (select >= 0) ledit->setSelection(select, cursor - select);
    else ledit->setCursorPosition(cursor);
}

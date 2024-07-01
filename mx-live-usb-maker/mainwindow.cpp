/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QScrollBar>

#include "about.h"
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(const QStringList &args, QDialog *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeneralConnections();
    // Setup options
    LUM = "/usr/local/bin/";
    QSettings settings("/etc/mx-live-usb-maker/mx-live-usb-maker.conf", QSettings::NativeFormat);
    LUM += settings.value("LUM", "live-usb-maker").toString();
    size_check = settings.value("SizeCheck", 128).toUInt(); // in GB
    qDebug() << "LUM is:" << LUM;

    setWindowFlags(Qt::Window); // for the close, min, and max buttons
    setup();
    ui->comboUsb->addItems(buildUsbList());
    if (args.size() > 1 && args.at(1) != "%f") {
        QString fileName = QFileInfo(args.at(1)).absoluteFilePath();
        if (QFileInfo(fileName).isFile()) {
            ui->pushSelectSource->setText(fileName);
            ui->pushSelectSource->setProperty("filename", fileName);
            ui->pushSelectSource->setToolTip(fileName);
            ui->pushSelectSource->setIcon(QIcon::fromTheme("media-cdrom"));
            ui->pushSelectSource->setStyleSheet("text-align: left;");
            setDefaultMode(fileName);
        }
    }
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::checkDestSize()
{

    const quint64 disk_size
        = cmd.getOut("lsblk --output SIZE -n --bytes /dev/" + device + " | head -1", true).toULongLong()
          / static_cast<quint64>(1024 * 1024 * 1024);

    if (disk_size > size_check) { // When writing on large drives (potentially unintended)
        return (QMessageBox::Yes
                == QMessageBox::question(
                    this, tr("Confirmation"),
                    tr("Target device %1 is larger than %2 GB. Do you wish to proceed?").arg(device).arg(size_check),
                    QMessageBox::No | QMessageBox::Yes, QMessageBox::No));
    } else {
        return true;
    }
}

bool MainWindow::isRunningLive()
{
    QProcess process;
    process.start("df", {"-T", "/"});
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QRegularExpression re(R"(.*\n\S+\s+(\S+)\s+)");
    QRegularExpressionMatch match = re.match(output);
    QString fileSystemType = match.captured(1);
    return (fileSystemType == "aufs" || fileSystemType == "overlay");
}

bool MainWindow::isToRam()
{
    return QFileInfo::exists("/live/config/did-toram");
}

void MainWindow::makeUsb(const QString &options)
{
    device = ui->comboUsb->currentText().split(' ').first();

    QString source = '"' + ui->pushSelectSource->property("filename").toString() + '"';
    QString source_size;
    if (!ui->checkCloneLive->isChecked() && !ui->checkCloneMode->isChecked()) {
        source_size = cmd.getOut("du -m " + source + " 2>/dev/null | cut -f1", true);
    } else if (ui->checkCloneMode->isChecked()) {
        source_size = cmd.getOut("du -m --summarize " + source + " 2>/dev/null | cut -f1", true);
        QString root_partition = cmd.getOut("df --output=source " + source + " | awk 'END{print $1}'");
        source = "clone=" + source.remove('"');
        if ("/dev/" + device == cmd.getOut(cli_utils + "get_drive " + root_partition)) {
            QMessageBox::critical(this, tr("Failure"),
                                  tr("Source and destination are on the same device, please select again."));
            ui->stackedWidget->setCurrentWidget(ui->selectionPage);
            ui->pushNext->setEnabled(true);
            setCursor(Qt::ArrowCursor);
            return;
        }
    } else if (ui->checkCloneLive->isChecked()) {
        source = "clone";
        QString path = isToRam() ? "/live/to-ram" : "/live/boot-dev";
        source_size = cmd.getOut("du -m --summarize " + path + " 2>/dev/null | cut -f1", true);
    }

    if (!checkDestSize()) {
        ui->stackedWidget->setCurrentWidget(ui->selectionPage);
        ui->pushNext->setEnabled(true);
        setCursor(Qt::ArrowCursor);
        return;
    }

    // Check amount of io on device before copy, this is in sectors
    const quint64 start_io = cmd.getOut("awk '{print $7}' /sys/block/" + device + "/stat", true).toULongLong();
    ui->progBar->setMinimum(static_cast<int>(start_io));
    const quint64 iso_sectors = source_size.toULongLong() * 2048; // source_size * 1024 / 512 * 1024
    ui->progBar->setMaximum(static_cast<int>(iso_sectors + start_io));

    QString cmdstr = (LUM + " gui " + options + " -C off --from=%1 -t /dev/%2").arg(source, device);
    if (ui->radioDd->isChecked()) {
        cmdstr = LUM + " gui partition-clear -NC off --target " + device;
        connect(&cmd, &Cmd::readyReadStandardOutput, this, &MainWindow::updateOutput);
        qDebug() << cmd.getOutAsRoot(cmdstr, true);
        cmdstr = "dd bs=1M if=" + source + " of=/dev/" + device;
        ui->outputBox->insertPlainText(tr("Writing %1 using 'dd' command to /dev/%2,\n\n"
                                          "Please wait until the process is completed")
                                           .arg(source, device));
    }
    setConnections();
    stat_file = new QFile("/sys/block/" + device + "/stat");
    cmd.runAsRoot(cmdstr);
}

void MainWindow::setup()
{
    connect(QApplication::instance(), &QApplication::aboutToQuit, this, &MainWindow::cleanup);
    setWindowTitle(tr("MX Live Usb Maker"));

    QFont font("monospace");
    font.setStyleHint(QFont::Monospace);
    ui->outputBox->setFont(font);

    ui->groupAdvOptions->setVisible(false);
    advancedOptions = false;
    ui->pushBack->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->pushCancel->setEnabled(true);
    ui->pushNext->setEnabled(true);
    ui->outputBox->setCursorWidth(0);
    height = heightMM();

    QRegularExpression rx("\\w*");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->textLabel->setValidator(validator);

    // Set save boot directory option to disable unless update mode is checked
    ui->checkSaveBoot->setEnabled(false);

    // Enable clone live option only if running live and not encrypted
    bool isEncrypted = QFile::exists("/live/config/encrypted");
    ui->checkCloneLive->setEnabled(isRunningLive() && !isEncrypted);

    // Dynamically show or hide data format options based on availability
    bool dataFirstAvailable = cmd.run(LUM + " --help | grep -q -- --data-first", true);
    ui->comboBoxDataFormat->setVisible(dataFirstAvailable);
    ui->checkDataFirst->setVisible(dataFirstAvailable);
    ui->spinBoxDataSize->setVisible(dataFirstAvailable);
    ui->labelFormat->setVisible(dataFirstAvailable);
}

void MainWindow::setGeneralConnections()
{
    connect(ui->checkCloneLive, &QCheckBox::clicked, this, &MainWindow::checkCloneLive_clicked);
    connect(ui->checkCloneMode, &QCheckBox::clicked, this, &MainWindow::checkCloneMode_clicked);
    connect(ui->checkDataFirst, &QCheckBox::clicked, this, &MainWindow::checkDataFirst_clicked);
    connect(ui->checkUpdate, &QCheckBox::clicked, this, &MainWindow::checkUpdate_clicked);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushBack, &QPushButton::clicked, this, &MainWindow::pushBack_clicked);
    connect(ui->pushCancel, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushLumLogFile, &QPushButton::clicked, this, &MainWindow::pushLumLogFile_clicked);
    connect(ui->pushNext, &QPushButton::clicked, this, &MainWindow::pushNext_clicked);
    connect(ui->pushOptions, &QPushButton::clicked, this, &MainWindow::pushOptions_clicked);
    connect(ui->pushRefresh, &QPushButton::clicked, this, &MainWindow::pushRefresh_clicked);
    connect(ui->pushSelectSource, &QPushButton::clicked, this, &MainWindow::pushSelectSource_clicked);
    connect(ui->radioDd, &QRadioButton::clicked, this, &MainWindow::radioDd_clicked);
    connect(ui->radioNormal, &QRadioButton::clicked, this, &MainWindow::radioNormal_clicked);
    connect(ui->spinBoxSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::spinBoxSize_valueChanged);
    connect(ui->textLabel, &QLineEdit::textChanged, this, &MainWindow::textLabel_textChanged);
}

// Build the option list to be passed to live-usb-maker
QString MainWindow::buildOptionList()
{
    QStringList optionsList {"-N"};

    // Map the checkboxes to the corresponding options
    std::map<QCheckBox *, QString> checkboxOptions {
        {ui->checkEncrypt, "-E"},
        {ui->checkGpt, "-g"},
        {ui->checkKeep, "-k"},
        {ui->checkPretend, "-p"},
        {ui->checkSaveBoot, "-S"},
        {ui->checkUpdate, "-u"},
        {ui->checkSetPmbrBoot, "--gpt-pmbr"},
        {ui->checkForceUsb, "--force=usb"},
        {ui->checkForceAutomount, "--force=automount"},
        {ui->checkForceMakefs, "--force=makefs"},
        {ui->checkForceNofuse, "--force=nofuse"},
    };

    // Add options for the checked checkboxes
    for (auto [checkBox, option] : checkboxOptions) {
        if (checkBox->isChecked()) {
            optionsList.append(option);
        }
    }

    // Add additional options
    if (ui->spinBoxEsp->value() != 50) {
        optionsList.append("--esp-size=" + ui->spinBoxEsp->cleanText());
    }
    if (ui->spinBoxSize->value() < ui->spinBoxSize->maximum()) {
        optionsList.append("--size=" + ui->spinBoxSize->cleanText());
    }
    if (!ui->textLabel->text().isEmpty()) {
        optionsList.append("--label=" + ui->textLabel->text());
    }
    if (ui->checkDataFirst->isChecked()) {
        optionsList.append("--data-first=" + ui->spinBoxDataSize->cleanText() + ","
                           + ui->comboBoxDataFormat->currentText());
    }

    // Add the verbosity option
    if (ui->sliderVerbosity->value() == 1) {
        optionsList.append("-V");
    } else if (ui->sliderVerbosity->value() == 2) {
        optionsList.append("-VV");
    }

    QString options = optionsList.join(' ');
    qDebug() << "Options: " << options;
    return options;
}

void MainWindow::cleanup()
{
    QFileInfo lum(LUM);
    QFileInfo logfile("/tmp/" + lum.baseName() + ".log");
    if (logfile.exists()) {
        QFile::remove(logfile.absoluteFilePath());
    }
    if (cmd.state() != QProcess::NotRunning) {
        QTimer::singleShot(10s, this, [this] {
            if (cmd.state() != QProcess::NotRunning) {
                Cmd().runAsRoot("kill -9 -- -" + QString::number(cmd.processId()), true);
            }
        });
        Cmd().runAsRoot("kill -- -" + QString::number(cmd.processId()), true);
    }
    const QString mount_path = "/run/live-usb-maker";
    if (Cmd().run("mountpoint -q " + mount_path, true)) {
        Cmd().runAsRoot("umount -Rl " + mount_path, true);
    }
    if (Cmd().run("mountpoint -q " + mount_path + "/main", true)) {
        Cmd().runAsRoot("umount -l " + mount_path + "/{main,uefi}", true);
    }
    QString pid = QString::number(QApplication::applicationPid());
    if (!Cmd().run("ps --ppid " + pid, true)) {
        Cmd().runAsRoot("kill -- -" + pid, true);
    }
}

QStringList MainWindow::buildUsbList()
{
    QString drives = cmd.getOut("lsblk --nodeps -nlo NAME,SIZE,MODEL,VENDOR -I 3,8,22,179,259", true).trimmed();
    return removeUnsuitable(drives.split('\n'));
}

// Remove unsuitable drives from the list (live and unremovable)
QStringList MainWindow::removeUnsuitable(const QStringList &devices)
{
    QStringList suitableDevices;
    suitableDevices.reserve(devices.size());
    QString liveDrive
        = cmd.getOut(cli_utils + "get_drive $(get_live_dev)", true).trimmed().remove(QRegularExpression("^/dev/"));
    QString rootDrive
        = cmd.getOut("lsblk -nlso NAME,PKNAME,TYPE $(findmnt / -no SOURCE) | grep 'disk' | awk '{print $1}'", true)
              .trimmed();
    for (const QString &deviceInfo : devices) {
        QString deviceName = deviceInfo.split(' ').first();
        bool isUsbOrRemovable
            = ui->checkForceUsb->isChecked() || cmd.run(cli_utils + "is_usb_or_removable " + deviceName.toUtf8(), true);
        if (isUsbOrRemovable && deviceName != liveDrive && deviceName != rootDrive) {
            suitableDevices.append(deviceInfo);
        }
    }
    return suitableDevices;
}

void MainWindow::cmdDone()
{
    timer.stop();
    ui->progBar->setValue(ui->progBar->maximum());
    setCursor(Qt::ArrowCursor);
    ui->pushBack->show();
    if ((cmd.exitCode() == 0 && cmd.exitStatus() == QProcess::NormalExit) || ui->checkPretend->isChecked()) {
        QMessageBox::information(this, tr("Success"), tr("LiveUSB creation successful!"));
    } else {
        cleanup();
        QMessageBox::critical(this, tr("Failure"), tr("Error encountered in the LiveUSB creation process"));
    }
    cmd.disconnect();
}

void MainWindow::setConnections()
{
    timer.start(1s);
    connect(&cmd, &Cmd::readyReadStandardOutput, this, &MainWindow::updateOutput);
    connect(&timer, &QTimer::timeout, this, &MainWindow::updateBar);
    connect(&cmd, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::cmdDone);
}

// Set proper default mode based on iso contents
void MainWindow::setDefaultMode(const QString &iso_name)
{
    if (!isantiX_mx_family(iso_name)) {
        ui->radioDd->click();
        ui->radioNormal->setChecked(false);
    } else {
        ui->radioDd->setChecked(false);
        ui->radioNormal->click();
    }
}

void MainWindow::updateBar()
{
    stat_file->open(QIODevice::ReadOnly);
    QString out = stat_file->readAll();
    quint64 current_io = out.section(QRegularExpression("\\s+"), 7, 7).toULongLong();
    ui->progBar->setValue(static_cast<int>(current_io));
    stat_file->close();
}

void MainWindow::updateOutput()
{
    QString output = cmd.readAllStandardOutput();
    // Remove escape sequences that are not handled by code
    const QRegularExpression re(u8R"(\[0m|\]0;|\|\|\[1000D|\[74C||\[\?25l|\[\?25h|\[0;36m|\[1;37m)");
    output.remove(re);
    ui->outputBox->moveCursor(QTextCursor::End);
    if (output.contains('\r')) {
        ui->outputBox->moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
        ui->outputBox->moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    ui->outputBox->insertPlainText(output);

    QScrollBar *sb = ui->outputBox->verticalScrollBar();
    sb->setValue(sb->maximum());
    QApplication::processEvents();
}

void MainWindow::pushNext_clicked()
{
    if (ui->stackedWidget->currentIndex() != 0) {
        return;
    }
    if (ui->comboUsb->currentText().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please select a USB device to write to"));
        return;
    }
    QString msg = tr("These actions will destroy all data on \n\n") + ui->comboUsb->currentText().simplified() + "\n\n "
                  + tr("Do you wish to continue?");
    if (QMessageBox::Yes != QMessageBox::warning(this, windowTitle(), msg, QMessageBox::Yes, QMessageBox::No)) {
        return;
    }
    QString sourceFilename = ui->pushSelectSource->property("filename").toString();
    if (!QFileInfo::exists(sourceFilename) && sourceFilename != "clone") {
        emit ui->pushSelectSource->clicked();
        return;
    }
    if (cmd.state() != QProcess::NotRunning) {
        ui->stackedWidget->setCurrentWidget(ui->outputPage);
        return;
    }
    ui->pushNext->setEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->outputPage);
    makeUsb(buildOptionList());
}

void MainWindow::pushBack_clicked()
{
    setWindowTitle("MX Live Usb Maker");
    ui->stackedWidget->setCurrentIndex(0);
    ui->pushNext->setEnabled(true);
    ui->pushBack->hide();
    ui->outputBox->clear();
    ui->progBar->setValue(0);
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About %1").arg(windowTitle()),
        "<p align=\"center\"><b><h2>" + windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Program for creating a live-usb from an iso-file, another live-usb, a live-cd/dvd, or a running live "
                 "system.")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-live-usb-maker/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    QString url = "/usr/share/doc/mx-live-usb-maker/mx-live-usb-maker.html";
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::pushSelectSource_clicked()
{
    const QString user = cmd.getOut("logname", true);
    const QString home = "/home/" + user;
    if (QString selected; !ui->checkCloneLive->isChecked() && !ui->checkCloneMode->isChecked()) {
        selected = QFileDialog::getOpenFileName(this, tr("Select an ISO file to write to the USB drive"), home,
                                                tr("ISO Files (*.iso);;All Files (*.*)"));
        if (!selected.isEmpty()) {
            ui->pushSelectSource->setText(selected);
            ui->pushSelectSource->setProperty("filename", selected);
            ui->pushSelectSource->setToolTip(selected);
            ui->pushSelectSource->setIcon(QIcon::fromTheme("media-cdrom"));
            ui->pushSelectSource->setStyleSheet("text-align: left;");
            setDefaultMode(selected); // Set proper default mode based on iso contents
        }
    } else if (ui->checkCloneMode->isChecked()) {
        selected = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"), QDir::rootPath(),
                                                     QFileDialog::ShowDirsOnly);
        if (QFileInfo::exists(selected + "/antiX/linuxfs") || QFileInfo::exists(selected + "/linuxfs")) {
            ui->pushSelectSource->setText(selected);
            ui->pushSelectSource->setProperty("filename", selected);
        } else {
            if (selected == '/') {
                selected.clear();
            }
            QMessageBox::critical(this, tr("Failure"),
                                  tr("Could not find %1/antiX/linuxfs file")
                                      .arg(selected)); // TODO: -- the file might be in %/linuxfs too for frugal
        }
    }
}

void MainWindow::pushRefresh_clicked()
{
    ui->comboUsb->clear();
    ui->comboUsb->addItems(buildUsbList());
}

void MainWindow::pushOptions_clicked()
{
    if (advancedOptions) {
        ui->pushOptions->setText(tr("Show advanced options"));
        ui->groupAdvOptions->hide();
        advancedOptions = false;
        ui->pushOptions->setIcon(QIcon::fromTheme("go-down"));
        setMaximumHeight(height);
    } else {
        ui->pushOptions->setText(tr("Hide advanced options"));
        ui->groupAdvOptions->show();
        advancedOptions = true;
        ui->pushOptions->setIcon(QIcon::fromTheme("go-up"));
    }
}

void MainWindow::textLabel_textChanged(QString arg1)
{
    ui->textLabel->setText(arg1.remove(' '));
    ui->textLabel->setCursorPosition(arg1.length());
}

void MainWindow::checkUpdate_clicked(bool checked)
{
    ui->checkSaveBoot->setEnabled(checked);
    if (!checked) {
        ui->checkSaveBoot->setChecked(false);
    }
}

void MainWindow::checkCloneMode_clicked(bool checked)
{
    if (checked) {
        ui->pushSelectSource->setStyleSheet("text-align: center;");
        ui->checkCloneLive->setChecked(false);
        checkCloneLive_clicked(false);
        ui->label_3->setText("<b>" + tr("Select Source") + "</b>");
        ui->pushSelectSource->setText(tr("Select Source Directory"));
        ui->pushSelectSource->setIcon(QIcon::fromTheme("folder"));
        ui->radioDd->setDisabled(true);
    } else {
        ui->label_3->setText("<b>" + tr("Select ISO file") + "</b>");
        ui->pushSelectSource->setText(tr("Select ISO"));
        ui->pushSelectSource->setIcon(QIcon::fromTheme("user-home"));
        ui->checkCloneLive->setEnabled(isRunningLive());
        ui->radioDd->setEnabled(true);
    }
    ui->pushSelectSource->setProperty("filename", "");
}

void MainWindow::checkCloneLive_clicked(bool checked)
{
    if (checked) {
        ui->pushSelectSource->setStyleSheet("text-align: center;");
        ui->checkCloneMode->setChecked(false);
        ui->label_3->setText("<b>" + tr("Select Source") + "</b>");
        ui->pushSelectSource->setEnabled(false);
        ui->pushSelectSource->setText(tr("clone"));
        ui->pushSelectSource->setProperty("filename", "clone");
        ui->pushSelectSource->setIcon(QIcon::fromTheme("tools-media-optical-copy"));
        ui->pushSelectSource->blockSignals(true);
        ui->radioDd->setDisabled(true);
    } else {
        ui->label_3->setText("<b>" + tr("Select ISO file") + "</b>");
        ui->pushSelectSource->setEnabled(true);
        ui->pushSelectSource->setText(tr("Select ISO"));
        ui->pushSelectSource->setProperty("filename", "");
        ui->pushSelectSource->setIcon(QIcon::fromTheme("user-home"));
        ui->pushSelectSource->blockSignals(false);
        ui->radioDd->setDisabled(false);
    }
}

void MainWindow::radioDd_clicked()
{
    ui->radioNormal->setChecked(false);
    ui->checkCloneLive->setChecked(false);
    ui->checkCloneMode->setChecked(false);
    ui->checkEncrypt->setChecked(false);
    ui->checkCloneLive->setEnabled(false);
    ui->checkCloneMode->setEnabled(false);
    ui->checkEncrypt->setEnabled(false);
    ui->checkPretend->setEnabled(false);
    if (ui->groupAdvOptions->isVisible()) {
        pushOptions_clicked();
    }
    ui->pushOptions->setEnabled(false);
    ui->label_percent->setEnabled(false);
    ui->label_part_label->setEnabled(false);
    ui->spinBoxSize->setEnabled(false);
    ui->textLabel->setEnabled(false);
}

void MainWindow::radioNormal_clicked()
{
    ui->checkCloneLive->setEnabled(isRunningLive());
    ui->checkCloneMode->setEnabled(true);
    ui->checkEncrypt->setEnabled(true);
    ui->checkPretend->setEnabled(true);
    ui->pushOptions->setEnabled(true);
    ui->label_percent->setEnabled(true);
    ui->label_part_label->setEnabled(true);
    ui->spinBoxSize->setEnabled(true);
    ui->textLabel->setEnabled(true);
}

bool MainWindow::isantiX_mx_family(const QString &selected)
{
    return Cmd().run(
        QStringLiteral("xorriso -indev '%1' -find /antiX -name linuxfs -prune  2>/dev/null | grep -q /antiX/linuxfs")
            .arg(selected),
        true);
}

void MainWindow::pushLumLogFile_clicked()
{
    QFileInfo lum(LUM);
    QString logFileName = lum.baseName() + ".log";
    QString logFilePath = "/var/log/" + logFileName;
    QString tempLogFilePath = "/tmp/" + logFileName;
    qDebug() << "lumlog" << tempLogFilePath;

    if (!QFileInfo::exists(logFilePath)) {
        QMessageBox::information(this, QApplication::applicationName(),
                                 tr("Could not find a log file at: ") + logFilePath);
        return;
    }

    // Generate temporary log file by reversing the log file until the delimiter, then reversing it back
    QString cmd_str = QString("tac %1 | sed '/^={60}=$/q' | tac > %2").arg(logFilePath, tempLogFilePath);
    Cmd().run(cmd_str);
    displayDoc(tempLogFilePath, lum.baseName());
}

void MainWindow::spinBoxSize_valueChanged(int arg1)
{
    const int max = ui->spinBoxSize->maximum();
    ui->checkDataFirst->setEnabled(arg1 == max);
    ui->spinBoxDataSize->setEnabled(arg1 == max);
    ui->comboBoxDataFormat->setEnabled(arg1 == max);
    ui->labelFormat->setEnabled(arg1 == max);
}

void MainWindow::checkDataFirst_clicked(bool checked)
{
    ui->spinBoxSize->setDisabled(checked);
}

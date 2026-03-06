/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2024-2025 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
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
#include "qapplication.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QRegularExpression>

#include <QCollator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QStorageInfo>
#include <QTextStream>
#include <QTimer>

#include "about.h"
#include "cmd.h"
#include "common.h"

// Trying to map all the persistence type to values that make sense
// when passed to the kernel at boot time for frugal installation
const QMap<QString, QString> MainWindow::PERSISTENCE_TYPES = {{"persist_all", "persist_all"},
                                                              {"persist_root", "persist_root"},
                                                              {"persist_static", "persist_static"},
                                                              {"persist_static_root", "persist_static_root"},
                                                              {"p_static_root", "persist_static_root"},
                                                              {"persist_home", "persist_home"},
                                                              {"frugal_persist", "persist_all"},
                                                              {"frugal_root", "persist_root"},
                                                              {"frugal_static", "persist_static"},
                                                              {"frugal_static_root", "persist_static_root"},
                                                              {"f_static_root", "persist_static_root"},
                                                              {"frugal_home", "persist_home"},
                                                              {"frugal_only", "frugal_only"}};

MainWindow::MainWindow(const QCommandLineParser &argParser, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setWindowTitle(QApplication::applicationDisplayName());

    setup();
    setConnections();

    if (argParser.isSet("frugal")) {
        qDebug() << "Frugal mode";
        promptFrugalStubInstall();
        ui->tabWidget->setCurrentIndex(Tab::Frugal);
        refreshFrugal();
    }
}

MainWindow::~MainWindow()
{
    settings.setValue("geometry", saveGeometry());
    QStringList cleanupArgs = {"cleanup_temp"};
    if (!newMounts.isEmpty()) {
        cleanupArgs << "--mounts" << newMounts;
    }
    if (!newDirectories.isEmpty()) {
        cleanupArgs << "--dirs" << newDirectories;
    }
    if (!newLuksDevices.isEmpty()) {
        cleanupArgs << "--luks" << newLuksDevices;
    }
    if (!cmd.procElevated("/usr/lib/uefi-manager/uefimanager-lib", cleanupArgs)) {
        qWarning() << "Cleanup failed";
    }

    cmd.procElevated("/usr/lib/uefi-manager/uefimanager-lib", {"copy_log"});

    delete ui;
}

QStringList MainWindow::getEspDevicePaths()
{
    QString lsblkJson;
    cmd.proc("lsblk", {"-ln", "--json", "-o", "PATH,PARTTYPE,FSTYPE,TYPE"}, &lsblkJson);

    QJsonDocument doc = QJsonDocument::fromJson(lsblkJson.toUtf8());
    QJsonArray devices = doc.object().value("blockdevices").toArray();

    QStringList paths;
    for (const QJsonValue &val : devices) {
        QJsonObject dev = val.toObject();
        if (dev.value("type").toString() != "part") {
            continue;
        }
        const QString parttype = dev.value("parttype").toString().toLower();
        if ((parttype == ESP_GUID_GPT || parttype == ESP_TYPE_MBR)
            && dev.value("fstype").toString().compare("vfat", Qt::CaseInsensitive) == 0) {
            paths.append(dev.value("path").toString());
        }
    }
    return paths;
}

void MainWindow::addUefiEntry(QListWidget *listEntries, QWidget *dialogUefi)
{
    // Mount all ESPs
    const QStringList partList = getEspDevicePaths();

    for (const auto &device : std::as_const(partList)) {
        if (!cmd.procAsRoot("findmnt", {"-n", device})) {
            QString partName = device.section('/', -1);
            QString mountDir = "/boot/efi/" + partName;
            if (!QDir(mountDir).exists()) {
                cmd.procAsRoot("mkdir", {"-p", mountDir});
                newDirectories.append(mountDir);
            }
            if (cmd.procAsRoot("mount", {device, mountDir})) {
                newMounts.append(mountDir);
            }
        }
    }

    const QString initialPath = QFile::exists("/boot/efi/EFI") ? "/boot/efi/EFI" : "/boot/efi/";
    QString fileName
        = QFileDialog::getOpenFileName(dialogUefi, tr("Select EFI file"), initialPath, tr("EFI files (*.efi *.EFI)"));

    if (!QFile::exists(fileName)) {
        return;
    }

    QString dfOut;
    cmd.proc("df", {"--output=source", fileName}, &dfOut);
    const QStringList dfLines = dfOut.split('\n', Qt::SkipEmptyParts);
    const QString partitionName = dfLines.size() >= 2 ? dfLines.last().trimmed() : QString();

    if (partitionName.isEmpty() || !partitionName.startsWith("/dev/")) {
        QMessageBox::critical(dialogUefi, tr("Error"), tr("Could not find the source mountpoint for %1").arg(fileName));
        return;
    }

    QString pkname;
    cmd.proc("lsblk", {"-no", "PKNAME", partitionName}, &pkname);
    const QString disk = "/dev/" + pkname.trimmed();
    const QRegularExpressionMatch partMatch = QRegularExpression("[0-9]+$").match(partitionName);
    const QString partition = partMatch.hasMatch() ? partMatch.captured() : QString();

    if (cmd.exitCode() != 0 || partition.isEmpty() || pkname.trimmed().isEmpty()) {
        QMessageBox::critical(dialogUefi, tr("Error"), tr("Could not find the source mountpoint for %1").arg(fileName));
        return;
    }

    QString name = QInputDialog::getText(dialogUefi, tr("Set name"), tr("Enter the name for the UEFI menu item:"));
    if (name.isEmpty()) {
        name = "New entry";
    }

    const int efiIdx = fileName.indexOf("/EFI/", 0, Qt::CaseInsensitive);
    if (efiIdx < 0) {
        QMessageBox::critical(dialogUefi, tr("Error"), tr("Selected file is not in an EFI directory"));
        return;
    }
    fileName = "/EFI/" + fileName.mid(efiIdx + 5);
    QString out;
    cmd.procAsRoot("efibootmgr", {"-c", "-L", name, "-d", disk, "-p", partition, "-l", fileName}, &out);

    if (cmd.exitCode() != 0) {
        QMessageBox::critical(dialogUefi, tr("Error"), tr("Something went wrong, could not add entry."));
        return;
    }

    QStringList outList = out.split('\n', Qt::SkipEmptyParts);
    listEntries->insertItem(0, outList.constLast());
    emit listEntries->itemSelectionChanged();
}

void MainWindow::checkDoneStub()
{
    bool allDone = !ui->comboDriveStub->currentText().isEmpty() && !ui->comboPartitionStub->currentText().isEmpty()
                   && !ui->comboKernel->currentText().isEmpty() && !ui->textEntryName->text().isEmpty()
                   && !ui->textKernelOptions->text().isEmpty();
    ui->pushNext->setEnabled(allDone);
}

// Clear existing widgets and layout from tabManageUefi
void MainWindow::clearEntryWidget()
{
    if (ui->tabManageUefi->layout() != nullptr) {
        QLayoutItem *child;
        while ((child = ui->tabManageUefi->layout()->takeAt(0))) {
            if (child->widget()) {
                delete child->widget();
            }
            delete child;
        }
    }
    delete ui->tabManageUefi->layout();
}

void MainWindow::centerWindow()
{
    const auto screenGeometry = QApplication::primaryScreen()->geometry();
    const auto x = (screenGeometry.width() - this->width()) / 2;
    const auto y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

void MainWindow::setup()
{
#ifdef IS_ARCH_BUILD
    ui->tabWidget->setTabVisible(Tab::Frugal, false);
#endif

    auto size = this->size();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        if (this->isMaximized()) { // Add option to resize if maximized
            this->resize(size);
            centerWindow();
        }
    }

    // Refresh blkid cache early
    cmd.proc("blkid");

    // Detect root device/partition once at startup
    detectRootDevice();

    // Refresh appropriate tab content based on current tab
    const auto currentTab = ui->tabWidget->currentIndex();
    switch (currentTab) {
    case Tab::Entries:
        refreshEntries();
        break;
    case Tab::Frugal:
        refreshFrugal();
        break;
    case Tab::StubInstall:
        refreshStubInstall();
        break;
    }
}

void MainWindow::cmdStart()
{
    setCursor(QCursor(Qt::BusyCursor));
}

void MainWindow::cmdDone()
{
    setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::setConnections()
{
    connect(&cmd, &Cmd::done, this, &MainWindow::cmdDone);
    connect(&cmd, &Cmd::started, this, &MainWindow::cmdStart);

    connect(ui->comboDrive, &QComboBox::currentTextChanged, this, &MainWindow::filterDrivePartitions);
    connect(ui->comboDriveStub, &QComboBox::currentTextChanged, this, &MainWindow::filterDrivePartitions);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAboutClicked);
    connect(ui->pushBack, &QPushButton::clicked, this, &MainWindow::pushBackClicked);
    connect(ui->pushCancel, &QPushButton::pressed, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelpClicked);
    connect(ui->pushNext, &QPushButton::clicked, this, &MainWindow::pushNextClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabWidgetCurrentChanged);

    connect(ui->comboDriveStub, &QComboBox::currentTextChanged, this, &MainWindow::checkDoneStub);
    connect(ui->comboKernel, &QComboBox::currentTextChanged, this, &MainWindow::checkDoneStub);
    connect(ui->comboPartitionStub, &QComboBox::currentTextChanged, this, &MainWindow::checkDoneStub);
    connect(ui->textEntryName, &QLineEdit::textChanged, this, &MainWindow::checkDoneStub);
    connect(ui->textKernelOptions, &QLineEdit::textChanged, this, &MainWindow::checkDoneStub);
}

void MainWindow::toggleUefiActive(QListWidget *listEntries)
{
    if (!listEntries) {
        return;
    }

    auto currentItem = listEntries->currentItem();
    if (!currentItem) {
        return;
    }

    QString item = currentItem->text().section(' ', 0, 0).remove(QRegularExpression("^Boot"));
    QString rest = currentItem->text().section(' ', 1, -1);

    if (!item.contains(QRegularExpression(R"(^[0-9A-Z]{4}\*?$)"))) {
        return;
    }

    bool isActive = item.endsWith('*');
    if (isActive) {
        item.chop(1);
    }

    if (Cmd().procAsRoot("efibootmgr", {isActive ? "--inactive" : "--active", "-b", item})) {
        listEntries->currentItem()->setText(QString("Boot%1%2 %3").arg(item, isActive ? "" : "*", rest));
        listEntries->currentItem()->setBackground(isActive ? QBrush(Qt::gray) : QBrush());
    }

    emit listEntries->itemSelectionChanged();
}

void MainWindow::tabWidgetCurrentChanged()
{
    const int currentTab = ui->tabWidget->currentIndex();
    ui->pushNext->setVisible(currentTab == Tab::Frugal || currentTab == Tab::StubInstall);
    ui->pushBack->setVisible(currentTab == Tab::Frugal);

    switch (currentTab) {
    case Tab::Entries:
        refreshEntries();
        break;
    case Tab::Frugal:
        refreshFrugal();
        break;
    case Tab::StubInstall:
        refreshStubInstall();
        break;
    }
}

[[nodiscard]] QString MainWindow::getBootLocation()
{
    QString partition = ui->comboPartitionStub->currentText().section(' ', 0, 0);
    QString mountPoint = getMountPoint(partition);
    if (mountPoint.isEmpty()) {
        mountPoint = mountPartition(partition);
    }
    if (mountPoint.isEmpty()) {
        qWarning() << "Failed to mount partition" << partition;
        return {};
    }

    return getBootLocation(mountPoint);
}

[[nodiscard]] QString MainWindow::getBootLocation(const QString &mountPoint)
{

    // Check /etc/fstab for separate /boot partition
    QFile fstab(mountPoint + "/etc/fstab");
    if (!fstab.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open" << fstab.fileName();
        return mountPoint;
    }

    QString bootPartition;
    QTextStream in(&fstab);
    QString line;
    QRegularExpression regex(R"(^(.+?)\s+(/boot)\s+.*$)");
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        // Skip empty lines or comment lines
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        // Match lines with /boot
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            bootPartition = match.captured(1).trimmed();
            // replace "\040" codes for spaces in LABELs used within fstab
            bootPartition.replace("\\040", " ");
            qDebug().noquote() << "/boot partition :" << bootPartition;
            break;
        }
    }
    fstab.close();

    if (bootPartition.isEmpty()) {
        if (QDir(mountPoint + "/boot").exists()) {
            return mountPoint + "/boot";
        } else {
            qWarning() << "Failed to find boot directory as " << mountPoint;
            return mountPoint;
        }
    }

    // Mount the boot partition
    QString bootMountPoint = mountPartition(bootPartition);
    if (bootMountPoint.isEmpty()) {
        qWarning() << "Failed to mount boot partition" << bootPartition;
        return mountPoint;
    }

    return bootMountPoint;
}

bool MainWindow::copyKernel()
{
    if (espMountPoint.isEmpty()) {
        qWarning() << "ESP mount point is empty.";
        return false;
    }

    const bool isFrugal = ui->tabWidget->currentIndex() == Tab::Frugal;

    const QString subDir = isFrugal ? "/frugal" : "/stub";
    const QString targetPath = espMountPoint + "/EFI/" + distro + subDir;

    // Create target directory if it doesn't exist
    if (!QDir().exists(targetPath)) {
        if (!cmd.procAsRoot("mkdir", {"-p", targetPath})) {
            qWarning() << "Failed to create directory:" << targetPath;
            return false;
        }
    }

    // Copy kernel and initrd files
    const QString sourceDir = isFrugal ? frugalDir : [this]() {
        QString dir = getBootLocation();

        return dir;
    }();
    const QString kernelVersion = ui->comboKernel->currentText();
    QString vmlinuz = QString("%1/vmlinuz%2").arg(sourceDir, isFrugal ? "" : "-" + kernelVersion);
    if (!QFile::exists(vmlinuz)) {
        // Fallback for Arch-style
        vmlinuz = QString("%1/vmlinuz-linux").arg(sourceDir);
    }

    QString initrd = QString("%1/initrd%2").arg(sourceDir, isFrugal ? ".gz" : ".img-" + kernelVersion);
    QString initramfs = QString("%1/initramfs-%2").arg(sourceDir, isFrugal ? "" : kernelVersion + ".img");
    if (!QFile::exists(initrd) && !QFile::exists(initramfs)) {
        // Fallback for Arch-style initramfs
        initramfs = QString("%1/initramfs-linux.img").arg(sourceDir);
    }

    const QString amdUcode = QString("%1/amd-ucode.img").arg(sourceDir);
    const QString intelUcode = QString("%1/intel-ucode.img").arg(sourceDir);

    const QStringList filesToCopy = {vmlinuz, initrd, amdUcode, intelUcode};
    const QStringList targetFiles = {"/vmlinuz", "/initrd.img", "/amducode.img", "/intucode.img"};

    for (int i = 0; i < filesToCopy.size(); ++i) {
        QString file = filesToCopy.at(i);
        const QString targetFile = targetPath + targetFiles.at(i);

        if (targetFile.endsWith("initrd.img") && !QFile::exists(initrd) && QFile::exists(initramfs)) {
            file = initramfs;
        }

        if (!QFile::exists(file) && file.endsWith("ucode.img")) {
            continue;
        }

        if (!QFile::exists(file) && !file.endsWith("ucode.img")) {
            qWarning() << "Source file does not exist:" << file;
            return false;
        }

        if (!cmd.procAsRoot("cp", {file, targetFile})) {
            qWarning() << "Failed to copy file:" << file << "to" << targetFile;
            return false;
        }
    }

    qInfo() << "Kernel and initrd files copied successfully to" << targetPath;
    return true;
}

bool MainWindow::installEfiStub(const QString &esp)
{
    if (esp.isEmpty() || !copyKernel()) {
        return false;
    }

    const QString disk = "/dev/" + utils::extractDiskFromPartition(esp);
    const QRegularExpressionMatch partMatch = QRegularExpression("[0-9]+$").match(esp);
    QString part = partMatch.hasMatch() ? partMatch.captured() : QString();

    if (disk.isEmpty() || part.isEmpty()) {
        return false;
    }

    const bool isFrugal = ui->tabWidget->currentIndex() == Tab::Frugal;
    const QString efiDir = isFrugal ? "frugal" : "stub";
    const QString entryName = isFrugal ? ui->textUefiEntryFrugal->text() : ui->textEntryName->text();

    QStringList args;
    args << "--disk" << disk << "--part" << part << "--create"
         << "--label" << entryName << "--loader"
         << QString("\\EFI\\%1\\%2\\vmlinuz").arg(distro, efiDir) << "--unicode";

    const QString espPath = espMountPoint + "/EFI/" + distro + "/" + efiDir;

    const QString initrdEfi = QString("initrd=\\EFI\\%1\\%2\\initrd.img").arg(distro, efiDir);
    const QString amdUcodeEfi = QString("initrd=\\EFI\\%1\\%2\\amducode.img").arg(distro, efiDir);
    const QString intUcodeEfi = QString("initrd=\\EFI\\%1\\%2\\intucode.img").arg(distro, efiDir);

    const QString amdUcode = QString("%1/amducode.img").arg(espPath);
    const QString intUcode = QString("%1/intucode.img").arg(espPath);

    QString initrd = !QFile::exists(amdUcode) ? "" : amdUcodeEfi;

    if (QFile::exists(intUcode)) {
        initrd += initrd.isEmpty() ? intUcodeEfi : (" " + intUcodeEfi);
    }

    initrd += initrd.isEmpty() ? initrdEfi : (" " + initrdEfi);

    QString bootOptions;
    if (isFrugal) {
        bootOptions
            = QString("bdir=%1 buuid=%2 %3 %4 %5")
                  .arg(options.bdir, options.uuid, ui->textOptionsFrugal->text(), ui->comboFrugalMode->currentText(), initrd);
    } else {
        bootOptions = QString("%1 %2").arg(ui->textKernelOptions->text(), initrd);
    }

    if (!cmd.procAsRoot("efibootmgr", args << bootOptions)) {
        return false;
    }
    return true;
}

bool MainWindow::isLuks(const QString &part)
{
    return cmd.procAsRoot("cryptsetup", {"isLuks", part});
}

QString MainWindow::mountPartition(QString part)
{
    if (part == rootPartition) {
        return "/";
    }
    if ((part.startsWith("/dev/") && part == QString("/dev/%1").arg(rootPartition))) {
        return "/";
    }

    // allow LABEL= UUID= PARTUUID=, PARTLABEL etc  as "part" argument
    // convert to /dev/devicename reliable if partion given as LABEL= UUID= PARTUUID= etc.
    // get UUID= if any of /dev/... formats are given
    QString uuid;
    if (!part.contains("=")) {
        if (!part.startsWith("/dev/")) {
            part = "/dev/" + part;
        }
        cmd.procAsRoot("blkid", {"--output", "value", "--match-tag", "UUID", part}, &uuid, nullptr);
        part = "UUID=" + uuid.trimmed();
    }
    // get /dev/devicename  from token UUID, LABEL=, PARTUUID=, PARTLABEL= and /dev/disk-by/...
    cmd.procAsRoot("blkid", {"--list-one", "--output", "device", "--match-token", part}, &part, nullptr);

    QString mountDir;
    // use TARGET to get mountpoint with spaces
    cmd.procAsRoot("findmnt", {"--noheadings", "--first-only", "--output", "TARGET", "--source", part}, &mountDir,
                   nullptr);
    if (!mountDir.isEmpty()) {
        return mountDir;
    }

    if (isLuks(part)) {

        mountDir = getMountPoint(part);

        if (!mountDir.isEmpty()) {
            return mountDir;
        }

        QString luksDevice;
        luksDevice = openLuks(part);
        if (!luksDevice.isEmpty()) {
            mountDir = MOUNT_BASE + "/" + luksDevice;
            if (!QDir(mountDir).exists()) {
                if (!cmd.procAsRoot("mkdir", {"-p", mountDir})) {
                    return {};
                }
                newDirectories.append(mountDir);
            }
            if (!cmd.procAsRoot("mount", {"/dev/mapper/" + luksDevice, mountDir})) {
                return {};
            }
            newMounts.append(mountDir);
            return mountDir;
        }
    }

    // Reduce to bare device name (e.g. "sda1")
    if (part.startsWith("/dev/")) {
        part = part.mid(5);
    }

    mountDir = MOUNT_BASE + "/" + part;
    if (!QDir(mountDir).exists()) {
        if (!cmd.procAsRoot("mkdir", {"-p", mountDir})) {
            return {};
        }
        newDirectories.append(mountDir);
    }

    if (!cmd.procAsRoot("mount", {"/dev/" + part, mountDir})) {
        return {};
    }
    newMounts.append(mountDir);
    return mountDir;
}

// Add list of devices to comboLocation
void MainWindow::addDevToList()
{
    listDevices();

    auto *comboDrive = (ui->tabWidget->currentIndex() == Tab::Frugal) ? ui->comboDrive : ui->comboDriveStub;
    comboDrive->blockSignals(true);
    comboDrive->clear();
    comboDrive->addItems(driveList);

    QString currentDrive = comboDrive->currentText().section(' ', 0, 0);

    const int driveCount = comboDrive->count();

    // Set current index based on rootDrive
    if (!rootDrive.isEmpty() && !currentDrive.isEmpty() && rootDrive != currentDrive) {
        for (int index = 0; index < driveCount; ++index) {
            const QString drive = comboDrive->itemText(index).section(' ', 0, 0);
            if (drive == rootDrive) {
                comboDrive->setCurrentIndex(index);
                break;
            }
        }
    }

    comboDrive->blockSignals(false);

    filterDrivePartitions();
}

bool MainWindow::checkSizeEsp()
{
    const bool isFrugal = ui->tabWidget->currentIndex() == Tab::Frugal;
    const QString sourceDir = isFrugal ? frugalDir : getBootLocation();
    qDebug() << "Source Dir:" << sourceDir;
    const QString kernelVersion = ui->comboKernel->currentText();
    QString vmlinuz = QString("%1/vmlinuz%2").arg(sourceDir, isFrugal ? "" : "-" + kernelVersion);
    if (!QFile::exists(vmlinuz)) {
        vmlinuz = QString("%1/vmlinuz-linux").arg(sourceDir);
    }

    QString initrd = QString("%1/initrd%2").arg(sourceDir, isFrugal ? ".gz" : ".img-" + kernelVersion);
    if (!QFile::exists(initrd)) {
        QString initramfs = QString("%1/initramfs-%2").arg(sourceDir, isFrugal ? "" : kernelVersion + ".img");
        if (!QFile::exists(initramfs)) {
            initramfs = QString("%1/initramfs-linux.img").arg(sourceDir);
        }
        if (QFile::exists(initramfs)) {
            initrd = initramfs;
        }
    }

    const QString amdUcode = QString("%1/amd-ucode.img").arg(sourceDir);
    const QString intUcode = QString("%1/intel-ucode.img").arg(sourceDir);

    qDebug() << "VMLINUZ:" << vmlinuz;
    qDebug() << "INITRD :" << initrd;
    if (QFile(intUcode).exists()) {
        qDebug() << "INTEL-UCODE :" << intUcode;
    }
    if (QFile(amdUcode).exists()) {
        qDebug() << "AMD-UCODE :" << amdUcode;
    }
    const qint64 vmlinuzSize = QFile(vmlinuz).size();
    const qint64 initrdSize = QFile(initrd).size();
    const qint64 amdUcodeSize = QFile(amdUcode).exists() ? QFile(amdUcode).size() : 0;
    const qint64 intUcodeSize = QFile(intUcode).exists() ? QFile(intUcode).size() : 0;
    const qint64 totalSize = vmlinuzSize + initrdSize + amdUcodeSize + intUcodeSize;
    qDebug() << "Total needed:" << totalSize;

    const qint64 espFreeSpace = QStorageInfo(espMountPoint).bytesAvailable();
    qDebug() << "ESP Free    :" << espFreeSpace;
    if (totalSize > espFreeSpace) {
        return false;
    }
    return true;
}

void MainWindow::filterDrivePartitions()
{
    auto *comboDrive = (ui->tabWidget->currentIndex() == Tab::Frugal) ? ui->comboDrive : ui->comboDriveStub;
    auto *comboPartition = (ui->tabWidget->currentIndex() == Tab::Frugal) ? ui->comboPartition : ui->comboPartitionStub;

    QStringList filteredPartitionList
        = (ui->tabWidget->currentIndex() == Tab::Frugal) ? frugalPartitionList : linuxPartitionList;

    comboPartition->blockSignals(true);
    comboPartition->clear();
    comboPartition->blockSignals(false);
    QString drive = comboDrive->currentText().section(' ', 0, 0);
    if (!drive.isEmpty()) {
        QStringList drivePart = filteredPartitionList.filter(QRegularExpression("^" + QRegularExpression::escape(drive)));
        comboPartition->blockSignals(true);
        comboPartition->addItems(drivePart);
        comboPartition->blockSignals(false);
    }

    guessPartition();
}

void MainWindow::selectKernel(const QString &rootDir)
{
    QDir bootDir {getBootLocation(rootDir)};
    QStringList kernelFiles = bootDir.entryList({"vmlinuz-*"}, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    std::transform(kernelFiles.begin(), kernelFiles.end(), kernelFiles.begin(),
                   [](const QString &file) { return file.mid(QStringLiteral("vmlinuz-").length()); });
    ui->comboKernel->clear();
    ui->textKernelOptions->setText("");

    QStringList sortedKernelFiles = utils::sortKernelVersions(kernelFiles);
    if (sortedKernelFiles.count() == 0) {
        return;
    }

    ui->comboKernel->addItems(sortedKernelFiles);

    if (rootDir == "/") {
        QString kernel;
        cmd.proc("uname", {"-r"}, &kernel, nullptr, QuietMode::Yes);
        kernel = kernel.trimmed();
        if (ui->comboKernel->findText(kernel) != -1) {
            ui->comboKernel->setCurrentText(kernel);
        }
    }

    getKernelOptions(bootDir.absolutePath(), rootDir, ui->comboKernel->currentText());
    QString distroName;
    if (!QFile::exists(rootDir + "/etc/antix-version") && !QFile::exists(rootDir + "/etc/mx-version")
        && QFile::exists(rootDir + "/etc/os-release")) {
        distroName = getDistroName(true, rootDir, "os-release");
        distro = getDistroName(false, rootDir, "os-release");
    } else {
        distroName = getDistroName(true, rootDir, "lsb-release");
        distro = getDistroName(false, rootDir, "initrd_release");
    }
    distroName = distroName.trimmed();

    // Remove " GNU/Linux" if it exists
    distroName.replace(" GNU/Linux", "");
    // Remove " Linux" if it exists
    distroName.replace(" Linux", "");

    if (!distroName.isEmpty()) {
        ui->textEntryName->setText(distroName);
    }
}

void MainWindow::promptFrugalStubInstall()
{
    int ret = QMessageBox::question(this, tr("UEFI Installer"),
                                    tr("A recent frugal install has been detected. Do you wish to add a UEFI entry "
                                       "direct to your UEFI system menu?"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::No) {
        cmd.procElevated("/usr/lib/uefi-manager/uefimanager-lib", {"write_checkfile"});
        qApp->quit();
        return;
    } else {
        ui->tabWidget->setCurrentIndex(Tab::Frugal);
    }
}

void MainWindow::readBootEntries(QListWidget *listEntries, QLabel *textTimeout, QLabel *textBootNext,
                                 QLabel *textBootCurrent, QStringList *bootorder)
{
    QString efiOut;
    cmd.proc("efibootmgr", {}, &efiOut);
    QStringList entries = efiOut.split('\n', Qt::SkipEmptyParts);
    QRegularExpression bootEntryRegex(R"(^Boot[0-9A-F]{4}\*?\s+)");

    for (const auto &item : std::as_const(entries)) {
        if (bootEntryRegex.match(item).hasMatch()) {
            auto *listItem = new QListWidgetItem(item);
            if (!item.contains("*")) {
                listItem->setBackground(QBrush(Qt::gray));
            }
            listEntries->addItem(listItem);
        } else if (item.startsWith("Timeout:")) {
            textTimeout->setText(tr("Timeout: %1 seconds").arg(item.section(' ', 1).trimmed()));
        } else if (item.startsWith("BootNext:")) {
            textBootNext->setText(tr("Boot Next: %1").arg(item.section(' ', 1).trimmed()));
        } else if (item.startsWith("BootCurrent:")) {
            textBootCurrent->setText(tr("Boot Current: %1").arg(item.section(' ', 1).trimmed()));
        } else if (item.startsWith("BootOrder:")) {
            *bootorder = item.section(' ', 1).split(',', Qt::SkipEmptyParts);
        }
    }
}

void MainWindow::refreshEntries()
{
    Cmd::resetElevation();
    clearEntryWidget();

    auto *layout = new QGridLayout(ui->tabManageUefi);
    auto *listEntries = new QListWidget(ui->tabManageUefi);
    auto *textIntro = new QLabel(tr("You can use the Up/Down buttons, or drag & drop items to change boot order.\n"
                                    "- Items are listed in the boot order.\n"
                                    "- Grayed out lines are inactive."),
                                 ui->tabManageUefi);

    auto createButton = [&](const QString &text, const QString &iconName) {
        auto *button = new QPushButton(text, ui->tabManageUefi);
        button->setIcon(QIcon::fromTheme(iconName));
        return button;
    };

    auto *pushActive = createButton(tr("Set ac&tive"), "star-on");
    auto *pushAddEntry = createButton(tr("&Add entry"), "list-add");
    auto *pushBootNext = createButton(tr("Boot &next"), "go-next");
    auto *pushDown = createButton(tr("Move &down"), "arrow-down");
    auto *pushRemove = createButton(tr("&Remove entry"), "trash-empty");

    auto *pushResetNext = createButton(tr("Re&set next"), "edit-undo");
    auto *pushTimeout = createButton(tr("Change &timeout"), "timer-symbolic");
    auto *pushUp = createButton(tr("Move &up"), "arrow-up");

    auto *spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto *textBootCurrent = new QLabel(ui->tabManageUefi);
    auto *textBootNext
        = new QLabel(tr("Boot Next: %1").arg(tr("not set, will boot using list order")), ui->tabManageUefi);
    auto *textTimeout = new QLabel(tr("Timeout: %1 seconds").arg("0"), ui->tabManageUefi);
    listEntries->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(pushResetNext, &QPushButton::clicked, ui->tabManageUefi, [textBootNext]() {
        Cmd::resetElevation();
        if (Cmd().procAsRoot("efibootmgr", {"-N"})) {
            textBootNext->setText(tr("Boot Next: %1").arg(tr("not set, will boot using list order")));
        }
    });
    connect(pushTimeout, &QPushButton::clicked, this,
            [this, textTimeout]() { Cmd::resetElevation(); setUefiTimeout(ui->tabManageUefi, textTimeout); });
    connect(pushAddEntry, &QPushButton::clicked, this,
            [this, listEntries]() { Cmd::resetElevation(); addUefiEntry(listEntries, ui->tabManageUefi); });
    connect(pushBootNext, &QPushButton::clicked, this,
            [listEntries, textBootNext]() { Cmd::resetElevation(); setUefiBootNext(listEntries, textBootNext); });
    connect(pushRemove, &QPushButton::clicked, this,
            [this, listEntries]() { Cmd::resetElevation(); removeUefiEntry(listEntries, ui->tabManageUefi); });

    connect(pushActive, &QPushButton::clicked, ui->tabManageUefi, [listEntries]() { Cmd::resetElevation(); toggleUefiActive(listEntries); });
    connect(pushUp, &QPushButton::clicked, ui->tabManageUefi, [this, listEntries, pushUp, pushDown]() {
        pushUp->setEnabled(false);
        pushDown->setEnabled(false);
        listEntries->model()->moveRow(QModelIndex(), listEntries->currentRow(), QModelIndex(),
                                      listEntries->currentRow() - 1);
        pushUp->setEnabled(listEntries->currentRow() != 0);
        pushDown->setEnabled(listEntries->currentRow() != listEntries->count() - 1);
    });
    connect(pushDown, &QPushButton::clicked, ui->tabManageUefi, [this, listEntries, pushUp, pushDown]() {
        pushUp->setEnabled(false);
        pushDown->setEnabled(false);
        listEntries->model()->moveRow(QModelIndex(), listEntries->currentRow() + 1, QModelIndex(),
                                      listEntries->currentRow()); // move next entry down
        pushUp->setEnabled(listEntries->currentRow() != 0);
        pushDown->setEnabled(listEntries->currentRow() != listEntries->count() - 1);
    });
    connect(listEntries, &QListWidget::itemSelectionChanged, ui->tabManageUefi,
            [listEntries, pushUp, pushDown, pushActive]() {
                if (!listEntries || !pushUp || !pushDown || !pushActive) {
                    return;
                }

                pushUp->setEnabled(listEntries->currentRow() != 0);
                pushDown->setEnabled(listEntries->currentRow() != listEntries->count() - 1);

                auto currentItem = listEntries->currentItem();
                if (currentItem && currentItem->text().section(' ', 0, 0).endsWith('*')) {
                    pushActive->setText(tr("Set &inactive"));
                    pushActive->setIcon(QIcon::fromTheme("star-off"));
                } else {
                    pushActive->setText(tr("Set ac&tive"));
                    pushActive->setIcon(QIcon::fromTheme("star-on"));
                }
            });

    QStringList bootorder;
    readBootEntries(listEntries, textTimeout, textBootNext, textBootCurrent, &bootorder);
    sortUefiBootOrder(bootorder, listEntries);

    listEntries->setDragDropMode(QAbstractItemView::InternalMove);
    connect(listEntries->model(), &QAbstractItemModel::rowsMoved, this, [this, listEntries]() {
        Cmd::resetElevation();
        if (!saveBootOrder(listEntries)) {
            QTimer::singleShot(0, this, &MainWindow::refreshEntries);
            return;
        }
        emit listEntries->itemSelectionChanged();
    });

    int row = 0;
    const int rowspan = 7;
    layout->addWidget(textIntro, row++, 0, 1, 2);
    layout->addWidget(listEntries, row, 0, rowspan, 1);
    layout->addWidget(pushRemove, row++, 1);

    layout->addWidget(pushAddEntry, row++, 1);
    layout->addWidget(pushUp, row++, 1);
    layout->addWidget(pushDown, row++, 1);
    layout->addWidget(pushActive, row++, 1);
    layout->addWidget(pushBootNext, row++, 1);
    layout->addItem(spacer, row++, 1);
    layout->addWidget(textBootCurrent, row++, 0);
    layout->addWidget(textTimeout, row, 0);
    layout->addWidget(pushTimeout, row++, 1);
    layout->addWidget(textBootNext, row, 0);
    layout->addWidget(pushResetNext, row++, 1);
    ui->tabManageUefi->setLayout(layout);

    ui->tabManageUefi->resize(this->size());
    ui->tabManageUefi->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->pushNext->setHidden(true);
    ui->pushBack->setHidden(true);
}

void MainWindow::refreshFrugal()
{
    Cmd::resetElevation();
    addDevToList();
    ui->stackedFrugal->setCurrentIndex(Page::Location);
    ui->pushCancel->setEnabled(true);
    ui->pushBack->setEnabled(false);
    ui->pushNext->setEnabled(true);
    ui->pushNext->setText(tr("Next"));
    ui->pushNext->setIcon(QIcon::fromTheme("go-next"));
}

void MainWindow::refreshStubInstall()
{
    Cmd::resetElevation();
    addDevToList();
    ui->pushCancel->setEnabled(true);
    ui->pushNext->setText(tr("Install"));
    ui->pushNext->setIcon(QIcon::fromTheme("run-install"));
    if (ui->textEntryName->text().isEmpty()) {
        ui->textEntryName->setText(getDistroName(true));
    }
    if (!ui->comboDriveStub->currentText().isEmpty() && !ui->comboPartitionStub->currentText().isEmpty()
        && !ui->comboKernel->currentText().isEmpty() && !ui->textEntryName->text().isEmpty()) {
        ui->pushNext->setEnabled(true);
    }
}

bool MainWindow::readGrubEntry()
{
    QFile grubEntryFile(frugalDir + "/grub.entry");
    if (!grubEntryFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("UEFI Installer"), tr("Failed to open grub.entry file."));
        return false;
    }
    options.bdir.clear();
    options.entryName.clear();
    options.persistenceType.clear();
    options.uuid.clear();
    options.stringOptions.clear();

    QTextStream in(&grubEntryFile);
    QString line;
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.startsWith("menuentry")) {
            options.entryName = line.section('"', 1, 1).trimmed();
        } else if (line.startsWith("search")) {
            options.uuid = line.section("--fs-uuid", 1, 1).trimmed();
        } else if (line.startsWith("linux")) {
            QStringList optionsList = line.split(' ').mid(1); // Skip the first "linux" element
            for (const QString &option : optionsList) {
                if (option.startsWith("bdir=")) {
                    options.bdir = option.section('=', 1, 1).trimmed();
                } else if (PERSISTENCE_TYPES.contains(option)) {
                    options.persistenceType = PERSISTENCE_TYPES[option];
                } else if (!option.startsWith("buuid=") && !option.endsWith("vmlinuz")) {
                    options.stringOptions.append(option + ' ');
                }
            }
        }
    }
    options.stringOptions = options.stringOptions.trimmed();
    grubEntryFile.close();
    return true;
}

void MainWindow::loadStubOption()
{
    options.bdir.clear();
    options.persistenceType.clear();
    options.entryName = ui->textEntryName->text();
    options.uuid.clear();
    options.stringOptions = ui->textKernelOptions->text();
}

QString MainWindow::openLuks(const QString &partition)
{
    QString uuid;
    if (!cmd.procAsRoot("cryptsetup", {"luksUUID", partition}, &uuid) || uuid.trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Could not retrieve UUID for %1").arg(partition));
        return {};
    }
    const QString luksDevice = "luks-" + uuid.trimmed();

    bool ok;
    QByteArray pass = QInputDialog::getText(this, this->windowTitle(),
                                            tr("Enter passphrase to unlock %1 encrypted partition:").arg(partition),
                                            QLineEdit::Password, QString(), &ok)
                          .toUtf8();

    if (!ok || pass.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Passphrase entry cancelled or empty for %1").arg(partition));
        return {};
    }

    // Try to open the LUKS container
    if (!cmd.procAsRoot("cryptsetup", {"luksOpen", "--allow-discards", partition, luksDevice, "-"}, nullptr, &pass)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open %1 LUKS container").arg(partition));
        pass.fill(SCRUB_BYTE);
        return {};
    }
    pass.fill(SCRUB_BYTE);
    qDebug() << "openLuks:" << luksDevice;
    newLuksDevices.append(luksDevice);
    return luksDevice;
}

void MainWindow::sortUefiBootOrder(const QStringList &order, QListWidget *list)
{
    if (order.isEmpty()) {
        return;
    }

    int index = 0;
    for (const auto &orderItem : order) {
        auto items = list->findItems("Boot" + orderItem, Qt::MatchStartsWith);
        if (items.isEmpty()) {
            continue;
        }

        auto *item = items.constFirst();
        list->takeItem(list->row(item));
        list->insertItem(index, item);
        ++index;
    }

    list->setCurrentRow(0);
    if (list->currentItem()) {
        list->currentItem()->setSelected(true);
    }
    emit list->itemSelectionChanged();
}

QString MainWindow::getDistroName(bool pretty, const QString &mountPoint, const QString &releaseFile) const
{

    QFile file(QString("%1/etc/%2").arg(mountPoint, releaseFile));
    QString searchTerm;
    if (releaseFile == "initrd_release") {
        searchTerm = pretty ? "PRETTY_NAME=" : "NAME=";
    } else if (releaseFile == "lsb-release") {
        searchTerm = pretty ? "PRETTY_NAME=" : "DISTRIB_DESCRIPTION=";
    } else if (releaseFile == "os-release") {
        searchTerm = pretty ? "PRETTY_NAME=" : "ID=";
    } else {
        return pretty ? "MX Linux" : "MX";
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return pretty ? "MX Linux" : "MX";
    }
    QTextStream in(&file);
    QString line;
    QString distroName;

    while (!in.atEnd()) {
        line = in.readLine();
        if (line.startsWith(searchTerm)) {
            distroName = line.section('=', 1, 1).remove('"').trimmed();
            break;
        }
    }

    file.close();
    if (distroName.isEmpty()) {
        return "Linux";
    }
    return distroName;
}

QString MainWindow::getLuksUUID(const QString &part)
{
    QString uuid;
    cmd.procAsRoot("cryptsetup", {"luksUUID", part}, &uuid);
    return uuid;
}

[[nodiscard]] QString MainWindow::getMountPoint(const QString &partition)
{
    if ((partition == rootPartition)
        || (partition.startsWith("/dev/") && partition == QString("/dev/%1").arg(rootPartition))) {
        return "/";
    }

    QString device = partition.startsWith("/dev/") ? partition : "/dev/" + partition;

    QString lsblkOut;
    cmd.proc("lsblk", {"--pairs", "--output", "MOUNTPOINT", device}, &lsblkOut);

    // Parse MOUNTPOINT="..." from first matching line
    static const QRegularExpression mpRegex(R"delim(MOUNTPOINT="([^"]+)")delim");
    QRegularExpressionMatch match = mpRegex.match(lsblkOut);
    return match.hasMatch() ? match.captured(1) : QString();
}

void MainWindow::getGrubOptions(const QString &mountPoint)
{
    QFile grubFile(QString("%1/etc/default/grub").arg(mountPoint));
    if (!grubFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open grub file for reading.";
        return;
    }

    const QString grubConfig = grubFile.readAll();
    grubFile.close();

    QRegularExpression regex("GRUB_CMDLINE_LINUX_DEFAULT=\"([^\"]*)\"");
    QRegularExpressionMatch match = regex.match(grubConfig);

    if (match.hasMatch()) {
        QString bootOptions = match.captured(1);
        if (!bootOptions.isEmpty()) {
            ui->textKernelOptions->setText(bootOptions);
        } else {
            qWarning() << "Captured boot options are empty.";
        }
    } else {
        qWarning() << "No match found for GRUB_CMDLINE_LINUX_DEFAULT.";
    }
}

void MainWindow::getKernelOptions(const QString &bootDir)
{
    QString kernelBootDir = bootDir;
    // Remove the last '/' if it exists
    if (kernelBootDir.endsWith("/")) {
        kernelBootDir.chop(1); // Remove the last character
    }
    QString grubFile;
    if (QFile::exists(kernelBootDir + "/boot/grub/grub.cfg")) {
        grubFile = kernelBootDir + "/boot/grub/grub.cfg";
    } else if (QFile::exists(kernelBootDir + "/grub/grub.cfg")) {
        grubFile = kernelBootDir + "/grub/grub.cfg";
    } else {
        return;
    }

    QString grepOut;
    cmd.procAsRoot("grep", {"-m1", "-oP", R"(^[[:space:]]*linux[[:space:]]+(/boot)?/vmlinuz-[^[:space:]]+\K.*)", grubFile}, &grepOut);
    QString bootOptions = grepOut.trimmed();
    if (isSystemd()) {
        bootOptions = bootOptions + " init=/lib/systemd/systemd";
    }

    if (!bootOptions.isEmpty()) {
        ui->textKernelOptions->setText(bootOptions);

    } else {
        qWarning() << "Captured boot options are empty.";
    }
}

void MainWindow::getKernelOptions(const QString &bootDir, const QString &rootDir, const QString &kernel)
{
    QString kernelDir = determineKernelDir(bootDir, rootDir);
    QString vmlinuz = kernel;
    if (!vmlinuz.startsWith("vmlinuz-")) {
        vmlinuz = "vmlinuz-" + kernel;
    }

    auto [rootPatterns, rootUUID] = getRootIdentifiers(rootDir);
    QString grubFile = bootDir.endsWith("/") ? bootDir + "grub/grub.cfg" : bootDir + "/grub/grub.cfg";

    QString bootOptions = parseGrubOptions(grubFile, rootPatterns, kernelDir, vmlinuz);
    if (bootOptions.isEmpty()) {
        bootOptions = getFallbackOptions(rootDir, rootUUID);
    }
    bootOptions = combineBootOptions(bootOptions, rootDir);
    ui->textKernelOptions->setText(bootOptions);
}
QString MainWindow::determineKernelDir(const QString &bootDir, const QString &rootDir)
{
    QString kernelDir;
    if (bootDir == "/boot" || bootDir == "/boot/") {
        if (!Cmd().procAsRoot("mountpoint", {"-q", bootDir})) {
            kernelDir = "/boot";
        } else {
            kernelDir = "";
        }
    } else if (bootDir.startsWith(rootDir)) {
        kernelDir = "/boot";
    } else {
        kernelDir = "";
    }
    return kernelDir;
}

QPair<QStringList, QString> MainWindow::getRootIdentifiers(const QString &rootDir)
{
    QString dfOut;
    cmd.proc("df", {"--output=source", rootDir}, &dfOut);
    const QStringList dfLines = dfOut.split('\n', Qt::SkipEmptyParts);
    QString rootDevicePath = dfLines.size() >= 2 ? dfLines.last().trimmed() : QString();
    if (rootDevicePath.isEmpty() || !rootDevicePath.startsWith("/dev/")) {
        qWarning() << "Could not determine root device for" << rootDir;
        return {{}, {}};
    }
    QStringList rootPatternList = {rootDevicePath};
    QString rootUUID;
    cmd.procAsRoot("blkid", {"--output", "value", "--match-tag", "UUID", rootDevicePath}, &rootUUID, nullptr);
    if (!rootUUID.isEmpty()) {
        rootPatternList << "UUID=" + rootUUID;
    }

    if (rootDevicePath.startsWith("/dev/mapper")) {
        QString rootParentDevice;
        QString rootParentUUID;
        QString rootParentPARTUUID;
        QString rootParentPARTLABEL;
        QString rootDevMapper;
        QStringList rootParentPatternList;
        cmd.proc("lsblk", {"-ln", "-o", "PKNAME", rootDevicePath}, &rootParentDevice);
        rootParentDevice = rootParentDevice.trimmed();

        if (!rootParentDevice.isEmpty()) {
            rootParentPatternList << rootParentDevice;
            // UUID
            cmd.procAsRoot("blkid", {"--output", "value", "--match-tag", "UUID", "/dev/" + rootParentDevice}, &rootParentUUID, nullptr);
            if (!rootParentUUID.isEmpty()) {
                rootParentPatternList << "UUID=" + rootParentUUID;
            }
            // PARTUUID
            cmd.procAsRoot("blkid", {"--output", "value", "--match-tag", "PARTUUID", "/dev/" + rootParentDevice}, &rootParentPARTUUID, nullptr);
            if (!rootParentPARTUUID.isEmpty()) {
                rootParentPatternList << "PARTUUID=" + rootParentPARTUUID;
            }
            // PARTLABEL
            cmd.procAsRoot("blkid", {"--output", "value", "--match-tag", "PARTLABEL", "/dev/" + rootParentDevice}, &rootParentPARTLABEL, nullptr);
            if (!rootParentPARTLABEL.isEmpty()) {
                rootParentPARTLABEL.replace(" ", "\\040");
                rootParentPatternList << "PARTLABEL=" + rootParentPARTLABEL;
            }

            QString crypttab = rootDir.endsWith("/") ? rootDir + "etc/crypttab" : rootDir + "/etc/crypttab";

            if (QFile::exists(crypttab)) {
                QStringList escapedPatterns;
                for (const QString &token : std::as_const(rootParentPatternList)) {
                    escapedPatterns << QRegularExpression::escape(token);
                }
                cmd.procAsRoot("grep", {"-m1", "-oP", QString("^([^[:space:]]+)[[:space:]]+(?=(%1).*)").arg(escapedPatterns.join("|")), crypttab}, &rootDevMapper, nullptr);
                if (!rootDevMapper.trimmed().isEmpty()) {
                    rootPatternList << "/dev/mapper/" + rootDevMapper.trimmed();
                }
            }
        }
    }
    return {rootPatternList, rootUUID};
}

QString MainWindow::parseGrubOptions(const QString &grubFile, const QStringList &rootPatterns, const QString &kernelDir, const QString &vmlinuz)
{
    QString bootOptions;
    if (!QFile::exists(grubFile)) {
        qWarning() << "GRUB file not found:" << grubFile;
        return "";
    } else {
        QStringList escapedRootPatterns;
        for (const QString &p : rootPatterns) {
            escapedRootPatterns << QRegularExpression::escape(p);
        }
        QString pattern = QString("^[[:space:]]*linux[[:space:]]+(/@)?%1/%2[[:space:]]+\\K.*root=(%3).*")
                              .arg(kernelDir, QRegularExpression::escape(vmlinuz), escapedRootPatterns.join("|"));
        cmd.procAsRoot("grep", {"-m1", "-oiP", pattern, grubFile}, &bootOptions);
        bootOptions = bootOptions.trimmed();
    }
    return bootOptions;
}

QString MainWindow::getFallbackOptions(const QString &rootDir, const QString &rootUUID)
{
    QString bootOptions;
    // Obtain root= from rootUUID
    if (!rootUUID.isEmpty()) {
        bootOptions = "root=UUID=" + rootUUID;
    }

    // Try to read options from /etc/default/grub if it exists
    QString defaultGrubFile = rootDir.endsWith("/") ? rootDir + "etc/default/grub" : rootDir + "/etc/default/grub";
    if (QFile::exists(defaultGrubFile)) {
        // Get options from GRUB_CMDLINE_LINUX
        QString linuxOptions;
        cmd.procAsRoot("grep", {"-m1", "-oP", R"(^GRUB_CMDLINE_LINUX="\K[^"]+)", defaultGrubFile}, &linuxOptions);
        linuxOptions = linuxOptions.trimmed();

        // Get options from GRUB_CMDLINE_LINUX_DEFAULT
        QString defaultOptions;
        cmd.procAsRoot("grep", {"-m1", "-oP", R"(^GRUB_CMDLINE_LINUX_DEFAULT="\K[^"]+)", defaultGrubFile}, &defaultOptions);
        defaultOptions = defaultOptions.trimmed();

        // Combine both options
        if (!linuxOptions.isEmpty()) {
            bootOptions += " " + linuxOptions;
            qDebug() << "Boot options from GRUB_CMDLINE_LINUX:" << linuxOptions;
        }

        if (!defaultOptions.isEmpty()) {
            bootOptions += " " + defaultOptions;
            qDebug() << "Boot options from GRUB_CMDLINE_LINUX_DEFAULT:" << defaultOptions;
        }

        if (!linuxOptions.isEmpty() || !defaultOptions.isEmpty()) {
            qDebug() << "Combined boot options:" << bootOptions;
        }
    }
    return bootOptions.trimmed();
}

QString MainWindow::combineBootOptions(const QString &parsedOptions, const QString &rootDir)
{
    QString bootOptions = parsedOptions;
    const QString initSystemd = "init=/lib/systemd/systemd";
    if (!bootOptions.isEmpty()) {
        if (isSystemd() && !bootOptions.contains(initSystemd)) {
            if (isShimSystemd(rootDir)) {
                bootOptions = bootOptions + " " + initSystemd;
                qDebug() << "System init boot options added:" << bootOptions;
            }
        }
    } else {
        qWarning() << "Captured boot options are empty.";
    }
    return bootOptions;
}


// Try to guess root partition by checking partition labels and types
void MainWindow::guessPartition()
{
    const bool isFrugal = ui->tabWidget->currentIndex() == Tab::Frugal;
    auto *comboDrive = (ui->tabWidget->currentIndex() == Tab::Frugal) ? ui->comboDrive : ui->comboDriveStub;
    auto *comboPartition = isFrugal ? ui->comboPartition : ui->comboPartitionStub;

    const int partitionCount = comboPartition->count();

    // Define local lambda function findKernel
    auto findKernel = [this]() {
        if (!ui->comboPartitionStub->currentText().isEmpty()) {
            const QString mountPoint = mountPartition(ui->comboPartitionStub->currentText().section(' ', 0, 0));
            if (mountPoint.isEmpty()) {
                return;
            }
            selectKernel(mountPoint);
        }
    };

    if (ui->tabWidget->currentIndex() == Tab::StubInstall) {
        disconnect(ui->comboPartitionStub, nullptr, this, nullptr);
        connect(ui->comboPartitionStub, &QComboBox::currentTextChanged, this, findKernel);
    }

    // Known identifiers for Linux root partitions
    const QString rootMXLabel = "rootMX";
    const QStringList linuxPartTypes = {
        "0x83",                                 // Linux native partition
        "0fc63daf-8483-4772-8e79-3d69d8477de4", // Linux filesystem
        "44479540-F297-41B2-9AF7-D131D5F0458A", // Linux root (x86)
        "4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709"  // Linux root (x86-64)
    };

    // Helper: check lsblk field for a partition against a regex pattern
    auto findPartition = [&](const QString &field, const QRegularExpression &pattern) -> bool {
        QString drive = comboDrive->currentText().section(' ', 0, 0);
        if (drive == rootDrive) {
            for (int index = 0; index < partitionCount; ++index) {
                const QString part = comboPartition->itemText(index).section(' ', 0, 0);
                if (part == rootPartition) {
                    comboPartition->setCurrentIndex(index);
                    return true;
                }
            }
        }

        for (int index = 0; index < partitionCount; ++index) {
            const QString part = comboPartition->itemText(index).section(' ', 0, 0);
            QString value;
            cmd.procAsRoot("lsblk", {"-ln", "-o", field, "/dev/" + part}, &value, nullptr, QuietMode::Yes);
            if (pattern.match(value.trimmed()).hasMatch()) {
                comboPartition->setCurrentIndex(index);
                return true;
            }
        }
        return false;
    };

    // Try to find partition with rootMX* label
    if (!findPartition("LABEL", QRegularExpression(rootMXLabel))) {
        // Fall back to checking for any Linux partition type
        findPartition("PARTTYPE", QRegularExpression(linuxPartTypes.join('|'), QRegularExpression::CaseInsensitiveOption));
    }

    findKernel();
}

void MainWindow::detectRootDevice()
{
    QString dfRoot;
    cmd.proc("df", {"--output=source", "/"}, &dfRoot);
    const QStringList dfLines = dfRoot.split('\n', Qt::SkipEmptyParts);
    rootDevicePath = dfLines.size() >= 2 ? dfLines.last().trimmed() : QString();
    if (rootDevicePath.isEmpty() || !rootDevicePath.startsWith("/dev/")) {
        qWarning() << "Could not determine root device";
        return;
    }

    if (rootDevicePath.startsWith("/dev/mapper")) {
        cmd.proc("lsblk", {"-ln", "-o", "PKNAME", rootDevicePath}, &rootPartition);
        rootPartition = rootPartition.trimmed();
    } else {
        rootPartition = rootDevicePath.split('/').last().trimmed();
    }

    rootDrive = utils::extractDiskFromPartition(rootPartition);
}

void MainWindow::listDevices()
{
    // Regex for physical disk/partition device names (sd*, hd*, vd*, xvd*, mmcblk*, nvme*)
    static const QRegularExpression driveNameRegex("^x?[hsv]d[a-z]|^mmcblk|^nvme");
    static const QRegularExpression partNameRegex("^x?[hsv]d[a-z]\\d|^mmcblk\\d+p|^nvme\\d+n\\d+p");

    // Filesystem types excluded from Linux partition list
    static const QStringList excludedLinuxFs = {"ntfs", "exfat", "vfat", "BitLocker", "swap"};
    // Filesystem types excluded from frugal partition list
    static const QStringList excludedFrugalFs = {"swap", "BitLocker"};

    static constexpr qint64 ONE_GB = 1'073'741'824LL;
    static constexpr qint64 SIX_GB = 6 * ONE_GB;

    // Single lsblk call to get all block device info as JSON
    QString lsblkJson;
    if (!cmd.proc("lsblk", {"-ln", "--json", "--bytes", "-o",
                            "NAME,SIZE,FSTYPE,MOUNTPOINT,LABEL,MODEL,PARTTYPE,TYPE", "-e", "2,11"},
                   &lsblkJson)) {
        qWarning() << "lsblk failed; device lists will be empty";
    }

    QJsonDocument doc = QJsonDocument::fromJson(lsblkJson.toUtf8());
    if (doc.isNull()) {
        qWarning() << "Failed to parse lsblk JSON output";
    }
    QJsonArray devices = doc.object().value("blockdevices").toArray();

    // Helper: format display string with name first (for .section(' ', 0, 0) extraction)
    auto formatSize = [](qint64 bytes) -> QString {
        if (bytes >= ONE_GB) {
            return QString::number(static_cast<double>(bytes) / ONE_GB, 'f', 1) + "G";
        }
        return QString::number(static_cast<double>(bytes) / (1024 * 1024), 'f', 1) + "M";
    };

    espList.clear();
    driveList.clear();
    partitionList.clear();
    linuxPartitionList.clear();
    frugalPartitionList.clear();

    for (const QJsonValue &val : devices) {
        QJsonObject dev = val.toObject();
        const QString name = dev.value("name").toString();
        const qint64 size = dev.value("size").toInteger();
        const QString fstype = dev.value("fstype").toString();
        const QString mountpoint = dev.value("mountpoint").toString();
        const QString label = dev.value("label").toString();
        const QString model = dev.value("model").toString();
        const QString parttype = dev.value("parttype").toString().toLower();
        const QString type = dev.value("type").toString();
        const QString sizeStr = formatSize(size);

        bool isDrive = (type == "disk") && driveNameRegex.match(name).hasMatch();
        bool isPartition = (type == "part") && partNameRegex.match(name).hasMatch();

        // ESP list: EFI System Partitions with vfat filesystem
        if (isPartition && fstype.compare("vfat", Qt::CaseInsensitive) == 0
            && (parttype == ESP_GUID_GPT || parttype == ESP_TYPE_MBR)) {
            espList.append(QString("%1 %2 %3").arg(name, sizeStr, label).trimmed());
        }

        // Drive list: physical disk devices
        if (isDrive) {
            driveList.append(QString("%1 %2 %3 %4").arg(name, sizeStr, label, model).trimmed());
        }

        // Partition list: all partitions on physical disks
        if (isPartition) {
            // Show mountpoint as "/" for root partition
            QString mp = (name == rootPartition) ? "/" : mountpoint;
            partitionList.append(QString("%1 %2 %3 %4 %5").arg(name, sizeStr, fstype, mp, label).trimmed());
        }

        // Linux partition list: non-Windows/swap filesystems, >= 6 GB
        if (isPartition && size >= SIX_GB && !fstype.isEmpty()
            && !excludedLinuxFs.contains(fstype, Qt::CaseInsensitive)) {
            QString mp = (name == rootPartition) ? "/" : mountpoint;
            linuxPartitionList.append(
                QString("%1 %2 %3 %4 %5").arg(name, sizeStr, fstype, mp, label).trimmed());
        }

        // Frugal partition list: broader FS set, >= 1 GB
        if (isPartition && size >= ONE_GB && !fstype.isEmpty()
            && !excludedFrugalFs.contains(fstype, Qt::CaseInsensitive)) {
            QString mp = (name == rootPartition) ? "/" : mountpoint;
            frugalPartitionList.append(
                QString("%1 %2 %3 %4 %5").arg(name, sizeStr, fstype, mp, label).trimmed());
        }
    }

    // Sort partition lists by version (matches original sort -V behavior)
    auto versionSort = [](QStringList &list) {
        QCollator collator;
        collator.setNumericMode(true);
        std::sort(list.begin(), list.end(), [&collator](const QString &a, const QString &b) {
            return collator.compare(a, b) < 0;
        });
    };
    versionSort(espList);
    versionSort(partitionList);
    versionSort(linuxPartitionList);
    versionSort(frugalPartitionList);
}

void MainWindow::validateAndLoadOptions(const QString &frugalDir)
{
    QDir dir(frugalDir);
    const QStringList requiredFiles = {"vmlinuz", "linuxfs", "grub.entry"};
    const QStringList existingFiles = dir.entryList(requiredFiles, QDir::Files);

    QStringList missingFiles;
    missingFiles.reserve(requiredFiles.size());
    for (const QString &file : requiredFiles) {
        if (!existingFiles.contains(file)) {
            missingFiles.append(file);
        }
    }

    if (!missingFiles.isEmpty()) {
        QMessageBox::critical(this, tr("UEFI Installer"),
                              tr("Are you sure this is the MX or antiX Frugal installation location?\nMissing "
                                 "mandatory files in directory: ")
                                  + missingFiles.join(", "));
        ui->pushBack->click();
        return;
    }

    bool success = readGrubEntry();
    if (!success) {
        QMessageBox::critical(this, tr("UEFI Installer"), tr("Failed to read grub.entry file."));
        ui->pushBack->click();
        return;
    }

    if (!options.persistenceType.isEmpty()) {
        ui->comboFrugalMode->setCurrentText(options.persistenceType);
    }
    if (!options.entryName.isEmpty()) {
        ui->textUefiEntryFrugal->setText(options.entryName);
    }
    ui->textOptionsFrugal->setText(options.stringOptions);
    ui->pushNext->setEnabled(true);
    ui->pushNext->setText(tr("Install"));
    ui->pushNext->setIcon(QIcon::fromTheme("run-install"));
}

QString MainWindow::selectFrugalDirectory(const QString &partition)
{
    return QFileDialog::getExistingDirectory(this, tr("Select Frugal Directory"), partition, QFileDialog::ShowDirsOnly);
}

QString MainWindow::selectESP()
{

    if (espList.isEmpty()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("No EFI System Partitions found."));
        return {};
    }

    bool ok;
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Select EFI System Partition"));
    dialog.setLabelText(tr("EFI System Partitions:"));
    dialog.setComboBoxItems(espList);
    dialog.setMinimumWidth(400);
    dialog.resize(dialog.minimumWidth(), dialog.height());
    QString selectedEsp;
    if (dialog.exec() == QDialog::Accepted) {
        selectedEsp = dialog.textValue().section(' ', 0, 0);
        ok = true;
    } else {
        ok = false;
    }

    if (!ok || selectedEsp.isEmpty()) {
        QMessageBox::warning(this, QApplication::applicationDisplayName(), tr("No EFI System Partition selected"));
        return {};
    }

    espMountPoint = mountPartition(selectedEsp);
    if (espMountPoint.isEmpty()) {
        QMessageBox::warning(this, QApplication::applicationDisplayName(),
                             tr("Could not mount selected EFI System Partition"));
        return {};
    }

    if (!checkSizeEsp()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Not enough space on the EFI System Partition to copy the kernel and initrd files."));
        return {};
    }

    const bool isFrugal = ui->tabWidget->currentIndex() == Tab::Frugal;
    const QString subDir = isFrugal ? "/frugal" : "/stub";
    const QString targetPath = espMountPoint + "/EFI/" + distro + subDir;
    cmd.procAsRoot("rm", {"-f", targetPath + "/vmlinuz",
                          targetPath + "/initrd.img", targetPath + "/initrd.gz",
                          targetPath + "/amducode.img", targetPath + "/amducode.gz",
                          targetPath + "/intucode.img", targetPath + "/intucode.gz"});
    return selectedEsp;
}

void MainWindow::pushNextClicked()
{
    Cmd::resetElevation();
    if (ui->tabWidget->currentIndex() == Tab::Frugal) {
        if (ui->stackedFrugal->currentIndex() == Page::Location) {
            ui->pushNext->setEnabled(false);
            if (!ui->comboDrive->currentText().isEmpty() && !ui->comboPartition->currentText().isEmpty()) {
                QString part = mountPartition(ui->comboPartition->currentText().section(' ', 0, 0));
                if (part.isEmpty()) {
                    QMessageBox::critical(
                        this, QApplication::applicationDisplayName(),
                        tr("Could not mount partition. Please make sure you selected the correct partition."));
                    refreshFrugal();
                    return;
                }
                frugalDir = selectFrugalDirectory(part);
                if (!frugalDir.isEmpty()) {
                    ui->stackedFrugal->setCurrentIndex(Page::Options);
                    ui->pushBack->setEnabled(true);
                } else {
                    QMessageBox::warning(this, QApplication::applicationDisplayName(), tr("No directory selected"));
                    refreshFrugal();
                    return;
                }
                validateAndLoadOptions(frugalDir);
            }
        } else if (ui->stackedFrugal->currentIndex() == Page::Options) {
            ui->pushNext->setEnabled(false);
            ui->pushCancel->setEnabled(false);
            const auto restoreButtons = qScopeGuard([this]() {
                ui->pushNext->setEnabled(true);
                ui->pushCancel->setEnabled(true);
            });

            QString esp = selectESP();
            if (esp.isEmpty()) {
                return;
            }
            if (installEfiStub(esp)) {
                QMessageBox::information(this, QApplication::applicationDisplayName(),
                                         tr("EFI stub installed successfully."));
            } else {
                QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Failed to install EFI stub."));
            }
        }
    } else if (ui->tabWidget->currentIndex() == Tab::StubInstall) {
        if (ui->comboDriveStub->currentText().isEmpty() || ui->comboPartitionStub->currentText().isEmpty()
            || ui->textEntryName->text().isEmpty()) {
            QMessageBox::warning(this, QApplication::applicationDisplayName(), tr("All fields are required"));
            return;
        }
        QString part = mountPartition(ui->comboPartitionStub->currentText().section(' ', 0, 0));
        if (part.isEmpty()) {
            QMessageBox::critical(
                this, QApplication::applicationDisplayName(),
                tr("Could not mount partition. Please make sure you selected the correct partition."));
            refreshStubInstall();
            return;
        }

        loadStubOption();

        ui->pushNext->setEnabled(false);
        ui->pushCancel->setEnabled(false);
        const auto restoreButtons = qScopeGuard([this]() {
            ui->pushNext->setEnabled(true);
            ui->pushCancel->setEnabled(true);
        });

        QString esp = selectESP();
        if (esp.isEmpty()) {
            QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Could not select ESP"));
            refreshStubInstall();
            return;
        }
        if (installEfiStub(esp)) {
            QMessageBox::information(this, QApplication::applicationDisplayName(),
                                     tr("EFI stub installed successfully."));
        } else {
            QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Failed to install EFI stub."));
            refreshStubInstall();
        }
    }
}

void MainWindow::pushAboutClicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(QApplication::applicationDisplayName()),
        R"(<p align="center"><b><h2>UEFI Manager</h2></b></p><p align="center">)" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Tool for managing UEFI boot entries")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/uefi-manager/license.html", tr("%1 License").arg(this->windowTitle()));

    this->show();
}

void MainWindow::pushHelpClicked()
{
    const QString url = "https://mxlinux.org/wiki/uefi-manager/";
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::pushBackClicked()
{
    if (ui->stackedFrugal->currentIndex() == Page::Options) {
        ui->stackedFrugal->setCurrentIndex(Page::Location);
        ui->pushBack->setEnabled(false);
        ui->pushNext->setText(tr("Next"));
        ui->pushNext->setIcon(QIcon::fromTheme("go-next"));
        ui->pushNext->setEnabled(true);
    }
}

bool MainWindow::saveBootOrder(const QListWidget *list)
{
    QStringList orderList;
    orderList.reserve(list->count());
    for (int i = 0; i < list->count(); ++i) {
        QString item = list->item(i)->text().section(' ', 0, 0);
        item.remove(QRegularExpression("^Boot|\\*$"));
        if (item.contains(QRegularExpression("^[0-9A-Z]{4}$"))) {
            orderList.append(item);
        }
    }

    const QString order = orderList.join(',');
    if (!cmd.procAsRoot("efibootmgr", {"-o", order})) {
        // Only show generic error if it wasn't already an elevation failure (which shows its own message)
        if (cmd.exitCode() != 126 && cmd.exitCode() != 127) {
            QMessageBox::critical(this, tr("Error"), tr("Something went wrong, could not save boot order."));
        }
        return false;
    }
    return true;
}

void MainWindow::setUefiTimeout(QWidget *uefiDialog, QLabel *textTimeout)
{
    bool ok = false;
    ushort initialTimeout = textTimeout->text().section(' ', 1, 1).toUInt();
    ushort newTimeout = QInputDialog::getInt(uefiDialog, tr("Set timeout"), tr("Timeout in seconds:"), initialTimeout,
                                             0, 65535, 1, &ok);

    if (ok && Cmd().procAsRoot("efibootmgr", {"-t", QString::number(newTimeout)})) {
        textTimeout->setText(tr("Timeout: %1 seconds").arg(newTimeout));
    }
}

void MainWindow::setUefiBootNext(QListWidget *listEntries, QLabel *textBootNext)
{
    if (!listEntries || !textBootNext) {
        return;
    }

    if (auto currentItem = listEntries->currentItem()) {
        QString item = currentItem->text().section(' ', 0, 0);
        item.remove(QRegularExpression("^Boot"));
        item.remove(QRegularExpression(R"(\*$)"));

        if (QRegularExpression("^[0-9A-Z]{4}$").match(item).hasMatch()
            && Cmd().procAsRoot("efibootmgr", {"-n", item})) {
            textBootNext->setText(tr("Boot Next: %1").arg(item));
        }
    }
}

void MainWindow::removeUefiEntry(QListWidget *listEntries, QWidget *uefiDialog)
{
    if (!listEntries || !uefiDialog) {
        return;
    }

    auto *currentItem = listEntries->currentItem();
    if (!currentItem) {
        return;
    }

    QString itemText = currentItem->text();
    if (QMessageBox::Yes
        != QMessageBox::question(uefiDialog, tr("Removal confirmation"),
                                 tr("Are you sure you want to delete this boot entry?\n%1").arg(itemText))) {
        return;
    }

    QString item = itemText.section(' ', 0, 0);
    item.remove(QRegularExpression("^Boot"));
    item.remove(QRegularExpression(R"(\*$)"));

    if (!item.contains(QRegularExpression("^[0-9A-Z]{4}$"))) {
        return;
    }

    if (Cmd().procAsRoot("efibootmgr", {"-B", "-b", item})) {
        delete currentItem;
    }
    emit listEntries->itemSelectionChanged();
}

// Helper function to check system is running with systemd
bool MainWindow::isSystemd() const
{
    // Check if the directory /run/systemd/system exists
    QDir systemdDir("/run/systemd/system");
    if (!systemdDir.exists()) {
        qDebug() << "systemDir does not exist:"
                 << "/run/systemd/system";
        return false; // Directory does not exist, not pure systemd
    }
    return true;
}

// Helper function to check whether system under rootPath is a shim-systemd system
bool MainWindow::isShimSystemd(const QString &rootPath) const
{
    QString root = rootPath; // Create a mutable copy of rootPath

    // Remove trailing slash if it exists
    if (root.endsWith("/")) {
        root.chop(1); // Remove the last character
    }

    // sanity check if full path to init was given
    if (root.endsWith("/usr/sbin/init")) {
        root.chop(strlen("/usr/sbin/init"));
    } else if (root.endsWith("/sbin/init")) {
        root.chop(strlen("/sbin/init"));
    } else if (root.endsWith("/usr/bin/init")) {
        root.chop(strlen("/usr/bin/init"));
    } else if (root.endsWith("/usr/lib/systemd/systemd")) {
        root.chop(strlen("/usr/lib/systemd/systemd"));
    } else if (root.endsWith("/lib/systemd/systemd")) {
        root.chop(strlen("/lib/systemd/systemd"));
    }

    // Check if /sbin/init or /bin/init is a symlink to systemd
    QString initPath;
    if (QFile::exists(root + "/sbin/init")) {
        initPath = root + "/sbin/init";
    } else if (QFile::exists(root + "/bin/init")) {
        initPath = root + "/bin/init";
    } else {
        return false;
    }

    if (!QFile::exists(root + "/lib/systemd/systemd")) {
        return false;
    }

    // Read symlink target without resolving through host filesystem
    QFileInfo initInfo(initPath);
    if (!initInfo.isSymLink()) {
        return true; // init exists but is not a symlink to systemd
    }

    const QString target = initInfo.symLinkTarget();
    // symLinkTarget returns absolute path within the mounted root, strip the root prefix
    QString relativeTarget = target;
    if (relativeTarget.startsWith(root)) {
        relativeTarget = relativeTarget.mid(root.length());
    }
    return !relativeTarget.endsWith("/systemd");
}

bool MainWindow::renameUefiEntry(const QString &oldLabel, const QString &newLabel, const QString &oldBootNum)
{
    // Validate input parameters
    if (oldLabel.isEmpty() || newLabel.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Both old and new EFI labels must be specified"));
        return false;
    }

    // Retrieve disk data
    QString lsblkRaw;
    cmd.procAsRoot("lsblk", {"--nodeps", "--noheadings", "--pairs"}, &lsblkRaw);
    // Filter to only disk type entries
    QStringList diskDataLines;
    for (const QString &line : lsblkRaw.split('\n', Qt::SkipEmptyParts)) {
        if (line.contains("TYPE=\"disk\"")) {
            diskDataLines.append(line);
        }
    }
    QString diskData = diskDataLines.join('\n');
    QStringList diskNames;
    QRegularExpression diskRegex(R"delim(^NAME=\"([^\"]+)\".*$)delim");

    for (const QString &line : diskData.split('\n', Qt::SkipEmptyParts)) {
        QRegularExpressionMatch match = diskRegex.match(line);
        if (match.hasMatch()) {
            diskNames.append(match.captured(1));
        }
    }

    // Map partition UUIDs to devices
    QMap<QString, QString> partitions;
    for (const QString &diskName : diskNames) {
        QString sfdiskOut;
        cmd.procAsRoot("sfdisk", {"-d", "/dev/" + diskName}, &sfdiskOut);
        // Filter to partition lines only
        QStringList partLines;
        for (const QString &line : sfdiskOut.split('\n', Qt::SkipEmptyParts)) {
            if (line.contains(": start=")) {
                partLines.append(line);
            }
        }
        QString partitionData = partLines.join('\n');
        QRegularExpression partRegex(R"(^([^[:blank:]]+)[[:blank:]]:[[:blank:]].*[[:blank:]]uuid=([^,]+))");

        for (const QString &line : partitionData.split('\n', Qt::SkipEmptyParts)) {
            QRegularExpressionMatch match = partRegex.match(line);
            if (match.hasMatch()) {
                QString device = match.captured(1);
                QString uuid = match.captured(2).toLower();
                partitions[uuid] = device;
            }
        }
    }

    // Retrieve EFI data
    QString efiData;
    cmd.procAsRoot("efibootmgr", {"--verbose"}, &efiData);
    QString targetBootNum, targetPart, targetUuid, targetLoader;

    QRegularExpression efiRegex(
        R"(^Boot([[:xdigit:]]{4})\*?[[:blank:]]+(.+)[[:blank:]]+HD\(([[:digit:]]+),[^,]+,([^,]+)[^\)]+\)/File\(([^\)]+)\))");

    for (const QString &line : efiData.split('\n', Qt::SkipEmptyParts)) {
        QRegularExpressionMatch match = efiRegex.match(line);
        if (match.hasMatch()) {
            QString label = match.captured(2);
            if (label == oldLabel || oldLabel == "*") {
                if (targetBootNum.isEmpty()) {
                    if (oldBootNum.isEmpty() || match.captured(1) == oldBootNum) {
                        targetBootNum = match.captured(1);
                        targetPart = match.captured(3);
                        targetUuid = match.captured(4);
                        targetLoader = match.captured(5);
                    }
                } else if (oldBootNum.isEmpty()) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Multiple boot entries found for label '%1': %2 and %3;")
                                              .arg(oldLabel, targetBootNum, match.captured(1)));
                    return false;
                }
            }
        }
    }

    // Ensure a matching label was found
    if (targetBootNum.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("No EFI data found for label '%1'.").arg(oldLabel));
        return false;
    }

    // Find device for partition with matching UUID
    QString deviceForUuid = partitions[targetUuid.toLower()];
    if (deviceForUuid.isEmpty()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("EFI label '%1' is linked to an unknown partition '%2'.").arg(oldLabel, targetUuid));
        return false;
    }

    // Validate device/partition name format
    QRegularExpression deviceRegex(
        R"(^(/dev/sd[a-z]|nvme[[:digit:]]+n[[:digit:]]+|mmcblk[[:digit:]]+)p?([[:digit:]]+)$)");
    QRegularExpressionMatch deviceMatch = deviceRegex.match(deviceForUuid);
    if (!deviceMatch.hasMatch()) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Unexpected device name format '%1' for partition related to the label.").arg(deviceForUuid));
        return false;
    }

    QString deviceName = deviceMatch.captured(1);
    QString devicePart = deviceMatch.captured(2);

    // Confirm partition number matches
    if (devicePart != targetPart) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Device partition number [%1] differs from EFI entry partition number [%2].")
                                  .arg(devicePart, targetPart));
        return false;
    }

    // Execute efibootmgr commands
    QStringList deleteCmd = {"--bootnum", targetBootNum, "--delete-bootnum"};

    QStringList createCmd
        = {"--create", "--disk",     deviceName, "--part", targetPart, "--label", newLabel,
           "--loader", targetLoader};

    if (!cmd.procAsRoot("efibootmgr", deleteCmd)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to delete old boot entry"));
        return false;
    }

    if (!cmd.procAsRoot("efibootmgr", createCmd)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create new boot entry"));
        return false;
    }

    return true;
}

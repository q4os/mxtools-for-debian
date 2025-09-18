/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian, Dolphin Oracle
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
#include <QInputDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QProgressDialog>
#include <QScreen>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>

#include "about.h"

#include <chrono>
#include <unistd.h>

using namespace std::chrono_literals;
extern const QString starting_home;

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    qDebug().noquote() << QApplication::applicationName() << "version:" << QApplication::applicationVersion();
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    setGeneralConnections();
    setup();
}

MainWindow::~MainWindow()
{
    cleanup();
    delete ui;
}

void MainWindow::loadPlymouthThemes()
{
    ui->comboTheme->clear();

    const QString plymouthCmd = "/sbin/plymouth-set-default-theme";
    QString output;
    if (chroot.isEmpty()) {
        output = cmd.getOut(plymouthCmd + " -l");
    } else {
        output = cmd.getOutAsRoot(chroot + plymouthCmd + " -l");
    }
    if (cmd.exitCode() != 0) {
        qWarning() << "Failed to get Plymouth themes list.";
        return;
    }
    if (!output.isEmpty()) {
        const QStringList themes = output.split('\n', Qt::SkipEmptyParts);
        ui->comboTheme->addItems(themes);
        const QString currentTheme
            = chroot.isEmpty() ? cmd.getOut(plymouthCmd).trimmed() : cmd.getOutAsRoot(chroot + plymouthCmd).trimmed();
        if (cmd.exitCode() == 0 && !currentTheme.isEmpty()) {
            const int index = ui->comboTheme->findText(currentTheme);
            if (index != -1) {
                ui->comboTheme->setCurrentIndex(index);
            }
        } else {
            qWarning() << "Failed to get the current Plymouth theme.";
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        const bool isProcessActive = (cmd.state() == QProcess::Running || cmd.state() == QProcess::Starting);

        if (isProcessActive) {
            const auto response = QMessageBox::question(this, tr("Still running"),
                                                        tr("A process is still running. Do you really want to quit?"),
                                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (response != QMessageBox::Yes) {
                return;
            }
        }

        QApplication::quit();
    }
    QDialog::keyPressEvent(event);
}

// Setup various items the first time the program runs
void MainWindow::setup()
{
    chroot.clear();
    bar = nullptr;
    optionsChanged = false;
    splashChanged = false;
    user = QString::fromUtf8(getlogin());
    if (user.isEmpty()) {
        QProcess proc;
        proc.start("logname", {}, QIODevice::ReadOnly);
        proc.waitForFinished();
        user = QString::fromUtf8(proc.readAllStandardOutput().trimmed());
    }
    if (user.isEmpty()) {
        qWarning() << "Error: Failed to get the username.";
        user = "unknown";
    }
    justInstalled = false;
    setWindowTitle("MX Boot Options");
    setupUiElements();
    handleLiveSystem();
    setupGrubSettings();
    handleSpecialFilesystems();

    // Final UI adjustments
    ui->radioLimitedMsg->setVisible(!ui->checkBootsplash->isChecked());
    ui->pushApply->setDisabled(true);
    adjustSize();
}

void MainWindow::setupUiElements()
{
    ui->pushCancel->setEnabled(true);
    ui->pushApply->setEnabled(true);
    ui->comboTheme->setDisabled(true);
    ui->pushPreview->setDisabled(true);
    ui->checkEnableFlatmenus->setEnabled(true);
    ui->pushUefi->setVisible(isUefi() && isInstalled("efibootmgr"));

    // Check if splash is enabled in kernel command line
    if (!isSplashEnabled()) {
        ui->pushPreview->setDisabled(true);
        ui->pushPreview->setToolTip(tr("Preview is disabled because 'splash' parameter is not present in kernel command line. "
                                       "To enable preview, add 'splash' to boot parameters and reboot."));
    } else {
        ui->pushPreview->setToolTip("");
    }

    // Configure GRUB theme related UI elements
    const bool grubThemesExist = QFile::exists("/boot/grub/themes");
    ui->checkGrubTheme->setVisible(grubThemesExist);
    ui->pushThemeFile->setVisible(grubThemesExist);
    ui->pushThemeFile->setDisabled(true);
}

void MainWindow::handleLiveSystem()
{
    if (!live) {
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Live System Detected"));
    msgBox.setText(
        tr("You are currently running a live system. Would you like to modify the boot options for the live system "
           "or for an installed system?"));
    QPushButton *liveButton = msgBox.addButton(tr("Live System"), QMessageBox::ActionRole);
    QPushButton *installedButton = msgBox.addButton(tr("Installed System"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == installedButton) {
        QString partition = selectPartition(getLinuxPartitions());
        if (!partition.isEmpty()) {
            createChrootEnv(partition);
        }
    } else if (msgBox.clickedButton() == liveButton) {
        installedMode = false;
        bootLocation = QFileInfo::exists("/live/config/did-toram") ? "/live/to-ram" : "/live/boot-dev";
    }
}

void MainWindow::setupGrubSettings()
{
    grubInstalled
        = cmd.run("dpkg -s grub-common | grep -q 'Status: install ok installed'", nullptr, nullptr, QuietMode::Yes);
    ui->groupBoxOptions->setHidden(!grubInstalled);
    ui->groupBoxBackground->setHidden(!grubInstalled);

    if (grubInstalled) {
        readGrubCfg();
        readDefaultGrub();
    }
}

void MainWindow::handleSpecialFilesystems()
{
    const QString fstype
        = cmd.getOut("df --output=fstype " + (chroot.isEmpty() ? "/boot" : tempDir.path()) + " | tail -n1");
    if (cmd.exitCode() != 0 || fstype.isEmpty()) {
        qWarning() << "Failed to get filesystem type.";
        return;
    }
    if (fstype == "btrfs") {
        ui->checkSaveDefault->setChecked(false);
        ui->checkSaveDefault->setDisabled(true);
    }
}

void MainWindow::unmountAndClean(const QStringList &mountList)
{
    for (const auto &mountPoint : std::as_const(mountList)) {
        // Skip if mount point is already mounted at /boot/efi
        if (QProcess::execute("findmnt", {"-n", mountPoint, "/boot/efi"}) != 0) {
            // Extract partition name from mount point path
            QString partName = mountPoint.section('/', 2, 2);
            QString efiMount = "/boot/efi/" + partName;

            if (!cmd.procAsRoot("umount", {efiMount})) {
                qWarning() << "Failed to unmount" << efiMount;
                continue;
            }

            if (!cmd.procAsRoot("rmdir", {efiMount})) {
                qWarning() << "Failed to remove directory" << efiMount;
            }
        }
    }
}

// Set mouse in the corner and move it to advance splash preview
void MainWindow::sendMouseEvents()
{
    QCursor::setPos(QApplication::primaryScreen()->geometry().width(),
                    QApplication::primaryScreen()->geometry().height() + 1);
}

void MainWindow::setGeneralConnections()
{
    connect(ui->checkBootsplash, &QCheckBox::clicked, this, &MainWindow::comboBootsplashClicked);
    connect(ui->checkBootsplash, &QCheckBox::toggled, this, &MainWindow::comboBootsplashToggled);
    connect(ui->checkBootsplash, &QCheckBox::toggled, ui->comboTheme, &QComboBox::setEnabled);
    connect(ui->checkEnableFlatmenus, &QCheckBox::clicked, this, &MainWindow::comboEnableFlatmenusClicked);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, this, &MainWindow::comboGrubThemeToggled);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, ui->pushBgFile, &QPushButton::setDisabled);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, ui->pushThemeFile, &QPushButton::setEnabled);
    connect(ui->checkSaveDefault, &QCheckBox::clicked, this, &MainWindow::comboSaveDefaultClicked);
    connect(ui->comboMenuEntry, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::comboMenuEntryCurrentIndexChanged);
    connect(ui->comboTheme, qOverload<int>(&QComboBox::activated), this, &MainWindow::comboThemeActivated);
    connect(ui->comboTheme, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::comboThemeCurrentIndexChanged);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAboutClicked);
    connect(ui->pushApply, &QPushButton::clicked, this, &MainWindow::pushApplyClicked);
    connect(ui->pushBgFile, &QPushButton::clicked, this, &MainWindow::btnBgFileClicked);
    connect(ui->pushCancel, &QPushButton::pressed, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelpClicked);
    connect(ui->pushLog, &QPushButton::clicked, this, &MainWindow::pushLogClicked);
    connect(ui->pushPreview, &QPushButton::clicked, this, &MainWindow::pushPreviewClicked);
    connect(ui->pushThemeFile, &QPushButton::clicked, this, &MainWindow::btnThemeFileClicked);
    connect(ui->pushUefi, &QPushButton::clicked, this, &MainWindow::pushUefiClicked);
    connect(ui->radioDetailedMsg, &QRadioButton::toggled, this, &MainWindow::radioDetailedMsgToggled);
    connect(ui->radioLimitedMsg, &QRadioButton::toggled, this, &MainWindow::radioLimitedMsgToggled);
    connect(ui->radioVeryDetailedMsg, &QRadioButton::toggled, this, &MainWindow::radioVeryDetailedMsgToggled);
    connect(ui->spinBoxTimeout, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::spinBoxTimeoutValueChanged);
    connect(ui->textKernel, &QLineEdit::textChanged, this, &MainWindow::lineEditKernelTextEdited);
}

bool MainWindow::isInstalled(const QString &package)
{
    QString cmdStr = QString("dpkg -s %1 | grep -q 'Status: install ok installed'").arg(package);
    return chroot.isEmpty() ? Cmd().run(cmdStr, nullptr, nullptr, QuietMode::Yes)
                            : Cmd().runAsRoot(chroot + cmdStr, nullptr, nullptr, QuietMode::Yes);
}

// Checks if a list of packages is installed, return false if one of them is not
bool MainWindow::isInstalled(const QStringList &packages)
{
    return std::all_of(packages.begin(), packages.end(), [&](const QString &package) { return isInstalled(package); });
}

// Check if running from a live environment
bool MainWindow::isLive()
{
    return QProcess::execute("mountpoint", {"-q", "/live/aufs"}) == 0;
}

bool MainWindow::isUefi()
{
    QDir dir("/sys/firmware/efi/efivars");
    return dir.exists() && !dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
}

void MainWindow::appendLogWithColors(QTextEdit *textEdit, const QString &logContent)
{
    // Regular expression to match ANSI escape sequences
    QRegularExpression ansiRegex(R"(\x1B\[(\d+)(;\d+)*m)");

    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    int lastPos = 0;
    QRegularExpressionMatchIterator matches = ansiRegex.globalMatch(logContent);

    QTextCharFormat format;
    format.setForeground(Qt::black); // Default text color

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        // Add text before the ANSI sequence
        int matchStart = match.capturedStart();
        if (lastPos < matchStart) {
            cursor.insertText(logContent.mid(lastPos, matchStart - lastPos), format);
        }

        // Process the ANSI sequence
        QStringList codes = match.captured(0).mid(2).split(';');
        for (const QString &code : codes) {
            QString cleanCode = code;
            cleanCode.remove(QRegularExpression("[^0-9]")); // Remove non-numeric chars
            int value = cleanCode.toInt();

            if (value == 0) {
                // Reset to default
                format.setForeground(Qt::black);
                format.setFontWeight(QFont::Normal);
            } else if (value == 1) {
                format.setFontWeight(QFont::Bold);
            } else if (value == 31) {
                format.setForeground(Qt::red);
            } else if (value == 32) {
                format.setForeground(Qt::darkGreen);
            } else if (value == 33) {
                format.setForeground(Qt::yellow);
            } else if (value == 39) {
                format.setForeground(Qt::black);
            }
        }

        lastPos = match.capturedEnd();
    }

    // Add remaining text after the last match
    if (lastPos < logContent.length()) {
        cursor.insertText(logContent.mid(lastPos), format);
    }
}

void MainWindow::installSplash()
{
    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButton(nullptr);
    progress->setWindowTitle(tr("Installing bootsplash, please wait"));
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->resize(500, progress->height());
    progress->show();

    setConnections();
    progress->setLabelText(tr("Updating sources"));

    if (!cmd.runAsRoot(chroot + "apt-get update")) {
        progress->close();
        QMessageBox::critical(this, tr("Error"), tr("Failed to update package sources."));
        return;
    }

    progress->setLabelText(tr("Installing packages:") + " " + requiredPackages.join(", "));

    if (!cmd.runAsRoot(chroot + "apt-get install -y " + requiredPackages.join(' '))) {
        progress->close();
        QMessageBox::critical(this, tr("Error"), tr("Could not install the bootsplash."));
        ui->checkBootsplash->setChecked(false);
        return;
    }

    progress->close();
    QMessageBox::information(this, tr("Success"), tr("Bootsplash installed successfully."));
}

// Detect Virtual Machine to let user know Plymouth is not fully functional
bool MainWindow::inVirtualMachine()
{
    // "lspci -d 15ad:" for VMWare detection
    // -- plymouth seems to work in VMWare, might work in VM depending on driver setup
    QString out = cmd.getOut("lspci -d 80ee:beef;lspci -d 80ee:cafe", QuietMode::Yes);
    return (!out.isEmpty());
}

// Write new config in /etc/default/grub
void MainWindow::writeDefaultGrub()
{
    const QString chr = chroot.section(' ', 1, 1);
    const QString grubFilePath = chr + "/etc/default/grub";
    const QString backupFilePath = grubFilePath + ".bak";

    // Create a new backup file
    cmd.procAsRoot("cp", {backupFilePath, backupFilePath + ".0"});
    cmd.procAsRoot("rm", {backupFilePath});
    cmd.procAsRoot("cp", {grubFilePath, backupFilePath});
    cmd.procAsRoot("chown", {"root:", backupFilePath}, nullptr, nullptr, QuietMode::Yes);
    cmd.procAsRoot("chmod", {"644", backupFilePath}, nullptr, nullptr, QuietMode::Yes);

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create temporary file."));
        return;
    }

    QTextStream stream(&tmpFile);
    for (const QString &line : defaultGrub) {
        stream << line << '\n';
    }
    tmpFile.flush();
    tmpFile.close();

    cmd.procAsRoot("mv", {tmpFile.fileName(), grubFilePath});
    cmd.procAsRoot("chown", {"root:", grubFilePath}, nullptr, nullptr, QuietMode::Yes);
    cmd.procAsRoot("chmod", {"644", grubFilePath}, nullptr, nullptr, QuietMode::Yes);
}

QStringList MainWindow::getLinuxPartitions()
{
    const QStringList partitions
        = cmd.getOutAsRoot("lsblk -ln -o NAME,SIZE,FSTYPE,MOUNTPOINT,LABEL -e 2,11 -x NAME | "
                           "grep -E '^x?[h,s,v].[a-z][0-9]|^mmcblk[0-9]+p|^nvme[0-9]+n[0-9]+p'")
              .split('\n', Qt::SkipEmptyParts);
    QStringList validPartitions;
    validPartitions.reserve(partitions.size());
    for (const QString &part_info : partitions) {
        QString partName = part_info.section(' ', 0, 0);
        QString partType
            = cmd.getOutAsRoot("lsblk -ln -o PARTTYPE /dev/" + partName, QuietMode::Yes).trimmed().toLower();

        if (partType.contains(QRegularExpression(
                R"(0x83|0fc63daf-8483-4772-8e79-3d69d8477de4|44479540-f297-41b2-9af7-d131d5f0458a|4f68bce3-e8cd-4db1-96e7-fbcaf984b709|ca7d7ccb-63ed-4c53-861c-1742536059cc)"))) {
            validPartitions << part_info;
        }
    }
    return validPartitions;
}

// Cleanup chroot environment and temporary directory
void MainWindow::cleanup()
{
    qDebug() << "Running MXBO cleanup code";
    if (chroot.isEmpty()) {
        return;
    }

    const QString path = chroot.section(' ', 1, 1);
    if (path.isEmpty()) {
        return;
    }

    // Umount EFI partition if mounted
    if (cmd.proc("mountpoint", {"-q", path + "/boot/efi"})) {
        if (!cmd.procAsRoot("umount", {path + "/boot/efi"})) {
            qWarning() << "Failed to unmount" << path + "/boot/efi";
        }
    }

    // Unmount virtual filesystems in reverse order of mounting
    const QStringList mounts = {"/run", "/proc", "/sys", "/dev"};

    for (const auto &mount : mounts) {
        if (!cmd.procAsRoot("umount", {"-R", path + mount})) {
            qWarning() << "Failed to unmount" << path + mount;
        }
    }

    // Finally unmount and remove the chroot directory
    if (!cmd.procAsRoot("umount", {"-R", path})) {
        qWarning() << "Failed to unmount" << path;
    }
    if (!cmd.procAsRoot("rmdir", {path})) {
        qWarning() << "Failed to remove directory" << path;
    }
}

QString MainWindow::selectPartition(const QStringList &list)
{
    auto *dialog = new CustomDialog(list);

    // Guess MX install by finding the first partition with a rootMX* label
    auto it = std::find_if(list.cbegin(), list.cend(), [&](const QString &part_info) {
        QString label = part_info.section(' ', 0, 0);
        QString command = QString("lsblk -ln -o LABEL /dev/%1 | grep -q rootMX").arg(label);
        return cmd.run(command);
    });

    if (it != list.cend()) {
        int index = dialog->comboBox()->findText(*it);
        if (index != -1) {
            dialog->comboBox()->setCurrentIndex(index);
        }
    }

    if (dialog->exec() == QDialog::Accepted) {
        QString selectedText = dialog->comboBox()->currentText().section(' ', 0, 0);
        qDebug() << "Dialog accepted:" << selectedText;
        return selectedText;
    } else {
        qDebug() << "Dialog rejected:" << dialog->comboBox()->currentText().section(' ', 0, 0);
        return {};
    }
}

void MainWindow::addGrubLine(const QString &item)
{
    defaultGrub << item;
}

void MainWindow::createChrootEnv(const QString &root)
{
    if (!tempDir.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("Could not create a temporary folder"));
        exit(EXIT_FAILURE);
    }

    // Build mount commands
    QStringList mountCommands;

    if (isLuks("/dev/" + root)) {
        if (!openLuks("/dev/" + root, tempDir.path())) {
            QMessageBox::critical(this, tr("Cannot continue"), tr("Cannot open LUKS device. Exiting..."));
            cleanup();
            exit(EXIT_FAILURE);
        }
    } else {
        mountCommands << QString("mount /dev/%1 %2").arg(root, tempDir.path());
    }

    // Add remaining mount commands
    mountCommands << QString("mkdir -p %1/{dev,sys,proc,run}").arg(tempDir.path())
                  << QString("mount --rbind --make-rslave /dev %1/dev").arg(tempDir.path())
                  << QString("mount --rbind --make-rslave /sys %1/sys").arg(tempDir.path())
                  << QString("mount --rbind /proc %1/proc").arg(tempDir.path())
                  << QString("mount -t tmpfs -o size=100m,nodev,mode=755 tmpfs %1/run").arg(tempDir.path())
                  << QString("mkdir -p %1/run/udev").arg(tempDir.path())
                  << QString("mount --rbind /run/udev %1/run/udev").arg(tempDir.path());

    // Execute mount commands
    for (const QString &command : mountCommands) {
        if (!cmd.runAsRoot(command)) {
            QMessageBox::critical(this, tr("Cannot continue"),
                                  tr("Cannot create chroot environment, cannot change boot options. Exiting..."));
            cleanup();
            exit(EXIT_FAILURE);
        }
    }

    chroot = "chroot " + tempDir.path() + " ";
    ui->pushPreview->setDisabled(true); // Disable preview when running chroot
}

// Uncomment or add line in /etc/default/grub
void MainWindow::enableGrubLine(const QString &item)
{
    QStringList new_list;
    bool isItemFound = false;

    for (const QString &line : std::as_const(defaultGrub)) {
        if (line == item || line.startsWith("#" + item)) {
            isItemFound = true;
            new_list << item; // Add the item as enabled
        } else {
            new_list << line; // Keep the existing line
        }
    }

    // If the item was not found, add it to the list
    if (!isItemFound) {
        new_list.prepend("\n"); // Add a newline before the new item for better formatting
        new_list << item;
    }

    defaultGrub = new_list; // Update the defaultGrub list
}

// Comment out lines in /etc/default/grub that start with the specified item
void MainWindow::disableGrubLine(const QString &item)
{
    QStringList new_list;
    new_list.reserve(defaultGrub.size());
    for (const QString &line : std::as_const(defaultGrub)) {
        new_list << (line.startsWith(item) ? "#" + line : line);
    }
    defaultGrub = new_list;
}

// Replace the argument in /etc/default/grub and return false if nothing was replaced
bool MainWindow::replaceGrubArg(const QString &key, const QString &item)
{
    QStringList new_list;
    bool replaced = false;

    for (const QString &line : std::as_const(defaultGrub)) {
        if (line.startsWith(key + "=")) {
            new_list << key + "=" + item; // Replace the entire line with the new argument
            replaced = true;
        } else {
            new_list << line; // Keep the existing line
        }
    }

    defaultGrub = new_list; // Update the defaultGrub list
    return replaced;        // Return whether a replacement occurred
}

void MainWindow::replaceLiveGrubArgs(const QString &args)
{
    QString liveGrubsavePath = "/usr/local/bin/live-grubsave";
    if (!QFile::exists(liveGrubsavePath)) {
        liveGrubsavePath = "/usr/bin/live-grubsave";
    }

    if (!cmd.procAsRoot(liveGrubsavePath, {"-r"})) {
        qWarning() << "Failed to reset live-grub settings";
        return;
    }

    QString filteredArgs = args;
    filteredArgs.remove(QRegularExpression("BOOT_IMAGE=[^ ]*"));
    filteredArgs = filteredArgs.trimmed();

    if (!filteredArgs.isEmpty()) {
        if (!cmd.procAsRoot(liveGrubsavePath, {filteredArgs})) {
            qWarning() << "Failed to save new live-grub arguments:" << filteredArgs;
        }
    }
}

void MainWindow::replaceSyslinuxArgs(const QString &args)
{
    const QStringList configFiles
        = {bootLocation + "/boot/syslinux/syslinux.cfg", bootLocation + "/boot/isolinux/isolinux.cfg"};

    for (const QString &configFile : configFiles) {
        QFile file(configFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << configFile << "for reading.";
            continue;
        }

        QStringList new_list;
        bool inLiveSection = false;
        bool replaced = false;

        while (!file.atEnd()) {
            QString line = file.readLine().trimmed();

            if (line.startsWith("LABEL live")) {
                inLiveSection = true;
            } else if (line.startsWith("LABEL") && inLiveSection) {
                inLiveSection = false;
            }

            if (inLiveSection && line.trimmed().startsWith("APPEND")) {
                QString filteredArgs = args;
                filteredArgs.remove(QRegularExpression("BOOT_IMAGE=[^ ]*"));
                line = line.left(line.indexOf("APPEND") + 7) + filteredArgs.trimmed();
                replaced = true;
            }

            if (inLiveSection && line.trimmed().startsWith("KERNEL")) {
                QString bootImage = args.section("BOOT_IMAGE=", 1, 1).section(' ', 0, 0);
                line = line.left(line.indexOf("KERNEL") + 7) + bootImage;
            }

            new_list << line;
        }

        file.close();

        if (!replaced) {
            qWarning() << "No APPEND line found in LABEL live section in" << configFile;
            continue;
        }

        // Write to a temporary file using QTemporaryFile
        QTemporaryFile tempFile(QDir::tempPath() + "/XXXXXX.tmp");
        if (!tempFile.open()) {
            qWarning() << "Failed to open temporary file for writing.";
            continue;
        }

        QTextStream stream(&tempFile);
        stream.setEncoding(QStringConverter::Utf8);
        stream << new_list.join('\n') << '\n';
        tempFile.flush();
        tempFile.close();

        // Move the temporary file to the original file
        QString tempFilePath = tempFile.fileName();
        if (!cmd.procAsRoot("mv", {tempFilePath, configFile})) {
            qWarning() << "Failed to move" << tempFilePath << "to" << configFile;
        }
    }
}

void MainWindow::readGrubCfg()
{
    QString grubFilePath = chroot.isEmpty() ? "/boot/grub/grub.cfg" : chroot.section(' ', 1, 1) + "/boot/grub/grub.cfg";
    QStringList content = cmd.getOutAsRoot("cat " + grubFilePath, QuietMode::Yes).split('\n', Qt::SkipEmptyParts);

    if (content.isEmpty()) {
        qDebug() << "Could not read grub.cfg file";
        return;
    }

    ui->comboMenuEntry->clear();
    int menuLevel = 0;
    int menuCount = 0;
    int submenuCount = 0;
    QString menuId;

    for (const auto &line : content) {
        QString trimmedLine = line.trimmed();
        grubCfg << trimmedLine;

        if (trimmedLine.startsWith("menuentry ") || trimmedLine.startsWith("submenu ")) {
            menuId = trimmedLine.section("$menuentry_id_option", 1, -1).section(' ', 1, 1);
            QString item = trimmedLine.section(QRegularExpression("['\"]"), 1, 1);
            QString info;

            if (menuLevel > 0) {
                info = QString("%1 %2>%3").arg(menuId, QString::number(menuCount - 1), QString::number(submenuCount));
                item.prepend("    ");
                ++submenuCount;
            } else {
                info = QString("%1 %2").arg(menuId, QString::number(menuCount));
                ++menuCount;
            }
            ui->comboMenuEntry->addItem(item, info);
        }

        // Adjust menu level based on braces
        menuLevel += trimmedLine.contains('{') ? 1 : 0;
        menuLevel -= trimmedLine.contains('}') ? 1 : 0;

        // Reset submenu count when returning to top level
        if (menuLevel == 0) {
            submenuCount = 0;
        }
    }
}

void MainWindow::readDefaultGrub()
{
    QFile file(chroot.section(' ', 1, 1) + "/etc/default/grub");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file:" << file.fileName();
        return;
    }

    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        defaultGrub << line;

        if (line.startsWith("GRUB_DEFAULT=")) {
            processGrubDefault(line);
        } else if (line.startsWith("GRUB_TIMEOUT=")) {
            ui->spinBoxTimeout->setValue(line.section('=', 1).remove(QRegularExpression("[\"']")).toInt());
        } else if (line.startsWith("export GRUB_MENU_PICTURE=")) {
            QString picturePath = line.section('=', 1).remove('"');
            ui->pushBgFile->setText(picturePath);
            ui->pushBgFile->setProperty("file", picturePath);
        } else if (line.startsWith("GRUB_THEME=")) {
            processGrubTheme(line);
        } else if (line.startsWith("GRUB_CMDLINE_LINUX_DEFAULT=")) {
            processKernelCommandLine(line);
        } else if (line.startsWith("GRUB_DISABLE_SUBMENU=")) {
            QString token = line.section('=', 1).remove(QRegularExpression("[\"']"));
            ui->checkEnableFlatmenus->setChecked(QStringList {"y", "yes", "true"}.contains(token));
        }
    }
    file.close();
}

void MainWindow::processGrubDefault(const QString &line)
{
    QString entry = line.section('=', 1).remove(QRegularExpression("[\"']"));
    bool ok = false;
    int number = entry.toInt(&ok);

    if (ok) {
        ui->comboMenuEntry->setCurrentIndex(
            ui->checkEnableFlatmenus->isChecked()
                ? number
                : ui->comboMenuEntry->findData(" " + entry, Qt::UserRole, Qt::MatchEndsWith));
    } else if (entry == QLatin1String("saved")) {
        ui->checkSaveDefault->setChecked(true);
    } else {
        int index = entry.length() > 3 ? ui->comboMenuEntry->findData(entry, Qt::UserRole, Qt::MatchContains)
                                       : ui->comboMenuEntry->findData(entry, Qt::UserRole, Qt::MatchEndsWith);
        ui->comboMenuEntry->setCurrentIndex(index != -1 ? index : ui->comboMenuEntry->findText(entry));
    }
}

void MainWindow::processGrubTheme(const QString &line)
{
    const QString themePath = line.section('=', 1).remove('"');
    ui->pushThemeFile->setText(themePath);
    ui->pushThemeFile->setProperty("file", themePath);
    bool themeExists = QFile::exists(themePath);
    ui->pushThemeFile->setEnabled(themeExists);
    ui->checkGrubTheme->setChecked(themeExists);
    ui->pushBgFile->setDisabled(themeExists);
}

void MainWindow::processKernelCommandLine(QString line)
{
    const QString cmdline = line.remove("GRUB_CMDLINE_LINUX_DEFAULT=").remove(QRegularExpression("[\"']"));
    ui->textKernel->setText(live && !installedMode ? kernelOptions : cmdline);

    bool hasHush = cmdline.contains("hush");
    bool hasQuiet = cmdline.contains("quiet");
    ui->radioLimitedMsg->setChecked(hasHush);
    ui->radioDetailedMsg->setChecked(hasQuiet);
    ui->radioVeryDetailedMsg->setChecked(!hasHush && !hasQuiet);

    ui->checkBootsplash->setChecked(cmdline.contains("splash") && isInstalled(requiredPackages));
}

// Read kernel line and options from /proc/cmdline
QString MainWindow::readKernelOpts()
{
    QFile file("/proc/cmdline");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << file.fileName() << "- Error:" << file.errorString();
        return {};
    }
    return file.readAll().trimmed();
}

void MainWindow::cmdStart()
{
    setCursor(Qt::BusyCursor);
    bar->setValue(0);
    timer.start(100ms);
}

void MainWindow::cmdDone()
{
    setCursor(Qt::ArrowCursor);
    bar->setValue(bar->maximum());
    timer.stop();
}

void MainWindow::procTime()
{
    bar->setValue((bar->value() + 10) % bar->maximum() + 1);
}

void MainWindow::setConnections()
{
    timer.disconnect();
    timer.stop();

    connect(&timer, &QTimer::timeout, this, &MainWindow::procTime);
    connect(&cmd, &QProcess::started, this, &MainWindow::cmdStart);
    connect(&cmd, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::cmdDone);
}

void MainWindow::pushApplyClicked()
{
    ui->pushCancel->setDisabled(true);
    ui->pushApply->setDisabled(true);

    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButton(nullptr);
    progress->setWindowTitle(tr("Updating configuration, please wait"));
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->resize(500, progress->height());
    progress->show();

    setConnections();

    if (kernelOptionsChanged) {
        replaceGrubArg("GRUB_CMDLINE_LINUX_DEFAULT", "\"" + ui->textKernel->text() + "\"");
        if (live && !installedMode) {
            replaceLiveGrubArgs(ui->textKernel->text());
            replaceSyslinuxArgs(ui->textKernel->text());
        }
    }

    if (optionsChanged) {
        cmd.runAsRoot("grub-editenv /boot/grub/grubenv unset next_entry"); // unset the saved entry from grubenv

        const QString bgFilePath = ui->pushBgFile->property("file").toString();
        const QString themeFilePath = ui->pushThemeFile->property("file").toString();

        if (ui->pushBgFile->isEnabled() && QFile::exists(bgFilePath)) {
            if (!replaceGrubArg("export GRUB_MENU_PICTURE", "\"" + bgFilePath + "\"")) {
                addGrubLine("export GRUB_MENU_PICTURE=\"" + bgFilePath + "\"");
            }
        } else if (ui->checkGrubTheme->isChecked() && QFile::exists(themeFilePath)) {
            disableGrubLine("export GRUB_MENU_PICTURE");
            if (!replaceGrubArg("GRUB_THEME", "\"" + themeFilePath + "\"")) {
                addGrubLine("GRUB_THEME=\"" + themeFilePath + "\"");
            }
        }

        if (ui->checkGrubTheme->isVisible() && !ui->checkGrubTheme->isChecked()) {
            disableGrubLine("GRUB_THEME=");
        }

        // For simple menu index number is sufficient, if submenus exists use "1>1" format
        QString grub_entry = ui->checkEnableFlatmenus->isChecked()
                                 ? QString::number(ui->comboMenuEntry->currentIndex())
                                 : ui->comboMenuEntry->currentData().toString().section(' ', 1, 1);

        if (ui->comboMenuEntry->currentText().contains(QLatin1String("memtest"))) {
            ui->spinBoxTimeout->setValue(5);
            cmd.runAsRoot(chroot + "grub-reboot \"" + ui->comboMenuEntry->currentText() + '"');
        } else {
            replaceGrubArg("GRUB_DEFAULT", "\"" + grub_entry + '"');
        }

        if (ui->checkSaveDefault->isChecked()) {
            replaceGrubArg("GRUB_DEFAULT", "saved");
            enableGrubLine("GRUB_SAVEDEFAULT=true");
            cmd.runAsRoot(chroot + "grub-set-default \"" + grub_entry + '"');
        } else {
            disableGrubLine("GRUB_SAVEDEFAULT=true");
        }

        if (!replaceGrubArg("GRUB_TIMEOUT", QString::number(ui->spinBoxTimeout->value()))) {
            addGrubLine("GRUB_TIMEOUT=" + QString::number(ui->spinBoxTimeout->value()));
        }
    }

    if (splashChanged) {
        if (ui->checkBootsplash->isChecked()) {
            if (!ui->comboTheme->currentText().isEmpty()) {
                cmd.runAsRoot(chroot + "/sbin/plymouth-set-default-theme " + ui->comboTheme->currentText());
            }
            cmd.runAsRoot(chroot + "update-rc.d bootlogd disable");
        } else {
            cmd.runAsRoot(chroot + "update-rc.d bootlogd enable");
        }
        progress->setLabelText(tr("Updating initramfs..."));
        cmd.runAsRoot(chroot + "update-initramfs -u -k all");
    }

    if (messagesChanged && ui->radioLimitedMsg->isChecked()) {
        cmd.runAsRoot(chroot
                      + "grep -q hush /etc/default/rcS || echo \"\n# hush boot-log into /run/rc.log\n"
                        "[ \\\"\\$init\\\" ] && grep -qw hush /proc/cmdline && exec >> /run/rc.log 2>&1 || true \" >> "
                        "/etc/default/rcS");
    }

    if (optionsChanged || splashChanged || messagesChanged) {
        if (grubInstalled) {
            writeDefaultGrub();
            progress->setLabelText(tr("Updating grub..."));
            cmd.runAsRoot(chroot + "update-grub");
            if (live) {
                cmd.procAsRoot("cp", {"/boot/grub/grub.cfg", bootLocation + "/boot/grub/grub.cfg"});
            }
        }
        progress->close();
        QString message = live && bootLocation == "/live/to-ram"
                              ? tr("You are currently running in live mode with the 'toram' option. Please remember to "
                                   "save the persistence file or remaster, otherwise any changes made will be lost.")
                              : tr("Your changes have been successfully applied.");
        QMessageBox::information(this, tr("Operation Complete"), message);
    }

    // Reset change flags
    optionsChanged = false;
    splashChanged = false;
    messagesChanged = false;
    ui->pushCancel->setEnabled(true);
}

void MainWindow::pushAboutClicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(this->windowTitle()),
        R"(<p align="center"><b><h2>MX Boot Options</h2></b></p><p align="center">)" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Program for selecting common start-up choices")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-boot-options/license.html", tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void MainWindow::pushHelpClicked()
{
    QString url = "https://mxlinux.org/wiki/help-files/help-mx-boot-options/";

    // If curl exists use it to test if url is accessible, otherwise fallback
    QString executablePath = QStandardPaths::findExecutable("curl");
    if (!executablePath.isEmpty()) {
        const int timeout = 2000; // ms
        QProcess proc;
        proc.start("curl", {"-fsI", "-m2", url, "-o", "/dev/null"});
        proc.waitForFinished(timeout);
        proc.terminate();
        proc.waitForFinished(timeout);
        if (proc.exitCode() != 0) {
            url = "/usr/share/doc/mx-boot-options/mx-boot-options.html";
        }
    }
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::comboBootsplashClicked(bool checked)
{
    ui->radioLimitedMsg->setVisible(!checked);

    if (checked) {
        if (inVirtualMachine()) {
            QMessageBox::information(
                this, tr("Running in a Virtual Machine"),
                tr("You current system is running in a Virtual Machine,\n"
                   "Plymouth bootsplash will work in a limited way, you also won't be able to preview the theme"));
            // ui->pushPreview->setDisabled(true);
        }

        if (!isInstalled(requiredPackages)) {
            int response
                = QMessageBox::question(this, tr("Plymouth packages not installed"),
                                        tr("Plymouth packages are not currently installed.\nOK to go ahead and "
                                           "install them?"));
            if (response == QMessageBox::No) {
                ui->checkBootsplash->setChecked(false);
                ui->radioLimitedMsg->setVisible(!checked);
                return;
            }
            installSplash();
            justInstalled = true;
        }

        loadPlymouthThemes();
        if (ui->radioLimitedMsg->isChecked()) {
            ui->radioDetailedMsg->setChecked(true);
        }
    }

    splashChanged = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::btnBgFileClicked()
{
    QString initialPath = chroot.isEmpty() ? "/usr/share/backgrounds/MXLinux/grub"
                                           : chroot.section(' ', 1, 1) + "/usr/share/backgrounds/MXLinux/grub";
    QString selected = QFileDialog::getOpenFileName(this, tr("Select image to display in bootloader"), initialPath,
                                                    tr("Images (*.png *.jpg *.jpeg *.tga)"));

    if (!selected.isEmpty()) {
        if (!chroot.isEmpty()) {
            selected.remove(chroot.section(' ', 1, 1));
        }
        ui->pushBgFile->setText(selected);
        ui->pushBgFile->setProperty("file", selected);
        optionsChanged = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::radioDetailedMsgToggled(bool checked)
{
    if (checked) {
        messagesChanged = true;
        ui->pushApply->setEnabled(true);

        QString line = ui->textKernel->text();
        if (!line.contains(QLatin1String("quiet"))) {
            line.append(line.isEmpty() ? "quiet" : " quiet");
        }

        line.remove(QRegularExpression("\\s*hush"));
        ui->textKernel->setText(line.trimmed());
    }
}

void MainWindow::radioVeryDetailedMsgToggled(bool checked)
{
    if (checked) {
        messagesChanged = true;
        ui->pushApply->setEnabled(true);

        QString line = ui->textKernel->text();
        line.remove(QRegularExpression("\\s*quiet|\\s*hush"));
        ui->textKernel->setText(line.trimmed());
    }
}

void MainWindow::radioLimitedMsgToggled(bool checked)
{
    if (checked) {
        messagesChanged = true;
        ui->pushApply->setEnabled(true);

        QString line = ui->textKernel->text();
        QStringList options;

        if (!line.contains(QLatin1String("quiet"))) {
            options << "quiet";
        }
        if (!line.contains(QLatin1String("hush"))) {
            options << "hush";
        }

        if (!options.isEmpty()) {
            if (!line.isEmpty() && !line.endsWith(' ')) {
                line.append(' ');
            }
            line.append(options.join(' '));
        }

        ui->textKernel->setText(line.trimmed());
    }
}

void MainWindow::spinBoxTimeoutValueChanged(int /*unused*/)
{
    optionsChanged = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::comboMenuEntryCurrentIndexChanged()
{
    optionsChanged = true;
    ui->pushApply->setEnabled(true);
}

// Toggled either by user or when reading the status of bootsplash
void MainWindow::comboBootsplashToggled(bool checked)
{
    ui->comboTheme->setEnabled(checked);
    ui->pushPreview->setEnabled(checked && isSplashEnabled());

    QString line = ui->textKernel->text();
    if (checked) {
        loadPlymouthThemes();
        if (!line.contains(QLatin1String("splash"))) {
            line.append(line.isEmpty() ? "splash" : " splash");
        }
    } else {
        ui->comboTheme->clear();
        ui->pushPreview->setDisabled(true);
        line.remove(QRegularExpression("\\s*splash"));
    }

    ui->textKernel->setText(line.trimmed());
    kernelOptionsChanged = true;
}

void MainWindow::pushLogClicked()
{
    // Determine base path based on chroot status
    QString location = chroot.isEmpty() ? QString() : tempDir.path();

    // Primary log location based on kernel options
    location += kernelOptions.contains("hush") ? "/run/rc.log" : "/var/log/boot.log";

    // Fallback to alternate log location if primary doesn't exist
    if (!QFile::exists(location)) {
        location = chroot.isEmpty() ? "/var/log/boot" : tempDir.path() + "/var/log/boot";
    }

    if (QFile::exists(location)) {
        // Prepare command to remove formatting escape characters
        // QString sedCommand = R"(sed 's/\x1b\[?([0-9]{1,2}(;[0-9]{1,2})*)?m?//g; s/\r//;')";

        QString logContent = cmd.getOutAsRoot("cat " + location);

        // Create and configure the dialog to display the log
        QDialog logDialog;
        logDialog.setWindowTitle(tr("Boot Log"));

        auto *textEdit = new QTextEdit(&logDialog);
        textEdit->setReadOnly(true);
        textEdit->setMinimumSize(600, 500);
        appendLogWithColors(textEdit, logContent);
        // textEdit->setPlainText(logContent);

        auto *closeButton = new QPushButton(tr("&Close"), &logDialog);
        connect(closeButton, &QPushButton::clicked, &logDialog, &QDialog::accept);

        auto *layout = new QVBoxLayout(&logDialog);
        layout->addWidget(textEdit);
        layout->addWidget(closeButton);

        logDialog.setModal(true);
        logDialog.setSizeGripEnabled(true);
        logDialog.exec();
    } else {
        QMessageBox::critical(this, tr("Log not found"), tr("Could not find log at ") + location);
    }
}

void MainWindow::pushUefiClicked()
{
    this->hide();
    QProcess::execute("uefi-manager", {});
    this->show();
}

void MainWindow::comboThemeActivated(int /*unused*/)
{
    splashChanged = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::pushPreviewClicked()
{
    if (justInstalled) {
        QMessageBox::warning(
            this, tr("Needs reboot"),
            tr("Plymouth was just installed, you might need to reboot before being able to display previews"));
    }

    QString current_theme = cmd.getOutAsRoot(chroot + "/sbin/plymouth-set-default-theme");
    if (ui->comboTheme->currentText() == "details") {
        return;
    }

    if (inVirtualMachine()) {
        QMessageBox::information(
            this, tr("Running in a Virtual Machine"),
            tr("You current system is running in a Virtual Machine,\n"
               "Plymouth bootsplash will work in a limited way, you also won't be able to preview the theme"));
    }
    cmd.procAsRoot(chroot + "/sbin/plymouth-set-default-theme", {ui->comboTheme->currentText()});

    QTimer tick;
    tick.start(100ms);
    connect(&tick, &QTimer::timeout, this, &MainWindow::sendMouseEvents);
    cmd.runAsRoot("plymouthd; plymouth --show-splash; for ((i=0; i<4; i++)); do plymouth --update=test$i; sleep 1; "
                  "done; plymouth quit");
    cmd.procAsRoot(chroot + "/sbin/plymouth-set-default-theme", {current_theme}); // return to current theme
}

void MainWindow::comboEnableFlatmenusClicked(bool checked)
{
    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButton(nullptr);
    progress->setWindowTitle(tr("Updating configuration, please wait"));
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->resize(500, progress->height());
    progress->show();

    // Update GRUB configuration based on the checked state
    const QString grubLine = "GRUB_DISABLE_SUBMENU=y";
    if (checked) {
        enableGrubLine(grubLine);
    } else {
        disableGrubLine(grubLine);
    }

    writeDefaultGrub();
    progress->setLabelText(tr("Updating grub..."));
    setConnections();
    cmd.runAsRoot(chroot + "update-grub");
    readGrubCfg();
    progress->close();
}

void MainWindow::comboSaveDefaultClicked()
{
    optionsChanged = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::comboThemeCurrentIndexChanged(int index)
{
    ui->pushPreview->setDisabled(ui->comboTheme->itemText(index) == QLatin1String("details") || !isSplashEnabled());
}

void MainWindow::comboGrubThemeToggled(bool checked)
{
    if (checked && ui->pushThemeFile->property("file").toString().isEmpty()) {
        ui->pushThemeFile->setText(tr("Click to select theme"));
        ui->pushThemeFile->setProperty("file", "");
    } else {
        optionsChanged = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::btnThemeFileClicked()
{
    QString themeDirectory = chroot.section(' ', 1, 1) + "/boot/grub/themes";
    QString selected = QFileDialog::getOpenFileName(this, tr("Select GRUB theme"), themeDirectory, "*.txt;; *.*");

    if (!selected.isEmpty()) {
        if (!chroot.isEmpty()) {
            selected.remove(chroot.section(' ', 1, 1));
        }
        ui->pushThemeFile->setText(selected);
        ui->pushThemeFile->setProperty("file", selected);
        optionsChanged = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::lineEditKernelTextEdited()
{
    kernelOptionsChanged = true;
    optionsChanged = true;
    ui->pushApply->setEnabled(true);
}

bool MainWindow::isLuks(const QString &part)
{
    if (!cmd.procAsRoot("cryptsetup", {"isLuks", part})) {
        qDebug() << "Not a LUKS partion:" << part;
        return false;
    }
    return true;
}

bool MainWindow::openLuks(const QString &partition, const QString &path)
{
    QString uuid;
    if (!cmd.procAsRoot("cryptsetup", {"luksUUID", partition}, &uuid) || uuid.trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Could not retrieve UUID for %1").arg(partition));
        return false;
    }
    const QString mapper = "luks-" + uuid.trimmed();
    cmd.procAsRoot("cryptsetup", {"close", mapper}); // In case it was opened before

    bool ok;
    QByteArray pass = QInputDialog::getText(this, this->windowTitle(),
                                            tr("Enter password to unlock %1 encrypted partition:").arg(partition),
                                            QLineEdit::Password, QString(), &ok)
                          .toUtf8();

    if (!ok || pass.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Password entry cancelled or empty for %1").arg(partition));
        return false;
    }

    // Try to open the LUKS container
    if (!cmd.procAsRoot("cryptsetup", {"open", "--allow-discards", partition, mapper, "-"}, nullptr, &pass)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open %1 LUKS container").arg(partition));
        pass.fill(static_cast<char>(0xA5 & 0xFF));
        return false;
    }
    cmd.procAsRoot("mount", {"/dev/mapper/" + mapper, path});
    pass.fill(static_cast<char>(0xA5 & 0xFF));
    if (!mountBoot(path)) {
        return false;
    }
    return true;
}

bool MainWindow::mountBoot(const QString &path)
{
    // Check /etc/fstab for separate /boot partition
    QFile fstab(path + "/etc/fstab");
    if (!fstab.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open" << fstab.fileName();
        return false;
    }

    QString bootPartition;
    QTextStream in(&fstab);
    QString line;
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        QStringList fields = line.split(QRegularExpression("\\s+"));
        if (fields.size() >= 2 && fields.at(1) == "/boot") {
            bootPartition = fields.at(0);
            break;
        }
    }
    fstab.close();

    if (bootPartition.isEmpty()) {
        qWarning() << "No separate /boot partition found in /etc/fstab";
        return false;
    }

    // If UUID is used, convert to device name
    if (bootPartition.startsWith("UUID=")) {
        QString uuid = bootPartition.mid(5);
        bootPartition = "/dev/disk/by-uuid/" + uuid;
    }

    // Mount the boot partition
    if (!cmd.procAsRoot("mount", {bootPartition, path + "/boot"})) {
        qWarning() << "Could not mount" << bootPartition << "to" << path + "/boot";
        return false;
    }
    return true;
}

bool MainWindow::isSplashEnabled()
{
    QFile cmdlineFile("/proc/cmdline");
    if (!cmdlineFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not read /proc/cmdline";
        return false;
    }

    QString cmdline = QString::fromUtf8(cmdlineFile.readAll()).trimmed();
    cmdlineFile.close();

    // Split by spaces and check for exact "splash" parameter
    const QStringList params = cmdline.split(' ', Qt::SkipEmptyParts);
    return params.contains("splash");
}

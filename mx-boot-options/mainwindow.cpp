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
    // Clear the combobox and load themes
    ui->comboTheme->clear();
    QString command
        = chroot.isEmpty() ? "/sbin/plymouth-set-default-theme -l" : chroot + "/sbin/plymouth-set-default-theme -l";
    ui->comboTheme->addItems(cmd.getOut(command).split("\n"));

    // Retrieve and set the current theme
    QString current_theme = cmd.getOut(chroot.isEmpty() ? "/sbin/plymouth-set-default-theme"
                                                        : chroot + "/sbin/plymouth-set-default-theme")
                                .trimmed();
    if (!current_theme.isEmpty()) {
        int index = ui->comboTheme->findText(current_theme);
        if (index != -1) {
            ui->comboTheme->setCurrentIndex(index);
        }
    }
}

// Process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (cmd.state() == QProcess::Running || cmd.state() == QProcess::Starting) {
            auto response = QMessageBox::question(this, tr("Still running"),
                                                  tr("A process is still running. Do you really want to quit?"),
                                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (response != QMessageBox::Yes) {
                return;
            }
        }
        QApplication::quit();
    }
}

// Setup various items the first time the program runs
void MainWindow::setup()
{
    chroot.clear();
    bar = nullptr;
    options_changed = false;
    splash_changed = false;
    messages_changed = false;
    just_installed = false;

    user = cmd.getOut("logname", true);

    setWindowTitle("MX Boot Options");
    ui->pushCancel->setEnabled(true);
    ui->pushApply->setEnabled(true);
    ui->comboTheme->setDisabled(true);
    ui->pushPreview->setDisabled(true);
    ui->checkEnableFlatmenus->setEnabled(true);
    ui->pushUefi->setVisible(isUefi() && isInstalled("efibootmgr"));

    bool grubThemesExist = QFile::exists("/boot/grub/themes");
    ui->checkGrubTheme->setVisible(grubThemesExist);
    ui->pushThemeFile->setVisible(grubThemesExist);
    ui->pushThemeFile->setDisabled(true);

    // If running live, prompt user to choose between changing options for the live system or the installed system
    if (live) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Live System Detected"));
        msgBox.setText(
            tr("You are currently running a live system. Would you like to modify the boot options for the live system "
               "or for an installed system?"));
        QPushButton *liveButton = msgBox.addButton(tr("Live System"), QMessageBox::ActionRole);
        QPushButton *installedButton = msgBox.addButton(tr("Installed System"), QMessageBox::ActionRole);
        msgBox.exec();

        if (msgBox.clickedButton() == installedButton) {
            QString partition = selectPartiton(getLinuxPartitions());
            if (!partition.isEmpty()) {
                createChrootEnv(partition);
            }
        } else if (msgBox.clickedButton() == liveButton) {
            boot_location = QFileInfo::exists("/live/config/did-toram") ? "/live/to-ram" : "/live/boot-dev";
        }
    }

    grub_installed = cmd.run("dpkg -s grub-common | grep -q 'Status: install ok installed'", true);
    ui->groupBoxOptions->setHidden(!grub_installed);
    ui->groupBoxBackground->setHidden(!grub_installed);
    if (grub_installed) {
        readGrubCfg();
        readDefaultGrub();
    }

    if (cmd.getOut("df --output=fstype " + (chroot.isEmpty() ? "/boot" : tmpdir.path()) + " | tail -n1") == "btrfs") {
        ui->checkSaveDefault->setChecked(false);
        ui->checkSaveDefault->setDisabled(true);
    }

    readKernelOpts();
    ui->radioLimitedMsg->setVisible(!ui->checkBootsplash->isChecked());
    ui->pushApply->setDisabled(true);
    adjustSize();
}

void MainWindow::unmountAndClean(const QStringList &mount_list)
{
    for (const auto &mount_point : qAsConst(mount_list)) {
        if (QProcess::execute("findmnt", {"-n", mount_point, "/boot/efi"}) != 0) {
            QString part_name = mount_point.section("/", 2, 2);
            if (cmd.runAsRoot("umount /boot/efi/" + part_name)) {
                cmd.runAsRoot("rmdir /boot/efi/" + part_name);
            }
        }
    }
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
    list->currentItem()->setSelected(true);
    emit list->itemSelectionChanged();
}

// Set mouse in the corner and move it to advance splash preview
void MainWindow::sendMouseEvents()
{
    QCursor::setPos(QApplication::primaryScreen()->geometry().width(),
                    QApplication::primaryScreen()->geometry().height() + 1);
}

void MainWindow::setGeneralConnections()
{
    connect(ui->checkBootsplash, &QCheckBox::clicked, this, &MainWindow::combo_bootsplash_clicked);
    connect(ui->checkBootsplash, &QCheckBox::toggled, this, &MainWindow::combo_bootsplash_toggled);
    connect(ui->checkBootsplash, &QCheckBox::toggled, ui->comboTheme, &QComboBox::setEnabled);
    connect(ui->checkEnableFlatmenus, &QCheckBox::clicked, this, &MainWindow::combo_enable_flatmenus_clicked);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, this, &MainWindow::combo_grub_theme_toggled);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, ui->pushBgFile, &QPushButton::setDisabled);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, ui->pushThemeFile, &QPushButton::setEnabled);
    connect(ui->checkSaveDefault, &QCheckBox::clicked, this, &MainWindow::combo_save_default_clicked);
    connect(ui->comboMenuEntry, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::combo_menu_entry_currentIndexChanged);
    connect(ui->comboTheme, qOverload<int>(&QComboBox::activated), this, &MainWindow::combo_theme_activated);
    connect(ui->comboTheme, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &MainWindow::combo_theme_currentIndexChanged);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushApply, &QPushButton::clicked, this, &MainWindow::pushApply_clicked);
    connect(ui->pushBgFile, &QPushButton::clicked, this, &MainWindow::btn_bg_file_clicked);
    connect(ui->pushCancel, &QPushButton::pressed, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushLog, &QPushButton::clicked, this, &MainWindow::pushLog_clicked);
    connect(ui->pushPreview, &QPushButton::clicked, this, &MainWindow::push_preview_clicked);
    connect(ui->pushThemeFile, &QPushButton::clicked, this, &MainWindow::btn_theme_file_clicked);
    connect(ui->pushUefi, &QPushButton::clicked, this, &MainWindow::pushUefi_clicked);
    connect(ui->radioDetailedMsg, &QRadioButton::toggled, this, &MainWindow::radio_detailed_msg_toggled);
    connect(ui->radioLimitedMsg, &QRadioButton::toggled, this, &MainWindow::radio_limited_msg_toggled);
    connect(ui->radioVeryDetailedMsg, &QRadioButton::toggled, this, &MainWindow::radio_very_detailed_msg_toggled);
    connect(ui->spinBoxTimeout, qOverload<int>(&QSpinBox::valueChanged), this,
            &MainWindow::spinBoxTimeout_valueChanged);
    connect(ui->textKernel, &QLineEdit::textChanged, this, &MainWindow::lineEdit_kernel_textEdited);
}

void MainWindow::setUefiTimeout(QDialog *uefiDialog, QLabel *textTimeout)
{
    bool ok = false;
    ushort initialTimeout = textTimeout->text().section(' ', 1, 1).toUInt();
    ushort newTimeout = QInputDialog::getInt(uefiDialog, tr("Set timeout"), tr("Timeout in seconds:"), initialTimeout,
                                             0, 65535, 1, &ok);

    if (ok && Cmd().runAsRoot(QString("efibootmgr -t %1").arg(newTimeout))) {
        textTimeout->setText(tr("Timeout: %1 seconds").arg(newTimeout));
    }
}

void MainWindow::setUefiBootNext(QListWidget *listEntries, QLabel *textBootNext)
{
    if (auto currentItem = listEntries->currentItem()) {
        QString item = currentItem->text().section(' ', 0, 0);
        item.remove(QRegularExpression("^Boot"));
        item.remove(QRegularExpression(R"(\*$)"));

        if (QRegularExpression("^[0-9A-Z]{4}$").match(item).hasMatch() && Cmd().runAsRoot("efibootmgr -n " + item)) {
            textBootNext->setText(tr("Boot Next: %1").arg(item));
        }
    }
}

void MainWindow::removeUefiEntry(QListWidget *listEntries, QDialog *uefiDialog)
{
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

    if (Cmd().runAsRoot("efibootmgr -B -b " + item)) {
        delete currentItem;
    }
    emit listEntries->itemSelectionChanged();
}

void MainWindow::toggleUefiActive(QListWidget *listEntries)
{
    auto currentItem = listEntries->currentItem();
    if (!currentItem) {
        return; // Early exit if no item is selected
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

    QString command = QString("efibootmgr --%1 -b %2").arg(isActive ? "inactive" : "active").arg(item);
    if (Cmd().runAsRoot(command)) {
        listEntries->currentItem()->setText(QString("Boot%1%2 %3").arg(item).arg(isActive ? "" : "*").arg(rest));
        listEntries->currentItem()->setBackground(isActive ? QBrush(Qt::gray) : QBrush());
    }

    emit listEntries->itemSelectionChanged();
}

bool MainWindow::isInstalled(const QString &package)
{
    QString cmd_str = QString("dpkg -s %1 | grep -q 'Status: install ok installed'").arg(package);
    return chroot.isEmpty() ? Cmd().run(cmd_str, true) : Cmd().runAsRoot(chroot + cmd_str, true);
}

// Checks if a list of packages is installed, return false if one of them is not
bool MainWindow::isInstalled(const QStringList &packages)
{
    bool allPackagesInstalled
        = std::all_of(packages.begin(), packages.end(), [&](const QString &package) { return isInstalled(package); });
    return allPackagesInstalled;
}

// Check if running from a live envoronment
bool MainWindow::isLive()
{
    return QProcess::execute("mountpoint", {"-q", "/live/aufs"}) == 0;
}

bool MainWindow::isUefi()
{
    QDir dir("/sys/firmware/efi/efivars");
    return dir.exists() && !dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
}

void MainWindow::addUefiEntry(QListWidget *listEntries, QDialog *dialogUefi)
{
    // Mount all ESPs
    QStringList mountList
        = cmd.getOutAsRoot(
                 "lsblk -no PATH,PARTTYPE | grep -iE 'c12a7328-f81f-11d2-ba4b-00a0c93ec93b|0xef' | cut -d' ' -f1")
              .split("\n");

    for (const auto &mountPoint : qAsConst(mountList)) {
        if (QProcess::execute("findmnt", {"-n", mountPoint}) != 0) {
            QString partName = mountPoint.section('/', 2, 2);
            cmd.runAsRoot("mkdir -p /boot/efi/" + partName);
            cmd.runAsRoot("mount " + mountPoint + " /boot/efi/" + partName);
        }
    }

    QString initialPath = QFile::exists("/boot/efi/EFI") ? "/boot/efi/EFI" : "/boot/efi/";
    QString fileName
        = QFileDialog::getOpenFileName(dialogUefi, tr("Select EFI file"), initialPath, tr("EFI files (*.efi *.EFI)"));

    if (!QFile::exists(fileName)) {
        unmountAndClean(mountList);
        return;
    }

    QString partitionName = cmd.getOut("df " + fileName + " --output=source | sed 1d");
    QString disk = "/dev/" + cmd.getOut("lsblk -no PKNAME " + partitionName);
    QString partition = partitionName.mid(partitionName.lastIndexOf(QRegularExpression("[0-9]+$")));

    if (cmd.exitCode() != 0) {
        QMessageBox::critical(dialogUefi, tr("Error"), tr("Could not find the source mountpoint for %1").arg(fileName));
        unmountAndClean(mountList);
        return;
    }

    QString name = QInputDialog::getText(dialogUefi, tr("Set name"), tr("Enter the name for the UEFI menu item:"));
    if (name.isEmpty()) {
        name = "New entry";
    }

    fileName = "/EFI/" + fileName.section("/EFI/", 1);
    QString command = QString("efibootmgr -cL \"%1\" -d %2 -p %3 -l %4").arg(name, disk, partition, fileName);
    QString out = cmd.getOutAsRoot(command);

    if (cmd.exitCode() != 0) {
        QMessageBox::critical(dialogUefi, tr("Error"), tr("Something went wrong, could not add entry."));
        unmountAndClean(mountList);
        return;
    }

    QStringList outList = out.split('\n');
    listEntries->insertItem(0, outList.constLast());
    emit listEntries->itemSelectionChanged();
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

bool MainWindow::installSplash()
{
    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    const QStringList packages = {"plymouth", "plymouth-x11", "plymouth-themes", "plymouth-themes-mx"};
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
        return false;
    }

    progress->setLabelText(tr("Installing packages:") + " " + packages.join(", "));

    if (!cmd.runAsRoot(chroot + "apt-get install -y " + packages.join(" "))) {
        progress->close();
        QMessageBox::critical(this, tr("Error"), tr("Could not install the bootsplash."));
        ui->checkBootsplash->setChecked(false);
        return false;
    }

    progress->close();
    QMessageBox::information(this, tr("Success"), tr("Bootsplash installed successfully."));
    return true;
}

// Detect Virtual Machine to let user know Plymouth is not fully functional
bool MainWindow::inVirtualMachine()
{
    // "lspci -d 15ad:" for VMWare detection
    // -- plymouth seems to work in VMWare, might work in VM depending on driver setup
    QString out = cmd.getOut("lspci -d 80ee:beef;lspci -d 80ee:cafe", true);
    return (!out.isEmpty());
}

// Write new config in /etc/default/grub
void MainWindow::writeDefaultGrub()
{
    QString chr = chroot.section(' ', 1, 1);
    QString grubFilePath = chr + "/etc/default/grub";
    QString backupFilePath = grubFilePath + ".bak";

    // Create a new backup file
    cmd.runAsRoot("cp " + backupFilePath + " " + backupFilePath + ".0");
    cmd.runAsRoot("rm " + backupFilePath);
    cmd.runAsRoot("cp " + grubFilePath + " " + backupFilePath);
    cmd.runAsRoot("chown root: " + backupFilePath, true);
    cmd.runAsRoot("chmod 644 " + backupFilePath, true);

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create temporary file."));
        return;
    }

    QTextStream stream(&tmpFile);
    for (const QString &line : default_grub) {
        stream << line << "\n";
    }
    tmpFile.flush();
    tmpFile.close();

    cmd.runAsRoot("mv " + tmpFile.fileName() + " " + grubFilePath);
    cmd.runAsRoot("chown root: " + grubFilePath, true);
    cmd.runAsRoot("chmod 644 " + grubFilePath, true);
}

QStringList MainWindow::getLinuxPartitions()
{
    const QStringList partitions
        = cmd.getOutAsRoot("lsblk -ln -o NAME,SIZE,FSTYPE,MOUNTPOINT,LABEL -e 2,11 -x NAME | "
                           "grep -E '^x?[h,s,v].[a-z][0-9]|^mmcblk[0-9]+p|^nvme[0-9]+n[0-9]+p'")
              .split('\n', Qt::SkipEmptyParts);
    QStringList validPartitions;
    validPartitions.reserve(partitions.size());
    qDebug() << "PARTITIONS" << partitions;
    for (const QString &part_info : partitions) {
        QString partName = part_info.section(' ', 0, 0);
        QString partType = cmd.getOutAsRoot("lsblk -ln -o PARTTYPE /dev/" + partName).trimmed().toLower();

        if (partType.contains(QRegularExpression(
                R"(0x83|0fc63daf-8483-4772-8e79-3d69d8477de4|44479540-f297-41b2-9af7-d131d5f0458a|4f68bce3-e8cd-4db1-96e7-fbcaf984b709|ca7d7ccb-63ed-4c53-861c-1742536059cc)"))) {
            validPartitions << part_info;
        }
    }
    return validPartitions;
}

void MainWindow::readBootEntries(QListWidget *listEntries, QLabel *textTimeout, QLabel *textBootNext,
                                 QLabel *textBootCurrent, QStringList *bootorder)
{
    QStringList entries = cmd.getOutAsRoot("efibootmgr").split('\n', Qt::SkipEmptyParts);
    QRegularExpression bootEntryRegex(R"(^Boot[0-9A-F]{4}\*?\s+)");

    for (const auto &item : qAsConst(entries)) {
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

void MainWindow::cleanup()
{
    qDebug() << "Running MXBO cleanup code";
    if (!chroot.isEmpty()) {
        QString path = chroot.section(' ', 1, 1);
        if (path.isEmpty()) {
            return;
        }
        // Umount and clean temp folder
        Cmd().runAsRoot("mountpoint -q " + path + "/boot/efi && umount " + path + "/boot/efi");
        QString cmd_str = QStringLiteral("/bin/umount -R %1/run && /bin/umount -R %1/proc && /bin/umount -R %1/sys && "
                                         "/bin/umount -R %1/dev && umount %1 && rmdir %1")
                              .arg(path);
        Cmd().runAsRoot(cmd_str);
    }
}

QString MainWindow::selectPartiton(const QStringList &list)
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
        qDebug() << "exec true" << selectedText;
        return selectedText;
    } else {
        qDebug() << "exec false" << dialog->comboBox()->currentText().section(' ', 0, 0);
        return {};
    }
}

void MainWindow::addGrubLine(const QString &item)
{
    default_grub << item;
}

void MainWindow::createChrootEnv(const QString &root)
{
    if (!tmpdir.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("Could not create a temporary folder"));
        exit(EXIT_FAILURE);
    }

    // Create necessary directories for the chroot environment
    QStringList directories = {"dev", "sys", "proc", "run"};
    for (const QString &dir : directories) {
        cmd.run("mkdir -p " + tmpdir.path() + "/" + dir);
    }

    // Prepare the mount command
    QString cmd_str = QStringLiteral("/bin/mount /dev/%1 %2 && "
                                     "/bin/mount --rbind --make-rslave /dev %2/dev && "
                                     "/bin/mount --rbind --make-rslave /sys %2/sys && "
                                     "/bin/mount --rbind /proc %2/proc && "
                                     "/bin/mount -t tmpfs -o size=100m,nodev,mode=755 tmpfs %2/run && "
                                     "/bin/mkdir -p %2/run/udev && "
                                     "/bin/mount --rbind /run/udev %2/run/udev")
                          .arg(root, tmpdir.path());

    // Execute the mount command and handle failure
    if (!cmd.runAsRoot(cmd_str)) {
        QMessageBox::critical(this, tr("Cannot continue"),
                              tr("Cannot create chroot environment, cannot change boot options. Exiting..."));
        exit(EXIT_FAILURE);
    }

    chroot = "chroot " + tmpdir.path() + " ";
    ui->pushPreview->setDisabled(true); // Disable preview when running chroot
}

// Uncomment or add line in /etc/default/grub
void MainWindow::enableGrubLine(const QString &item)
{
    QStringList new_list;
    bool isItemFound = false;

    for (const QString &line : qAsConst(default_grub)) {
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

    default_grub = new_list; // Update the default_grub list
}

// Comment out lines in /etc/default/grub that start with the specified item
void MainWindow::disableGrubLine(const QString &item)
{
    QStringList new_list;
    new_list.reserve(default_grub.size());
    for (const QString &line : qAsConst(default_grub)) {
        new_list << (line.startsWith(item) ? "#" + line : line);
    }
    default_grub = new_list;
}

void MainWindow::saveBootOrder(const QListWidget *list)
{
    QStringList orderList;
    for (int i = 0; i < list->count(); ++i) {
        QString item = list->item(i)->text().section(' ', 0, 0);
        item.remove(QRegularExpression("^Boot|\\*$"));
        if (item.contains(QRegularExpression("^[0-9A-Z]{4}$"))) {
            orderList.append(item);
        }
    }

    QString order = orderList.join(",");
    if (!cmd.runAsRoot("efibootmgr -o " + order)) {
        qDebug() << "Order:" << order;
        QMessageBox::critical(this, tr("Error"), tr("Something went wrong, could not save boot order."));
    }
}

// Replace the argument in /etc/default/grub and return false if nothing was replaced
bool MainWindow::replaceGrubArg(const QString &key, const QString &item)
{
    QStringList new_list;
    bool replaced = false;

    for (const QString &line : qAsConst(default_grub)) {
        if (line.startsWith(key + "=")) {
            new_list << key + "=" + item; // Replace the entire line with the new argument
            replaced = true;
        } else {
            new_list << line; // Keep the existing line
        }
    }

    default_grub = new_list; // Update the default_grub list
    return replaced;         // Return whether a replacement occurred
}

void MainWindow::replaceSyslinuxArg(const QString &args)
{
    const QStringList configFiles
        = {boot_location + "/boot/syslinux/syslinux.cfg", boot_location + "/boot/isolinux/isolinux.cfg"};

    for (const QString &configFile : configFiles) {
        QFile file(configFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << configFile << "for reading.";
            continue;
        }

        QStringList new_list;
        bool replaced = false;

        while (!file.atEnd()) {
            QString line = file.readLine().trimmed();
            if (!kernel_options.isEmpty() && line.contains(kernel_options)) {
                line.replace(kernel_options, args);
                replaced = true;
            } else if (kernel_options.isEmpty() && line.startsWith("APPEND") && !replaced) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() > 1) {
                    parts[1] = args;
                } else {
                    parts.append(args);
                }
                line = parts.join(' ');
                replaced = true;
            }
            new_list << line;
        }

        file.close();

        if (!replaced) {
            qWarning() << "No" << (kernel_options.isEmpty() ? "APPEND line" : kernel_options) << "found to replace in"
                       << configFile << ".";
            continue;
        }

        // Write to a temporary file using QTemporaryFile
        QTemporaryFile tempFile(QDir::tempPath() + "/XXXXXX.tmp");
        if (!tempFile.open()) {
            qWarning() << "Failed to open temporary file for writing.";
            continue;
        }

        QTextStream stream(&tempFile);
        stream.setCodec("UTF-8");
        stream << new_list.join("\n") << "\n";
        tempFile.flush();
        tempFile.close();

        // Move the temporary file to the original file
        QString tempFilePath = tempFile.fileName();
        if (!cmd.runAsRoot("mv " + tempFilePath + " " + configFile)) {
            qWarning() << "Failed to move" << tempFilePath << "to" << configFile << ".";
        }
    }
}

void MainWindow::readGrubCfg()
{
    QStringList content;
    QString grubFilePath = chroot.isEmpty() ? "/boot/grub/grub.cfg" : chroot.section(' ', 1, 1) + "/boot/grub/grub.cfg";
    content = cmd.getOutAsRoot("cat " + grubFilePath, true).split('\n', Qt::SkipEmptyParts);

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
        grub_cfg << trimmedLine;

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
    QFile file(chroot.section(" ", 1, 1) + "/etc/default/grub");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file:" << file.fileName();
        return;
    }

    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        default_grub << line;

        if (line.startsWith("GRUB_DEFAULT=")) {
            QString entry = line.section("=", 1).remove("\"").remove("'");
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
        } else if (line.startsWith("GRUB_TIMEOUT=")) {
            ui->spinBoxTimeout->setValue(line.section("=", 1).remove("\"").remove("'").toInt());
        } else if (line.startsWith("export GRUB_MENU_PICTURE=")) {
            QString picturePath = line.section("=", 1).remove("\"");
            ui->pushBgFile->setText(picturePath);
            ui->pushBgFile->setProperty("file", picturePath);
        } else if (line.startsWith("GRUB_THEME=")) {
            QString themePath = line.section("=", 1).remove("\"");
            ui->pushThemeFile->setText(themePath);
            ui->pushThemeFile->setProperty("file", themePath);
            bool themeExists = QFile::exists(themePath);
            ui->pushThemeFile->setEnabled(themeExists);
            ui->checkGrubTheme->setChecked(themeExists);
            ui->pushBgFile->setDisabled(themeExists);
        } else if (line.startsWith("GRUB_CMDLINE_LINUX_DEFAULT=")) {
            QString cmdline = line.remove("GRUB_CMDLINE_LINUX_DEFAULT=").remove("\"").remove("'");
            ui->textKernel->setText(live ? kernel_options : cmdline);
            ui->radioLimitedMsg->setChecked(cmdline.contains("hush"));
            ui->radioDetailedMsg->setChecked(cmdline.contains("quiet"));
            ui->radioVeryDetailedMsg->setChecked(!cmdline.contains("hush") && !cmdline.contains("quiet"));
            ui->checkBootsplash->setChecked(
                cmdline.contains("splash")
                && isInstalled({"plymouth", "plymouth-x11", "plymouth-themes", "plymouth-themes-mx"}));
        } else if (line.startsWith("GRUB_DISABLE_SUBMENU=")) {
            QString token = line.section("=", 1).remove("\"").remove("'");
            ui->checkEnableFlatmenus->setChecked(token == "y" || token == "yes" || token == "true");
        }
    }
    file.close();
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

void MainWindow::pushApply_clicked()
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

    if (kernel_options_changed) {
        replaceGrubArg("GRUB_CMDLINE_LINUX_DEFAULT", "\"" + ui->textKernel->text() + "\"");
        if (live) {
            replaceSyslinuxArg(ui->textKernel->text());
        }
    }

    if (options_changed) {
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

    if (splash_changed) {
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

    if (messages_changed && ui->radioLimitedMsg->isChecked()) {
        cmd.runAsRoot(chroot
                      + "grep -q hush /etc/default/rcS || echo \"\n# hush boot-log into /run/rc.log\n"
                        "[ \\\"\\$init\\\" ] && grep -qw hush /proc/cmdline && exec >> /run/rc.log 2>&1 || true \" >> "
                        "/etc/default/rcS");
    }

    if (options_changed || splash_changed || messages_changed) {
        if (grub_installed) {
            writeDefaultGrub();
            progress->setLabelText(tr("Updating grub..."));
            cmd.runAsRoot(chroot + "update-grub");
            if (live) {
                cmd.runAsRoot("cp /boot/grub/grub.cfg " + boot_location + "/boot/grub/grub.cfg");
            }
        }
        progress->close();
        QString message = live && boot_location == "/live/to-ram"
                              ? tr("You are currently running in live mode with the 'toram' option. Please remember to "
                                   "save the persistence file or remaster, otherwise any changes made will be lost.")
                              : tr("Your changes have been successfully applied.");
        QMessageBox::information(this, tr("Operation Complete"), message);
    }

    // Reset change flags
    options_changed = false;
    splash_changed = false;
    messages_changed = false;
    ui->pushCancel->setEnabled(true);
}

void MainWindow::pushAbout_clicked()
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

void MainWindow::pushHelp_clicked()
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

void MainWindow::combo_bootsplash_clicked(bool checked)
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

        const QStringList requiredPackages = {"plymouth", "plymouth-x11", "plymouth-themes", "plymouth-themes-mx"};
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
            just_installed = true;
        }

        loadPlymouthThemes();
        if (ui->radioLimitedMsg->isChecked()) {
            ui->radioDetailedMsg->setChecked(true);
        }
    }

    splash_changed = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::btn_bg_file_clicked()
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
        options_changed = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::radio_detailed_msg_toggled(bool checked)
{
    if (checked) {
        messages_changed = true;
        ui->pushApply->setEnabled(true);

        QString line = ui->textKernel->text();
        if (!line.contains(QLatin1String("quiet"))) {
            line.append(line.isEmpty() ? "quiet" : " quiet");
        }

        line.remove(QRegularExpression("\\s*hush"));
        ui->textKernel->setText(line.trimmed());
    }
}

void MainWindow::radio_very_detailed_msg_toggled(bool checked)
{
    if (checked) {
        messages_changed = true;
        ui->pushApply->setEnabled(true);

        QString line = ui->textKernel->text();
        line.remove(QRegularExpression("\\s*quiet|\\s*hush"));
        ui->textKernel->setText(line.trimmed());
    }
}

void MainWindow::radio_limited_msg_toggled(bool checked)
{
    if (checked) {
        messages_changed = true;
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

void MainWindow::spinBoxTimeout_valueChanged(int /*unused*/)
{
    options_changed = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::combo_menu_entry_currentIndexChanged()
{
    options_changed = true;
    ui->pushApply->setEnabled(true);
}

// Toggled either by user or when reading the status of bootsplash
void MainWindow::combo_bootsplash_toggled(bool checked)
{
    ui->comboTheme->setEnabled(checked);
    ui->pushPreview->setEnabled(checked);

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
    kernel_options_changed = true;
}

void MainWindow::pushLog_clicked()
{
    QString location = kernel_options.contains("hush") ? "/run/rc.log" : "/var/log/boot.log";

    // Check for alternate log location if the primary one doesn't exist
    if (!QFile::exists(location)) {
        location = "/var/log/boot";
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

void MainWindow::pushUefi_clicked()
{
    auto *uefiDialog = new QDialog;
    uefiDialog->setWindowTitle(tr("Edit UEFI Boot Entries"));
    auto *layout = new QGridLayout(uefiDialog);
    auto *listEntries = new QListWidget(uefiDialog);
    auto *textIntro = new QLabel(tr("You can use the Up/Down buttons, or drag & drop items to change boot order.\n"
                                    "- Items are listed in the boot order.\n"
                                    "- Grayed out lines are inactive."),
                                 uefiDialog);

    auto createButton = [&](const QString &text, const QString &iconName) {
        auto *button = new QPushButton(text, uefiDialog);
        button->setIcon(QIcon::fromTheme(iconName));
        return button;
    };

    auto *pushActive = createButton(tr("Set ac&tive"), "star-on");
    auto *pushAddEntry = createButton(tr("&Add entry"), "list-add");
    auto *pushBootNext = createButton(tr("Boot &next"), "go-next");
    auto *pushCancel = createButton(tr("&Close"), "window-close");
    auto *pushDown = createButton(tr("Move &down"), "arrow-down");
    auto *pushRemove = createButton(tr("&Remove entry"), "trash-empty");
    auto *pushResetNext = createButton(tr("Re&set next"), "edit-undo");
    auto *pushTimeout = createButton(tr("Change &timeout"), "timer-symbolic");
    auto *pushUp = createButton(tr("Move &up"), "arrow-up");

    auto *spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto *textBootCurrent = new QLabel(uefiDialog);
    auto *textBootNext = new QLabel(tr("Boot Next: %1").arg(tr("not set, will boot using list order")), uefiDialog);
    auto *textTimeout = new QLabel(tr("Timeout: %1 seconds").arg("0"), uefiDialog);
    listEntries->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QStringList bootorder;
    connect(pushCancel, &QPushButton::clicked, uefiDialog, &QDialog::close);
    connect(pushResetNext, &QPushButton::clicked, uefiDialog, [textBootNext]() {
        if (Cmd().runAsRoot("efibootmg -N")) {
            textBootNext->setText(tr("Boot Next: %1").arg(tr("not set, will boot using list order")));
        }
    });
    connect(pushTimeout, &QPushButton::clicked, this,
            [uefiDialog, textTimeout]() { setUefiTimeout(uefiDialog, textTimeout); });
    connect(pushAddEntry, &QPushButton::clicked, this,
            [this, uefiDialog, listEntries]() { addUefiEntry(listEntries, uefiDialog); });
    connect(pushBootNext, &QPushButton::clicked, this,
            [listEntries, textBootNext]() { setUefiBootNext(listEntries, textBootNext); });
    connect(pushRemove, &QPushButton::clicked, this,
            [uefiDialog, listEntries]() { removeUefiEntry(listEntries, uefiDialog); });
    connect(pushActive, &QPushButton::clicked, uefiDialog, [listEntries]() { toggleUefiActive(listEntries); });
    connect(pushUp, &QPushButton::clicked, uefiDialog, [listEntries]() {
        listEntries->model()->moveRow(QModelIndex(), listEntries->currentRow(), QModelIndex(),
                                      listEntries->currentRow() - 1);
    });
    connect(pushDown, &QPushButton::clicked, uefiDialog, [listEntries]() {
        listEntries->model()->moveRow(QModelIndex(), listEntries->currentRow() + 1, QModelIndex(),
                                      listEntries->currentRow()); // move next entry down
    });
    connect(listEntries, &QListWidget::itemSelectionChanged, uefiDialog, [listEntries, pushUp, pushDown, pushActive]() {
        pushUp->setEnabled(listEntries->currentRow() != 0);
        pushDown->setEnabled(listEntries->currentRow() != listEntries->count() - 1);
        if (listEntries->currentItem()->text().section(' ', 0, 0).endsWith('*')) {
            pushActive->setText(tr("Set &inactive"));
            pushActive->setIcon(QIcon::fromTheme("star-off"));
        } else {
            pushActive->setText(tr("Set ac&tive"));
            pushActive->setIcon(QIcon::fromTheme("star-on"));
        }
    });

    readBootEntries(listEntries, textTimeout, textBootNext, textBootCurrent, &bootorder);
    sortUefiBootOrder(bootorder, listEntries);

    listEntries->setDragDropMode(QAbstractItemView::InternalMove);
    connect(listEntries->model(), &QAbstractItemModel::rowsMoved, this, [this, listEntries]() {
        saveBootOrder(listEntries);
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
    layout->addWidget(pushCancel, row, 0, 1, 2);
    uefiDialog->setLayout(layout);

    this->hide();
    uefiDialog->resize(this->size());
    uefiDialog->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    uefiDialog->exec();
    this->show();
}

void MainWindow::combo_theme_activated(int /*unused*/)
{
    splash_changed = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::push_preview_clicked()
{
    if (just_installed) {
        QMessageBox::warning(
            this, tr("Needs reboot"),
            tr("Plymouth was just installed, you might need to reboot before being able to display previews"));
    }

    QString current_theme = cmd.getOut("/sbin/plymouth-set-default-theme");
    if (ui->comboTheme->currentText() == "details") {
        return;
    }

    if (inVirtualMachine()) {
        QMessageBox::information(
            this, tr("Running in a Virtual Machine"),
            tr("You current system is running in a Virtual Machine,\n"
               "Plymouth bootsplash will work in a limited way, you also won't be able to preview the theme"));
    }
    cmd.runAsRoot("/sbin/plymouth-set-default-theme " + ui->comboTheme->currentText());

    QTimer tick;
    tick.start(100ms);
    connect(&tick, &QTimer::timeout, this, &MainWindow::sendMouseEvents);
    cmd.runAsRoot("plymouthd; plymouth --show-splash; for ((i=0; i<4; i++)); do plymouth --update=test$i; sleep 1; "
                  "done; plymouth quit");
    cmd.runAsRoot("plymouth-set-default-theme " + current_theme); // return to current theme
}

void MainWindow::combo_enable_flatmenus_clicked(bool checked)
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

void MainWindow::combo_save_default_clicked()
{
    options_changed = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::combo_theme_currentIndexChanged(int index)
{
    ui->pushPreview->setDisabled(ui->comboTheme->itemText(index) == QLatin1String("details"));
}

void MainWindow::combo_grub_theme_toggled(bool checked)
{
    if (checked && ui->pushThemeFile->property("file").toString().isEmpty()) {
        ui->pushThemeFile->setText(tr("Click to select theme"));
        ui->pushThemeFile->setProperty("file", "");
    } else {
        options_changed = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::btn_theme_file_clicked()
{
    QString themeDirectory = chroot.section(' ', 1, 1) + "/boot/grub/themes";
    QString selected = QFileDialog::getOpenFileName(this, tr("Select GRUB theme"), themeDirectory, "*.txt;; *.*");

    if (!selected.isEmpty()) {
        if (!chroot.isEmpty()) {
            selected.remove(chroot.section(' ', 1, 1));
        }
        ui->pushThemeFile->setText(selected);
        ui->pushThemeFile->setProperty("file", selected);
        options_changed = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::lineEdit_kernel_textEdited()
{
    kernel_options_changed = true;
    options_changed = true;
    ui->pushApply->setEnabled(true);
}

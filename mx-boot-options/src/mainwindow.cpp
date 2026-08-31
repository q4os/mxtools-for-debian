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
#include <QDate>
#include <QDir>
#include <QGuiApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QLocale>
#include <QScreen>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimer>

#include "about.h"

#include <chrono>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

using namespace std::chrono_literals;
extern const QString starting_home;

namespace
{

// Distinguishes a path that genuinely does not exist from one whose existence could not be determined
// (most commonly because a parent directory denies search/read permission). QFile::exists()/QDir::exists()
// collapse both cases to false, which is wrong for optional live-media paths: a missing path is a legitimate
// no-op, but an inaccessible one is a real failure that must not be silently swallowed.
enum class PathState { Absent, Present, Inaccessible };

[[nodiscard]] PathState pathState(const QString &path)
{
    struct stat st {};
    if (::stat(QFile::encodeName(path).constData(), &st) == 0) {
        return PathState::Present;
    }
    return (errno == ENOENT || errno == ENOTDIR) ? PathState::Absent : PathState::Inaccessible;
}

// RAII wrapper that zeroes sensitive data on scope exit
struct SecureBuffer
{
    QByteArray *data;

    explicit SecureBuffer(QByteArray *d)
        : data(d) {}

    SecureBuffer(const SecureBuffer &) = delete;
    SecureBuffer &operator=(const SecureBuffer &) = delete;

    ~SecureBuffer()
    {
        if (data && !data->isEmpty()) {
            data->fill(static_cast<char>(0));
            data->clear();
        }
    }
};

// Matches kernel command line tokens that are isolated or separated by whitespace
const QRegularExpression hushTokenRx(QStringLiteral(R"((^|\s+)hush(\s+|$))"));
const QRegularExpression quietTokenRx(QStringLiteral(R"((^|\s+)quiet(\s+|$))"));
const QRegularExpression splashTokenRx(QStringLiteral(R"((^|\s+)splash(\s+|$))"));
const QRegularExpression noSplashTokenRx(QStringLiteral(R"((^|\s+)nosplash(\s+|$))"));

[[nodiscard]] bool hasLiveGrubFiles(const QString &root)
{
    return QFile::exists(root + "/boot/grub/grub.cfg") || QFile::exists(root + "/boot/grub/config/theme.cfg")
        || QFile::exists(root + "/boot/grub/theme/theme.txt");
}

[[nodiscard]] QString releaseNameFromSystem()
{
    for (const QString &filePath : {QStringLiteral("/etc/lsb-release"), QStringLiteral("/etc/os-release")}) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        while (!file.atEnd()) {
            const QString line = QString::fromUtf8(file.readLine()).trimmed();
            const int equalsPos = line.indexOf('=');
            if (equalsPos <= 0) {
                continue;
            }

            const QString key = line.left(equalsPos);
            QString value = line.mid(equalsPos + 1).trimmed();
            if (value.startsWith('"') && value.endsWith('"') && value.size() >= 2) {
                value = value.mid(1, value.size() - 2);
            }

            if (key == QLatin1String("DISTRIB_DESCRIPTION") || key == QLatin1String("PRETTY_NAME")) {
                return value;
            }
        }
    }

    return {};
}

[[nodiscard]] QString extractMenuEntryTitle(const QString &entryLine)
{
    // The title is the menuentry/submenu's first argument: a run of adjacent quoted fragments ("...", '...',
    // or a gettext $"...") with no whitespace between them, concatenated. MX live entries such as
    // `menuentry " "$"Check integrity..."` therefore need the fragments joined, not just the first one.
    int i = entryLine.indexOf(' ');
    if (i < 0) {
        return {};
    }
    const int len = entryLine.size();
    while (i < len && entryLine[i] == ' ') {
        ++i;
    }

    QString title;
    while (i < len) {
        if (entryLine[i] == '$') {
            ++i; // gettext prefix immediately before a quote
        }
        if (i >= len || (entryLine[i] != '"' && entryLine[i] != '\'')) {
            break;
        }
        const QChar quote = entryLine[i];
        const int close = entryLine.indexOf(quote, i + 1);
        if (close < 0) {
            break;
        }
        title += entryLine.mid(i + 1, close - i - 1);
        i = close + 1;
        // Continue only if another fragment follows with no separating whitespace.
        if (i >= len || (entryLine[i] != '"' && entryLine[i] != '\'' && entryLine[i] != '$')) {
            break;
        }
    }
    return title;
}
} // namespace

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
    const QString rootPath = targetRootPath();
    QString output;
    if (rootPath.isEmpty()) {
        output = cmd.getOut(plymouthCmd + " -l");
    } else {
        output = cmd.getOutAsRootInTarget(rootPath, "plymouth-set-default-theme", {"-l"});
    }
    if (cmd.exitCode() != 0) {
        qWarning() << "Failed to get Plymouth themes list.";
        return;
    }
    if (!output.isEmpty()) {
        const QStringList themes = output.split('\n', Qt::SkipEmptyParts);
        ui->comboTheme->addItems(themes);
        const QString currentTheme = rootPath.isEmpty() ? cmd.getOut(plymouthCmd).trimmed()
                                                        : cmd.getOutAsRootInTarget(rootPath, "plymouth-set-default-theme").trimmed();
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
    handleLiveSystem();
    setupUiElements();
    setupGrubSettings();
    handleSpecialFilesystems();

    // Populating widgets above fires the same change signals used to detect user edits (e.g. setChecked,
    // setCurrentIndex), which would otherwise leave these flags spuriously set before the user touches anything.
    kernelOptionsChanged = false;
    messagesChanged = false;
    optionsChanged = false;
    splashChanged = false;

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
    } else if (waylandSession) {
        ui->pushPreview->setDisabled(true);
        ui->pushPreview->setToolTip(tr("Preview is disabled while running under Wayland. Please start an X11 session "
                                       "to preview Plymouth themes."));
    } else {
        ui->pushPreview->setToolTip("");
    }

    // Configure GRUB theme related UI elements
    const QString grubRoot = live && !installedMode ? bootLocation : targetRootPath();
    const bool grubThemesExist
        = QFile::exists(grubRoot + "/boot/grub/theme") || QFile::exists(grubRoot + "/boot/grub/themes");
    ui->checkGrubTheme->setVisible(grubThemesExist);
    ui->pushThemeFile->setVisible(grubThemesExist);
    ui->pushThemeFile->setDisabled(true);
    ui->checkBackground->setChecked(false);
    ui->pushBgFile->setDisabled(true);

    if (liveGrubMode()) {
        // Live media uses a platform-selected set of theme files (not a single pickable file), so the theme
        // picker does not apply -- only the background image plus the timeout and default entry can be changed.
        ui->checkGrubTheme->setVisible(false);
        ui->pushThemeFile->setVisible(false);
        // The live theme files always reference a background image, so it can be replaced but not disabled.
        ui->checkBackground->setChecked(true);
        ui->checkBackground->setEnabled(false);
        ui->pushBgFile->setEnabled(true);
        // save-default (grubenv) and flat-menus (an update-grub generation option) cannot apply to the
        // pre-generated live grub.cfg, so disable them.
        ui->checkSaveDefault->setEnabled(false);
        ui->checkEnableFlatmenus->setEnabled(false);
        // The live default entry is forced by MX's set_default_entry boot logic (sourced after grub.cfg's
        // own `set default=`), so a static edit here would not take effect -- show it read-only.
        ui->comboMenuEntry->setEnabled(false);
        ui->comboMenuEntry->setToolTip(
            tr("The live boot menu's default entry is controlled by the MX boot scripts and cannot be changed here."));
    }
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
        bootLocation = resolveLiveBootLocation();
        if (bootLocation.isEmpty()) {
            if (QFile::exists("/run/archiso/bootmnt")) {
                bootLocation = "/run/archiso/bootmnt";
            } else if (QFileInfo::exists("/live/config/did-toram")) {
                bootLocation = "/live/to-ram";
            } else {
                bootLocation = "/live/boot-dev";
            }
        }
    }
}

void MainWindow::setupGrubSettings()
{
    const QString grubPackage = grubPackageName();
    grubInstalled = (!grubPackage.isEmpty() && isInstalled(grubPackage)) || liveGrubMode();
    ui->groupBoxOptions->setHidden(!grubInstalled);
    ui->groupBoxBackground->setHidden(!grubInstalled);

    if (grubInstalled) {
        reloadGrubSettings();
    }
}

void MainWindow::reloadGrubSettings()
{
    defaultGrub.clear();
    grubCfg.clear();
    readGrubCfg();
    if (liveGrubMode()) {
        // The host /etc/default/grub does not describe the live media, so it is not read into defaultGrub.
        // Still initialize the kernel-options UI (text field, splash, verbosity) from the running kernel
        // command line -- processKernelCommandLine() substitutes kernelOptions when in live mode.
        processKernelCommandLine({});
    } else {
        readDefaultGrub();
    }
}

void MainWindow::handleSpecialFilesystems()
{
    const QString dfOut = [&] {
        QString out;
        cmd.proc("df", {"--output=fstype", chroot.isEmpty() ? "/boot" : tempDir.path()}, &out);
        return out;
    }();
    if (cmd.exitCode() != 0 || dfOut.isEmpty()) {
        qWarning() << "Failed to get filesystem type.";
        return;
    }
    const QStringList dfLines = dfOut.split('\n', Qt::SkipEmptyParts);
    const QString fstype = dfLines.isEmpty() ? QString() : dfLines.last().trimmed();
    if (fstype == "btrfs") {
        ui->checkSaveDefault->setChecked(false);
        ui->checkSaveDefault->setDisabled(true);
    }
}

bool MainWindow::hasLiveGrubTree() const
{
    // Require grub.cfg specifically: it is what readGrubCfg() reads and what the live-mode handling rewrites.
    // Theme files alone (used as a looser signal when locating the boot media) do not make a usable tree.
    return !bootLocation.isEmpty() && pathState(bootLocation + "/boot/grub/grub.cfg") == PathState::Present;
}

bool MainWindow::liveGrubMode() const
{
    return live && !installedMode && hasLiveGrubTree();
}

QString MainWindow::resolveLiveBootLocation()
{
    const auto validateCandidate = [](const QString &root) { return QFileInfo(root).isDir() && hasLiveGrubFiles(root); };

    for (const QString &candidate : {QStringLiteral("/run/archiso/bootmnt"), QStringLiteral("/live/boot-dev"),
                                     QStringLiteral("/live/to-ram")}) {
        if (validateCandidate(candidate)) {
            return candidate;
        }
    }

    QFile initrdFile(QStringLiteral("/live/config/initrd.out"));
    if (!initrdFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QString biosDev;
    QString biosMp;
    QString biosUuid;
    QString bootDev;
    QString bootMp;
    QString bootUuid;

    while (!initrdFile.atEnd()) {
        const QString line = QString::fromUtf8(initrdFile.readLine()).trimmed();
        const int equalsPos = line.indexOf('=');
        if (equalsPos <= 0) {
            continue;
        }

        const QString key = line.left(equalsPos);
        QString value = line.mid(equalsPos + 1).trimmed();
        if (value.startsWith('"') && value.endsWith('"') && value.size() >= 2) {
            value = value.mid(1, value.size() - 2);
        }

        if (key == QLatin1String("BIOS_DEV")) {
            biosDev = value;
        } else if (key == QLatin1String("BIOS_MP")) {
            biosMp = value;
        } else if (key == QLatin1String("BIOS_UUID")) {
            biosUuid = value;
        } else if (key == QLatin1String("BOOT_DEV")) {
            bootDev = value;
        } else if (key == QLatin1String("BOOT_MP")) {
            bootMp = value;
        } else if (key == QLatin1String("BOOT_UUID")) {
            bootUuid = value;
        }
    }

    const auto findMountPoint = [&](const QString &device) {
        QString output;
        if (!cmd.proc("lsblk", {"-no", "MOUNTPOINT", device}, &output, nullptr, QuietMode::Yes)) {
            return QString();
        }

        const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        return lines.isEmpty() ? QString() : lines.first().trimmed();
    };

    const auto mountDevice = [&](const QString &device, const QString &mountPoint) {
        if (device.isEmpty() || mountPoint.isEmpty()) {
            return false;
        }

        if (!QDir(mountPoint).exists()) {
            cmd.procAsRoot("mkdir", {"-p", mountPoint}, nullptr, nullptr, QuietMode::Yes);
        }

        QString fstype;
        cmd.proc("blkid", {"-o", "value", "-s", "TYPE", device}, &fstype, nullptr, QuietMode::Yes);
        if (!fstype.trimmed().isEmpty()) {
            if (cmd.procAsRoot("mount", {"-t", fstype.trimmed(), device, mountPoint}, nullptr, nullptr, QuietMode::Yes)) {
                return true;
            }
        }
        return cmd.procAsRoot("mount", {device, mountPoint}, nullptr, nullptr, QuietMode::Yes);
    };

    const auto deviceFromUuid = [&](const QString &uuid) {
        if (uuid.isEmpty()) {
            return QString();
        }
        const QString uuidPath = QStringLiteral("/dev/disk/by-uuid/") + uuid;
        return QFileInfo::exists(uuidPath) ? QFileInfo(uuidPath).canonicalFilePath() : QString();
    };

    const auto tryCandidate = [&](const QString &deviceValue, const QString &mountPointValue, const QString &uuidValue,
                                  const QString &fallbackMountPoint) {
        QString mountPoint = mountPointValue.isEmpty() ? fallbackMountPoint : mountPointValue;
        if (validateCandidate(mountPoint)) {
            return mountPoint;
        }

        QString device = deviceValue;
        if (device.isEmpty()) {
            device = deviceFromUuid(uuidValue);
        }
        if (device.isEmpty()) {
            return QString();
        }

        const QString currentMountPoint = findMountPoint(device);
        if (!currentMountPoint.isEmpty() && validateCandidate(currentMountPoint)) {
            return currentMountPoint;
        }

        if (mountDevice(device, mountPoint) && validateCandidate(mountPoint)) {
            return mountPoint;
        }

        return QString();
    };

    if (const QString candidate = tryCandidate(biosDev, biosMp, biosUuid, QStringLiteral("/live/bios-dev"));
        !candidate.isEmpty()) {
        return candidate;
    }
    if (const QString candidate = tryCandidate(bootDev, bootMp, bootUuid, QStringLiteral("/live/boot-dev"));
        !candidate.isEmpty()) {
        return candidate;
    }

    return {};
}

bool MainWindow::refreshLiveGrubTheme()
{
    if (bootLocation.isEmpty()) {
        return true;
    }

    switch (pathState(bootLocation + "/boot/grub/grub.cfg")) {
    case PathState::Absent:
        // No live GRUB tree on this media (e.g. syslinux-only media); nothing to refresh, which is a
        // legitimate no-op rather than a failure.
        return true;
    case PathState::Inaccessible:
        // grub.cfg (or one of its parent directories) exists but could not be checked, so we cannot tell
        // whether a live GRUB tree is actually present; treat this as a real failure rather than a no-op so
        // the pending change is retained and the user can retry.
        qWarning() << "Could not access the live GRUB tree at" << bootLocation << "; skipping label refresh.";
        return false;
    case PathState::Present:
        break;
    }

    const QString releaseName = releaseNameFromSystem();
    if (releaseName.isEmpty()) {
        qWarning() << "Could not determine the release name for live boot label refresh.";
        return false;
    }

    const QString dateText = QLocale::system().toString(QDate::currentDate(), "MMMM d, yyyy");
    const QString labelText = releaseName + " (" + dateText + ")";

    // Tracks genuine write failures across every file touched below so that one file's successful rewrite
    // cannot mask another's failure.
    bool anyWriteFailed = false;
    const auto writeLinesAsRoot = [this, &anyWriteFailed](const QString &path, const QStringList &lines) {
        const bool ok = writeFileLinesAsRoot(path, lines);
        if (!ok) {
            anyWriteFailed = true;
        }
        return ok;
    };

    // Returns true if the caller should stop and treat the path as having nothing to do: either it is
    // genuinely absent (a legitimate no-op on media that lacks this optional file) or it could not be
    // checked at all, in which case anyWriteFailed is set so the inaccessible path is reported as a failure
    // instead of silently being treated the same as a genuinely absent one.
    const auto optionalPathMissing = [&](const QString &path) {
        switch (pathState(path)) {
        case PathState::Absent:
            return true;
        case PathState::Inaccessible:
            qWarning() << "Failed to access" << path << "; it may exist but be unreadable.";
            anyWriteFailed = true;
            return true;
        case PathState::Present:
            return false;
        }
        return false;
    };

    const auto updateMenuTitle = [&](const QString &path) {
        if (optionalPathMissing(path)) {
            return true;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << path << "for reading.";
            anyWriteFailed = true;
            return false;
        }

        QStringList lines;
        bool changed = false;
        while (!file.atEnd()) {
            QString line = QString::fromUtf8(file.readLine());
            if (line.endsWith('\n')) {
                line.chop(1);
            }
            if (line.trimmed().startsWith("MENU TITLE", Qt::CaseInsensitive)) {
                line = QStringLiteral("MENU TITLE Welcome to ") + releaseName;
                changed = true;
            }
            lines << line;
        }

        return changed && writeLinesAsRoot(path, lines);
    };

    const auto updateThemeBanner = [&](const QString &path) {
        if (optionalPathMissing(path)) {
            return true;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << path << "for reading.";
            anyWriteFailed = true;
            return false;
        }

        QStringList lines;
        lines.reserve(64);
        bool changed = false;
        while (!file.atEnd()) {
            QString line = QString::fromUtf8(file.readLine());
            if (line.endsWith('\n')) {
                line.chop(1);
            }
            const int welcomePos = line.indexOf("Welcome to ");
            const int bangPos = welcomePos >= 0 ? line.indexOf('!', welcomePos) : -1;
            if (welcomePos >= 0 && bangPos > welcomePos) {
                line = line.left(welcomePos) + "Welcome to " + releaseName + line.mid(bangPos);
                changed = true;
            }
            lines << line;
        }

        return changed && writeLinesAsRoot(path, lines);
    };

    const auto updateMenuLabel = [&](const QString &path) {
        if (optionalPathMissing(path)) {
            return true;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << path << "for reading.";
            anyWriteFailed = true;
            return false;
        }

        const QRegularExpression labelRx(
            QStringLiteral(R"(^([ \t]*MENU[ \t]+LABEL[ \t]+).* \([A-Z][a-z]+ \d{1,2}, \d{4}\)([ \t]*\([^)]*\))?[ \t]*$)"),
            QRegularExpression::CaseInsensitiveOption);

        QStringList lines;
        lines.reserve(128);
        bool changed = false;
        while (!file.atEnd()) {
            QString line = QString::fromUtf8(file.readLine());
            if (line.endsWith('\n')) {
                line.chop(1);
            }
            const QRegularExpressionMatch match = labelRx.match(line);
            if (match.hasMatch()) {
                line = match.captured(1) + labelText + match.captured(2);
                changed = true;
            }
            lines << line;
        }

        return changed && writeLinesAsRoot(path, lines);
    };

    const auto updateReadmeMsg = [&](const QString &path) {
        if (optionalPathMissing(path)) {
            return true;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << path << "for reading.";
            anyWriteFailed = true;
            return false;
        }

        const QRegularExpression readmeRx(
            QStringLiteral(R"(^([ \t]+\S+[ \t]+).* \([A-Z][a-z]+ \d{1,2}, \d{4}\)([ \t]*\([^)]*\))?[ \t]*$)"));

        QStringList lines;
        lines.reserve(128);
        bool changed = false;
        while (!file.atEnd()) {
            QString line = QString::fromUtf8(file.readLine());
            if (line.endsWith('\n')) {
                line.chop(1);
            }
            const QRegularExpressionMatch match = readmeRx.match(line);
            if (match.hasMatch()) {
                line = match.captured(1) + labelText + match.captured(2);
                changed = true;
            }
            lines << line;
        }

        return changed && writeLinesAsRoot(path, lines);
    };

    const auto updateMenuEntryFile = [&](const QString &path) {
        if (optionalPathMissing(path)) {
            return true;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << path << "for reading.";
            anyWriteFailed = true;
            return false;
        }

        const QRegularExpression entryRx(
            QStringLiteral(R"(^(\s*menuentry\s+\")([^\"]*?)\s+\([A-Z][a-z]+ \d{1,2}, \d{4}\)(\s+\([^)]*\))?(\".*)$)"));

        QStringList lines;
        lines.reserve(128);
        bool changed = false;
        while (!file.atEnd()) {
            QString line = QString::fromUtf8(file.readLine());
            if (line.endsWith('\n')) {
                line.chop(1);
            }
            const QRegularExpressionMatch match = entryRx.match(line);
            if (match.hasMatch()) {
                line = match.captured(1) + labelText + match.captured(3) + match.captured(4);
                changed = true;
            }
            lines << line;
        }

        return changed && writeLinesAsRoot(path, lines);
    };

    for (const QString &file : {bootLocation + "/boot/isolinux/isolinux.cfg", bootLocation + "/boot/syslinux/syslinux.cfg"}) {
        updateMenuTitle(file);
        updateMenuLabel(file);
    }

    for (const QString &file : {bootLocation + "/boot/isolinux/readme.msg", bootLocation + "/boot/syslinux/readme.msg"}) {
        updateReadmeMsg(file);
    }

    const QString themeDir = bootLocation + "/boot/grub/theme";
    switch (pathState(themeDir)) {
    case PathState::Absent:
        break;
    case PathState::Inaccessible:
        qWarning() << "Failed to access" << themeDir << "; it may exist but be unreadable.";
        anyWriteFailed = true;
        break;
    case PathState::Present: {
        const QDir dir(themeDir);
        if (!dir.isReadable()) {
            // The directory exists but its contents could not be enumerated (e.g. a permissions problem),
            // which is a genuine failure rather than the directory legitimately having nothing in it.
            qWarning() << "Failed to enumerate" << themeDir << "; it may exist but be unreadable.";
            anyWriteFailed = true;
            break;
        }
        const QStringList themeFiles = dir.entryList({QStringLiteral("*.txt")}, QDir::Files, QDir::Name);
        for (const QString &themeFile : themeFiles) {
            updateThemeBanner(themeDir + "/" + themeFile);
        }
        break;
    }
    }

    const QString grubCfgPath = bootLocation + "/boot/grub/grub.cfg";
    updateMenuEntryFile(grubCfgPath);

    const QString configDir = bootLocation + "/boot/grub/config";
    switch (pathState(configDir)) {
    case PathState::Absent:
        break;
    case PathState::Inaccessible:
        qWarning() << "Failed to access" << configDir << "; it may exist but be unreadable.";
        anyWriteFailed = true;
        break;
    case PathState::Present: {
        const QDir dir(configDir);
        if (!dir.isReadable()) {
            // See the theme directory case above: an unreadable directory is a genuine failure, not an
            // empty-but-valid one.
            qWarning() << "Failed to enumerate" << configDir << "; it may exist but be unreadable.";
            anyWriteFailed = true;
            break;
        }
        const QStringList configFiles = dir.entryList({QStringLiteral("*.cfg")}, QDir::Files, QDir::Name);
        for (const QString &configFile : configFiles) {
            updateMenuEntryFile(configDir + "/" + configFile);
        }
        break;
    }
    }

    // A successful no-op (nothing needed changing) and a successful update both return true; only a genuine
    // write failure along the way returns false.
    return !anyWriteFailed;
}

bool MainWindow::writeFileLinesAsRoot(const QString &path, const QStringList &lines)
{
    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        qWarning() << "Failed to create temporary file for" << path;
        return false;
    }

    QTextStream stream(&tmpFile);
    for (const QString &line : lines) {
        stream << line << '\n';
    }
    stream.flush();
    const bool streamOk = stream.status() == QTextStream::Ok;
    const bool flushOk = tmpFile.flush();
    tmpFile.close();
    if (!streamOk || !flushOk) {
        qWarning() << "Failed to write temporary file for" << path;
        return false;
    }

    if (!cmd.procAsRoot("cp", {tmpFile.fileName(), path}, nullptr, nullptr, QuietMode::Yes)) {
        qWarning() << "Failed to update" << path;
        return false;
    }
    bool ok = true;
    if (!cmd.procAsRoot("chown", {"root:", path}, nullptr, nullptr, QuietMode::Yes)) {
        qWarning() << "Failed to set ownership of" << path;
        ok = false;
    }
    if (!cmd.procAsRoot("chmod", {"644", path}, nullptr, nullptr, QuietMode::Yes)) {
        qWarning() << "Failed to set permissions on" << path;
        ok = false;
    }
    return ok;
}

bool MainWindow::rewriteFileAsRoot(const QString &path, const std::function<bool(QString &)> &transform,
                                   bool *writeFailed)
{
    // A missing file is a legitimate no-op (an optional live config not present on this media), but a path
    // that exists yet can't be checked (e.g. a permissions problem) is a genuine failure callers need to
    // know about.
    switch (pathState(path)) {
    case PathState::Absent:
        return false;
    case PathState::Inaccessible:
        if (writeFailed) {
            *writeFailed = true;
        }
        return false;
    case PathState::Present:
        break;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (writeFailed) {
            *writeFailed = true;
        }
        return false;
    }

    QStringList lines;
    bool changed = false;
    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine());
        if (line.endsWith('\n')) {
            line.chop(1);
        }
        if (transform(line)) {
            changed = true;
        }
        lines << line;
    }
    file.close();

    if (!changed) {
        return false;
    }

    // The file existed and had matching content to rewrite, so a false return here means the write itself
    // failed (as opposed to there being nothing to change), which callers need to distinguish to avoid
    // treating a harmless no-op as a reportable failure.
    const bool ok = writeFileLinesAsRoot(path, lines);
    if (!ok && writeFailed) {
        *writeFailed = true;
    }
    return ok;
}

bool MainWindow::applyLiveGrubTimeout(int seconds)
{
    // Tracks only genuine write failures; a config file being absent or already at the target value is a
    // legitimate no-op and must not be reported as an apply failure.
    bool writeFailed = false;

    // GRUB: the top-level `set timeout=` line in grub.cfg.
    const QRegularExpression grubTimeoutRx(QStringLiteral(R"(^(\s*set\s+timeout=).*$)"));
    rewriteFileAsRoot(
        bootLocation + "/boot/grub/grub.cfg",
        [&](QString &line) {
            const QRegularExpressionMatch match = grubTimeoutRx.match(line);
            if (!match.hasMatch()) {
                return false;
            }
            const QString replacement = match.captured(1) + QString::number(seconds);
            if (replacement == line) {
                return false;
            }
            line = replacement;
            return true;
        },
        &writeFailed);

    // syslinux/isolinux: the top-level `timeout` directive, expressed in tenths of a second.
    const QRegularExpression syslinuxTimeoutRx(QStringLiteral(R"(^(\s*timeout\s+)\d+\s*$)"),
                                               QRegularExpression::CaseInsensitiveOption);
    const QString deciSeconds = QString::number(seconds * 10);
    for (const QString &cfg :
         {bootLocation + "/boot/syslinux/syslinux.cfg", bootLocation + "/boot/isolinux/isolinux.cfg"}) {
        rewriteFileAsRoot(
            cfg,
            [&](QString &line) {
                const QRegularExpressionMatch match = syslinuxTimeoutRx.match(line);
                if (!match.hasMatch()) {
                    return false;
                }
                const QString replacement = match.captured(1) + deciSeconds;
                if (replacement == line) {
                    return false;
                }
                line = replacement;
                return true;
            },
            &writeFailed);
    }
    return !writeFailed;
}

bool MainWindow::applyLiveGrubBackground(const QString &imagePath)
{
    const QString themeDir = bootLocation + "/boot/grub/theme";
    if (imagePath.isEmpty() || !QFile::exists(imagePath)) {
        // No image to apply; nothing to do.
        return true;
    }
    switch (pathState(themeDir)) {
    case PathState::Absent:
        // No live GRUB theme present on this media (e.g. syslinux-only media); nothing to apply.
        return true;
    case PathState::Inaccessible:
        qWarning() << "Failed to access" << themeDir << "; skipping live boot background image update.";
        return false;
    case PathState::Present:
        break;
    }

    // GRUB picks the image decoder by file extension, so keep the original suffix when copying it in.
    const QString suffix = QFileInfo(imagePath).suffix().toLower();
    const QString destName = suffix.isEmpty() ? QStringLiteral("background") : "background." + suffix;
    const QString destPath = themeDir + "/" + destName;

    if (!cmd.procAsRoot("cp", {imagePath, destPath}, nullptr, nullptr, QuietMode::Yes)) {
        qWarning() << "Failed to copy background image to" << destPath;
        return false;
    }
    bool ok = true;
    if (!cmd.procAsRoot("chown", {"root:", destPath}, nullptr, nullptr, QuietMode::Yes)) {
        qWarning() << "Failed to set ownership of" << destPath;
        ok = false;
    }
    if (!cmd.procAsRoot("chmod", {"644", destPath}, nullptr, nullptr, QuietMode::Yes)) {
        qWarning() << "Failed to set permissions on" << destPath;
        ok = false;
    }

    // Point the themed menu at the new image via the `desktop-image:` directive in each theme file, also
    // uncommenting it if present as `#desktop-image:`. (The non-theme gfx_background fallback, only seen when
    // the theme is disabled, keeps the original image.)
    const QRegularExpression desktopImageRx(QStringLiteral(R"(^(\s*)#?\s*desktop-image:\s*.*$)"));
    bool referenced = false;
    bool writeFailed = false;
    const QStringList themeFiles = QDir(themeDir).entryList({QStringLiteral("*.txt")}, QDir::Files, QDir::Name);
    for (const QString &themeFile : themeFiles) {
        rewriteFileAsRoot(
            themeDir + "/" + themeFile,
            [&](QString &line) {
                const QRegularExpressionMatch match = desktopImageRx.match(line);
                if (!match.hasMatch()) {
                    return false;
                }
                referenced = true;
                const QString replacement = match.captured(1) + "desktop-image: \"" + destName + '"';
                if (replacement == line) {
                    return false;
                }
                line = replacement;
                return true;
            },
            &writeFailed);
    }
    if (!referenced) {
        qWarning() << "No desktop-image directive found in the live GRUB theme; background image was copied but"
                   << "is not referenced.";
    }
    return ok && !writeFailed && referenced;
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
    connect(ui->checkBackground, &QCheckBox::clicked, this, &MainWindow::checkBackgroundToggled);
    connect(ui->checkBackground, &QCheckBox::clicked, ui->pushBgFile, &QPushButton::setEnabled);
    connect(ui->checkBootsplash, &QCheckBox::clicked, this, &MainWindow::comboBootsplashClicked);
    connect(ui->checkBootsplash, &QCheckBox::toggled, this, &MainWindow::comboBootsplashToggled);
    connect(ui->checkBootsplash, &QCheckBox::toggled, ui->comboTheme, &QComboBox::setEnabled);
    connect(ui->checkEnableFlatmenus, &QCheckBox::clicked, this, &MainWindow::comboEnableFlatmenusClicked);
    connect(ui->checkGrubTheme, &QCheckBox::clicked, this, &MainWindow::comboGrubThemeToggled);
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
    const PackageManager manager = detectPackageManager();
    const QString rootPath = targetRootPath();

    switch (manager) {
    case PackageManager::Pacman:
        return rootPath.isEmpty() ? QProcess::execute("pacman", {"-Q", package}) == 0
                                  : cmd.isPackageInstalledAsRoot("pacman", package, rootPath, QuietMode::Yes);
    case PackageManager::Apt: {
        if (!rootPath.isEmpty()) {
            return cmd.isPackageInstalledAsRoot("apt", package, rootPath, QuietMode::Yes);
        }

        QProcess proc;
        proc.start("dpkg-query", {"--show", "--showformat=${db:Status-Abbrev}", package});
        proc.waitForFinished();
        return proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0
            && QString::fromUtf8(proc.readAllStandardOutput()).trimmed().startsWith("ii");
    }
    default:
        return false;
    }
}

// Checks if a list of packages is installed, return false if one of them is not
bool MainWindow::isInstalled(const QStringList &packages)
{
    return std::all_of(packages.begin(), packages.end(), [&](const QString &package) { return isInstalled(package); });
}

// Check if running from a live environment
bool MainWindow::isLive()
{
    return QDir("/live/aufs").exists()
           || QFile::exists("/run/archiso/bootmnt");
}

bool MainWindow::isUefi()
{
    QDir dir("/sys/firmware/efi/efivars");
    return dir.exists() && !dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
}

bool MainWindow::isWaylandSession()
{
    const QString platformName = QGuiApplication::platformName();
    if (platformName.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    }

    const QString sessionType = QString::fromLocal8Bit(qgetenv("XDG_SESSION_TYPE"));
    if (sessionType.compare(QLatin1String("wayland"), Qt::CaseInsensitive) == 0) {
        return true;
    }

    return !qgetenv("WAYLAND_DISPLAY").isEmpty();
}

QString MainWindow::targetRootPath() const
{
    return chroot.isEmpty() ? QString() : chroot.section(' ', 1, 1);
}

MainWindow::PackageManager MainWindow::detectPackageManager() const
{
    const QString rootPath = targetRootPath();
    if (QFile::exists(rootPath + "/usr/bin/pacman")) {
        return PackageManager::Pacman;
    }
    if (QFile::exists(rootPath + "/usr/bin/apt-get")) {
        return PackageManager::Apt;
    }
    return PackageManager::Unknown;
}

QString MainWindow::grubPackageName() const
{
    switch (detectPackageManager()) {
    case PackageManager::Pacman:
        return QStringLiteral("grub");
    case PackageManager::Apt:
        return QStringLiteral("grub-common");
    default:
        return {};
    }
}

QStringList MainWindow::requiredPlymouthPackages() const
{
    if (detectPackageManager() == PackageManager::Pacman) {
        return {QStringLiteral("plymouth")};
    }
    return {QStringLiteral("plymouth"), QStringLiteral("plymouth-x11"), QStringLiteral("plymouth-themes"),
            QStringLiteral("plymouth-themes-mx")};
}

bool MainWindow::runPackageUpdate()
{
    const QString rootPath = targetRootPath();
    switch (detectPackageManager()) {
    case PackageManager::Pacman:
        return rootPath.isEmpty() ? cmd.procAsRoot("pacman", {"-Sy", "--noconfirm"}, nullptr, nullptr,
                                                   QuietMode::No, Cmd::NoTimeoutMs)
                                  : cmd.procAsRootInTarget(rootPath, "pacman", {"-Sy", "--noconfirm"}, nullptr,
                                                            nullptr, QuietMode::No, Cmd::NoTimeoutMs);
    case PackageManager::Apt:
        return rootPath.isEmpty() ? cmd.procAsRoot("apt-get", {"update"}, nullptr, nullptr, QuietMode::No,
                                                   Cmd::NoTimeoutMs)
                                  : cmd.procAsRootInTarget(rootPath, "apt-get", {"update"}, nullptr, nullptr,
                                                            QuietMode::No, Cmd::NoTimeoutMs);
    default:
        qWarning() << "No supported package manager found for update.";
        return false;
    }
}

bool MainWindow::installPackages(const QStringList &packages)
{
    if (packages.isEmpty()) {
        return true;
    }

    const QString rootPath = targetRootPath();
    QStringList packageArgs;
    switch (detectPackageManager()) {
    case PackageManager::Pacman:
        packageArgs = {"-S", "--noconfirm", "--needed"};
        packageArgs += packages;
        return rootPath.isEmpty() ? cmd.procAsRoot("pacman", packageArgs, nullptr, nullptr, QuietMode::No,
                                                   Cmd::NoTimeoutMs)
                                  : cmd.procAsRootInTarget(rootPath, "pacman", packageArgs, nullptr, nullptr,
                                                            QuietMode::No, Cmd::NoTimeoutMs);
    case PackageManager::Apt:
        packageArgs = {"install", "-y",
                       "-o", "Dpkg::Options::=--force-confdef",
                       "-o", "Dpkg::Options::=--force-confold"};
        packageArgs += packages;
        return rootPath.isEmpty() ? cmd.procAsRoot("apt-get", packageArgs, nullptr, nullptr, QuietMode::No,
                                                   Cmd::NoTimeoutMs)
                                  : cmd.procAsRootInTarget(rootPath, "apt-get", packageArgs, nullptr, nullptr,
                                                            QuietMode::No, Cmd::NoTimeoutMs);
    default:
        qWarning() << "No supported package manager found for install.";
        return false;
    }
}

bool MainWindow::runUpdateGrub()
{
    const QString rootPath = targetRootPath();
    if (detectPackageManager() == PackageManager::Pacman) {
        return rootPath.isEmpty() ? cmd.procAsRoot("grub-mkconfig", {"-o", "/boot/grub/grub.cfg"})
                                  : cmd.procAsRootInTarget(rootPath, "grub-mkconfig", {"-o", "/boot/grub/grub.cfg"});
    }

    if (QFile::exists(rootPath + "/usr/bin/update-grub") || QFile::exists(rootPath + "/sbin/update-grub")) {
        return rootPath.isEmpty() ? cmd.procAsRoot("update-grub") : cmd.procAsRootInTarget(rootPath, "update-grub");
    }

    if (QFile::exists(rootPath + "/usr/bin/grub-mkconfig")) {
        return rootPath.isEmpty() ? cmd.procAsRoot("grub-mkconfig", {"-o", "/boot/grub/grub.cfg"})
                                  : cmd.procAsRootInTarget(rootPath, "grub-mkconfig", {"-o", "/boot/grub/grub.cfg"});
    }

    qWarning() << "No GRUB update command found.";
    return false;
}

bool MainWindow::runUpdateInitramfs()
{
    const QString rootPath = targetRootPath();
    if (detectPackageManager() == PackageManager::Pacman) {
        return rootPath.isEmpty() ? cmd.procAsRoot("mkinitcpio", {"-P"}) : cmd.procAsRootInTarget(rootPath, "mkinitcpio", {"-P"});
    }

    if (QFile::exists(rootPath + "/usr/bin/update-initramfs") || QFile::exists(rootPath + "/sbin/update-initramfs")) {
        return rootPath.isEmpty() ? cmd.procAsRoot("update-initramfs", {"-u", "-k", "all"})
                                  : cmd.procAsRootInTarget(rootPath, "update-initramfs", {"-u", "-k", "all"});
    }

    if (QFile::exists(rootPath + "/usr/bin/mkinitcpio")) {
        return rootPath.isEmpty() ? cmd.procAsRoot("mkinitcpio", {"-P"}) : cmd.procAsRootInTarget(rootPath, "mkinitcpio", {"-P"});
    }

    qWarning() << "No initramfs update command found.";
    return false;
}

bool MainWindow::toggleBootlogd(bool enable)
{
    const QString rootPath = targetRootPath();
    if (detectPackageManager() == PackageManager::Pacman) {
        return true;
    }

    // update-rc.d ships with init-system-helpers on every Debian system, but it fails when the service it is
    // asked to toggle has no init script, so only attempt the toggle when bootlogd is actually installed.
    switch (pathState(rootPath + "/etc/init.d/bootlogd")) {
    case PathState::Absent:
        qWarning() << "bootlogd is not installed; skipping bootlogd toggle.";
        return true;
    case PathState::Inaccessible:
        qWarning() << "Could not access the bootlogd init script; unable to determine its service state.";
        return false;
    case PathState::Present:
        break;
    }

    if (QFile::exists(rootPath + "/usr/sbin/update-rc.d") || QFile::exists(rootPath + "/sbin/update-rc.d")
        || QFile::exists(rootPath + "/usr/bin/update-rc.d")) {
        const QString action = enable ? QStringLiteral("enable") : QStringLiteral("disable");
        const bool ok = rootPath.isEmpty() ? cmd.procAsRoot("update-rc.d", {"bootlogd", action})
                                            : cmd.procAsRootInTarget(rootPath, "update-rc.d", {"bootlogd", action});
        if (!ok) {
            qWarning() << "Failed to toggle bootlogd via update-rc.d.";
        }
        return ok;
    }

    qWarning() << "update-rc.d not found; skipping bootlogd toggle.";
    return true;
}

bool MainWindow::isSystemdEnvironment() const
{
    if (chroot.isEmpty()) {
        return QFile::exists(QStringLiteral("/run/systemd/system"));
    }

    const QString rootPath = tempDir.path();

    const auto hasSystemdBinary = [&](const QString &relativePath) {
        return QFile::exists(rootPath + relativePath);
    };

    QFileInfo initFile(rootPath + QStringLiteral("/sbin/init"));
    if (initFile.exists()) {
        const QString target = initFile.isSymLink() ? initFile.symLinkTarget() : initFile.canonicalFilePath();
        if (target.contains(QLatin1String("systemd"), Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
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
    cmd.clearCancelRequest();
    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButtonText(tr("Cancel"));
    connect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
    progress->setWindowTitle(tr("Installing bootsplash, please wait"));
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->resize(500, progress->height());
    progress->show();

    setConnections();
    progress->setLabelText(tr("Updating sources"));

    if (!runPackageUpdate()) {
        disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
        progress->close();
        bar = nullptr;
        progress->deleteLater();
        QMessageBox::critical(this, tr("Error"), tr("Failed to update package sources."));
        return;
    }

    const QStringList packages = requiredPlymouthPackages();
    progress->setLabelText(tr("Installing packages:") + " " + packages.join(", "));

    if (!installPackages(packages)) {
        disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
        progress->close();
        bar = nullptr;
        progress->deleteLater();
        QMessageBox::critical(this, tr("Error"), tr("Could not install the bootsplash."));
        ui->checkBootsplash->setChecked(false);
        return;
    }

    disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
    progress->close();
    bar = nullptr;
    progress->deleteLater();
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
bool MainWindow::writeDefaultGrub()
{
    const QString rootPath = targetRootPath();
    const QString grubFilePath = rootPath + "/etc/default/grub";
    const QString backupFilePath = grubFilePath + ".bak";

    if (!QFile::exists(grubFilePath)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot find %1 to update.").arg(grubFilePath));
        return false;
    }

    // Rotate the previous backup out of the way, if one exists. Both steps must succeed before the existing
    // backup is removed, so a failed rotation never destroys the only prior backup.
    if (QFile::exists(backupFilePath)) {
        if (!cmd.procAsRoot("cp", {backupFilePath, backupFilePath + ".0"})) {
            QMessageBox::critical(
                this, tr("Error"),
                tr("Failed to rotate backup %1; leaving the current configuration in place.").arg(backupFilePath));
            return false;
        }
        if (!cmd.procAsRoot("rm", {backupFilePath})) {
            QMessageBox::critical(
                this, tr("Error"),
                tr("Failed to remove old backup %1; leaving the current configuration in place.").arg(backupFilePath));
            return false;
        }
    }

    // The fresh backup below must succeed before the live file is touched.
    if (!cmd.procAsRoot("cp", {grubFilePath, backupFilePath})
        || !cmd.procAsRoot("chown", {"root:", backupFilePath}, nullptr, nullptr, QuietMode::Yes)
        || !cmd.procAsRoot("chmod", {"644", backupFilePath}, nullptr, nullptr, QuietMode::Yes)) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Failed to back up %1; leaving the current configuration in place.").arg(grubFilePath));
        return false;
    }

    QString content;
    QTextStream stream(&content);
    for (const QString &line : defaultGrub) {
        stream << line << '\n';
    }

    // Written and fsynced to a temporary file in the destination directory, then renamed into place, all
    // inside the privileged helper so the replacement is atomic even though this process cannot write there.
    bool durabilityUncertain = false;
    if (!cmd.writeFileAsRoot("/etc/default/grub", content.toUtf8(), rootPath, QuietMode::No, &durabilityUncertain)) {
        if (durabilityUncertain) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Wrote %1, but could not confirm the change was saved durably. Please verify "
                                     "it before rebooting.")
                                      .arg(grubFilePath));
        } else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Failed to write %1; the previous configuration was left in place.")
                                      .arg(grubFilePath));
        }
        return false;
    }

    return true;
}

QStringList MainWindow::getLinuxPartitions()
{
    const QRegularExpression partitionNameRx(
        QStringLiteral(R"(^(x?[hsv]d[a-z][0-9]+|mmcblk[0-9]+p[0-9]+|nvme[0-9]+n[0-9]+p[0-9]+)\b)"));
    const QStringList partitions
        = cmd.getOutAsRoot("lsblk", {"-ln", "-o", "NAME,SIZE,FSTYPE,MOUNTPOINT,LABEL", "-e", "2,11", "-x", "NAME"},
                           QuietMode::Yes)
              .split('\n', Qt::SkipEmptyParts);
    // Get PARTTYPE for all candidate partitions in one call instead of N per-partition calls
    const QStringList partTypeLines
        = cmd.getOutAsRoot("lsblk", {"-ln", "-o", "NAME,PARTTYPE"}, QuietMode::Yes)
              .split('\n', Qt::SkipEmptyParts);
    QHash<QString, QString> partTypes;
    partTypes.reserve(partTypeLines.size());
    for (const QString &line : partTypeLines) {
        partTypes[line.section(' ', 0, 0)] = line.section(' ', 1, 1).toLower();
    }

    QStringList validPartitions;
    validPartitions.reserve(partitions.size());
    for (const QString &part_info : partitions) {
        QString partName = part_info.section(' ', 0, 0);
        if (!partitionNameRx.match(partName).hasMatch()) {
            continue;
        }
        const QString partType = partTypes.value(partName);

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

    // Close the LUKS mapping opened by openLuks(), if any
    if (!luksMapper.isEmpty()) {
        if (!cmd.procAsRoot("cryptsetup", {"close", luksMapper})) {
            qWarning() << "Failed to close LUKS mapping" << luksMapper;
        }
        luksMapper.clear();
    }
}

QString MainWindow::selectPartition(const QStringList &list)
{
    auto *dialog = new CustomDialog(list);

    // Guess installed system by finding a partition with a known root label or /etc/os-release
    auto it = std::find_if(list.cbegin(), list.cend(), [&](const QString &part_info) {
        QString partName = part_info.section(' ', 0, 0);
        const QString label = [&] {
            QString out;
            cmd.proc("lsblk", {"-ln", "-o", "LABEL", "/dev/" + partName}, &out, nullptr, QuietMode::Yes);
            return out.trimmed();
        }();
        // Check for MX Linux label
        if (label.contains(QLatin1String("rootMX"))) {
            return true;
        }
        // Check for Arch Linux label
        if (label.contains(QLatin1String("arch"), Qt::CaseInsensitive)) {
            return true;
        }
        return false;
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

    if (isLuks("/dev/" + root)) {
        if (!openLuks("/dev/" + root, tempDir.path())) {
            QMessageBox::critical(this, tr("Cannot continue"), tr("Cannot open LUKS device. Exiting..."));
            cleanup();
            exit(EXIT_FAILURE);
        }
    } else if (!cmd.procAsRoot("mount", {"/dev/" + root, tempDir.path()})) {
        QMessageBox::critical(this, tr("Cannot continue"),
                              tr("Cannot create chroot environment, cannot change boot options. Exiting..."));
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Root filesystem is mounted from this point on; set chroot now so cleanup() unmounts
    // it (and any bind mounts below) on every remaining failure path, not just on success.
    chroot = "chroot " + tempDir.path() + " ";

    const QStringList chrootDirs = {tempDir.path() + "/dev", tempDir.path() + "/sys", tempDir.path() + "/proc",
                                    tempDir.path() + "/run"};
    QStringList mkdirArgs {"-p"};
    mkdirArgs += chrootDirs;
    if (!cmd.procAsRoot("mkdir", mkdirArgs)
        || !cmd.procAsRoot("mount", {"--rbind", "/dev", tempDir.path() + "/dev"})
        || !cmd.procAsRoot("mount", {"--make-rslave", tempDir.path() + "/dev"})
        || !cmd.procAsRoot("mount", {"--rbind", "/sys", tempDir.path() + "/sys"})
        || !cmd.procAsRoot("mount", {"--make-rslave", tempDir.path() + "/sys"})
        || !cmd.procAsRoot("mount", {"--rbind", "/proc", tempDir.path() + "/proc"})
        || !cmd.procAsRoot("mount", {"-t", "tmpfs", "-o", "size=100m,nodev,mode=755", "tmpfs", tempDir.path() + "/run"})
        || !cmd.procAsRoot("mkdir", {"-p", tempDir.path() + "/run/udev"})
        || !cmd.procAsRoot("mount", {"--rbind", "/run/udev", tempDir.path() + "/run/udev"})) {
        QMessageBox::critical(this, tr("Cannot continue"),
                              tr("Cannot create chroot environment, cannot change boot options. Exiting..."));
        cleanup();
        exit(EXIT_FAILURE);
    }

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

bool MainWindow::replaceLiveGrubArgs(const QString &args)
{
    if (!QFile::exists("/usr/local/bin/live-grubsave") && !QFile::exists("/usr/bin/live-grubsave")) {
        qDebug() << "live-grubsave not found, skipping live GRUB args update.";
        return true;
    }

    if (!cmd.procAsRoot("live-grubsave", {"-r"})) {
        qWarning() << "Failed to reset live-grub settings";
        return false;
    }

    QString filteredArgs = args;
    filteredArgs.remove(QRegularExpression("BOOT_IMAGE=[^ ]*"));
    filteredArgs = filteredArgs.trimmed();

    if (!filteredArgs.isEmpty()) {
        const QStringList argList = filteredArgs.split(' ', Qt::SkipEmptyParts);
        if (!cmd.procAsRoot("live-grubsave", argList)) {
            qWarning() << "Failed to save new live-grub arguments:" << filteredArgs;
            return false;
        }
    }
    return true;
}

bool MainWindow::replaceSyslinuxArgs(const QString &args)
{
    const QStringList configFiles
        = {bootLocation + "/boot/syslinux/syslinux.cfg", bootLocation + "/boot/isolinux/isolinux.cfg"};

    bool ok = true;
    for (const QString &configFile : configFiles) {
        // Only some live media ship both a syslinux and an isolinux config, so an individual file being
        // absent here is a legitimate no-op, not a failure; one that exists but can't be checked is a
        // genuine failure.
        switch (pathState(configFile)) {
        case PathState::Absent:
            continue;
        case PathState::Inaccessible:
            qWarning() << "Failed to access" << configFile << "; it may exist but be unreadable.";
            ok = false;
            continue;
        case PathState::Present:
            break;
        }

        QFile file(configFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open" << configFile << "for reading.";
            ok = false;
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
            // The config file exists and was readable, but has no live section to update, so this is a
            // genuine failure rather than a no-op (unlike an entirely absent config file, handled above).
            qWarning() << "No APPEND line found in LABEL live section in" << configFile;
            ok = false;
            continue;
        }

        // Write to a temporary file using QTemporaryFile
        QTemporaryFile tempFile(QDir::tempPath() + "/XXXXXX.tmp");
        if (!tempFile.open()) {
            qWarning() << "Failed to open temporary file for writing.";
            ok = false;
            continue;
        }

        QTextStream stream(&tempFile);
        stream.setEncoding(QStringConverter::Utf8);
        stream << new_list.join('\n') << '\n';
        stream.flush();
        const bool streamOk = stream.status() == QTextStream::Ok;
        const bool flushOk = tempFile.flush();
        tempFile.close();
        if (!streamOk || !flushOk) {
            qWarning() << "Failed to write temporary file for" << configFile;
            ok = false;
            continue;
        }

        // Move the temporary file to the original file
        QString tempFilePath = tempFile.fileName();
        if (!cmd.procAsRoot("mv", {tempFilePath, configFile})) {
            qWarning() << "Failed to move" << tempFilePath << "to" << configFile;
            ok = false;
            continue;
        }

        if (!cmd.procAsRoot("chown", {"root:", configFile}, nullptr, nullptr, QuietMode::Yes)) {
            qWarning() << "Failed to set ownership of" << configFile;
            ok = false;
        }
        if (!cmd.procAsRoot("chmod", {"644", configFile}, nullptr, nullptr, QuietMode::Yes)) {
            qWarning() << "Failed to set permissions of" << configFile;
            ok = false;
        }
    }
    return ok;
}

void MainWindow::readGrubCfg()
{
    const QString rootPath = targetRootPath();
    const QString grubFilePath = liveGrubMode() ? bootLocation + "/boot/grub/grub.cfg" : "/boot/grub/grub.cfg";
    QStringList content = cmd.readFileAsRoot(grubFilePath, QuietMode::Yes, rootPath).split('\n', Qt::SkipEmptyParts);

    if (content.isEmpty()) {
        qDebug() << "Could not read grub.cfg file";
        return;
    }

    ui->comboMenuEntry->clear();
    int menuLevel = 0;
    int menuCount = 0;
    int submenuCount = 0;
    QString menuId;
    const bool liveMode = liveGrubMode();

    for (const auto &line : content) {
        QString trimmedLine = line.trimmed();
        grubCfg << trimmedLine;

        // Live grub.cfg carries its own `set timeout=` (the host /etc/default/grub is not read), so initialize
        // the timeout control from the media itself.
        if (liveMode && trimmedLine.startsWith("set timeout=")) {
            ui->spinBoxTimeout->setValue(trimmedLine.section('=', 1).remove(QRegularExpression("[\"']")).toInt());
        }

        if (trimmedLine.startsWith("menuentry ") || trimmedLine.startsWith("submenu ")) {
            menuId = trimmedLine.section("$menuentry_id_option", 1, -1).section(' ', 1, 1);
            QString item = extractMenuEntryTitle(trimmedLine);
            if (item.trimmed().isEmpty()) {
                item = menuId; // entries with only a translated/blank literal title
            }
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
            ui->checkBackground->setChecked(true);
            ui->pushBgFile->setEnabled(true);
        } else if (line.startsWith("#export GRUB_MENU_PICTURE=")
                   && ui->pushBgFile->property("file").toString().isEmpty()) {
            // Remember the image from a disabled background line so re-enabling does not require re-picking it.
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

    // An active theme overrides the background image, regardless of the order of the lines in the file.
    if (ui->checkGrubTheme->isChecked()) {
        ui->checkBackground->setChecked(false);
        ui->pushBgFile->setDisabled(true);
    }
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
}

void MainWindow::processKernelCommandLine(QString line)
{
    const QString cmdline = line.remove("GRUB_CMDLINE_LINUX_DEFAULT=").remove(QRegularExpression("[\"']"));
    const QString effectiveCmdline = live && !installedMode ? kernelOptions : cmdline;
    ui->textKernel->setText(effectiveCmdline);

    bool hasHush = effectiveCmdline.contains(hushTokenRx);
    bool hasQuiet = effectiveCmdline.contains(quietTokenRx);
    bool hasSplash = effectiveCmdline.contains(splashTokenRx) && !effectiveCmdline.contains(noSplashTokenRx);

    ui->radioDetailedMsg->setChecked(hasQuiet);
    ui->radioLimitedMsg->setChecked(hasHush);
    ui->radioVeryDetailedMsg->setChecked(!hasHush && !hasQuiet);

    ui->checkBootsplash->setChecked(hasSplash && isInstalled(requiredPlymouthPackages()));
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
    if (!bar) {
        return;
    }
    setCursor(Qt::BusyCursor);
    bar->setValue(0);
    timer.start(100ms);
}

void MainWindow::cmdDone()
{
    timer.stop();
    if (!bar) {
        return;
    }
    setCursor(Qt::ArrowCursor);
    bar->setValue(bar->maximum());
}

void MainWindow::procTime()
{
    if (!bar) {
        timer.stop();
        return;
    }
    bar->setValue((bar->value() + 10) % bar->maximum() + 1);
}

void MainWindow::setConnections()
{
    timer.disconnect();
    timer.stop();

    connect(&timer, &QTimer::timeout, this, &MainWindow::procTime);
    connect(&cmd, &QProcess::started, this, &MainWindow::cmdStart, Qt::UniqueConnection);
    connect(&cmd, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::cmdDone,
            Qt::UniqueConnection);
}

void MainWindow::pushApplyClicked()
{
    ui->pushCancel->setDisabled(true);
    ui->pushApply->setDisabled(true);
    cmd.clearCancelRequest();

    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButtonText(tr("Cancel"));
    connect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
    progress->setWindowTitle(tr("Updating configuration, please wait"));
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->resize(500, progress->height());
    progress->show();

    setConnections();
    const QString rootPath = targetRootPath();
    const bool inLiveGrubMode = liveGrubMode();

    // Collected so the end-of-apply summary and pending-state handling reflect every operation that actually
    // ran, not just whether /etc/default/grub was written.
    QStringList failedSteps;
    auto recordStep = [&failedSteps](bool ok, const QString &label) {
        if (!ok) {
            failedSteps << label;
        }
        return ok;
    };

    if (!cmd.isCancelRequested() && kernelOptionsChanged) {
        replaceGrubArg("GRUB_CMDLINE_LINUX_DEFAULT", "\"" + ui->textKernel->text() + "\"");
        if (live && !installedMode) {
            recordStep(replaceLiveGrubArgs(ui->textKernel->text()),
                       tr("saving the kernel boot arguments to the live media"));
            if (!cmd.isCancelRequested()) {
                recordStep(replaceSyslinuxArgs(ui->textKernel->text()),
                           tr("saving the kernel boot arguments to the syslinux configuration"));
            }
        }
    }

    // On live media the timeout, default entry and theme background are written straight to the pre-generated
    // grub.cfg / syslinux config and theme files. The remaining GRUB_* defaults only take effect through
    // /etc/default/grub + update-grub, which do not apply to live media, so the installed-system branch below
    // is skipped in live mode (and those controls are disabled/hidden in the UI).
    if (!cmd.isCancelRequested() && optionsChanged && inLiveGrubMode) {
        recordStep(applyLiveGrubTimeout(ui->spinBoxTimeout->value()), tr("saving the live boot menu timeout"));
        const QString liveBgPath = ui->pushBgFile->property("file").toString();
        if (!cmd.isCancelRequested() && ui->pushBgFile->isEnabled() && QFile::exists(liveBgPath)) {
            recordStep(applyLiveGrubBackground(liveBgPath), tr("saving the live boot menu background image"));
        }
    } else if (!cmd.isCancelRequested() && optionsChanged) {
        recordStep(cmd.procAsRoot("grub-editenv", {"/boot/grub/grubenv", "unset", "next_entry"}),
                   tr("clearing the pending one-time boot entry"));

        const QString bgFilePath = ui->pushBgFile->property("file").toString();
        const QString themeFilePath = ui->pushThemeFile->property("file").toString();

        if (ui->checkBackground->isChecked() && QFile::exists(bgFilePath)) {
            if (!replaceGrubArg("export GRUB_MENU_PICTURE", "\"" + bgFilePath + "\"")) {
                // Uncomments a previously disabled line with the same image in place, or appends a new one.
                enableGrubLine("export GRUB_MENU_PICTURE=\"" + bgFilePath + "\"");
            }
        } else if (!ui->checkBackground->isChecked()) {
            disableGrubLine("export GRUB_MENU_PICTURE");
        }

        if (ui->checkGrubTheme->isChecked() && QFile::exists(themeFilePath)) {
            if (!replaceGrubArg("GRUB_THEME", "\"" + themeFilePath + "\"")) {
                addGrubLine("GRUB_THEME=\"" + themeFilePath + "\"");
            }
        } else if (ui->checkGrubTheme->isVisible() && !ui->checkGrubTheme->isChecked()) {
            disableGrubLine("GRUB_THEME=");
        }

        // For simple menu index number is sufficient, if submenus exists use "1>1" format
        QString grub_entry = ui->checkEnableFlatmenus->isChecked()
                                 ? QString::number(ui->comboMenuEntry->currentIndex())
                                 : ui->comboMenuEntry->currentData().toString().section(' ', 1, 1);

        if (!cmd.isCancelRequested() && ui->comboMenuEntry->currentText().contains(QLatin1String("memtest"))) {
            ui->spinBoxTimeout->setValue(5);
            if (rootPath.isEmpty()) {
                recordStep(cmd.procAsRoot("grub-reboot", {ui->comboMenuEntry->currentText()}),
                           tr("setting the one-time boot entry"));
            } else {
                recordStep(cmd.procAsRootInTarget(rootPath, "grub-reboot", {ui->comboMenuEntry->currentText()}),
                           tr("setting the one-time boot entry"));
            }
        } else {
            replaceGrubArg("GRUB_DEFAULT", "\"" + grub_entry + '"');
        }

        if (!cmd.isCancelRequested() && ui->checkSaveDefault->isChecked()) {
            replaceGrubArg("GRUB_DEFAULT", "saved");
            enableGrubLine("GRUB_SAVEDEFAULT=true");
            if (rootPath.isEmpty()) {
                recordStep(cmd.procAsRoot("grub-set-default", {grub_entry}), tr("saving the default boot entry"));
            } else {
                recordStep(cmd.procAsRootInTarget(rootPath, "grub-set-default", {grub_entry}),
                           tr("saving the default boot entry"));
            }
        } else if (!cmd.isCancelRequested()) {
            disableGrubLine("GRUB_SAVEDEFAULT=true");
        }

        if (!cmd.isCancelRequested() && !replaceGrubArg("GRUB_TIMEOUT", QString::number(ui->spinBoxTimeout->value()))) {
            addGrubLine("GRUB_TIMEOUT=" + QString::number(ui->spinBoxTimeout->value()));
        }
    }

    if (!cmd.isCancelRequested() && splashChanged) {
        if (ui->checkBootsplash->isChecked()) {
            if (!ui->comboTheme->currentText().isEmpty()) {
                if (rootPath.isEmpty()) {
                    recordStep(cmd.procAsRoot("plymouth-set-default-theme", {ui->comboTheme->currentText()}),
                               tr("setting the boot splash theme"));
                } else {
                    recordStep(cmd.procAsRootInTarget(rootPath, "plymouth-set-default-theme",
                                                       {ui->comboTheme->currentText()}),
                               tr("setting the boot splash theme"));
                }
            }
            if (!cmd.isCancelRequested()) {
                recordStep(toggleBootlogd(false), tr("updating the boot log service state"));
            }
        } else {
            recordStep(toggleBootlogd(true), tr("updating the boot log service state"));
        }
        progress->setLabelText(tr("Updating initramfs..."));
        if (!cmd.isCancelRequested() && !recordStep(runUpdateInitramfs(), tr("updating the initramfs"))) {
            qWarning() << "Failed to update initramfs.";
        }
    }

    if (!cmd.isCancelRequested() && messagesChanged && ui->radioLimitedMsg->isChecked()) {
        if (QFile::exists(rootPath + "/etc/default/rcS")) {
            const QString hushSnippet = QStringLiteral(
                "\n# hush boot-log into /run/rc.log\n"
                "[ \"$init\" ] && grep -qw hush /proc/cmdline && exec >> /run/rc.log 2>&1 || true ");
            recordStep(cmd.appendToFileAsRootIfMissing("/etc/default/rcS", "hush boot-log into /run/rc.log",
                                                         hushSnippet, QuietMode::Yes, rootPath),
                       tr("updating boot message settings"));
        } else {
            qWarning() << "Skipping hush configuration: /etc/default/rcS not found.";
        }
    }

    if (!cmd.isCancelRequested() && (optionsChanged || splashChanged || messagesChanged || kernelOptionsChanged)) {
        if (grubInstalled) {
            // In pure live mode the on-disk /etc/default/grub is not read into defaultGrub, so it must not be
            // written back (that would truncate it) nor regenerated via update-grub. Live edits go straight to
            // the media via replaceLiveGrubArgs/replaceSyslinuxArgs and refreshLiveGrubTheme instead.
            bool grubUpdated = false;
            if (!inLiveGrubMode) {
                if (recordStep(writeDefaultGrub(), tr("saving the GRUB configuration file"))) {
                    progress->setLabelText(tr("Updating grub..."));
                    grubUpdated = recordStep(runUpdateGrub(), tr("regenerating the GRUB boot menu"));
                    if (!grubUpdated) {
                        qWarning() << "Failed to update GRUB configuration.";
                    }
                    if (!cmd.isCancelRequested() && live && !bootLocation.isEmpty()) {
                        recordStep(cmd.procAsRoot("cp", {"/boot/grub/grub.cfg", bootLocation + "/boot/grub/grub.cfg"}),
                                   tr("copying the GRUB configuration to the boot media"));
                    }
                } else {
                    qWarning() << "Failed to write /etc/default/grub; skipping GRUB update.";
                }
            }
            bool liveLabelsUpdated = false;
            if (!cmd.isCancelRequested() && live && !installedMode) {
                progress->setLabelText(tr("Refreshing live boot labels..."));
                liveLabelsUpdated = recordStep(refreshLiveGrubTheme(), tr("refreshing the live boot menu labels"));
                if (!liveLabelsUpdated) {
                    qWarning() << "Failed to refresh the live boot labels.";
                }
            }
            if (grubUpdated || liveLabelsUpdated) {
                reloadGrubSettings();
            }
        } else if (optionsChanged || kernelOptionsChanged) {
            // Without GRUB installed there is nowhere to persist the pending GRUB/kernel option changes; treat
            // the request as failed (instead of silently dropping it) so the pending state is retained and the
            // user can retry once GRUB is present.
            recordStep(false, tr("saving the GRUB configuration file"));
            qWarning() << "GRUB is not installed; unable to apply pending GRUB/kernel option changes.";
        }
        disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
        progress->close();
        bar = nullptr;
        progress->deleteLater();
        if (!failedSteps.isEmpty()) {
            QMessageBox::critical(this, tr("Operation Incomplete"),
                                  tr("The following changes could not be applied: %1. These changes are still "
                                     "pending; please try applying again.")
                                      .arg(failedSteps.join(QStringLiteral(", "))));
        } else {
            QString message
                = live && bootLocation == "/live/to-ram"
                      ? tr("You are currently running in live mode with the 'toram' option. Please remember to "
                           "save the persistence file or remaster, otherwise any changes made will be lost.")
                      : tr("Your changes have been successfully applied.");
            QMessageBox::information(this, tr("Operation Complete"), message);
        }
    }

    if (cmd.isCancelRequested()) {
        disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
        progress->close();
        bar = nullptr;
        progress->deleteLater();
        ui->pushApply->setEnabled(true);
        ui->pushCancel->setEnabled(true);
        QMessageBox::warning(this, tr("Operation Cancelled"),
                             tr("The operation was cancelled. Changes already completed were kept; the remaining "
                                "changes are still pending."));
        return;
    }

    // Reset change flags, unless an apply step failed and needs to be retried.
    if (!failedSteps.isEmpty()) {
        ui->pushApply->setEnabled(true);
    } else {
        optionsChanged = false;
        splashChanged = false;
        messagesChanged = false;
        kernelOptionsChanged = false;
    }
    disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
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
    displayHelpDoc("/usr/share/doc/mx-boot-options/mx-boot-options.html", tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::comboBootsplashClicked(bool checked)
{
    ui->radioLimitedMsg->setVisible(!checked);

    if (checked) {
        if (inVirtualMachine()) {
            QMessageBox::information(
                this, tr("Running in a Virtual Machine"),
                tr("Your current system is running in a Virtual Machine,\n"
                   "Plymouth bootsplash will work in a limited way, you also won't be able to preview the theme"));
            // ui->pushPreview->setDisabled(true);
        }

        if (!isInstalled(requiredPlymouthPackages())) {
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
    const QString rootPath = chroot.isEmpty() ? QString() : chroot.section(' ', 1, 1);
    QString initialPath = rootPath + "/usr/share/backgrounds/MXLinux/grub";
    if (!QDir(initialPath).exists()) {
        initialPath = rootPath + "/usr/share/backgrounds";
    }
    if (!QDir(initialPath).exists()) {
        initialPath = rootPath + "/usr/share";
    }
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
        if (!line.contains(quietTokenRx)) {
            line.append(line.isEmpty() ? "quiet" : " quiet");
        }

        line.replace(hushTokenRx, " ");
        ui->textKernel->setText(line.trimmed());
    }
}

void MainWindow::radioVeryDetailedMsgToggled(bool checked)
{
    if (checked) {
        messagesChanged = true;
        ui->pushApply->setEnabled(true);

        QString line = ui->textKernel->text();
        line.replace(hushTokenRx, " ");
        line.replace(quietTokenRx, " ");
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

        if (!line.contains(quietTokenRx)) {
            options << "quiet";
        }
        if (!line.contains(hushTokenRx)) {
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
    const bool splashEnabled = isSplashEnabled();
    const bool previewAvailable = checked && splashEnabled && !waylandSession;
    ui->pushPreview->setEnabled(previewAvailable);

    if (!splashEnabled) {
        ui->pushPreview->setToolTip(tr("Preview is disabled because 'splash' parameter is not present in kernel command line. "
                                       "To enable preview, add 'splash' to boot parameters and reboot."));
    } else if (waylandSession) {
        ui->pushPreview->setToolTip(tr("Preview is disabled while running under Wayland. Please start an X11 session "
                                       "to preview Plymouth themes."));
    } else {
        ui->pushPreview->setToolTip("");
    }

    QString line = ui->textKernel->text();
    if (checked) {
        loadPlymouthThemes();
        line.replace(noSplashTokenRx, " ");
        if (!line.contains(splashTokenRx)) {
            line.append(line.isEmpty() ? "splash" : " splash");
        }
    } else {
        ui->comboTheme->clear();
        ui->pushPreview->setDisabled(true);
        line.replace(splashTokenRx, " ");
    }

    ui->textKernel->setText(line.trimmed());
    kernelOptionsChanged = true;
}

void MainWindow::pushLogClicked()
{
    QString logContent;
    QString fallbackLocation;
    QString offlineJournalDir;
    const QString rootPath = targetRootPath();

    const bool systemdEnv = isSystemdEnvironment();
    if (systemdEnv) {
        if (rootPath.isEmpty()) {
            logContent = cmd.getOutAsRoot("journalctl", {"-b", "--no-pager"}, QuietMode::Yes);
        } else {
            const QString journalRootPath = tempDir.path();
            QString journalDir = journalRootPath + QStringLiteral("/var/log/journal");
            if (!QDir(journalDir).exists()) {
                journalDir = journalRootPath + QStringLiteral("/run/log/journal");
            }
            if (QDir(journalDir).exists()) {
                offlineJournalDir = journalDir;
                logContent = cmd.getOutAsRoot("journalctl",
                                              {QStringLiteral("--directory=%1").arg(journalDir), "-b", "--no-pager",
                                               QStringLiteral("--root=%1").arg(journalRootPath)},
                                              QuietMode::Yes);
            }
        }
    }

    if (logContent.isEmpty()) {
        QString location = rootPath.isEmpty() ? QString() : tempDir.path();
        const bool hasHush = kernelOptions.contains(hushTokenRx);
        location += hasHush ? "/run/rc.log" : "/var/log/boot.log";

        if (!QFile::exists(location)) {
            location = rootPath.isEmpty() ? "/var/log/boot" : tempDir.path() + "/var/log/boot";
        }

        fallbackLocation = location;

        if (QFile::exists(location)) {
            const QString helperPath
                = rootPath.isEmpty() ? location : location.mid(tempDir.path().size());
            logContent = cmd.readFileAsRoot(helperPath, QuietMode::Yes, rootPath);
        }
    }

    if (logContent.isEmpty()) {
        QString message;
        if (systemdEnv) {
            message = fallbackLocation.isEmpty()
                ? tr("Could not read the systemd boot logs.")
                : tr("Could not read the systemd boot logs%1 or the fallback log at %2.")
                      .arg(offlineJournalDir.isEmpty() ? QString() : tr(" from %1").arg(offlineJournalDir),
                           fallbackLocation);
        } else {
            message = fallbackLocation.isEmpty() ? tr("Could not find any boot logs.")
                                                 : tr("Could not find log at %1").arg(fallbackLocation);
        }
        QMessageBox::critical(this, tr("Log not found"), message);
        return;
    }

    QDialog logDialog;
    logDialog.setWindowTitle(tr("Boot Log"));

    auto *textEdit = new QTextEdit(&logDialog);
    textEdit->setReadOnly(true);
    textEdit->setMinimumSize(600, 500);
    appendLogWithColors(textEdit, logContent);

    auto *closeButton = new QPushButton(tr("&Close"), &logDialog);
    connect(closeButton, &QPushButton::clicked, &logDialog, &QDialog::accept);

    auto *layout = new QVBoxLayout(&logDialog);
    layout->addWidget(textEdit);
    layout->addWidget(closeButton);

    logDialog.setModal(true);
    logDialog.setSizeGripEnabled(true);
    logDialog.exec();
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
    if (waylandSession) {
        QMessageBox::warning(
            this, tr("Preview Unavailable"),
            tr("Preview is not available while running under Wayland. Please start an X11 session to preview Plymouth themes."));
        return;
    }

    if (justInstalled) {
        QMessageBox::warning(
            this, tr("Needs reboot"),
            tr("Plymouth was just installed, you might need to reboot before being able to display previews"));
    }

    const QString rootPath = targetRootPath();
    QString current_theme = rootPath.isEmpty() ? cmd.getOutAsRoot("plymouth-set-default-theme").trimmed()
                                               : cmd.getOutAsRootInTarget(rootPath, "plymouth-set-default-theme").trimmed();
    if (ui->comboTheme->currentText() == "details") {
        return;
    }

    if (inVirtualMachine()) {
        QMessageBox::information(
            this, tr("Running in a Virtual Machine"),
            tr("Your current system is running in a Virtual Machine,\n"
               "Plymouth bootsplash will work in a limited way, you also won't be able to preview the theme"));
    }
    if (rootPath.isEmpty()) {
        cmd.procAsRoot("plymouth-set-default-theme", {ui->comboTheme->currentText()});
    } else {
        cmd.procAsRootInTarget(rootPath, "plymouth-set-default-theme", {ui->comboTheme->currentText()});
    }

    QTimer tick;
    tick.start(100ms);
    connect(&tick, &QTimer::timeout, this, &MainWindow::sendMouseEvents);
    cmd.previewPlymouthAsRoot();
    if (rootPath.isEmpty()) {
        cmd.procAsRoot("plymouth-set-default-theme", {current_theme});
    } else {
        cmd.procAsRootInTarget(rootPath, "plymouth-set-default-theme", {current_theme});
    }
}

void MainWindow::comboEnableFlatmenusClicked(bool checked)
{
    // Flat-menu layout is a /etc/default/grub setting applied via update-grub; it does not apply to live media.
    // Without this guard the click would write the (empty, unread) defaultGrub over the host /etc/default/grub.
    if (liveGrubMode()) {
        return;
    }

    cmd.clearCancelRequest();

    auto *progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);

    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButtonText(tr("Cancel"));
    connect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
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

    if (writeDefaultGrub()) {
        progress->setLabelText(tr("Updating grub..."));
        setConnections();
        if (!runUpdateGrub()) {
            qWarning() << "Failed to update GRUB configuration.";
        }
        readGrubCfg();
    } else {
        qWarning() << "Failed to write /etc/default/grub; GRUB configuration not updated.";
        optionsChanged = true;
        ui->pushApply->setEnabled(true);
    }
    disconnect(progress, &QProgressDialog::canceled, &cmd, &Cmd::cancel);
    progress->close();
    bar = nullptr;
    progress->deleteLater();
}

void MainWindow::comboSaveDefaultClicked()
{
    optionsChanged = true;
    ui->pushApply->setEnabled(true);
}

void MainWindow::comboThemeCurrentIndexChanged(int index)
{
    const QString themeName = ui->comboTheme->itemText(index);
    const bool isNonPreviewableTheme = (themeName == QLatin1String("details")
                                        || themeName == QLatin1String("text")
                                        || themeName == QLatin1String("tribar"));
    ui->pushPreview->setDisabled(isNonPreviewableTheme || !isSplashEnabled() || waylandSession);
}

void MainWindow::comboGrubThemeToggled(bool checked)
{
    // The theme replaces the plain background image, so only one of the two can be enabled at a time.
    // Disabling the background this way is a persistable change on its own, even before a theme is selected.
    const bool backgroundDisabled = checked && ui->checkBackground->isChecked();
    if (checked) {
        ui->checkBackground->setChecked(false);
        ui->pushBgFile->setDisabled(true);
    }
    if (checked && ui->pushThemeFile->property("file").toString().isEmpty()) {
        ui->pushThemeFile->setText(tr("Click to select theme"));
        ui->pushThemeFile->setProperty("file", "");
    }
    if (!checked || backgroundDisabled || !ui->pushThemeFile->property("file").toString().isEmpty()) {
        optionsChanged = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::checkBackgroundToggled(bool checked)
{
    // The theme replaces the plain background image, so only one of the two can be enabled at a time.
    // Disabling the theme this way is a persistable change on its own, even before an image is selected.
    const bool themeDisabled = checked && ui->checkGrubTheme->isVisible() && ui->checkGrubTheme->isChecked();
    if (themeDisabled) {
        ui->checkGrubTheme->setChecked(false);
        ui->pushThemeFile->setDisabled(true);
    }
    if (checked && ui->pushBgFile->property("file").toString().isEmpty()) {
        ui->pushBgFile->setText(tr("Click to select image"));
        ui->pushBgFile->setProperty("file", "");
    }
    if (!checked || themeDisabled || !ui->pushBgFile->property("file").toString().isEmpty()) {
        optionsChanged = true;
        ui->pushApply->setEnabled(true);
    }
}

void MainWindow::btnThemeFileClicked()
{
    QString themeDirectory = live && !installedMode && !bootLocation.isEmpty() ? bootLocation + "/boot/grub/theme"
                                                                           : chroot.section(' ', 1, 1) + "/boot/grub/themes";
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
        qDebug() << "Not a LUKS partition:" << part;
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
    SecureBuffer _scrub(&pass); // zeroes pass on scope exit (all return paths)

    if (!ok || pass.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Password entry cancelled or empty for %1").arg(partition));
        return false;
    }

    // Try to open the LUKS container
    if (!cmd.procAsRoot("cryptsetup", {"open", "--allow-discards", partition, mapper, "-"}, nullptr, &pass)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open %1 LUKS container").arg(partition));
        return false;
    }
    if (!cmd.procAsRoot("mount", {"/dev/mapper/" + mapper, path})) {
        QMessageBox::critical(this, tr("Error"), tr("Could not mount %1 LUKS container").arg(partition));
        cmd.procAsRoot("cryptsetup", {"close", mapper});
        return false;
    }
    // Root mount succeeded; record it now so cleanup() can unmount the root and close the
    // mapping if mountBoot() fails below, instead of leaking either while createChrootEnv()
    // still thinks nothing is mounted.
    chroot = "chroot " + path + " ";
    luksMapper = mapper;
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

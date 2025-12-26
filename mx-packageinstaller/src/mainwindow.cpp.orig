/**********************************************************************
 *  MainWindow.cpp
 **********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          Dolphin_Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-packageinstaller.
 *
 * mx-packageinstaller is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-packageinstaller is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-packageinstaller.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileInfo>
#include <QImageReader>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QProgressBar>
#include <QScreen>
#include <QScopedValueRollback>
#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>
#include <QtXml/QtXml>

#include "about.h"
#include "aptcache.h"
#include "checkableheaderview.h"
#include "versionnumber.h"
#include <algorithm>
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(const QCommandLineParser &argParser, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow),
      dictionary("/usr/share/mx-packageinstaller-pkglist/category.dict", QSettings::IniFormat),
      args {argParser},
      reply(nullptr)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << QCoreApplication::applicationVersion();
    ui->setupUi(this);
    setProgressDialog();

    connect(&timer, &QTimer::timeout, this, &MainWindow::updateBar);
    connect(&cmd, &Cmd::started, this, &MainWindow::cmdStart);
    connect(&cmd, &Cmd::done, this, &MainWindow::cmdDone);
    connect(&cmd, &Cmd::outputAvailable, this,
            [this](const QString &out) { if (!suppressCmdOutput) qDebug() << out.trimmed(); });
    connect(&cmd, &Cmd::errorAvailable, this,
            [this](const QString &out) { if (!suppressCmdOutput) qWarning() << out.trimmed(); });
    setWindowFlags(Qt::Window); // For the close, min and max buttons

    setup();

    // Run package display in a separate thread
    auto packageFuture [[maybe_unused]] = QtConcurrent::run([this] {
        AptCache cache;
        enabledList = cache.getCandidates();
        QMetaObject::invokeMethod(
            this,
            [this] {
                displayPackages();
                ui->tabWidget->setTabEnabled(Tab::Test, true);
                ui->tabWidget->setTabEnabled(Tab::Backports, true);
            },
            Qt::QueuedConnection);
    });

    // Run flatpak setup and display in a separate thread
    if (arch != "i386" && checkInstalled("flatpak")) {
        auto flatpakFuture [[maybe_unused]] = QtConcurrent::run([this] {
            Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib flatpak_add_repos", Cmd::QuietMode::Yes);
            QMetaObject::invokeMethod(this, [this] { displayFlatpaks(); }, Qt::QueuedConnection);
        });
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->blockSignals(true);
    ui->tabWidget->setCurrentWidget(ui->tabPopular);
    ui->tabWidget->setTabEnabled(Tab::Test, false);
    ui->tabWidget->setTabEnabled(Tab::Backports, false);
    ui->pushRemoveAutoremovable->setHidden(true);

    QFont font("monospace");
    font.setStyleHint(QFont::Monospace);
    ui->outputBox->setFont(font);

    QString defaultFSUser = settings.value("FlatpakUser", tr("For all users")).toString();
    fpUser = defaultFSUser == tr("For all users") ? "--system " : "--user ";
    ui->comboUser->blockSignals(true);
    ui->comboUser->setCurrentText(defaultFSUser);
    ui->comboUser->blockSignals(false);

    arch = AptCache::getArch();
    debianVersion = getDebianVerNum();
    verName = getDebianVerName(debianVersion);

    ui->tabWidget->setTabVisible(Tab::Flatpak, arch != "i386");
    ui->tabWidget->setTabVisible(Tab::Test, QFile::exists("/etc/apt/sources.list.d/mx.list")
                                                || QFile::exists("/etc/apt/sources.list.d/mx.sources"));

    testInitiallyEnabled
        = cmd.run("apt-get update --print-uris | grep -m1 -qE '/mx/testrepo/dists/" + verName + "/test/'");

    setWindowTitle(tr("MX Package Installer"));
    hideColumns();
    setIcons();
    loadPmFiles();
    refreshPopularApps();

    // Load persisted setting for hiding libraries/developer packages
    const bool savedHideLibs = settings.value("HideLibs", true).toBool();
    hideLibsChecked = savedHideLibs;
    ui->checkHideLibs->setChecked(savedHideLibs);
    ui->checkHideLibsMX->setChecked(savedHideLibs);
    ui->checkHideLibsBP->setChecked(savedHideLibs);

    // Ensure "Select all" checkboxes start hidden/unchecked
    // (Deprecated UI checkboxes remain hidden in UI; header checkboxes are used instead.)
    if (auto *w = ui->checkSelectAllEnabled) {
        w->setVisible(false);
        w->setChecked(false);
    }
    if (auto *w = ui->checkSelectAllMX) {
        w->setVisible(false);
        w->setChecked(false);
    }
    if (auto *w = ui->checkSelectAllBP) {
        w->setVisible(false);
        w->setChecked(false);
    }

    // Install custom header views with checkbox in column 0 (TreeCol::Check)
    headerEnabled = new CheckableHeaderView(Qt::Horizontal, ui->treeEnabled);
    headerEnabled->setTargetColumn(TreeCol::Check);
    headerEnabled->setMinimumSectionSize(22);
    ui->treeEnabled->setHeader(headerEnabled);

    headerMX = new CheckableHeaderView(Qt::Horizontal, ui->treeMXtest);
    headerMX->setTargetColumn(TreeCol::Check);
    headerMX->setMinimumSectionSize(22);
    ui->treeMXtest->setHeader(headerMX);

    headerBP = new CheckableHeaderView(Qt::Horizontal, ui->treeBackports);
    headerBP->setTargetColumn(TreeCol::Check);
    headerBP->setMinimumSectionSize(22);
    ui->treeBackports->setHeader(headerBP);
    setConnections();

    ui->searchPopular->setFocus();
    currentTree = ui->treePopularApps;

    ui->tabWidget->setTabEnabled(Tab::Output, false);
    ui->tabWidget->blockSignals(false);
    ui->pushUpgradeAll->setVisible(false);

    const auto size = this->size();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        if (isMaximized()) { // Add option to resize if maximized
            resize(size);
            centerWindow();
        }
    }
    const QString aptConfigOutput = cmd.getOut("apt-config shell APTOPT APT::Install-Recommends/b").trimmed();
    ui->checkBoxInstallRecommends->setChecked(aptConfigOutput == "APTOPT='true'");
    ui->checkBoxInstallRecommendsMX->setChecked(aptConfigOutput == "APTOPT='true'");
    ui->checkBoxInstallRecommendsBP->setChecked(aptConfigOutput == "APTOPT='true'");

    // Check/uncheck tree items spacebar press or double-click
    auto *shortcutToggle = new QShortcut(Qt::Key_Space, this);
    connect(shortcutToggle, &QShortcut::activated, this, &MainWindow::checkUncheckItem);

    QList listTree {ui->treePopularApps, ui->treeEnabled, ui->treeMXtest, ui->treeBackports, ui->treeFlatpak};
    for (const auto &tree : listTree) {
        if (tree != ui->treeFlatpak) {
            tree->setContextMenuPolicy(Qt::CustomContextMenu);
        }
        connect(tree, &QTreeWidget::itemDoubleClicked, [tree](QTreeWidgetItem *item) { tree->setCurrentItem(item); });
        connect(tree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::checkUncheckItem);
        connect(tree, &QTreeWidget::customContextMenuRequested, this,
                [this, tree](QPoint pos) { displayPackageInfo(tree, pos); });
    }
}

bool MainWindow::uninstall(const QString &names, const QString &preuninstall, const QString &postuninstall)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setCurrentWidget(ui->tabOutput);

    bool success = true;
    // Simulate install of selections and present for confirmation
    // if user selects cancel, break routine but return success to avoid error message
    if (!confirmActions(names, "remove")) {
        return true;
    }

    ui->tabWidget->setTabText(Tab::Output, tr("Uninstalling packages..."));
    enableOutput();

    if (!preuninstall.isEmpty()) {
        qDebug() << "Pre-uninstall";
        ui->tabWidget->setTabText(Tab::Output, tr("Running pre-uninstall operations..."));
        enableOutput();
        if (lockFile.isLockedGUI()) {
            return false;
        }
        success = cmd.runAsRoot(preuninstall);
    }

    if (success) {
        enableOutput();
        if (lockFile.isLockedGUI()) {
            return false;
        }
        success = cmd.runAsRoot("DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null | grep -sq ^i "
                                "&& echo kde || echo gnome) apt-get -o=Dpkg::Use-Pty=0 remove -y "
                                + names); // use -y since there is a confirm dialog already
    }

    if (success && !postuninstall.isEmpty()) {
        qDebug() << "Post-uninstall";
        ui->tabWidget->setTabText(Tab::Output, tr("Running post-uninstall operations..."));
        enableOutput();
        if (lockFile.isLockedGUI()) {
            return false;
        }
        success = cmd.runAsRoot(postuninstall);
    }
    return success;
}

bool MainWindow::updateApt()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (lockFile.isLockedGUI()) {
        return false;
    }
    ui->tabOutput->isVisible() // Don't display in output if calling to refresh from tabs
        ? ui->tabWidget->setTabText(Tab::Output, tr("Refreshing sources..."))
        : progress->show();
    if (!timer.isActive()) {
        timer.start(100ms);
    }

    enableOutput();
    if (cmd.run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib apt_update", Cmd::QuietMode::Yes)) {
        qDebug() << "sources updated OK";
        updatedOnce = true;
        return true;
    }
    qDebug() << "problem updating sources";
    QMessageBox::critical(this, tr("Error"),
                          tr("There was a problem updating sources. Some sources may not have "
                             "provided updates. For more info check: ")
                              + "<a href=\"/var/log/mxpi.log\">/var/log/mxpi.log</a>");
    return false;
}

// Convert different size units to bytes
quint64 MainWindow::convert(const QString &size)
{
    static const QMap<QString, quint64> multipliers {{"KB", KiB}, {"MB", MiB}, {"GB", GiB}};

    const QString number = size.section(QChar(160), 0, 0);
    const QString unit = size.section(QChar(160), 1).toUpper();
    const double value = number.toDouble();

    return static_cast<quint64>(value * multipliers.value(unit, 1)); // Default multiplier 1 for bytes
}

// Convert to string (#bytes, KiB, MiB, and GiB)
QString MainWindow::convert(quint64 bytes)
{
    auto size = static_cast<double>(bytes);
    if (bytes < KiB) {
        return QString::number(size) + " bytes";
    } else if (bytes < MiB) {
        return QString::number(size / KiB) + " KiB";
    } else if (bytes < GiB) {
        return QString::number(size / MiB, 'f', 1) + " MiB";
    } else {
        return QString::number(size / GiB, 'f', 2) + " GiB";
    }
}

void MainWindow::listSizeInstalledFP()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    auto sumSizes = [](const QList<QString> &sizes) {
        return std::accumulate(sizes.cbegin(), sizes.cend(), quint64(0),
                               [](quint64 acc, const QString &item) { return acc + convert(item); });
    };

    quint64 total = 0;
    if (cachedInstalledScope == fpUser && cachedInstalledFetched) {
        total = sumSizes(cachedInstalledSizeMap.values());
    } else {
        const QString command = "flatpak list " + fpUser + "--columns app,size";
        QScopedValueRollback<bool> guard(suppressCmdOutput, true);
        QStringList list = cmd.getOut(command, Cmd::QuietMode::No).split('\n', Qt::SkipEmptyParts);
        total = std::accumulate(list.cbegin(), list.cend(), quint64(0),
                                [](quint64 acc, const QString &item) { return acc + convert(item.section('\t', 1)); });
    }
    ui->labelNumSize->setText(convert(total));
}

// Keep Flatpak UI enabled; rely on modal progress dialog to block interaction
void MainWindow::blockInterfaceFP(bool)
{
    // Maintain cursor feedback without toggling widget enabled state
    const bool isBusy = displayFlatpaksIsRunning;
    setCursor(isBusy ? QCursor(Qt::BusyCursor) : QCursor(Qt::ArrowCursor));
}

// Update interface when changing Tab::Enabled, MX, Backports
void MainWindow::updateInterface() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (currentTree == ui->treePopularApps || currentTree == ui->treeFlatpak) {
        return;
    }
    if (displayPackagesIsRunning) {
        connect(this, &MainWindow::displayPackagesFinished, this, &MainWindow::updateInterface, Qt::UniqueConnection);
        return;
    }
    QApplication::restoreOverrideCursor();
    progress->hide();
    int upgradeCount = 0;
    int installCount = 0;

    for (QTreeWidgetItemIterator it(currentTree); *it; ++it) {
        auto userData = (*it)->data(TreeCol::Status, Qt::UserRole).toInt();
        switch (userData) {
        case Status::Upgradable:
            ++upgradeCount;
            break;
        case Status::Installed:
            ++installCount;
            break;
        }
        (*it)->setHidden(false);
    }

    auto updateLabelsAndFocus = [&](QLabel *labelNumApps, QLabel *labelNumUpgrade, QLabel *labelNumInstall,
                                    QPushButton *pushForceUpdate, QLineEdit *searchBox) {
        labelNumApps->setText(QString::number(currentTree->topLevelItemCount()));
        labelNumUpgrade->setText(QString::number(upgradeCount));
        labelNumInstall->setText(QString::number(installCount + upgradeCount));
        pushForceUpdate->setEnabled(true);
        searchBox->setFocus();
    };

    switch (ui->tabWidget->currentIndex()) {
    case Tab::EnabledRepos:
        ui->pushUpgradeAll->setVisible(upgradeCount > 0);
        updateLabelsAndFocus(ui->labelNumApps, ui->labelNumUpgr, ui->labelNumInst, ui->pushForceUpdateEnabled,
                             ui->searchBoxEnabled);
        break;
    case Tab::Test:
        updateLabelsAndFocus(ui->labelNumApps_2, ui->labelNumUpgrMX, ui->labelNumInstMX, ui->pushForceUpdateMX,
                             ui->searchBoxMX);
        break;
    case Tab::Backports:
        updateLabelsAndFocus(ui->labelNumApps_3, ui->labelNumUpgrBP, ui->labelNumInstBP, ui->pushForceUpdateBP,
                             ui->searchBoxBP);
        break;
    }
}

uchar MainWindow::getDebianVerNum()
{
    QFile file {"/etc/debian_version"};
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open /etc/debian_version:" << file.errorString();
        return showVersionDialog(tr("Could not determine Debian version. Please select your version:"));
    }

    QTextStream in(&file);
    const QString version = in.readLine().split('/').at(0); // Handle cases like "bookworm/sid"
    file.close();

    // First try parsing as numeric version
    bool ok = false;
    const int numericVer = version.split('.').at(0).toInt(&ok);
    if (ok) {
        return numericVer;
    }

    // Then try matching codename
    const QString codename = version.toLower();
    if (codename == "bullseye") {
        return Release::Bullseye;
    }
    if (codename == "bookworm") {
        return Release::Bookworm;
    }
    if (codename == "trixie") {
        return Release::Trixie;
    }

    qCritical() << "Unknown Debian version:" << version;
    return showVersionDialog(tr("Could not determine Debian version. Please select your version:"));
}

uchar MainWindow::showVersionDialog(const QString &message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Debian Version"));
    msgBox.setText(message);
    msgBox.addButton("Bookworm", QMessageBox::AcceptRole);
    msgBox.addButton("Trixie", QMessageBox::AcceptRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.show();

    int ret = msgBox.exec();

    if (ret == QMessageBox::Cancel) {
        exit(EXIT_FAILURE);
    }
    return ret == 0 ? Release::Bookworm : Release::Trixie;
}

QString MainWindow::getDebianVerName(uchar version)
{
    if (version == 0) {
        version = getDebianVerNum();
    }
    static const std::map<uchar, QString> versionMap
        = {{Release::Jessie, QStringLiteral("jessie")},     {Release::Stretch, QStringLiteral("stretch")},
           {Release::Buster, QStringLiteral("buster")},     {Release::Bullseye, QStringLiteral("bullseye")},
           {Release::Bookworm, QStringLiteral("bookworm")}, {Release::Trixie, QStringLiteral("trixie")}};

    if (const auto it = versionMap.find(version); it != versionMap.end()) {
        return it->second;
    }

    qWarning() << "Error: Invalid Debian version, assumes bookworm";
    return QStringLiteral("bookworm");
}

QString MainWindow::getLocalizedName(const QDomElement &element) const
{
    const QString &localeName = locale.name();
    QStringList tagCandidates = {localeName, localeName.section('_', 0, 0), "en", "en_US"};

    for (const auto &tag : tagCandidates) {
        for (auto child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
            if (child.tagName() == tag && !child.text().trimmed().isEmpty()) {
                return child.text().trimmed();
            }
        }
    }

    auto child = element.firstChildElement();
    return child.isNull() ? element.text().trimmed() : child.text().trimmed();
}

QString MainWindow::categoryTranslation(const QString &item)
{
    // Return original item for English locale
    if (locale.name() == "en_US") {
        return item;
    }

    // Try full locale name (e.g. "fr_FR") then language code only (e.g. "fr")
    const QStringList tagCandidates = {locale.name(), locale.name().section('_', 0, 0)};

    dictionary.beginGroup(item);
    for (const auto &tag : tagCandidates) {
        const QString translation = dictionary.value(tag).toString();
        if (!translation.isEmpty()) {
            dictionary.endGroup();
            return translation;
        }
    }
    dictionary.endGroup();

    return item; // Fallback to original if no translation found
}

QString MainWindow::getArchOption() const
{
    static const QMap<QString, QString> archMap {
        {"amd64", "--arch=x86_64"}, {"i386", "--arch=i386"}, {"armhf", "--arch=arm"}, {"arm64", "--arch=aarch64"}};
    return archMap.value(arch, QString()) + ' ';
}

void MainWindow::updateBar()
{
    QApplication::processEvents();
    bar->setValue((bar->value() + 1) % bar->maximum() + 1);
}

void MainWindow::checkUncheckItem()
{
    auto *currentTreeWidget = qobject_cast<QTreeWidget *>(focusWidget());

    if (!currentTreeWidget || !currentTreeWidget->currentItem() || currentTreeWidget->currentItem()->childCount() > 0) {
        return;
    }
    const auto col = (currentTreeWidget == ui->treePopularApps) ? static_cast<int>(PopCol::Check)
                                                                : static_cast<int>(TreeCol::Check);
    const auto newCheckState
        = (currentTreeWidget->currentItem()->checkState(col) == Qt::Checked) ? Qt::Unchecked : Qt::Checked;

    currentTreeWidget->currentItem()->setCheckState(col, newCheckState);
}

void MainWindow::outputAvailable(const QString &output)
{
    static const QRegularExpression ansiEscape {R"(\x1B\[[0-9;?]*[A-Za-z])"};

    // Remove ANSI escape sequences
    QString cleanOutput = output;
    cleanOutput.remove(ansiEscape);

    // Handle carriage return (overwrite current line)
    if (cleanOutput.contains('\r')) {
        QTextCursor cursor = ui->outputBox->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }

    // Move cursor to end and insert cleaned output
    ui->outputBox->moveCursor(QTextCursor::End);
    ui->outputBox->insertPlainText(cleanOutput);

    ui->outputBox->verticalScrollBar()->setValue(ui->outputBox->verticalScrollBar()->maximum());
}

void MainWindow::loadPmFiles()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    const QString pmFolderPath {"/usr/share/mx-packageinstaller-pkglist"};
    const QStringList pmFileList = QDir(pmFolderPath).entryList({"*.pm"});

    for (const QString &fileName : pmFileList) {
        QFile file(pmFolderPath + '/' + fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open file:" << file.fileName();
            continue;
        }

        QDomDocument doc;
        if (!doc.setContent(&file)) {
            qWarning() << "Could not load document:" << fileName << "-- not valid XML?";
            file.close();
            continue;
        }

        processDoc(doc);
        file.close();
    }
}

// Process DOM documents (from .pm files)
void MainWindow::processDoc(const QDomDocument &doc)
{
    PopularInfo info;
    QDomElement root = doc.firstChildElement("app");
    QDomElement element = root.firstChildElement();

    // Optimization: Use static lookup table instead of repeated string comparisons
    static const QHash<QString, int> tagLookup = {{"category", 0},
                                                  {"name", 1},
                                                  {"description", 2},
                                                  {"installable", 3},
                                                  {"screenshot", 4},
                                                  {"preinstall", 5},
                                                  {"install_package_names", 6},
                                                  {"postinstall", 7},
                                                  {"uninstall_package_names", 8},
                                                  {"postuninstall", 9},
                                                  {"preuninstall", 10}};

    while (!element.isNull()) {
        const QString tagName = element.tagName();

        // Fast lookup instead of multiple string comparisons
        auto it = tagLookup.find(tagName);
        if (it != tagLookup.end()) {
            QString trimmedText = element.text().trimmed();

            switch (it.value()) {
            case 0: // category
                info.category = categoryTranslation(trimmedText);
                break;
            case 1: // name
                info.name = std::move(trimmedText);
                break;
            case 2: // description
                info.description = getLocalizedName(element);
                break;
            case 3: // installable
                info.installable = std::move(trimmedText);
                break;
            case 4: // screenshot
                info.screenshot = std::move(trimmedText);
                break;
            case 5: // preinstall
                info.preInstall = std::move(trimmedText);
                break;
            case 6: // install_package_names
                info.installNames = trimmedText.replace('\n', ' ');
                break;
            case 7: // postinstall
                info.postInstall = std::move(trimmedText);
                break;
            case 8: // uninstall_package_names
                info.uninstallNames = std::move(trimmedText);
                break;
            case 9: // postuninstall
                info.postUninstall = std::move(trimmedText);
                break;
            case 10: // preuninstall
                info.preUninstall = std::move(trimmedText);
                break;
            }
        }
        element = element.nextSiblingElement();
    }

    const QString modArch = mapArchToFormat(arch);
    if (isPackageInstallable(info.installable, modArch)) {
        popularApps.append(info);
    }
}

QString MainWindow::mapArchToFormat(const QString &arch) const
{
    static const QMap<QString, QString> archMapping
        = {{"amd64", "64"}, {"i386", "32"}, {"armhf", "armhf"}, {"arm64", "armsixtyfour"}};

    return archMapping.value(arch, QString());
}

bool MainWindow::isPackageInstallable(const QString &installable, const QString &modArch) const
{
    return installable.split(',').contains(modArch) || installable == "all";
}

namespace
{
struct ParsedFlatpakRef {
    QString ref;
    bool isRuntime {false};
};

QString canonicalFlatpakRef(const QString &ref)
{
    QString cleaned = ref.trimmed();
    if (cleaned.startsWith("app/") || cleaned.startsWith("runtime/")) {
        cleaned = cleaned.section('/', 1);
    }
    return cleaned;
}

bool isRuntimeToken(const QString &token)
{
    return token.startsWith("runtime/") || token.contains(".runtime/") || token.contains(".Platform");
}

ParsedFlatpakRef parseInstalledFlatpakLine(const QString &line)
{
    static const QRegularExpression refRegex(R"((app|runtime)/\S+)");

    const QRegularExpressionMatch match = refRegex.match(line);
    if (match.hasMatch()) {
        const QString refWithType = match.captured(0);
        return {refWithType.section('/', 1), refWithType.startsWith("runtime/")};
    }

    const QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString &token : tokens) {
        if (token.contains('/')) {
            const bool isRuntime = isRuntimeToken(token);
            // If the token already lacks the app/runtime prefix, keep it intact
            const bool hasTypePrefix = token.startsWith("app/") || token.startsWith("runtime/");
            QString ref = hasTypePrefix ? token.section('/', 1) : token;
            return {ref.trimmed(), isRuntime};
        }
    }

    // Fallback: return the line as-is if it looks like a ref without type prefix
    const QString fallbackRef = line.contains('/') ? line.trimmed() : QString();
    const bool isRuntime = isRuntimeToken(fallbackRef);
    return {fallbackRef, isRuntime};
}

struct RemoteLsEntry {
    QString version;
    QString branch;
    QString ref;
    QString size;
};

RemoteLsEntry parseRemoteLsLine(const QString &line)
{
    RemoteLsEntry entry;

    const QStringList tabPartsRaw = line.split('\t', Qt::KeepEmptyParts);
    QStringList tabParts;
    tabParts.reserve(tabPartsRaw.size());
    for (const QString &part : tabPartsRaw) {
        tabParts.append(part.trimmed());
    }

    auto finalizeEntry = [&entry]() {
        if (entry.branch.isEmpty() && !entry.ref.isEmpty()) {
            entry.branch = entry.ref.section('/', -1);
        }
        if ((entry.version.isEmpty() || entry.version.contains('/')) && !entry.ref.isEmpty()) {
            entry.version = entry.ref.section('/', -1);
        }
        return entry;
    };

    if (tabParts.size() >= 4) {
        entry.version = tabParts.at(0);
        entry.branch = tabParts.at(1);
        entry.ref = tabParts.at(2);
        entry.size = tabParts.at(3);
        return finalizeEntry();
    }
    if (tabParts.size() == 3) {
        entry.version = tabParts.at(0);
        const QString possibleBranchOrRef = tabParts.at(1);
        if (possibleBranchOrRef.count('/') >= 2) {
            entry.ref = possibleBranchOrRef;
            entry.size = tabParts.at(2);
        } else {
            entry.branch = possibleBranchOrRef;
            entry.ref = tabParts.at(2);
        }
        return finalizeEntry();
    }

    QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int refIndex = -1;
    for (int i = 0; i < tokens.size(); ++i) {
        if (tokens.at(i).count('/') >= 2) { // Looks like a Flatpak ref
            entry.ref = tokens.at(i);
            refIndex = i;
            break;
        }
    }

    if (!entry.ref.isEmpty()) {
        entry.version = tokens.value(0);
        entry.branch = tokens.value(1);
        if (refIndex >= 0 && refIndex + 1 < tokens.size()) {
            entry.size = tokens.mid(refIndex + 1).join(' ');
        }
        return finalizeEntry();
    }

    entry.ref = line.trimmed();
    return finalizeEntry();
}
} // namespace

void MainWindow::refreshPopularApps()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    disableOutput();
    ui->treePopularApps->clear();
    ui->searchPopular->clear();
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);
    installedPackages = listInstalled();
    displayPopularApps();
}

// Handles duplicate Flatpak entries by adding context to their display names
void MainWindow::removeDuplicatesFP() const
{
    ui->treeFlatpak->setUpdatesEnabled(false);

    // First pass: identify duplicates
    QHash<QString, QList<QTreeWidgetItem *>> nameToItems;
    for (QTreeWidgetItemIterator it(ui->treeFlatpak); *it; ++it) {
        const QString name = (*it)->text(FlatCol::Name);
        nameToItems[name].append(*it);
    }

    // Second pass: rename duplicates with more context
    for (const auto &items : nameToItems) {
        if (items.size() > 1) {
            for (auto *item : items) {
                const QString longName = item->text(FlatCol::LongName);
                // Use the last two segments of the full name for better context
                const QString newName = longName.section('.', -2);
                item->setText(FlatCol::Name, newName);
            }
        }
    }

    ui->treeFlatpak->setUpdatesEnabled(true);
}

void MainWindow::setConnections() const
{
    connect(QApplication::instance(), &QApplication::aboutToQuit, this, &MainWindow::cleanup, Qt::QueuedConnection);
    // Connect search boxes
    connect(ui->searchPopular, &QLineEdit::textChanged, this, &MainWindow::findPopular);
    connect(ui->searchBoxEnabled, &QLineEdit::textChanged, this, &MainWindow::findPackage);
    connect(ui->searchBoxMX, &QLineEdit::textChanged, this, &MainWindow::findPackage);
    connect(ui->searchBoxBP, &QLineEdit::textChanged, this, &MainWindow::findPackage);
    connect(ui->searchBoxFlatpak, &QLineEdit::textChanged, this, &MainWindow::findPackage);

    // Connect combo filters
    connect(ui->comboFilterEnabled, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);
    connect(ui->comboFilterMX, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);
    connect(ui->comboFilterBP, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);
    connect(ui->comboFilterFlatpak, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);

    // Connect other UI elements to their respective slots
    connect(ui->checkHideLibs, &QCheckBox::toggled, this, &MainWindow::checkHideLibs_toggled);
    connect(ui->checkHideLibsBP, &QCheckBox::clicked, this, &MainWindow::checkHideLibsBP_clicked);
    connect(ui->checkHideLibsMX, &QCheckBox::clicked, this, &MainWindow::checkHideLibsMX_clicked);
    connect(ui->comboRemote, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::comboRemote_activated);
    connect(ui->comboUser, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::comboUser_currentIndexChanged);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::lineEdit_returnPressed);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushCancel, &QPushButton::clicked, this, &MainWindow::pushCancel_clicked);
    connect(ui->pushEnter, &QPushButton::clicked, this, &MainWindow::pushEnter_clicked);
    connect(ui->pushForceUpdateBP, &QPushButton::clicked, this, &MainWindow::pushForceUpdateBP_clicked);
    connect(ui->pushForceUpdateEnabled, &QPushButton::clicked, this, &MainWindow::pushForceUpdateEnabled_clicked);
    connect(ui->pushForceUpdateMX, &QPushButton::clicked, this, &MainWindow::pushForceUpdateMX_clicked);
    connect(ui->pushForceUpdateFP, &QPushButton::clicked, this, &MainWindow::pushForceUpdateFP_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushInstall, &QPushButton::clicked, this, &MainWindow::pushInstall_clicked);
    connect(ui->pushRemotes, &QPushButton::clicked, this, &MainWindow::pushRemotes_clicked);
    connect(ui->pushRemoveAutoremovable, &QPushButton::clicked, this, &MainWindow::pushRemoveAutoremovable_clicked);
    connect(ui->pushRemoveUnused, &QPushButton::clicked, this, &MainWindow::pushRemoveUnused_clicked);
    connect(ui->pushUninstall, &QPushButton::clicked, this, &MainWindow::pushUninstall_clicked);
    connect(ui->pushUpgradeAll, &QPushButton::clicked, this, &MainWindow::pushUpgradeAll_clicked);
    connect(ui->pushUpgradeFP, &QPushButton::clicked, this, &MainWindow::pushUpgradeFP_clicked);
    connect(ui->tabWidget, QOverload<int>::of(&QTabWidget::currentChanged), this,
            &MainWindow::tabWidget_currentChanged);
    // Header checkbox (Upgradable): select all
    connect(headerEnabled, &CheckableHeaderView::toggled, this, &MainWindow::selectAllUpgradable_toggled);
    connect(headerMX, &CheckableHeaderView::toggled, this, &MainWindow::selectAllUpgradable_toggled);
    connect(headerBP, &CheckableHeaderView::toggled, this, &MainWindow::selectAllUpgradable_toggled);
    connect(ui->treeBackports, &QTreeWidget::itemChanged, this, &MainWindow::treeBackports_itemChanged);
    connect(ui->treeEnabled, &QTreeWidget::itemChanged, this, &MainWindow::treeEnabled_itemChanged);
    connect(ui->treeFlatpak, &QTreeWidget::itemChanged, this, &MainWindow::treeFlatpak_itemChanged);
    connect(ui->treeMXtest, &QTreeWidget::itemChanged, this, &MainWindow::treeMXtest_itemChanged);
    connect(ui->treePopularApps, &QTreeWidget::customContextMenuRequested, this,
            &MainWindow::treePopularApps_customContextMenuRequested);
    connect(ui->treePopularApps, &QTreeWidget::itemChanged, this, &MainWindow::treePopularApps_itemChanged);
    connect(ui->treePopularApps, &QTreeWidget::itemCollapsed, this, &MainWindow::treePopularApps_itemCollapsed);
    connect(ui->treePopularApps, &QTreeWidget::itemExpanded, this, &MainWindow::treePopularApps_expanded);
    connect(ui->treePopularApps, &QTreeWidget::itemExpanded, this, &MainWindow::treePopularApps_itemExpanded);
}

void MainWindow::setProgressDialog()
{
    progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);
    bar->setMaximum(bar->maximum());
    pushCancel = new QPushButton(tr("Cancel"));
    connect(pushCancel, &QPushButton::clicked, this, &MainWindow::cancelDownload);
    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
    progress->setCancelButton(pushCancel);
    pushCancel->setDisabled(true);
    progress->setLabelText(tr("Please wait..."));
    progress->setAutoClose(false);
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->reset();
}

void MainWindow::setSearchFocus() const
{
    static const QMap<int, QLineEdit *> searchBoxMap {{Tab::EnabledRepos, ui->searchBoxEnabled},
                                                      {Tab::Test, ui->searchBoxMX},
                                                      {Tab::Backports, ui->searchBoxBP},
                                                      {Tab::Flatpak, ui->searchBoxFlatpak},
                                                      {Tab::Popular, ui->searchPopular}};
    const auto index = ui->tabWidget->currentIndex();
    if (auto *searchBox = searchBoxMap.value(index)) {
        searchBox->setFocus();
    }
}

void MainWindow::displayPopularApps() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    // Optimization: Pre-cache icons to avoid repeated QIcon::fromTheme() calls
    static const QIcon folderIcon = QIcon::fromTheme("folder");
    static const QIcon infoIcon = QIcon::fromTheme("dialog-information");

    // Pre-create bold font to avoid recreating it in loop
    static const QFont boldFont = []() {
        QFont font;
        font.setBold(true);
        return font;
    }();

    QHash<QString, QTreeWidgetItem *> categoryMap; // Use QHash instead of QMap for O(1) lookup
    ui->treePopularApps->setUpdatesEnabled(false);
    ui->treePopularApps->setSortingEnabled(false); // Disable sorting during population

    // Pre-allocate space for categories
    categoryMap.reserve(20); // Estimate reasonable number of categories

    for (const auto &item : popularApps) {
        QTreeWidgetItem *topLevelItem = nullptr;

        // Check if the category already exists, if not, create it
        if (!categoryMap.contains(item.category)) {
            topLevelItem = new QTreeWidgetItem();
            topLevelItem->setText(PopCol::Icon, item.category);
            ui->treePopularApps->addTopLevelItem(topLevelItem);

            topLevelItem->setFont(PopCol::Icon, boldFont);
            topLevelItem->setIcon(PopCol::Icon, folderIcon);
            topLevelItem->setFirstColumnSpanned(true);

            categoryMap.insert(item.category, topLevelItem);
        } else {
            topLevelItem = categoryMap.value(item.category);
        }

        // Add package name as childItem to treePopularApps
        auto *childItem = new QTreeWidgetItem(topLevelItem);
        childItem->setText(PopCol::Name, item.name);
        childItem->setIcon(PopCol::Info, infoIcon);
        childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
        childItem->setCheckState(PopCol::Check, Qt::Unchecked);
        childItem->setText(PopCol::Description, item.description);
        childItem->setText(PopCol::InstallNames, item.installNames);

        childItem->setData(PopCol::UninstallNames, Qt::UserRole, item.uninstallNames);
        childItem->setData(PopCol::Screenshot, Qt::UserRole, item.screenshot);
        childItem->setData(PopCol::PostUninstall, Qt::UserRole, item.postUninstall);
        childItem->setData(PopCol::PreUninstall, Qt::UserRole, item.preUninstall);
        if (checkInstalled(item.uninstallNames)) {
            childItem->setIcon(PopCol::Check, qiconInstalled);
        }
    }

    // Optimize: Enable sorting and do a single sort at the end
    ui->treePopularApps->setSortingEnabled(true);
    ui->treePopularApps->sortItems(PopCol::Icon, Qt::AscendingOrder);

    // Optimize: Resize columns only once at the end
    for (int i = 0; i < ui->treePopularApps->columnCount(); ++i) {
        ui->treePopularApps->resizeColumnToContents(i);
    }

    connect(ui->treePopularApps, &QTreeWidget::itemClicked, this, &MainWindow::displayPopularInfo,
            Qt::UniqueConnection);
    ui->treePopularApps->setUpdatesEnabled(true);
}

void MainWindow::displayFilteredFP(QStringList list, bool raw)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->treeFlatpak->blockSignals(true);
    ui->treeFlatpak->setUpdatesEnabled(false);

    auto normalizeRef = [](const QString &line) {
        const RemoteLsEntry entry = parseRemoteLsLine(line);
        QString ref = entry.ref.trimmed();
        if (ref.startsWith("app/") || ref.startsWith("runtime/")) {
            ref = ref.section('/', 1); // Strip leading type segment (app/runtime)
        }
        return ref;
    };

    QMutableStringListIterator i(list);
    if (raw) { // Raw format that needs to be edited
        while (i.hasNext()) {
            i.setValue(normalizeRef(i.next()));
        }
    }

    auto normalizeForMatch = [](const QString &ref) { return canonicalFlatpakRef(ref); };

    QSet<QString> refSet;
    for (const QString &ref : std::as_const(list)) {
        refSet.insert(normalizeForMatch(ref));
    }

    uint total = 0;
    for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
        const QString storedCanonical = (*it)->data(FlatCol::FullName, Qt::UserRole + 1).toString();
        const QString itemRef = normalizeForMatch(
            !storedCanonical.isEmpty() ? storedCanonical : (*it)->data(FlatCol::FullName, Qt::UserRole).toString());
        if (refSet.contains(itemRef)) {
            ++total;
            (*it)->setHidden(false);
            (*it)->setData(0, Qt::UserRole, true); // Displayed flag
            if ((*it)->checkState(FlatCol::Check) == Qt::Checked
                && (*it)->data(FlatCol::Status, Qt::UserRole) == Status::Installed) {
                ui->pushUninstall->setEnabled(true);
                ui->pushInstall->setEnabled(false);
            } else {
                ui->pushUninstall->setEnabled(false);
                ui->pushInstall->setEnabled(true);
            }
        } else {
            (*it)->setHidden(true);
            (*it)->setData(0, Qt::UserRole, false); // Displayed flag
            if ((*it)->checkState(FlatCol::Check) == Qt::Checked) {
                (*it)->setCheckState(FlatCol::Check, Qt::Unchecked); // Uncheck hidden item
                changeList.removeOne((*it)->data(FlatCol::FullName, Qt::UserRole).toString());
            }
        }
        if (changeList.isEmpty()) { // Reset comboFilterFlatpak if nothing is selected
            ui->pushUninstall->setEnabled(false);
            ui->pushInstall->setEnabled(false);
        }
    }
    if (lastItemClicked) {
        ui->treeFlatpak->scrollToItem(lastItemClicked);
    }
    ui->labelNumAppFP->setText(QString::number(total));
    ui->treeFlatpak->blockSignals(false);
    blockInterfaceFP(false);
    ui->treeFlatpak->setUpdatesEnabled(true);

    // Auto-adjust column widths after filter changes for Flatpak tab
    for (int i = 0; i < ui->treeFlatpak->columnCount(); ++i) {
        ui->treeFlatpak->resizeColumnToContents(i);
    }
}

void MainWindow::displayPackages()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    displayPackagesIsRunning = true;

    auto *newTree = getCurrentTree();
    auto *list = getCurrentList();

    if (!newTree || !list) {
        displayPackagesIsRunning = false;
        emit displayPackagesFinished();
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    newTree->setUpdatesEnabled(false);
    newTree->blockSignals(true);

    newTree->clear();
    newTree->setSortingEnabled(false);
    newTree->addTopLevelItems(createTreeItemsList(list));
    newTree->sortItems(TreeCol::Name, Qt::AscendingOrder);

    updateTreeItems(newTree);
    QMetaObject::invokeMethod(this, [this, newTree] { displayAutoremovable(newTree); }, Qt::QueuedConnection);

    newTree->blockSignals(false);
    newTree->setUpdatesEnabled(true);
    QApplication::restoreOverrideCursor();

    displayPackagesIsRunning = false;
    emit displayPackagesFinished();
}

void MainWindow::displayAutoremovable(const QTreeWidget *newTree)
{
    if (!newTree || newTree == ui->treePopularApps || newTree == ui->treeFlatpak) {
        return;
    }
    QStringList names
        = cmd.getOut("LANG=C apt-get --dry-run autoremove | grep -Po '^Remv \\K[^ ]+'").split('\n', Qt::SkipEmptyParts);

    ui->pushRemoveAutoremovable->setVisible(!names.isEmpty());
    if (names.isEmpty()) {
        return;
    }

    QSet<QString> nameSet(names.begin(), names.end());
    for (QTreeWidgetItemIterator it(const_cast<QTreeWidget *>(newTree)); *it; ++it) {
        if (nameSet.contains((*it)->text(TreeCol::Name))) {
            (*it)->setData(TreeCol::Status, Qt::UserRole, Status::Autoremovable);
        }
    }
}

QTreeWidget *MainWindow::getCurrentTree()
{
    const QMap<QTreeWidget *, bool *> treeMap
        = {{ui->treeMXtest, &dirtyTest}, {ui->treeBackports, &dirtyBackports}, {ui->treeEnabled, &dirtyEnabledRepos}};

    if (auto it = treeMap.find(currentTree); it != treeMap.end() && *it.value()) {
        *it.value() = false;
        return it.key();
    }

    return nullptr;
}

QHash<QString, PackageInfo> *MainWindow::getCurrentList()
{
    if (currentTree == ui->treeMXtest) {
        return &mxList;
    } else if (currentTree == ui->treeBackports) {
        return &backportsList;
    } else {
        return &enabledList;
    }
}

QList<QTreeWidgetItem *> MainWindow::createTreeItemsList(QHash<QString, PackageInfo> *list) const
{
    QList<QTreeWidgetItem *> items;
    items.reserve(list->size() + installedPackages.size());

    for (auto it = list->constBegin(); it != list->constEnd(); ++it) {
        items.append(createTreeItem(it.key(), it.value().version, it.value().description));
    }

    for (auto it = installedPackages.constBegin(); it != installedPackages.constEnd(); ++it) {
        if (!list->contains(it.key())) {
            items.append(createTreeItem(it.key(), QString(), it.value().description));
        }
    }

    return items;
}

void MainWindow::updateTreeItems(QTreeWidget *tree)
{
    tree->setUpdatesEnabled(false);

    const bool hideLibraries = ui->checkHideLibs->isChecked();
    const auto installedVersions = listInstalledVersions();

    // Optimization: Pre-cache VersionNumber objects for repo versions to avoid repeated parsing
    QHash<QString, VersionNumber> repoVersionCache;
    repoVersionCache.reserve(tree->topLevelItemCount() * 2);

    for (QTreeWidgetItemIterator it(tree); *it; ++it) {
        auto *item = *it;
        const QString &appName = item->text(TreeCol::Name);

        if (hideLibraries && isFilteredName(appName)) {
            item->setHidden(true);
            continue; // Skip further processing for hidden items
        }

        // Get installed version information
        const VersionNumber installedVersion = installedVersions.value(appName);
        const QString installedVersionStr = installedVersion.toString();

        // Update installed version text only if changed
        if (!installedVersionStr.isEmpty() && item->text(TreeCol::InstalledVersion) != installedVersionStr) {
            item->setText(TreeCol::InstalledVersion, installedVersionStr);
        }

        // Set status based on installation state
        if (installedVersionStr.isEmpty()) {
            item->setData(TreeCol::Status, Qt::UserRole, Status::NotInstalled);
        } else {
            // Optimization: Cache VersionNumber objects for repo versions
            const QString repoVersionStr = item->text(TreeCol::RepoVersion);
            VersionNumber repoVersion;

            auto cacheIt = repoVersionCache.find(repoVersionStr);
            if (cacheIt != repoVersionCache.end()) {
                repoVersion = cacheIt.value();
            } else {
                repoVersion = VersionNumber(repoVersionStr);
                repoVersionCache.insert(repoVersionStr, repoVersion);
            }

            // Compare versions and set appropriate icon
            const bool isUpToDate = installedVersion >= repoVersion;
            item->setIcon(TreeCol::Check, isUpToDate ? qiconInstalled : qiconUpgradable);
            item->setData(TreeCol::Status, Qt::UserRole, isUpToDate ? Status::Installed : Status::Upgradable);
        }
    }

    // Optimization: Defer column resizing until the end and only resize visible columns
    tree->setUpdatesEnabled(true); // Enable updates first so resizing is efficient

    for (int i = 0; i < tree->columnCount(); ++i) {
        if (!tree->isColumnHidden(i)) {
            tree->resizeColumnToContents(i);
        }
    }
}

void MainWindow::displayFlatpaks(bool force_update)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (!flatpaks.isEmpty() && !force_update) {
        return;
    }

    setupFlatpakDisplay();
    loadFlatpakData();
    populateFlatpakTree();
    finalizeFlatpakDisplay();
}

void MainWindow::setupFlatpakDisplay()
{
    ui->treeFlatpak->setUpdatesEnabled(false);
    displayFlatpaksIsRunning = true;
    lastItemClicked = nullptr;

    const bool isCurrentTabFlatpak = ui->tabWidget->currentIndex() == Tab::Flatpak;
    if (isCurrentTabFlatpak) {
        setCursor(QCursor(Qt::BusyCursor));
        if (!flatpakCancelHidden && pushCancel) {
            pushCancel->setEnabled(false);
            pushCancel->hide();
            flatpakCancelHidden = true;
        }
        progress->show();
        if (!timer.isActive()) {
            timer.start(100ms);
        }
    }

    listFlatpakRemotes();
    ui->treeFlatpak->blockSignals(true);
    ui->treeFlatpak->clear();
    changeList.clear();
    blockInterfaceFP(true);
}

void MainWindow::loadFlatpakData()
{
    flatpaks = listFlatpaks(ui->comboRemote->currentText());
    flatpaksApps.clear();
    flatpaksRuntimes.clear();
    QSet<QString> installedCanonical;
    cachedInstalledFlatpaks.clear();
    cachedInstalledSizeMap.clear();
    cachedInstalledScope.clear();
    cachedInstalledFetched = false;

    // Optimize: Get all installed packages with one command (ref + size), then split by type
    const QString allInstalledCommand = "flatpak list " + fpUser + "2>/dev/null --columns=ref,size";
    QScopedValueRollback<bool> guard(suppressCmdOutput, true);
    const QStringList allInstalled = cmd.getOut(allInstalledCommand, Cmd::QuietMode::No).split('\n', Qt::SkipEmptyParts);
    cachedInstalledFlatpaks = allInstalled;
    cachedInstalledScope = fpUser;
    cachedInstalledFetched = true;

    // Clear and reserve space for better performance
    installedAppsFP.clear();
    installedRuntimesFP.clear();
    installedAppsFP.reserve(allInstalled.size() / 2);
    installedRuntimesFP.reserve(allInstalled.size() / 2);

    // Split by type based on flatpak naming convention
    for (const QString &itemRaw : allInstalled) {
        if (itemRaw.startsWith("Ref")) { // header row on some versions
            continue;
        }

        const ParsedFlatpakRef parsed = parseInstalledFlatpakLine(itemRaw.section('\t', 0, 0));
        if (parsed.ref.isEmpty()) {
            continue;
        }

        const QString canonicalRef = canonicalFlatpakRef(parsed.ref);
        if (canonicalRef.isEmpty()) {
            continue;
        }
        installedCanonical.insert(canonicalRef);

        const QString sizeStr = itemRaw.section('\t', 1);
        if (!sizeStr.isEmpty()) {
            cachedInstalledSizeMap.insert(canonicalRef, sizeStr);
        }

        if (parsed.isRuntime) {
            installedRuntimesFP.append(canonicalRef);
        } else {
            installedAppsFP.append(canonicalRef);
        }
    }

    // Ensure installed refs are present in the display even if missing from remote listings
    QSet<QString> listedCanonicalRefs;
    for (const QString &entry : std::as_const(flatpaks)) {
        const RemoteLsEntry parsed = parseRemoteLsLine(entry);
        const QString canonical = canonicalFlatpakRef(parsed.ref);
        if (!canonical.isEmpty()) {
            listedCanonicalRefs.insert(canonical);
        }
    }

    const auto buildEntry = [](const QString &ref) {
        const QString branch = ref.section('/', -1);
        const QString version = branch;
        return version + '\t' + branch + '\t' + ref + '\t';
    };

    for (const QString &ref : std::as_const(installedCanonical)) {
        if (!listedCanonicalRefs.contains(ref)) {
            flatpaks.append(buildEntry(ref));
        }
    }

    // Build cached app/runtime lists from the already-fetched remote data to avoid re-querying
    flatpaksApps.clear();
    flatpaksRuntimes.clear();
    for (const QString &entry : std::as_const(flatpaks)) {
        const RemoteLsEntry parsed = parseRemoteLsLine(entry);
        const QString refForType = !parsed.ref.isEmpty() ? parsed.ref : canonicalFlatpakRef(parsed.ref);
        const bool isRuntime = isRuntimeToken(refForType) || isRuntimeToken(canonicalFlatpakRef(refForType));
        (isRuntime ? flatpaksRuntimes : flatpaksApps).append(entry);
    }
}

void MainWindow::populateFlatpakTree()
{
    const QStringList installed_all = installedAppsFP + installedRuntimesFP;
    uint total_count = 0;

    for (const QString &item : std::as_const(flatpaks)) {
        if (createFlatpakItem(item, installed_all)) {
            ++total_count;
        }
    }

    updateFlatpakCounts(total_count);
    formatFlatpakTree();
}

QTreeWidgetItem *MainWindow::createFlatpakItem(const QString &item, const QStringList &installed_all) const
{
    const RemoteLsEntry entry = parseRemoteLsLine(item);

    const QString originalRef = entry.ref.trimmed();
    QString ref = originalRef;
    const QString branch = entry.branch.isEmpty() ? ref.section('/', -1) : entry.branch;
    QString version = entry.version;
    if (version.isEmpty() || version.contains('/')) {
        version = branch;
    }
    const QString size = entry.size;
    const QString canonicalRef = canonicalFlatpakRef(ref);
    if (canonicalRef.isEmpty()) {
        return nullptr;
    }
    const QString long_name = canonicalRef.section('/', 0, 0);
    const QString short_name = long_name.section('.', -1);
    const QString name = canonicalRef;

    // Skip unwanted packages
    static const QSet<QString> unwantedPackages
        = {QLatin1String("Locale"), QLatin1String("Sources"), QLatin1String("Debug")};
    if (unwantedPackages.contains(short_name)) {
        return nullptr;
    }

    auto *widget_item = new QTreeWidgetItem(ui->treeFlatpak);
    widget_item->setCheckState(FlatCol::Check, Qt::Unchecked);
    widget_item->setText(FlatCol::Name, short_name);
    widget_item->setText(FlatCol::LongName, long_name);
    widget_item->setText(FlatCol::Version, version);
    widget_item->setText(FlatCol::Branch, branch);
    widget_item->setText(FlatCol::Size, size);
    widget_item->setData(FlatCol::FullName, Qt::UserRole, originalRef.isEmpty() ? name : originalRef);
    widget_item->setData(FlatCol::FullName, Qt::UserRole + 1, name); // canonical for matching
    widget_item->setData(0, Qt::UserRole, true);

    if (installed_all.contains(name)) {
        widget_item->setIcon(FlatCol::Check, QIcon::fromTheme("package-installed-updated",
                                                              QIcon(":/icons/package-installed-updated.png")));
        widget_item->setData(FlatCol::Status, Qt::UserRole, Status::Installed);
    } else {
        widget_item->setData(FlatCol::Status, Qt::UserRole, Status::NotInstalled);
    }

    return widget_item;
}

void MainWindow::updateFlatpakCounts(uint total_count)
{
    listSizeInstalledFP();
    ui->labelNumAppFP->setText(QString::number(total_count));
    ui->labelNumInstFP->setText(QString::number(!installedAppsFP.isEmpty() ? installedAppsFP.count() : 0));
}

void MainWindow::formatFlatpakTree() const
{
    ui->treeFlatpak->sortByColumn(FlatCol::Name, Qt::AscendingOrder);
    removeDuplicatesFP();

    for (int i = 0; i < ui->treeFlatpak->columnCount(); ++i) {
        ui->treeFlatpak->resizeColumnToContents(i);
    }
}

void MainWindow::finalizeFlatpakDisplay()
{
    ui->treeFlatpak->blockSignals(false);

    const bool isCurrentTabFlatpak = ui->tabWidget->currentIndex() == Tab::Flatpak;
    if (isCurrentTabFlatpak) {
        if (!ui->comboFilterFlatpak->currentText().isEmpty()) {
            filterChanged(ui->comboFilterFlatpak->currentText());
        }
        ui->searchBoxFlatpak->setFocus();
    }

    displayFlatpaksIsRunning = false;
    firstRunFP = false;
    blockInterfaceFP(false);
    if (holdProgressForFlatpakRefresh) {
        holdProgressForFlatpakRefresh = false;
        progress->hide();
    }
    if (flatpakCancelHidden && pushCancel) {
        pushCancel->show();
        pushCancel->setEnabled(true);
        flatpakCancelHidden = false;
    }
    ui->treeFlatpak->setUpdatesEnabled(true);
}

void MainWindow::displayWarning(const QString &repo)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    bool *displayed = nullptr;
    QString msg;
    QString key;

    if (repo == "test") {
        displayed = &warningTest;
        key = "NoWarningTest";
        msg = tr("You are about to use the MX Test repository, whose packages are provided for "
                 "testing purposes only. It is possible that they might break your system, so it "
                 "is suggested that you back up your system and install or update only one package "
                 "at a time. Please provide feedback in the Forum so the package can be evaluated "
                 "before moving up to Main.");

    } else if (repo == "backports") {
        displayed = &warningBackports;
        key = "NoWarningBackports";
        msg = tr("You are about to use Debian Backports, which contains packages taken from the next "
                 "Debian release (called 'testing'), adjusted and recompiled for usage on Debian stable. "
                 "They cannot be tested as extensively as in the stable releases of Debian and MX Linux, "
                 "and are provided on an as-is basis, with risk of incompatibilities with other components "
                 "in Debian stable. Use with care!");
    } else if (repo == "flatpaks") {
        displayed = &warningFlatpaks;
        key = "NoWarningFlatpaks";
        msg = tr("MX Linux includes this repository of flatpaks for the users' convenience only, and "
                 "is not responsible for the functionality of the individual flatpaks themselves. "
                 "For more, consult flatpaks in the Wiki.");
    }
    if ((displayed == nullptr) || *displayed || settings.value(key, false).toBool()) {
        return;
    }

    QMessageBox msgBox(QMessageBox::Warning, tr("Warning"), msg);
    msgBox.addButton(QMessageBox::Close);
    auto *cb = new QCheckBox();
    msgBox.setCheckBox(cb);
    cb->setText(tr("Do not show this message again"));
    connect(cb, &QCheckBox::clicked, this, [this, key, cb] { settings.setValue(key, cb->isChecked()); });
    msgBox.exec();
    *displayed = true;
}

void MainWindow::ifDownloadFailed() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    progress->hide();
}

void MainWindow::invalidateFlatpakRemoteCache()
{
    cachedFlatpakRemotes.clear();
    cachedFlatpakRemotesScope.clear();
    cachedFlatpakRemotesFetched = false;
}

void MainWindow::listFlatpakRemotes() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QString currentRemote = ui->comboRemote->currentText();
    ui->comboRemote->blockSignals(true);
    ui->comboRemote->clear();
    const bool isUserScope = fpUser.startsWith("--user");

    auto applyRemotes = [&](const QStringList &list) {
        ui->comboRemote->addItems(list);
        QString savedRemote = firstRunFP ? settings.value("FlatpakRemote", "flathub").toString() : currentRemote;
        ui->comboRemote->setCurrentText(savedRemote.isEmpty() ? "flathub" : savedRemote);
        ui->comboRemote->blockSignals(false);
    };

    if (cachedFlatpakRemotesFetched && cachedFlatpakRemotesScope == fpUser) {
        applyRemotes(cachedFlatpakRemotes);
        return;
    }

    auto fetchRemotes = [this](QStringList &outList) {
        Cmd shell;
        outList
            = shell.getOut("flatpak remote-list " + fpUser + "| cut -f1").remove(' ').split('\n', Qt::SkipEmptyParts);
        return shell.exitCode() == 0;
    };

    auto addUserRemotes = []() {
        Cmd addRemotes;
        return addRemotes.run("/usr/lib/mx-packageinstaller/mxpi-lib flatpak_add_repos_user", Cmd::QuietMode::Yes);
    };

    QStringList list;
    bool listOk = fetchRemotes(list);

    // If user scope listing failed (common when user has never set up flatpak), attempt to set up defaults
    if (!listOk && isUserScope) {
        qDebug() << "User remote-list failed; attempting to set up user remotes";
        if (addUserRemotes()) {
            listOk = fetchRemotes(list);
        }
    }

    // If no user remotes exist, set up the default ones
    if (list.isEmpty() && isUserScope) {
        qDebug() << "No flatpak remotes found for user, setting up default remotes";

        if (addUserRemotes()) {
            qDebug() << "Successfully set up flatpak remotes for user";

            // Re-fetch the remote list after setup
            listOk = fetchRemotes(list);
        } else {
            qDebug() << "Failed to set up flatpak remotes for user";
        }
    }

    if (!listOk) {
        ui->comboRemote->blockSignals(false);
        return;
    }

    cachedFlatpakRemotes = list;
    cachedFlatpakRemotesScope = fpUser;
    cachedFlatpakRemotesFetched = true;

    applyRemotes(list);
}

bool MainWindow::confirmActions(const QString &names, const QString &action)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    qDebug() << "names" << names << "and" << changeList;
    QString msg;

    QString detailed_names;
    QStringList detailed_installed_names;
    QString detailed_to_install;
    QString detailed_removed_names;
    QString recommends;
    QString recommends_aptitude;
    QString aptitude_info;

    const QString frontend {"DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null | grep -sq ^i && echo "
                            "kde || echo gnome) LANG=C "};
    const QString aptget {"apt-get -s -V -o=Dpkg::Use-Pty=0 "};
    const QString aptitude {"aptitude -sy -V -o=Dpkg::Use-Pty=0 "};
    if (currentTree == ui->treeFlatpak && names != "flatpak") {
        detailed_installed_names = changeList;
    } else if (currentTree == ui->treeBackports) {
        recommends
            = (ui->checkBoxInstallRecommendsBP->isChecked()) ? "--install-recommends " : "--no-install-recommends ";
        recommends_aptitude
            = (ui->checkBoxInstallRecommendsBP->isChecked()) ? "--with-recommends " : "--without-recommends ";
        detailed_names = cmd.getOutAsRoot(
            frontend + aptget + action + ' ' + recommends + "-t " + verName + "-backports --reinstall " + names
            + R"lit(|grep 'Inst\|Remv' | awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info = cmd.getOutAsRoot(frontend + aptitude + action + ' ' + recommends_aptitude + "-t " + verName
                                         + "-backports " + names + " |tail -2 |head -1");
    } else if (currentTree == ui->treeMXtest) {
        recommends
            = (ui->checkBoxInstallRecommendsMX->isChecked()) ? "--install-recommends " : "--no-install-recommends ";
        recommends_aptitude
            = (ui->checkBoxInstallRecommendsMX->isChecked()) ? "--with-recommends " : "--without-recommends ";
        detailed_names = cmd.getOutAsRoot(
            frontend + aptget + action + " -t mx " + recommends + "--reinstall " + names
            + R"lit(|grep 'Inst\|Remv' | awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info = cmd.getOutAsRoot(frontend + aptitude + action + " -t mx " + recommends_aptitude + names
                                         + " |tail -2 |head -1");
    } else {
        recommends
            = (ui->checkBoxInstallRecommends->isChecked()) ? "--install-recommends " : "--no-install-recommends ";
        recommends_aptitude
            = (ui->checkBoxInstallRecommends->isChecked()) ? "--with-recommends " : "--without-recommends ";
        detailed_names = cmd.getOutAsRoot(
            frontend + aptget + action + ' ' + recommends + "--reinstall " + names
            + R"lit(|grep 'Inst\|Remv'| awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info
            = cmd.getOutAsRoot(frontend + aptitude + action + ' ' + recommends_aptitude + names + " |tail -2 |head -1");
    }

    if (currentTree != ui->treeFlatpak) {
        detailed_installed_names = detailed_names.split('\n');
    }

    detailed_installed_names.sort();
    qDebug() << "detailed installed names sorted " << detailed_installed_names;
    QStringListIterator iterator(detailed_installed_names);

    if (currentTree != ui->treeFlatpak) {
        while (iterator.hasNext()) {
            QString value = iterator.next();
            if (value.contains(QLatin1String("Remv"))) {
                value = value.section(';', 0, 0) + ' ' + value.section(';', 1, 1);
                detailed_removed_names = detailed_removed_names + value + '\n';
            }
            if (value.contains(QLatin1String("Inst"))) {
                value = value.section(';', 0, 0) + ' ' + value.section(';', 1, 1);
                detailed_to_install = detailed_to_install + value + '\n';
            }
        }
        if (!detailed_removed_names.isEmpty()) {
            detailed_removed_names.prepend(tr("Remove") + '\n');
        }
        if (!detailed_to_install.isEmpty()) {
            detailed_to_install.prepend(tr("Install") + '\n');
        }
    } else {
        if (action == QLatin1String("remove")) {
            detailed_removed_names = changeList.join('\n');
            detailed_to_install.clear();
        }
        if (action == QLatin1String("install")) {
            detailed_to_install = changeList.join('\n');
            detailed_removed_names.clear();
        }
    }

    msg = "<b>" + tr("The following packages were selected. Click Show Details for list of changes.") + "</b>";

    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.setInformativeText('\n' + names + "\n\n" + aptitude_info);

    if (action == QLatin1String("install")) {
        msgBox.setDetailedText(detailed_to_install + '\n' + detailed_removed_names);
    } else {
        msgBox.setDetailedText(detailed_removed_names + '\n' + detailed_to_install);
    }

    // Find Detailed Info box and set heigth, set box height between 100 - 400 depending on length of content
    const auto min = 100;
    const auto max = 400;
    auto *const detailedInfo = msgBox.findChild<QTextEdit *>();
    const auto recommended = qMax(msgBox.detailedText().length() / 2, min); // Half of length is just guesswork
    const auto height = qMin(recommended, max);
    detailedInfo->setFixedHeight(height);

    msgBox.addButton(QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);

    const auto width = 600;
    auto *horizontalSpacer = new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto *layout = qobject_cast<QGridLayout *>(msgBox.layout());
    layout->addItem(horizontalSpacer, 0, 1);
    return msgBox.exec() == QMessageBox::Ok;
}

bool MainWindow::install(const QString &names)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    if (!isOnline()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Internet is not available, won't be able to download the list of packages"));
        return false;
    }
    ui->tabWidget->setTabText(Tab::Output, tr("Installing packages..."));

    // Simulate install of selections and present for confirmation
    // if user selects cancel, break routine but return success to avoid error message
    if (!confirmActions(names, "install")) {
        return true;
    }
    enableOutput();
    QString frontend {
        "DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null | grep -sq ^i && echo kde || echo gnome) "};
    QString aptget {"apt-get -o=Dpkg::Use-Pty=0 install -y "};

    if (lockFile.isLockedGUI()) {
        return false;
    }
    QString recommends;
    bool success = false;
    if (currentTree == ui->treeBackports) {
        recommends
            = (ui->checkBoxInstallRecommendsBP->isChecked()) ? "--install-recommends " : "--no-install-recommends ";
        success = cmd.runAsRoot(frontend + aptget + recommends + "-t " + verName + "-backports --reinstall " + names);
    } else if (currentTree == ui->treeMXtest) {
        recommends
            = (ui->checkBoxInstallRecommendsMX->isChecked()) ? "--install-recommends " : "--no-install-recommends ";
        success = cmd.runAsRoot(frontend + aptget + recommends + " -t mx " + names);
    } else {
        recommends
            = (ui->checkBoxInstallRecommends->isChecked()) ? "--install-recommends " : "--no-install-recommends ";
        success = cmd.runAsRoot(frontend + aptget + recommends + "--reinstall " + names);
    }
    return success;
}

// Install a list of application and run postprocess for each of them.
bool MainWindow::installBatch(const QStringList &name_list)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    bool result = true;
    QString postinstall;
    QString install_names;

    for (const QString &name : name_list) {
        for (const auto &item : std::as_const(popularApps)) {
            if (item.name == name) {
                postinstall += item.postInstall + '\n';
                install_names += item.installNames + ' ';
            }
        }
    }

    if (!install_names.isEmpty()) {
        if (!install(install_names)) {
            result = false;
        }
    }

    if (postinstall != '\n') {
        qDebug() << "Post-install";
        ui->tabWidget->setTabText(Tab::Output, tr("Post-processing..."));
        if (lockFile.isLockedGUI()) {
            return false;
        }
        enableOutput();
        if (!cmd.runAsRoot(postinstall)) {
            result = false;
        }
    }
    return result;
}

bool MainWindow::installPopularApp(const QString &name)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    bool result = true;
    QString preinstall;
    QString postinstall;
    QString install_names;

    // Get all the app info
    for (const auto &item : std::as_const(popularApps)) {
        if (item.name == name) {
            preinstall = item.preInstall;
            postinstall = item.postInstall;
            install_names = item.installNames;
        }
    }
    enableOutput();
    // Preinstall
    if (!preinstall.isEmpty()) {
        qDebug() << "Pre-install";
        ui->tabWidget->setTabText(Tab::Output, tr("Pre-processing for ") + name);
        if (lockFile.isLockedGUI()) {
            return false;
        }
        if (!cmd.runAsRoot(preinstall)) {
            if (QFile::exists(tempList)) {
                Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", Cmd::QuietMode::Yes);
                updateApt();
            }
            return false;
        }
    }
    // Install
    if (!install_names.isEmpty()) {
        ui->tabWidget->setTabText(Tab::Output, tr("Installing ") + name);
        result = install(install_names);
    }
    enableOutput();
    // Postinstall
    if (!postinstall.isEmpty()) {
        qDebug() << "Post-install";
        ui->tabWidget->setTabText(Tab::Output, tr("Post-processing for ") + name);
        if (lockFile.isLockedGUI()) {
            return false;
        }
        cmd.runAsRoot(postinstall);
    }
    if (QFile::exists(tempList)) {
        Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", Cmd::QuietMode::Yes);
        updateApt();
    }
    return result;
}

bool MainWindow::installPopularApps()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QStringList batch_names;
    bool result = true;

    if (!isOnline()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Internet is not available, won't be able to download the list of packages"));
        return false;
    }
    if (!updatedOnce) {
        updateApt();
    }

    // Make a list of apps to be installed together
    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
            QString name = (*it)->text(2);
            for (const auto &item : std::as_const(popularApps)) {
                if (item.name == name) {
                    const QString &preinstall = item.preInstall;
                    if (preinstall.isEmpty()) { // Add to batch processing if there is no preinstall command
                        batch_names << name;
                        (*it)->setCheckState(PopCol::Check, Qt::Unchecked);
                    }
                }
            }
        }
    }
    if (!installBatch(batch_names)) {
        result = false;
    }

    // Install the rest of the apps
    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
            if (!installPopularApp((*it)->text(PopCol::Name))) {
                result = false;
            }
        }
    }
    setCursor(QCursor(Qt::ArrowCursor));

    ui->treePopularApps->clearSelection();
    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        (*it)->setCheckState(PopCol::Check, Qt::Unchecked);
    }
    return result;
}

bool MainWindow::installSelected()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setTabEnabled(Tab::Output, true);
    QString names = changeList.join(' ');

    // Change sources as needed
    if (currentTree == ui->treeMXtest) {
        // Add testrepo unless already enabled
        if (!testInitiallyEnabled) {
            QString suite = verName;
            if (arch == "amd64") {
                cmd.runAsRoot("apt-get update --print-uris | tac | "
                              "grep -m1 -oE 'https?://.*/mx/repo/dists/"
                              + suite + "/main' | sed 's:^:deb :; s:/repo/dists/:/testrepo :; s:/main: test:' > "
                              + tempList);
            } else {
                cmd.runAsRoot("apt-get update --print-uris | tac | "
                              "grep -m1 -oE 'https?://.*/mx/repo/dists/"
                              + suite
                              + "/main' | sed 's:^:deb [arch='$(dpkg --print-architecture)'] :; "
                                "s:/repo/dists/:/testrepo :; s:/main: test:' > "
                              + tempList);
            }
        }
        updateApt();
    } else if (currentTree == ui->treeBackports) {
        cmd.runAsRoot("echo deb http://ftp.debian.org/debian " + verName + "-backports main contrib non-free > "
                      + tempList);
        updateApt();
    }
    bool result = install(names);
    if (currentTree == ui->treeBackports || currentTree == ui->treeMXtest) {
        if (QFile::exists(tempList)) {
            Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", Cmd::QuietMode::Yes);
            updateApt();
        }
    }
    changeList.clear();
    installedPackages = listInstalled();
    return result;
}

bool MainWindow::markKeep()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setTabEnabled(Tab::Output, true);
    QString names = changeList.join(' ');
    enableOutput();
    return cmd.runAsRoot("apt-mark manual " + names);
}

bool MainWindow::isFilteredName(const QString &name)
{
    static const QRegularExpression filterRegex(R"((^lib(?!reoffice|rewolf))|(-dev$)|(-dbg$)|(-dbgsym$)|(-libs$))",
                                                QRegularExpression::CaseInsensitiveOption);

    return filterRegex.match(name).hasMatch();
}

bool MainWindow::isOnline()
{
    if (settings.value("skiponlinecheck", false).toBool() || args.isSet("skip-online-check")) {
        return true;
    }

    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + '/'
                                           + QApplication::applicationVersion().toUtf8() + " (linux-gnu)");

    auto error = QNetworkReply::NoError;
    for (const QString address : {"https://mxrepo.com", "https://google.com"}) {
        error = QNetworkReply::NoError; // reset for each tried address
        QNetworkProxyQuery query {QUrl(address)};
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
        if (!proxies.isEmpty()) {
            manager.setProxy(proxies.first());
        }
        request.setUrl(QUrl(address));
        reply = manager.head(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);
        auto timeout = settings.value("timeout", 7000).toUInt();
        manager.setTransferTimeout(timeout);
        loop.exec();
        reply->disconnect();
        if (reply->error() == QNetworkReply::NoError) {
            return true;
        }
    }
    qDebug() << "No network detected:" << reply->url() << error;
    return false;
}

bool MainWindow::downloadFile(const QString &url, QFile &file)
{
    qDebug() << "... downloading: " << url;
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open file:" << file.fileName();
        return false;
    }

    QNetworkRequest request {QUrl(url)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + '/'
                                           + QApplication::applicationVersion().toUtf8() + " (linux-gnu)");

    reply = manager.get(request);
    QEventLoop loop;

    connect(reply, &QNetworkReply::readyRead, this, [&file, this]() {
        if (file.write(reply->readAll()) == -1) {
            qDebug() << "Failed to write data to file:" << file.fileName();
            reply->abort();
            file.close();
            file.remove();
        }
    });
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    file.close();

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, tr("Error"),
                             tr("There was an error downloading or writing the file: %1. Please check your internet "
                                "connection and free space on your drive")
                                 .arg(file.fileName()));
        qDebug() << "There was an error downloading the file:" << url << "Error:" << reply->errorString();
        file.remove();
        reply->deleteLater();
        return false;
    }
    reply->deleteLater();
    return true;
}

bool MainWindow::downloadAndUnzip(const QString &url, QFile &file)
{
    if (!downloadFile(url, file)) {
        // Clean up both compressed and potential uncompressed files
        const QString basePath = QFileInfo(file.fileName()).path();
        const QString baseName = QFileInfo(file.fileName()).baseName();
        file.remove();
        QFile::remove(basePath + '/' + baseName);
        return false;
    }

    // Determine and execute unzip command based on file extension
    const QString fileExt = QFileInfo(file).suffix();
    const QString unzipCommand = (fileExt == "gz") ? "gunzip -f " : "unxz -f ";

    if (!cmd.run(unzipCommand + file.fileName())) {
        qDebug() << "Could not unzip file:" << file.fileName();
        file.remove();
        return false;
    }

    return true;
}

bool MainWindow::downloadAndUnzip(const QString &url, const QString &repo_name, const QString &branch,
                                  const QString &format, QFile &file)
{
    return downloadAndUnzip(url + repo_name + branch + "/binary-" + arch + "/Packages." + format, file);
}

bool MainWindow::buildPackageLists(bool forceDownload)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (forceDownload) {
        setDirty();
    }
    clearUi();
    if (!downloadPackageList(forceDownload)) {
        ifDownloadFailed();
        return false;
    }
    if (!readPackageList(forceDownload)) {
        ifDownloadFailed();
        return false;
    }
    displayPackages();
    return true;
}

// Download the Packages.gz from sources
bool MainWindow::downloadPackageList(bool forceDownload)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (!isOnline()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Internet is not available, won't be able to download the list of packages"));
        return false;
    }
    if (!tempDir.isValid()) {
        qDebug() << "Can't create temp folder";
        return false;
    }
    QDir::setCurrent(tempDir.path());
    progress->setLabelText(tr("Downloading package info..."));
    pushCancel->setEnabled(true);

    auto runUpdateApt = [this, forceDownload]() {
        QScopedValueRollback<bool> holdGuard(holdProgressForAptRefresh, holdProgressForAptRefresh || forceDownload);
        const bool ok = updateApt();
        return ok;
    };

    // Handle enabled list download/update
    if (enabledList.isEmpty() || forceDownload) {
        if (forceDownload && !runUpdateApt()) {
            return false;
        }
        progress->show();
        if (!timer.isActive()) {
            timer.start(100ms);
        }
        AptCache cache;
        enabledList = cache.getCandidates();
        if (enabledList.isEmpty()) {
            runUpdateApt();
            enabledList = AptCache().getCandidates();
        }
    }

    // Handle MX test repo packages
    if (currentTree == ui->treeMXtest) {
        const QString mxPackagesPath = tempDir.path() + "/mxPackages";
        if (!QFile::exists(mxPackagesPath) || forceDownload) {
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
            }

            QFile file(mxPackagesPath + ".gz");
            QString url = getMXTestRepoUrl();
            if (!downloadAndUnzip(url, verName, "/test", "gz", file)) {
                return false;
            }
        }
    }
    // Handle backports packages
    else if (currentTree == ui->treeBackports) {
        const QStringList components = {"main", "contrib", "non-free"};
        const QString basePath = tempDir.path() + "/";
        bool needsDownload = forceDownload;

        // Check if any package files are missing
        for (const QString &component : components) {
            if (!QFile::exists(basePath + component + "Packages")) {
                needsDownload = true;
                break;
            }
        }

        if (needsDownload) {
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
            }

            // Download and process each component
            const QString url = "http://deb.debian.org/debian/dists/";
            for (const QString &component : components) {
                QFile file(basePath + component + "Packages.xz");
                const QString branch = "-backports/" + component;
                if (!downloadAndUnzip(url, verName, branch, "xz", file)) {
                    return false;
                }
            }

            // Combine all package files
            pushCancel->setDisabled(true);
            QFile outputFile("allPackages");
            if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qWarning() << "Could not open:" << outputFile.fileName();
                return false;
            }

            QTextStream outStream(&outputFile);
            bool success = true;
            for (const QString &component : components) {
                QFile inputFile(basePath + component + "Packages");
                if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qWarning() << "Could not read file:" << inputFile.fileName();
                    success = false;
                    break;
                }
                outStream << inputFile.readAll();
                inputFile.close();
            }
            outputFile.close();
            if (!success) {
                return false;
            }
        }
    }
    return true;
}

void MainWindow::enableTabs(bool enable)
{
    for (int tab = 0; tab < ui->tabWidget->count() - 1; ++tab) { // Enable all except last (Console)
        ui->tabWidget->setTabEnabled(tab, enable);
    }
    ui->tabWidget->setTabVisible(Tab::Test, QFile::exists("/etc/apt/sources.list.d/mx.list")
                                                || QFile::exists("/etc/apt/sources.list.d/mx.sources"));
    ui->tabWidget->setTabVisible(Tab::Flatpak, arch != "i386");
    setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::hideColumns() const
{
    ui->tabWidget->setCurrentIndex(Tab::Popular);
    const bool showFlatpakBranch = debianVersion < Release::Trixie;
    ui->treeFlatpak->setColumnHidden(FlatCol::Branch, !showFlatpakBranch);
    ui->treeEnabled->hideColumn(TreeCol::Status); // Status of the package: installed, upgradable, etc
    ui->treeMXtest->hideColumn(TreeCol::Status);
    ui->treeBackports->hideColumn(TreeCol::Status);
    ui->treeFlatpak->hideColumn(FlatCol::Status);
    ui->treeFlatpak->hideColumn(FlatCol::Duplicate);
    ui->treeFlatpak->hideColumn(FlatCol::FullName);
}

// Hide library packages and development files
void MainWindow::hideLibs() const
{
    if (currentTree == ui->treeFlatpak || !ui->checkHideLibs->isChecked()) {
        return;
    }
    currentTree->setUpdatesEnabled(false);
    for (QTreeWidgetItemIterator it(currentTree); *it; ++it) {
        if (isFilteredName((*it)->text(TreeCol::Name))) {
            (*it)->setHidden(true);
        }
    }
    currentTree->setUpdatesEnabled(true);
}

// Process downloaded *Packages.gz files
bool MainWindow::readPackageList(bool forceDownload)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    pushCancel->setDisabled(true);

    // Early return if lists are already populated and not forced to download
    if (!forceDownload
        && ((currentTree == ui->treeEnabled && !enabledList.isEmpty())
            || (currentTree == ui->treeMXtest && !mxList.isEmpty())
            || (currentTree == ui->treeBackports && !backportsList.isEmpty()))) {
        return true;
    }

    // treeEnabled is updated at downloadPackageList
    if (currentTree == ui->treeEnabled) {
        return true;
    }

    // Determine the file path based on the current tree
    QString filePath = tempDir.filePath((currentTree == ui->treeMXtest) ? "mxPackages" : "allPackages");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file:" << filePath;
        return false;
    }

    // Select the target package map based on the current tree
    auto &targetMap = (currentTree == ui->treeMXtest) ? mxList : backportsList;
    targetMap.clear();

    // Parse package information from the file
    QTextStream stream(&file);
    QString line, package, version, description;
    while (stream.readLineInto(&line)) {
        if (line.startsWith("Package: ")) {
            package = line.section(' ', 1);
        } else if (line.startsWith("Version: ")) {
            version = line.section(' ', 1);
        } else if (line.startsWith("Description: ")) {
            description = line.section(' ', 1);
            if (!package.isEmpty()) {
                targetMap.insert(package, {version, description});
                package.clear();
                version.clear();
                description.clear();
            }
        }
    }

    file.close();
    return true;
}

void MainWindow::cancelDownload()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    holdProgressForAptRefresh = false;
    holdProgressForFlatpakRefresh = false;
    cmd.terminate();
}

void MainWindow::centerWindow()
{
    const auto screenGeometry = QApplication::primaryScreen()->geometry();
    const auto x = (screenGeometry.width() - width()) / 2;
    const auto y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::clearUi()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    blockSignals(true);

    // Configure buttons
    ui->pushCancel->setEnabled(true);
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);

    // Clear UI elements based on the current tree
    if (currentTree == ui->treeEnabled) {
        ui->labelNumApps->clear();
        ui->labelNumInst->clear();
        ui->labelNumUpgr->clear();
        ui->treeEnabled->clear();
        ui->pushUpgradeAll->setHidden(true);
    } else if (currentTree == ui->treeMXtest) {
        ui->labelNumApps_2->clear();
        ui->labelNumInstMX->clear();
        ui->labelNumUpgrMX->clear();
        ui->treeMXtest->clear();
    } else if (currentTree == ui->treeBackports) {
        ui->labelNumApps_3->clear();
        ui->labelNumInstBP->clear();
        ui->labelNumUpgrBP->clear();
        ui->treeBackports->clear();
    }

    // Reset all filter combos
    const QList<QComboBox *> filterCombos = {ui->comboFilterBP, ui->comboFilterMX, ui->comboFilterEnabled};
    for (auto combo : filterCombos) {
        combo->setCurrentIndex(savedComboIndex);
    }
    ui->comboFilterFlatpak->setCurrentIndex(0);

    blockSignals(false);
}

void MainWindow::cleanup()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (cmd.state() != QProcess::NotRunning) {
        qDebug() << "Command" << cmd.program() << cmd.arguments() << "terminated" << cmd.terminateAndKill();
    }
    if (QFile::exists(tempList)) {
        Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", Cmd::QuietMode::Yes);
        updateApt();
    }
    Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib copy_log", Cmd::QuietMode::Yes);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("FlatpakRemote", ui->comboRemote->currentText());
    settings.setValue("FlatpakUser", ui->comboUser->currentText());
}

QString MainWindow::getVersion(const QString &name) const
{
    return Cmd().getOut("LANG=Cdpkg-query -f '${Version}' -W " + name);
}

// Return true if all the packages listed are installed
bool MainWindow::checkInstalled(const QVariant &names) const
{

    QStringList name_list;
    if (names.canConvert<QStringList>()) {
        name_list = names.toStringList();
        // Flatten any strings in the list that contain newlines
        QStringList expanded_list;
        for (const QString &name : name_list) {
            if (name.contains('\n')) {
                expanded_list.append(name.split('\n', Qt::SkipEmptyParts));
            } else {
                expanded_list.append(name);
            }
        }
        name_list = expanded_list;
    } else if (names.canConvert<QString>()) {
        name_list = names.toString().split('\n', Qt::SkipEmptyParts);
    } else {
        return false;
    }

    if (name_list.isEmpty()) {
        return false;
    }

    // Trim whitespace from all package names
    for (QString &name : name_list) {
        name = name.trimmed();
    }
    for (const QString &name : name_list) {
        if (!installedPackages.contains(name)) {
            return false;
        }
    }
    return true;
}

// Return true if all the items in the list are upgradable
bool MainWindow::checkUpgradable(const QStringList &name_list) const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (name_list.isEmpty()) {
        return false;
    }
    for (const QString &name : name_list) {
        auto item_list = currentTree->findItems(name, Qt::MatchExactly, TreeCol::Name);
        if (item_list.isEmpty() || item_list.at(0)->data(TreeCol::Status, Qt::UserRole) != Status::Upgradable) {
            return false;
        }
    }
    return true;
}

QHash<QString, PackageInfo> MainWindow::listInstalled()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    Cmd shell;
    const QString list
        = shell.getOut("LANG=C dpkg-query -W -f='${db:Status-Abbrev} ${Package} ${Version} ${binary:Synopsis}\\n'");

    if (shell.exitStatus() != QProcess::NormalExit || shell.exitCode() != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("dpkg-query command returned an error. Please run 'dpkg-query -W' in terminal "
                                 "and check the output."));
        exit(EXIT_FAILURE);
    }

    QHash<QString, PackageInfo> installedPackagesMap;
    const QString statusPrefix = "ii ";
    const auto lines = list.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        if (!line.startsWith(statusPrefix)) {
            continue;
        }

        const QStringList parts = line.mid(statusPrefix.length()).split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        const QString packageName = parts.at(0);
        const QString version = parts.at(1);
        const QString description = parts.size() > 2 ? parts.mid(2).join(' ') : QString();

        installedPackagesMap.insert(packageName, {version, description});
    }

    return installedPackagesMap;
}

QStringList MainWindow::listFlatpaks(const QString &remote, const QString &type) const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    QString arch_fp = getArchOption();
    if (arch_fp.isEmpty()) {
        return {};
    }

    // Check if remote parameter is empty (which would happen if no remotes are configured)
    if (remote.isEmpty()) {
        qDebug() << "Remote parameter is empty - no flatpak remotes configured for user";
        return {};
    }

    const bool isUserScope = fpUser.startsWith("--user");

    auto buildRemoteLsCommand = [&](const QString &scope) {
        return "flatpak remote-ls " + scope + remote + ' ' + arch_fp + "--columns=ver,branch,ref,installed-size ";
    };

    QString typeFlag;
    if (type == QLatin1String("--app")) {
        typeFlag = "--app ";
    } else if (type == QLatin1String("--runtime")) {
        typeFlag = "--runtime ";
    }
    const QString commandSuffix = typeFlag + "2>/dev/null";

    // Construct the base command for listing flatpaks
    QString baseCommand = buildRemoteLsCommand(fpUser) + commandSuffix;

    auto runRemoteLs = [](const QString &command) {
        Cmd shell;
        QStringList output;
        if (shell.run(command)) {
            output = shell.readAllOutput().split('\n', Qt::SkipEmptyParts);
        }
        return output;
    };

    // Execute the command and process the output
    QStringList list = runRemoteLs(baseCommand);

    if (list.isEmpty()) {
        qDebug() << QString("Could not list packages from %1 remote, attempting to update remote").arg(remote);

        // Try to update the remote if it's empty
        QString updateCommand = "flatpak update " + fpUser + "--appstream " + remote + " 2>/dev/null";
        qDebug() << "Running remote update command:" << updateCommand;

        Cmd updateShell;
        if (updateShell.run(updateCommand)) {
            qDebug() << "Remote update completed, retrying package list";

            // Retry the original command after update
            list = runRemoteLs(baseCommand);
            if (!list.isEmpty()) {
                qDebug() << QString("Successfully retrieved %1 packages after remote update").arg(list.size());
            } else {
                qDebug() << QString("Remote %1 still empty after update").arg(remote);
            }
        } else {
            qDebug() << "Failed to update remote" << remote;
        }
    }

    // If user scope returned nothing (e.g. only system remotes exist), fall back to system remotes for listing
    if (list.isEmpty() && isUserScope) {
        const QString systemCommand = buildRemoteLsCommand(QStringLiteral("--system ")) + commandSuffix;
        qDebug() << "User remotes empty; retrying flatpak listing using system remotes:" << systemCommand;
        list = runRemoteLs(systemCommand);
    }

    return list;
}

// List installed flatpaks by type: apps, runtimes, or all (if no type is provided)
QStringList MainWindow::listInstalledFlatpaks(const QString &type)
{
    QStringList lines;
    if (cachedInstalledScope == fpUser && cachedInstalledFetched) {
        lines = cachedInstalledFlatpaks;
    } else {
        const QString command = "flatpak list " + fpUser + "2>/dev/null " + type + " --columns=ref";
        QScopedValueRollback<bool> guard(suppressCmdOutput, true);
        lines = cmd.getOut(command, Cmd::QuietMode::No).split('\n', Qt::SkipEmptyParts);

        if (type.isEmpty()) {
            cachedInstalledFlatpaks = lines;
            cachedInstalledScope = fpUser;
            cachedInstalledFetched = true;
        }
    }

    QStringList refs;
    for (const QString &lineRaw : lines) {
        if (lineRaw.startsWith("Ref")) { // skip header if present
            continue;
        }
        const QString line = lineRaw.section('\t', 0, 0);

        const ParsedFlatpakRef parsed = parseInstalledFlatpakLine(line);
        if (parsed.ref.isEmpty()) {
            continue;
        }

        if (type == QLatin1String("--app") && parsed.isRuntime) {
            continue;
        }
        if (type == QLatin1String("--runtime") && !parsed.isRuntime) {
            continue;
        }

        refs.append(parsed.ref);
    }
    return refs;
}

QTreeWidgetItem *MainWindow::createTreeItem(const QString &name, const QString &version,
                                            const QString &description) const
{
    auto *widget_item = new QTreeWidgetItem();
    widget_item->setCheckState(TreeCol::Check, Qt::Unchecked);
    widget_item->setText(TreeCol::Name, name);
    widget_item->setText(TreeCol::RepoVersion, version);
    widget_item->setText(TreeCol::Description, description);
    widget_item->setData(0, Qt::UserRole, true); // All items are displayed till filtered
    return widget_item;
}

void MainWindow::setCurrentTree()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    const QList<QTreeWidget *> trees
        = {ui->treePopularApps, ui->treeEnabled, ui->treeMXtest, ui->treeBackports, ui->treeFlatpak};

    auto it = std::find_if(trees.cbegin(), trees.cend(), [](const QTreeWidget *tree) { return tree->isVisible(); });

    if (it != trees.cend()) {
        currentTree = *it;
    }
}

void MainWindow::setDirty()
{
    dirtyBackports = true;
    dirtyEnabledRepos = true;
    dirtyTest = true;
}

void MainWindow::setIcons()
{

    const QString icon_upgradable {"package-installed-outdated"};
    const QString icon_installed {"package-installed-updated"};

    const QIcon backup_icon_upgradable(":/icons/package-installed-outdated.png");
    const QIcon backup_icon_installed(":/icons/package-installed-updated.png");

    const QIcon theme_icon_upgradable = QIcon::fromTheme(icon_upgradable, backup_icon_upgradable);
    const QIcon theme_icon_installed = QIcon::fromTheme(icon_installed, backup_icon_installed);

    const bool force_backup_icon = (theme_icon_upgradable.name() == theme_icon_installed.name());

    qiconInstalled = force_backup_icon ? backup_icon_installed : theme_icon_installed;
    qiconUpgradable = force_backup_icon ? backup_icon_upgradable : theme_icon_upgradable;
    const auto upgradableIcons = {ui->iconUpgradable, ui->iconUpgradable_2, ui->iconUpgradable_3};
    const auto installedIcons = {ui->iconInstalledPackages, ui->iconInstalledPackages_2, ui->iconInstalledPackages_3,
                                 ui->iconInstalledPackages_4, ui->iconInstalledPackages_5};
    for (auto *icon : upgradableIcons) {
        icon->setIcon(qiconUpgradable);
    }
    for (auto *icon : installedIcons) {
        icon->setIcon(qiconInstalled);
    }
}

QHash<QString, VersionNumber> MainWindow::listInstalledVersions()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QHash<QString, VersionNumber> installedVersions;
    Cmd shell;
    const QString command = "LANG=C dpkg-query -W -f='${db:Status-Abbrev} ${Package} ${Version}\\n'";
    const QStringList packageList = shell.getOut(command, Cmd::QuietMode::Yes).split('\n', Qt::SkipEmptyParts);

    if (shell.exitStatus() != QProcess::NormalExit || shell.exitCode() != 0) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("dpkg-query command returned an error, please run 'dpkg-query -W' in terminal and check the output."));
        return installedVersions;
    }
    for (const QString &line : packageList) {
        const QString statusPrefix = "ii ";
        if (!line.startsWith(statusPrefix)) {
            continue;
        }
        const QStringList packageInfo = line.mid(statusPrefix.length()).split(' ', Qt::SkipEmptyParts);
        if (packageInfo.size() == 2) {
            installedVersions.insert(packageInfo.at(0), VersionNumber(packageInfo.at(1)));
        }
    }
    return installedVersions;
}

QUrl MainWindow::getScreenshotUrl(const QString &name)
{
    QUrl url(QString("https://screenshots.debian.net/json/package/%1").arg(name));
    QNetworkProxyQuery query(url);
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
    if (!proxies.isEmpty()) {
        manager.setProxy(proxies.first());
    }

    reply = manager.get(QNetworkRequest(url));

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5s, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return {};
    }

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
    if (jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj.contains("screenshots") && jsonObj["screenshots"].isArray()) {
            QJsonArray screenshotsArray = jsonObj["screenshots"].toArray();
            if (!screenshotsArray.isEmpty()) {
                QJsonObject firstScreenshot = screenshotsArray.first().toObject();
                if (firstScreenshot.contains("small_image_url")) {
                    QUrl result = QUrl(firstScreenshot["small_image_url"].toString());
                    reply->deleteLater();
                    return result;
                }
            }
        }
    }
    reply->deleteLater();
    return {};
}

void MainWindow::cmdStart()
{
    if (!timer.isActive()) {
        timer.start(100ms);
    }
    setCursor(QCursor(Qt::BusyCursor));
    ui->lineEdit->setFocus();
}

void MainWindow::cmdDone()
{
    timer.stop();
    setCursor(QCursor(Qt::ArrowCursor));
    disableOutput();
    if (!holdProgressForFlatpakRefresh && !holdProgressForAptRefresh) {
        progress->hide();
    }
}

void MainWindow::enableOutput()
{
    connect(&cmd, &Cmd::outputAvailable, this, &MainWindow::outputAvailable, Qt::UniqueConnection);
    connect(&cmd, &Cmd::errorAvailable, this, &MainWindow::outputAvailable, Qt::UniqueConnection);
}

void MainWindow::disableOutput()
{
    disconnect(&cmd, &Cmd::outputAvailable, this, &MainWindow::outputAvailable);
    disconnect(&cmd, &Cmd::errorAvailable, this, &MainWindow::outputAvailable);
}

void MainWindow::displayInfoTestOrBackport(const QTreeWidget *tree, const QTreeWidgetItem *item)
{
    QString file_name = (tree == ui->treeMXtest) ? tempDir.filePath("mxPackages") : tempDir.filePath("allPackages");

    QFile file(file_name);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Could not open file:" << file.fileName();
        return;
    }

    QString msg;
    const QString item_name = item->text(TreeCol::Name);
    QTextStream in(&file);
    bool packageFound = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("Package: ")) {
            if (line == "Package: " + item_name) {
                packageFound = true;
                msg += line + '\n';
            } else if (packageFound) {
                break;
            }
        } else if (packageFound) {
            msg += line + '\n';
        }
    }
    auto msg_list = msg.split('\n', Qt::SkipEmptyParts);
    if (msg_list.isEmpty()) {
        qWarning() << "Package info not found in file:" << file.fileName() << "Show info from enabled repos";
        displayPackageInfo(tree->currentItem());
        return;
    }
    auto max_no_chars = 2000;        // Around 15-17 lines
    if (msg.size() > max_no_chars) { // Split msg into details if too large
        uchar max_no_lines = 20;     // Cut message after these many lines
        msg = msg_list.mid(0, max_no_lines).join('\n');
    }
    QMessageBox info(QMessageBox::NoIcon, tr("Package info"), msg, QMessageBox::Close);

    // Make it wider
    auto *horizontalSpacer = new QSpacerItem(width(), 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto *layout = qobject_cast<QGridLayout *>(info.layout());
    layout->addItem(horizontalSpacer, 0, 1);
    info.exec();
}

void MainWindow::displayPackageInfo(const QTreeWidget *tree, QPoint pos)
{
    auto *t_widget = qobject_cast<QTreeWidget *>(focusWidget());
    if (!t_widget) {
        qWarning() << "No tree widget in focus";
        return;
    }

    auto *action = new QAction(QIcon::fromTheme("dialog-information"), tr("More &info..."), this);
    if (tree == ui->treePopularApps) {
        if (t_widget->currentItem()->parent() == nullptr) { // Skip categories
            action->deleteLater();
            return;
        }
        connect(action, &QAction::triggered, this,
                [this, t_widget] { displayPopularInfo(t_widget->currentItem(), 3); });
    }
    QMenu menu(this);
    menu.addAction(action);
    if (tree == ui->treeEnabled) {
        connect(action, &QAction::triggered, this, [this, t_widget] { displayPackageInfo(t_widget->currentItem()); });
    } else {
        connect(action, &QAction::triggered, this,
                [this, tree, t_widget] { displayInfoTestOrBackport(tree, t_widget->currentItem()); });
    }
    menu.exec(t_widget->mapToGlobal(pos));
}

void MainWindow::displayPopularInfo(const QTreeWidgetItem *item, int column)
{
    if (column != PopCol::Info || item->parent() == nullptr) {
        return;
    }

    QString desc = item->text(PopCol::Description);
    QString install_names = item->text(PopCol::InstallNames);
    QString title = item->text(PopCol::Name);
    QString msg = "<b>" + title + "</b><p>" + desc + "<p>";
    if (!install_names.isEmpty()) {
        msg += tr("Packages to be installed: ") + install_names;
    }

    QUrl url = item->data(PopCol::Screenshot, Qt::UserRole).toString(); // screenshot url

    if (!url.isValid() || url.isEmpty() || url.url() == QLatin1String("none")) {
        url = getScreenshotUrl(install_names.split(' ').first());
    }

    if (!url.isValid() || url.isEmpty() || url.url() == QLatin1String("none")) {
        qDebug() << "no screenshot for: " << title;
    } else {
        QNetworkProxyQuery query {QUrl(url)};
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
        if (!proxies.isEmpty()) {
            manager.setProxy(proxies.first());
        }
        reply = manager.get(QNetworkRequest(url));

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5s, &loop, &QEventLoop::quit);
        ui->treePopularApps->blockSignals(true);
        loop.exec();
        ui->treePopularApps->blockSignals(false);

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Download of " << url.url() << " failed: " << qPrintable(reply->errorString());
            url = getScreenshotUrl(install_names.split(' ').first());
            if (url.isValid() && !url.isEmpty() && url.url() != QLatin1String("none")) {
                reply = manager.get(QNetworkRequest(url));
                connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                QTimer::singleShot(5s, &loop, &QEventLoop::quit);
                ui->treePopularApps->blockSignals(true);
                loop.exec();
                ui->treePopularApps->blockSignals(false);
            }
        }

        if (reply->error() == QNetworkReply::NoError) {
            QImage image;
            QByteArray data;
            QBuffer buffer(&data);
            QImageReader imageReader(reply);
            image = imageReader.read();
            if (imageReader.error() != 0) {
                qDebug() << "loading screenshot: " << imageReader.errorString();
            } else {
                image = image.scaled(QSize(200, 300), Qt::KeepAspectRatioByExpanding);
                image.save(&buffer, "PNG");
                msg += QString("<p><img src='data:image/png;base64, %0'>").arg(QString(data.toBase64()));
            }
        }
    }
    if (reply) {
        reply->deleteLater();
    }
    QMessageBox info(QMessageBox::NoIcon, tr("Package info"), msg, QMessageBox::Close);
    info.exec();
}

void MainWindow::displayPackageInfo(const QTreeWidgetItem *item)
{
    QString msg = cmd.getOut("aptitude show " + item->text(TreeCol::Name));
    // Remove first 5 lines from aptitude output "Reading package..."
    QString details = cmd.getOut("DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null "
                                 "| grep -sq ^i && echo kde || echo gnome) aptitude -sy -V -o=Dpkg::Use-Pty=0 install "
                                 + item->text(TreeCol::Name) + " |tail -5");

    auto detail_list = details.split('\n');
    auto msg_list = msg.split('\n');
    auto max_no_chars = 2000;        // Around 15-17 lines
    if (msg.size() > max_no_chars) { // Split msg into details if too large
        uchar max_no_lines = 17;     // Cut message after these many lines
        msg = msg_list.mid(0, max_no_lines).join('\n');
        detail_list = msg_list.mid(max_no_lines, msg_list.length()) + QStringList {} + detail_list;
        details = detail_list.join('\n');
    }
    msg += "\n\n" + detail_list.at(detail_list.size() - 2); // Add info about space needed/freed

    QMessageBox info(QMessageBox::NoIcon, tr("Package info"), msg.trimmed(), QMessageBox::Close);
    info.setDetailedText(details.trimmed());

    // Make it wider
    auto *horizontalSpacer = new QSpacerItem(width(), 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto *layout = qobject_cast<QGridLayout *>(info.layout());
    layout->addItem(horizontalSpacer, 0, 1);
    info.exec();
}

void MainWindow::findPopular() const
{
    const QString word = ui->searchPopular->text();
    if (word.length() == 1) {
        return;
    }

    auto *tree = ui->treePopularApps;
    tree->setUpdatesEnabled(false);

    // Handle empty search - show all items collapsed
    if (word.isEmpty()) {
        for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
            QTreeWidgetItem *item = *it;
            item->setExpanded(false);
            item->setHidden(false);
            if (!item->parent()) {
                item->setFirstColumnSpanned(true);
            }
        }
    } else {
        // Search in multiple columns and collect matches
        QSet<QTreeWidgetItem *> foundItems;
        const QVector<int> searchColumns {PopCol::Name, PopCol::Icon, PopCol::Description};

        // Check if the search term contains wildcards (* or ?)
        bool hasWildcards = word.contains('*') || word.contains('?');
        QRegularExpression regExp;

        if (hasWildcards) {
            // Convert the glob pattern to a regular expression
            QString pattern = QRegularExpression::escape(word);
            pattern.replace("\\*", ".*");
            pattern.replace("\\?", ".");
            regExp = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        }

        for (int column : searchColumns) {
            if (hasWildcards) {
                // Use regex matching for wildcard searches
                for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
                    QTreeWidgetItem *item = *it;
                    if (regExp.match(item->text(column)).hasMatch()) {
                        // Add the matching item and all its ancestors
                        for (QTreeWidgetItem *ancestor = item; ancestor; ancestor = ancestor->parent()) {
                            foundItems.insert(ancestor);
                        }
                    }
                }
            } else {
                // Use standard search for non-wildcard searches
                const auto matches = tree->findItems(word, Qt::MatchContains | Qt::MatchRecursive, column);
                for (QTreeWidgetItem *match : matches) {
                    // Add the matching item and all its ancestors
                    for (QTreeWidgetItem *item = match; item; item = item->parent()) {
                        foundItems.insert(item);
                    }
                }
            }
        }

        // Show only matching items and their ancestors
        for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
            QTreeWidgetItem *item = *it;
            const bool isFound = foundItems.contains(item);
            item->setHidden(!isFound);

            // Expand and span top-level matches
            if (isFound && !item->parent()) {
                item->setExpanded(true);
                item->setFirstColumnSpanned(true);
            }
        }
    }

    // Resize columns except the first one
    for (int i = 1; i < tree->columnCount(); ++i) {
        tree->resizeColumnToContents(i);
    }

    tree->setUpdatesEnabled(true);
}

void MainWindow::findPackage()
{
    // Get search text from appropriate search box
    const QMap<QTreeWidget *, QLineEdit *> searchBoxMap = {{ui->treeEnabled, ui->searchBoxEnabled},
                                                           {ui->treeMXtest, ui->searchBoxMX},
                                                           {ui->treeBackports, ui->searchBoxBP},
                                                           {ui->treeFlatpak, ui->searchBoxFlatpak}};

    const QString word = searchBoxMap.value(currentTree)->text();

    // Skip single character searches
    if (word.length() == 1) {
        return;
    }

    currentTree->setUpdatesEnabled(false);

    // Track matching items and their ancestors
    QSet<QTreeWidgetItem *> foundItems;

    // Search appropriate columns based on tree type
    QVector<int> searchColumns;
    if (currentTree == ui->treeFlatpak) {
        searchColumns = {FlatCol::LongName};
    } else {
        searchColumns = {TreeCol::Name, TreeCol::Description};
    }

    // Find matches in each column
    for (int column : searchColumns) {
        // Check if the search term contains wildcards (* or ?)
        QRegularExpression regExp;
        if (word.contains('*') || word.contains('?')) {
            // Convert the glob pattern to a regular expression
            QString pattern = QRegularExpression::escape(word);
            pattern.replace("\\*", ".*");
            pattern.replace("\\?", ".");
            regExp = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        } else {
            // Use standard search for non-wildcard searches
            regExp = QRegularExpression(QRegularExpression::escape(word), QRegularExpression::CaseInsensitiveOption);
        }

        // Check each item against the regex pattern
        for (QTreeWidgetItemIterator it(currentTree); *it; ++it) {
            QTreeWidgetItem *item = *it;
            if (regExp.match(item->text(column)).hasMatch()) {
                // Add match and its ancestors to found set
                QTreeWidgetItem *ancestor = item;
                while (ancestor) {
                    foundItems.insert(ancestor);
                    ancestor = ancestor->parent();
                }
            }
        }
    }

    // Show/hide items based on search results
    for (QTreeWidgetItemIterator it(currentTree); *it; ++it) {
        QTreeWidgetItem *item = *it;
        const bool isHidden = item->data(0, Qt::UserRole) == false;
        item->setHidden(!foundItems.contains(item) || isHidden);
    }

    // Apply library filtering if needed
    if (currentTree != ui->treeFlatpak) {
        hideLibs();
    }

    currentTree->setUpdatesEnabled(true);
}

void MainWindow::showOutput()
{
    ui->outputBox->clear();
    ui->tabWidget->setTabEnabled(Tab::Output, true);
    ui->tabWidget->setCurrentWidget(ui->tabOutput);
    enableTabs(false);
}

void MainWindow::pushInstall_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();
    if (currentTree == ui->treeFlatpak) {
        // Confirmation dialog
        if (!confirmActions(changeList.join(' '), "install")) {
            displayFlatpaks(true);
            indexFilterFP.clear();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            enableTabs(true);
            return;
        }
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        if (cmd.run("socat SYSTEM:'flatpak install -y " + fpUser + ui->comboRemote->currentText() + ' '
                    + changeList.join(' ') + "',stderr STDIO")) {
            displayFlatpaks(true);
            indexFilterFP.clear();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
        } else {
            setCursor(QCursor(Qt::ArrowCursor));
            QMessageBox::critical(this, tr("Error"),
                                  tr("Problem detected while installing, please inspect the console output."));
        }
    } else {
        bool success = false;
        if (currentTree == ui->treePopularApps) {
            success = installPopularApps();
        } else if (ui->comboFilterEnabled->currentText() == tr("Autoremovable")) {
            success = markKeep();
        } else {
            success = installSelected();
        }
        setDirty();
        buildPackageLists();
        refreshPopularApps();
        if (success) {
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(currentTree->parentWidget());
        } else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Problem detected while installing, please inspect the console output."));
        }
    }
    enableTabs(true);
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About %1").arg(windowTitle()),
        "<p align=\"center\"><b><h2>" + windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + QCoreApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Package Installer for MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-packageinstaller/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    QString lang = locale.bcp47Name();
    QString url {"/usr/share/doc/mx-packageinstaller/mx-package-installer.html"};

    if (lang.startsWith("fr")) {
        url = "https://mxlinux.org/wiki/help-files/help-mx-installateur-de-paquets";
    }
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

// Resize columns when expanding
void MainWindow::treePopularApps_expanded()
{
    ui->treePopularApps->resizeColumnToContents(PopCol::Name);
    ui->treePopularApps->resizeColumnToContents(PopCol::Description);
}

void MainWindow::treePopularApps_itemExpanded(QTreeWidgetItem *item)
{
    item->setIcon(PopCol::Icon, QIcon::fromTheme("folder-open"));
    ui->treePopularApps->resizeColumnToContents(PopCol::Name);
    ui->treePopularApps->resizeColumnToContents(PopCol::Description);
}

void MainWindow::treePopularApps_itemCollapsed(QTreeWidgetItem *item)
{
    item->setIcon(PopCol::Icon, QIcon::fromTheme("folder"));
    ui->treePopularApps->resizeColumnToContents(PopCol::Name);
    ui->treePopularApps->resizeColumnToContents(PopCol::Description);
}

void MainWindow::pushUninstall_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    showOutput();

    QString names;
    QString preuninstall;
    QString postuninstall;
    if (currentTree == ui->treePopularApps) {
        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
                names += (*it)->data(PopCol::UninstallNames, Qt::UserRole).toString().replace('\n', ' ') + ' ';
                postuninstall += (*it)->data(PopCol::PostUninstall, Qt::UserRole).toString() + '\n';
                preuninstall += (*it)->data(PopCol::PreUninstall, Qt::UserRole).toString() + '\n';
                (*it)->setCheckState(PopCol::Check, Qt::Unchecked);
            }
        }
    } else if (currentTree == ui->treeFlatpak) {
        bool success = true;

        // Confirmation dialog
        if (!confirmActions(changeList.join(' '), "remove")) {
            displayFlatpaks(true);
            indexFilterFP.clear();
            listFlatpakRemotes();
            ui->comboRemote->setCurrentIndex(0);
            comboRemote_activated();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            enableTabs(true);
            return;
        }

        setCursor(QCursor(Qt::BusyCursor));
        for (const QString &app : std::as_const(changeList)) {
            enableOutput();
            if (!cmd.run("socat SYSTEM:'flatpak uninstall " + fpUser + "-y " + app
                         + "',stderr STDIO")) { // success if all processed successfuly,
                                                // failure if one failed
                success = false;
            }
        }
        if (success) { // Success if all processed successfuly, failure if one failed
            displayFlatpaks(true);
            indexFilterFP.clear();
            listFlatpakRemotes();
            ui->comboRemote->setCurrentIndex(0);
            comboRemote_activated();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("We encountered a problem uninstalling, please check output"));
        }
        enableTabs(true);
        return;
    } else {
        names = changeList.join(' ');
    }

    bool success = uninstall(names, preuninstall, postuninstall);
    setDirty();
    buildPackageLists();
    refreshPopularApps();
    if (success) {
        QMessageBox::information(this, tr("Success"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(currentTree->parentWidget());
    } else {
        QMessageBox::critical(this, tr("Error"), tr("We encountered a problem uninstalling the program"));
    }
    enableTabs(true);
}

void MainWindow::tabWidget_currentChanged(int index)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setTabText(Tab::Output, tr("Console Output"));
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);

    resetCheckboxes();
    QString search_str;
    saveSearchText(search_str, savedComboIndex);
    if (index != Tab::Output) {
        setCurrentTree();
    }
    currentTree->blockSignals(true);
    auto setTabsEnabled = [this](bool enable) {
        for (auto tab : {Tab::Popular, Tab::EnabledRepos, Tab::Test, Tab::Backports, Tab::Flatpak}) {
            if (tab != ui->tabWidget->currentIndex()) {
                ui->tabWidget->setTabEnabled(tab, enable);
            }
        }
    };
    setTabsEnabled(false);
    switch (index) {
    case Tab::Popular:
        handleTab(search_str, ui->searchPopular, "", false);
        break;
    case Tab::EnabledRepos:
        handleEnabledReposTab(search_str);
        break;
    case Tab::Test:
        handleTab(search_str, ui->searchBoxMX, "test", dirtyTest);
        break;
    case Tab::Backports:
        handleTab(search_str, ui->searchBoxBP, "backports", dirtyBackports);
        break;
    case Tab::Flatpak:
        handleFlatpakTab(search_str);
        break;
    case Tab::Output:
        handleOutputTab();
        break;
    }
    setTabsEnabled(true);
    ui->pushUpgradeAll->setVisible((currentTree == ui->treeEnabled) && (ui->labelNumUpgr->text().toInt() > 0));
}

void MainWindow::resetCheckboxes()
{
    currentTree->blockSignals(true);
    // Popular apps are processed in a different way, tree is reset after install/removal
    if (currentTree != ui->treePopularApps) {
        currentTree->clearSelection();
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            (*it)->setCheckState(TreeCol::Check, Qt::Unchecked);
        }
    } else if (ui->tabWidget->currentIndex() != Tab::Output) { // Don't clear selections on output tab for pop apps
        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            (*it)->setCheckState(PopCol::Check, Qt::Unchecked);
        }
    }
}

void MainWindow::saveSearchText(QString &search_str, int &filter_idx)
{
    if (currentTree == ui->treePopularApps) {
        search_str = ui->searchPopular->text();
    } else if (currentTree == ui->treeEnabled) {
        search_str = ui->searchBoxEnabled->text();
        filter_idx = ui->comboFilterEnabled->currentIndex();
    } else if (currentTree == ui->treeMXtest) {
        search_str = ui->searchBoxMX->text();
        filter_idx = ui->comboFilterMX->currentIndex();
    } else if (currentTree == ui->treeBackports) {
        search_str = ui->searchBoxBP->text();
        filter_idx = ui->comboFilterBP->currentIndex();
    } else if (currentTree == ui->treeFlatpak) {
        search_str = ui->searchBoxFlatpak->text();
    }
}

void MainWindow::handleEnabledReposTab(const QString &search_str)
{
    ui->searchBoxEnabled->setText(search_str);
    changeList.clear();
    if (displayPackagesIsRunning) {
        progress->show();
        if (!timer.isActive()) {
            timer.start(100ms);
        }
    } else if (currentTree->topLevelItemCount() == 0 || dirtyEnabledRepos) {
        if (!buildPackageLists()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Could not download the list of packages. Please check your APT sources."));
            currentTree->blockSignals(false);
            return;
        }
    }
    if (!displayPackagesIsRunning) {
        ui->comboFilterEnabled->setCurrentIndex(savedComboIndex);
        ui->comboFilterMX->setCurrentIndex(savedComboIndex);
        ui->comboFilterBP->setCurrentIndex(savedComboIndex);
        filterChanged(ui->comboFilterEnabled->currentText());
    }
    if (!ui->searchBoxEnabled->text().isEmpty()) {
        QMetaObject::invokeMethod(this, [this] { findPackage(); }, Qt::QueuedConnection);
    }
    if (!displayPackagesIsRunning) {
        currentTree->blockSignals(false);
    }
}

void MainWindow::handleTab(const QString &search_str, QLineEdit *searchBox, const QString &warningMessage,
                           bool dirtyFlag)
{
    if (searchBox) {
        searchBox->setText(search_str);
    }
    if (!warningMessage.isEmpty()) {
        displayWarning(warningMessage);
    }
    changeList.clear();
    if (currentTree->topLevelItemCount() == 0 || dirtyFlag) {
        if (!buildPackageLists()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Could not download the list of packages. Please check your APT sources."));
            currentTree->blockSignals(false);
            return;
        }
    }
    if (Tab::Popular != ui->tabWidget->currentIndex()) {
        ui->comboFilterEnabled->setCurrentIndex(savedComboIndex);
        ui->comboFilterMX->setCurrentIndex(savedComboIndex);
        ui->comboFilterBP->setCurrentIndex(savedComboIndex);
        filterChanged(ui->comboFilterEnabled->currentText());
    }
    currentTree->blockSignals(false);
}

void MainWindow::handleFlatpakTab(const QString &search_str)
{
    lastItemClicked = nullptr;
    ui->searchBoxFlatpak->setText(search_str);
    setCurrentTree();
    displayWarning("flatpaks");
    ui->searchBoxFlatpak->setFocus();
    listFlatpakRemotes();
    if (!firstRunFP && checkInstalled("flatpak")) {
        ui->searchBoxBP->setText(search_str);
        if (!search_str.isEmpty()) {
            QMetaObject::invokeMethod(this, [this] { findPackage(); }, Qt::QueuedConnection);
        }
        if (!displayFlatpaksIsRunning) {
            filterChanged(ui->comboFilterFlatpak->currentText());
        }
        currentTree->blockSignals(false);
        return;
    }
    firstRunFP = false;
    blockInterfaceFP(true);
    if (!checkInstalled("flatpak")) {
        int ans = QMessageBox::question(this, tr("Flatpak not installed"),
                                        tr("Flatpak is not currently installed.\nOK to go ahead and install it?"));
        if (ans == QMessageBox::No) {
            ui->tabWidget->setCurrentIndex(Tab::Popular);
            enableTabs(true);
            return;
        }
        installFlatpak();
    } else {
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        setCursor(QCursor(Qt::ArrowCursor));
        if (ui->comboRemote->currentText().isEmpty()) {
            listFlatpakRemotes();
        }
        if (displayFlatpaksIsRunning) {
            if (!flatpakCancelHidden && pushCancel) {
                pushCancel->setEnabled(false);
                pushCancel->hide();
                flatpakCancelHidden = true;
            }
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
                qApp->processEvents();
            }
        }
        ui->searchBoxBP->setText(search_str);
        if (!search_str.isEmpty()) {
            QMetaObject::invokeMethod(this, [this] { findPackage(); }, Qt::QueuedConnection);
        }
    }
}

void MainWindow::installFlatpak()
{
    ui->tabWidget->setTabEnabled(Tab::Output, true);
    ui->tabWidget->setCurrentWidget(ui->tabOutput);
    setCursor(QCursor(Qt::BusyCursor));
    showOutput();
    displayFlatpaksIsRunning = true;
    install("flatpak");
    installedPackages = listInstalled();
    setDirty();
    buildPackageLists();
    if (!checkInstalled("flatpak")) {
        QMessageBox::critical(this, tr("Flatpak not installed"), tr("Flatpak was not installed"));
        ui->tabWidget->setCurrentIndex(Tab::Popular);
        setCursor(QCursor(Qt::ArrowCursor));
        enableTabs(true);
        currentTree->blockSignals(false);
        return;
    }
    Cmd().run(elevate + "/usr/lib/mx-packageinstaller/mxpi-lib flatpak_add_repos", Cmd::QuietMode::Yes);
    enableOutput();
    invalidateFlatpakRemoteCache();
    listFlatpakRemotes();
    if (displayFlatpaksIsRunning) {
        progress->show();
        if (!timer.isActive()) {
            timer.start(100ms);
        }
    }
    setCursor(QCursor(Qt::ArrowCursor));
    ui->tabWidget->setTabText(Tab::Output, tr("Console Output"));
    ui->tabWidget->blockSignals(true);
    displayFlatpaks(true);
    ui->tabWidget->blockSignals(false);
    QMessageBox::warning(this, tr("Needs re-login"),
                         tr("You might need to logout/login to see installed items in the menu"));
    ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
    enableTabs(true);
}

void MainWindow::handleOutputTab()
{
    // Block signals and clear all search boxes
    const QList<QLineEdit *> searchBoxes
        = {ui->searchPopular, ui->searchBoxEnabled, ui->searchBoxMX, ui->searchBoxBP, ui->searchBoxFlatpak};

    for (auto searchBox : searchBoxes) {
        searchBox->blockSignals(true);
        searchBox->clear();
        searchBox->blockSignals(false);
    }

    // Disable install/uninstall buttons
    ui->pushInstall->setDisabled(true);
    ui->pushUninstall->setDisabled(true);
}

void MainWindow::filterChanged(const QString &arg1)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    currentTree->blockSignals(true);
    currentTree->setUpdatesEnabled(false);
    updateInterface();

    // Helper functions
    auto resetTree = [this]() {
        // Optimization: Disable updates during bulk operations
        currentTree->setUpdatesEnabled(false);
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            (*it)->setData(0, Qt::UserRole, true);
            (*it)->setHidden(false);
            (*it)->setCheckState(TreeCol::Check, Qt::Unchecked);
        }
        currentTree->setUpdatesEnabled(true);

        QMetaObject::invokeMethod(this, [this] { findPackage(); }, Qt::QueuedConnection);
        setSearchFocus();
        ui->pushInstall->setEnabled(false);
        ui->pushUninstall->setEnabled(false);
    };

    auto uncheckAllItems = [this]() {
        // Optimization: Disable updates during bulk operations
        currentTree->setUpdatesEnabled(false);
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            (*it)->setCheckState(TreeCol::Check, Qt::Unchecked);
        }
        currentTree->setUpdatesEnabled(true);
    };

    auto handleFlatpakFilter = [this, uncheckAllItems](const QStringList &data, bool raw = true) {
        uncheckAllItems();
        displayFilteredFP(data, raw);
    };

    auto updateButtonStates = [this](bool installEnabled, bool uninstallEnabled) {
        ui->pushInstall->setEnabled(installEnabled);
        ui->pushUninstall->setEnabled(uninstallEnabled);
    };

    auto clearChangeListAndButtons = [this, updateButtonStates]() {
        updateButtonStates(false, false);
        changeList.clear();
    };

    auto blockSignalsForAll = [this](bool block) {
        ui->checkHideLibs->blockSignals(block);
        ui->checkHideLibsBP->blockSignals(block);
        ui->checkHideLibsMX->blockSignals(block);
    };

    auto resizeCurrentRepoTree = [this]() {
        if (currentTree == ui->treeFlatpak || currentTree == ui->treePopularApps) {
            return;
        }
        for (int i = 0; i < currentTree->columnCount(); ++i) {
            if (!currentTree->isColumnHidden(i)) {
                currentTree->resizeColumnToContents(i);
            }
        }
    };

    // Hide and reset all header checkboxes by default
    if (headerEnabled) {
        headerEnabled->setCheckboxVisible(false);
        headerEnabled->setChecked(false);
    }
    if (headerMX) {
        headerMX->setCheckboxVisible(false);
        headerMX->setChecked(false);
    }
    if (headerBP) {
        headerBP->setCheckboxVisible(false);
        headerBP->setChecked(false);
    }

    bool isAutoremovable = (arg1 == tr("Autoremovable"));
    bool shouldHideLibs = !isAutoremovable && hideLibsChecked;

    // Handle Flatpak tree
    if (currentTree == ui->treeFlatpak) {
        if (arg1 == tr("Installed runtimes")) {
            handleFlatpakFilter(installedRuntimesFP, false);
            clearChangeListAndButtons();
        } else if (arg1 == tr("Installed apps")) {
            handleFlatpakFilter(installedAppsFP, false);
            clearChangeListAndButtons();
        } else if (arg1 == tr("All apps")) {
            if (flatpaksApps.isEmpty()) {
                flatpaksApps = listFlatpaks(ui->comboRemote->currentText(), "--app");
            }
            handleFlatpakFilter(flatpaksApps);
            clearChangeListAndButtons();
        } else if (arg1 == tr("All runtimes")) {
            if (flatpaksRuntimes.isEmpty()) {
                flatpaksRuntimes = listFlatpaks(ui->comboRemote->currentText(), "--runtime");
            }
            handleFlatpakFilter(flatpaksRuntimes);
            clearChangeListAndButtons();
        } else if (arg1 == tr("All available")) {
            resetTree();
            ui->labelNumAppFP->setText(QString::number(currentTree->topLevelItemCount()));
            clearChangeListAndButtons();
        } else if (arg1 == tr("All installed")) {
            displayFilteredFP(installedAppsFP + installedRuntimesFP);
        } else if (arg1 == tr("Not installed")) {
            for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
                bool isNotInstalled = (*it)->data(FlatCol::Status, Qt::UserRole) == Status::NotInstalled;
                if (!isNotInstalled) {
                    (*it)->setHidden(true);
                    (*it)->setCheckState(FlatCol::Check, Qt::Unchecked);
                    changeList.removeOne((*it)->data(FlatCol::FullName, Qt::UserRole).toString());
                }
                (*it)->setData(0, Qt::UserRole, isNotInstalled);
            }
            ui->pushUninstall->setEnabled(false);
        }
        QMetaObject::invokeMethod(this, [this] { findPackage(); }, Qt::QueuedConnection);
        setSearchFocus();
    } else if (arg1 == tr("All packages")) {
        savedComboIndex = 0;
        blockSignalsForAll(true);

        ui->checkHideLibs->setChecked(shouldHideLibs);
        ui->checkHideLibsMX->setChecked(shouldHideLibs);
        ui->checkHideLibsBP->setChecked(shouldHideLibs);
        blockSignalsForAll(false);
        resetTree();
        clearChangeListAndButtons();
        ui->pushInstall->setText(isAutoremovable ? tr("Mark keep") : tr("Install"));
    } else {
        blockSignalsForAll(true);
        ui->checkHideLibs->setChecked(shouldHideLibs);
        ui->checkHideLibsMX->setChecked(shouldHideLibs);
        ui->checkHideLibsBP->setChecked(shouldHideLibs);
        blockSignalsForAll(false);

        ui->pushInstall->setText(isAutoremovable ? tr("Mark keep") : tr("Install"));

        const QHash<QString, int> statusMap {{tr("Installed"), Status::Installed},
                                             {tr("Upgradable"), Status::Upgradable},
                                             {tr("Not installed"), Status::NotInstalled},
                                             {tr("Autoremovable"), Status::Autoremovable}};

        auto itStatus = statusMap.find(arg1);
        if (itStatus != statusMap.end()) {
            savedComboIndex = itStatus.value();
            bool hasVisibleMatches = false;
            for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
                int itemStatus = (*it)->data(TreeCol::Status, Qt::UserRole).toInt();
                bool shouldShow = (itStatus.value() == Status::Installed && itemStatus == Status::Upgradable)
                                  || (itemStatus == itStatus.value());
                (*it)->setHidden(!shouldShow);
                (*it)->setData(0, Qt::UserRole, shouldShow);
                hasVisibleMatches = hasVisibleMatches || shouldShow;
            }
            // Show the header checkbox when filtering Upgradable or Autoremovable
            if (itStatus.value() == Status::Upgradable || itStatus.value() == Status::Autoremovable) {
                const QString tip = (itStatus.value() == Status::Upgradable) ? tr("Select/deselect all upgradable")
                                                                             : tr("Select/deselect all autoremovable");
                const bool allowAutoremovableCheckbox = itStatus.value() != Status::Autoremovable || hasVisibleMatches;

                if (currentTree == ui->treeEnabled && headerEnabled) {
                    headerEnabled->setCheckboxVisible(allowAutoremovableCheckbox);
                    if (allowAutoremovableCheckbox) {
                        headerEnabled->setToolTip(tip);
                        headerEnabled->resizeSection(TreeCol::Check, qMax(headerEnabled->sectionSize(0), 22));
                    }
                } else if (currentTree == ui->treeMXtest && headerMX) {
                    headerMX->setCheckboxVisible(allowAutoremovableCheckbox);
                    if (allowAutoremovableCheckbox) {
                        headerMX->setToolTip(tip);
                        headerMX->resizeSection(TreeCol::Check, qMax(headerMX->sectionSize(0), 22));
                    }
                } else if (currentTree == ui->treeBackports && headerBP) {
                    headerBP->setCheckboxVisible(allowAutoremovableCheckbox);
                    if (allowAutoremovableCheckbox) {
                        headerBP->setToolTip(tip);
                        headerBP->resizeSection(TreeCol::Check, qMax(headerBP->sectionSize(0), 22));
                    }
                }
            }
        }
        uncheckAllItems();
        QMetaObject::invokeMethod(this, [this] { findPackage(); }, Qt::QueuedConnection);
        setSearchFocus();
        clearChangeListAndButtons();
    }
    resizeCurrentRepoTree();
    currentTree->setUpdatesEnabled(true);
    currentTree->blockSignals(false);
}

// Toggle selection of all visible upgradable items in the current tab
void MainWindow::selectAllUpgradable_toggled(bool checked)
{
    QTreeWidget *tree = nullptr;
    QObject *s = sender();
    if (s == headerEnabled) {
        tree = ui->treeEnabled;
    } else if (s == headerMX) {
        tree = ui->treeMXtest;
    } else if (s == headerBP) {
        tree = ui->treeBackports;
    } else {
        return;
    }

    // Batch update: avoid emitting itemChanged for every item
    tree->setUpdatesEnabled(false);
    tree->blockSignals(true);
    // Determine desired status based on current filter text
    int targetStatus = Status::Upgradable;
    QString filterText;
    if (tree == ui->treeEnabled) {
        filterText = ui->comboFilterEnabled->currentText();
    } else if (tree == ui->treeMXtest) {
        filterText = ui->comboFilterMX->currentText();
    } else if (tree == ui->treeBackports) {
        filterText = ui->comboFilterBP->currentText();
    }
    if (filterText == tr("Autoremovable")) {
        targetStatus = Status::Autoremovable;
    }

    for (QTreeWidgetItemIterator it(tree); *it; ++it) {
        QTreeWidgetItem *item = *it;
        const bool visible = !item->isHidden();
        const bool match = item->data(TreeCol::Status, Qt::UserRole).toInt() == targetStatus;
        if (visible && match) {
            item->setCheckState(TreeCol::Check, checked ? Qt::Checked : Qt::Unchecked);
        }
    }
    // Rebuild changeList and update buttons once, instead of per-item
    changeList.clear();
    if (checked) {
        for (QTreeWidgetItemIterator it(tree); *it; ++it) {
            QTreeWidgetItem *item = *it;
            const bool visible = !item->isHidden();
            const bool match = item->data(TreeCol::Status, Qt::UserRole).toInt() == targetStatus;
            if (visible && match && item->checkState(TreeCol::Check) == Qt::Checked) {
                changeList.append(item->text(TreeCol::Name));
            }
        }
    }

    // Update action buttons coherently after batch toggle
    ui->pushInstall->setEnabled(!changeList.isEmpty());
    if (tree != ui->treeFlatpak) {
        ui->pushUninstall->setEnabled(checkInstalled(changeList));
        if (targetStatus == Status::Autoremovable) {
            ui->pushInstall->setText(tr("Mark keep"));
        } else {
            ui->pushInstall->setText(checkUpgradable(changeList) ? tr("Upgrade") : tr("Install"));
        }
    }

    tree->blockSignals(false);
    tree->setUpdatesEnabled(true);
}

void MainWindow::treeEnabled_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(TreeCol::Check) == Qt::Checked) {
        ui->treeEnabled->setCurrentItem(item);
    }
    buildChangeList(item);
}

void MainWindow::treeMXtest_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(TreeCol::Check) == Qt::Checked) {
        ui->treeMXtest->setCurrentItem(item);
    }
    buildChangeList(item);
}

void MainWindow::treeBackports_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(TreeCol::Check) == Qt::Checked) {
        ui->treeBackports->setCurrentItem(item);
    }
    buildChangeList(item);
}

void MainWindow::treeFlatpak_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(FlatCol::Check) == Qt::Checked) {
        ui->treeFlatpak->setCurrentItem(item);
    }
    buildChangeList(item);
}

// Build the changeList when selecting on item in the tree
void MainWindow::buildChangeList(QTreeWidgetItem *item)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    /* if all apps are uninstalled (or some installed) -> enable Install, disable Uinstall
     * if all apps are installed or upgradable -> enable Uninstall, enable Install
     * if all apps are upgradable -> change Install label to Upgrade;
     */

    lastItemClicked = item;
    QString newapp;
    if (currentTree == ui->treeFlatpak) {
        if (changeList.isEmpty()
            && indexFilterFP.isEmpty()) { // remember the Flatpak combo location first time this is called
            indexFilterFP = ui->comboFilterFlatpak->currentText();
        }
        newapp = (item->data(FlatCol::FullName, Qt::UserRole).toString());
    } else {
        newapp = (item->text(TreeCol::Name));
    }

    if (item->checkState(0) == Qt::Checked) {
        ui->pushInstall->setEnabled(true);
        changeList.append(newapp);
    } else {
        changeList.removeOne(newapp);
    }

    if (currentTree != ui->treeFlatpak) {
        ui->pushUninstall->setEnabled(checkInstalled(changeList));
        ui->pushInstall->setText(checkUpgradable(changeList) ? tr("Upgrade") : tr("Install"));
        if (ui->comboFilterEnabled->currentText() == tr("Autoremovable")) {
            ui->pushInstall->setText(tr("Mark keep"));
        }
    } else { // For Flatpaks allow selection only of installed or not installed items so one clicks
             // on an installed item only installed items should be displayed and the other way round
        ui->pushInstall->setText(tr("Install"));
        if (item->data(FlatCol::Status, Qt::UserRole) == Status::Installed) {
            if (item->checkState(FlatCol::Check) == Qt::Checked
                && ui->comboFilterFlatpak->currentText() != tr("All installed")) {
                ui->comboFilterFlatpak->setCurrentText(tr("All installed"));
            }
            ui->pushUninstall->setEnabled(true);
            ui->pushInstall->setEnabled(false);
        } else {
            if (item->checkState(FlatCol::Check) == Qt::Checked
                && ui->comboFilterFlatpak->currentText() != tr("Not installed")) {
                ui->comboFilterFlatpak->setCurrentText(tr("Not installed"));
            }
            ui->pushUninstall->setEnabled(false);
            ui->pushInstall->setEnabled(true);
        }
        if (changeList.isEmpty()) { // Reset comboFilterFlatpak if nothing is selected
            ui->comboFilterFlatpak->setCurrentText(indexFilterFP);
            indexFilterFP.clear();
        }
        ui->treeFlatpak->setFocus();
    }

    if (changeList.isEmpty()) {
        ui->pushInstall->setEnabled(false);
        ui->pushUninstall->setEnabled(false);
    }
}

// Force repo upgrade
void MainWindow::pushForceUpdateEnabled_clicked()
{
    ui->searchBoxEnabled->clear();
    ui->comboFilterEnabled->setCurrentIndex(0);
    buildPackageLists(true);
    updateInterface();
}

void MainWindow::pushForceUpdateMX_clicked()
{
    ui->searchBoxMX->clear();
    ui->comboFilterMX->setCurrentIndex(0);
    buildPackageLists(true);
    updateInterface();
}

void MainWindow::pushForceUpdateFP_clicked()
{
    ui->searchBoxFlatpak->clear();
    if (!flatpakCancelHidden && pushCancel) {
        pushCancel->setEnabled(false);
        pushCancel->hide();
        flatpakCancelHidden = true;
    }
    holdProgressForFlatpakRefresh = true;
    progress->show();
    cmd.run("flatpak update --appstream");
    displayFlatpaks(true);
    updateInterface();
}

void MainWindow::pushForceUpdateBP_clicked()
{
    ui->searchBoxBP->clear();
    ui->comboFilterBP->setCurrentIndex(0);
    buildPackageLists(true);
    updateInterface();
}

// Hide/unhide lib/-dev packages
void MainWindow::checkHideLibs_toggled(bool checked)
{
    ui->treeEnabled->setUpdatesEnabled(false);
    hideLibsChecked = checked;
    // Persist user preference
    settings.setValue("HideLibs", checked);
    ui->checkHideLibsMX->setChecked(checked);
    ui->checkHideLibsBP->setChecked(checked);

    for (QTreeWidgetItemIterator it(ui->treeEnabled); (*it) != nullptr; ++it) {
        (*it)->setHidden(isFilteredName((*it)->text(TreeCol::Name)) && checked);
    }
    filterChanged(ui->comboFilterEnabled->currentText());
    ui->treeEnabled->setUpdatesEnabled(true);
}

void MainWindow::pushUpgradeAll_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();

    QList<QTreeWidgetItem *> foundItems;
    for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
        auto userData = (*it)->data(TreeCol::Status, Qt::UserRole);
        if (userData == Status::Upgradable) {
            foundItems.append(*it);
        }
    }
    QString names;
    for (QTreeWidgetItemIterator it(ui->treeEnabled); (*it) != nullptr; ++it) {
        if (foundItems.contains(*it)) {
            names += (*it)->text(TreeCol::Name) + ' ';
        }
    }
    bool success = install(names);
    setDirty();
    buildPackageLists();
    if (success) {
        QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(currentTree->parentWidget());
    } else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Problem detected while installing, please inspect the console output."));
    }
    enableTabs(true);
}

// Pressing Enter or buttonEnter will do the same thing
void MainWindow::pushEnter_clicked()
{
    if (currentTree == ui->treeFlatpak
        && ui->lineEdit->text().isEmpty()) { // Add "Y" as default response for flatpaks to work like apt-get
        cmd.write("y");
    }
    lineEdit_returnPressed();
}

void MainWindow::lineEdit_returnPressed()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    cmd.write(ui->lineEdit->text().toUtf8() + '\n');
    ui->outputBox->appendPlainText(ui->lineEdit->text() + '\n');
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

void MainWindow::pushCancel_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (cmd.state() != QProcess::NotRunning) {
        if (QMessageBox::warning(this, tr("Quit?"),
                                 tr("Process still running, quitting might leave the system in an unstable "
                                    "state.<p><b>Are you sure you want to exit MX Package Installer?</b>"),
                                 QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::No) {
            return;
        }
    }
    cleanup();
    QApplication::quit();
}

void MainWindow::checkHideLibsMX_clicked(bool checked)
{
    hideLibsChecked = checked;
    ui->checkHideLibs->setChecked(checked);
    ui->checkHideLibsBP->setChecked(checked);
}

void MainWindow::checkHideLibsBP_clicked(bool checked)
{
    hideLibsChecked = checked;
    ui->checkHideLibs->setChecked(checked);
    ui->checkHideLibsMX->setChecked(checked);
}

// On change flatpak remote
void MainWindow::comboRemote_activated(int /*index*/)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    lastItemClicked = nullptr;
    displayFlatpaks(true);
}

void MainWindow::pushUpgradeFP_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();
    setCursor(QCursor(Qt::BusyCursor));
    enableOutput();
    if (cmd.run("socat SYSTEM:'flatpak update " + fpUser + "',pty STDIO")) {
        displayFlatpaks(true);
        setCursor(QCursor(Qt::ArrowCursor));
        QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
        QMessageBox::critical(this, tr("Error"),
                              tr("Problem detected while installing, please inspect the console output."));
    }
    enableTabs(true);
}

void MainWindow::pushRemotes_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    auto *dialog = new ManageRemotes(this, fpUser);
    dialog->exec();
    if (dialog->isChanged()) {
        invalidateFlatpakRemoteCache();
        listFlatpakRemotes();
        displayFlatpaks(true);
    }
    if (!dialog->getInstallRef().isEmpty()) {
        showOutput();
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        if (cmd.run("socat SYSTEM:'flatpak install -y " + dialog->getUser() + "--from "
                    + dialog->getInstallRef().replace(':', "\\:") + "',stderr STDIO\"")) {
            invalidateFlatpakRemoteCache();
            listFlatpakRemotes();
            displayFlatpaks(true);
            setCursor(QCursor(Qt::ArrowCursor));
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            ui->tabWidget->blockSignals(false);
        } else {
            setCursor(QCursor(Qt::ArrowCursor));
            QMessageBox::critical(this, tr("Error"),
                                  tr("Problem detected while installing, please inspect the console output."));
        }
        enableTabs(true);
    }
}

void MainWindow::comboUser_currentIndexChanged(int index)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (index == 0) {
        fpUser = "--system ";
    } else {
        fpUser = "--user ";
        static bool updated = false;
        if (!updated) {
            setCursor(QCursor(Qt::BusyCursor));
            enableOutput();
            Cmd().run("/usr/lib/mx-packageinstaller/mxpi-lib flatpak_add_repos_user", Cmd::QuietMode::Yes);
            setCursor(QCursor(Qt::ArrowCursor));
            updated = true;
        }
    }
    lastItemClicked = nullptr;
    invalidateFlatpakRemoteCache();
    listFlatpakRemotes();
    displayFlatpaks(true);
}

void MainWindow::treePopularApps_customContextMenuRequested(QPoint pos)
{
    auto *t_widget = qobject_cast<QTreeWidget *>(focusWidget());
    if (t_widget->currentItem()->parent() == nullptr) { // skip categories
        return;
    }
    auto *action = new QAction(QIcon::fromTheme("dialog-information"), tr("More &info..."), this);
    QMenu menu(this);
    menu.addAction(action);
    connect(action, &QAction::triggered, this, [this, t_widget] { displayPopularInfo(t_widget->currentItem(), 3); });
    menu.exec(ui->treePopularApps->mapToGlobal(pos));
}

// Process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        pushCancel_clicked();
    } else if (event->matches(QKeySequence::Find)
               || (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_F)) {
        if (ui->tabWidget->currentWidget() == ui->tabPopular) {
            ui->searchPopular->setFocus();
        } else if (ui->tabWidget->currentWidget() == ui->tabEnabled) {
            ui->searchBoxEnabled->setFocus();
        } else if (ui->tabWidget->currentWidget() == ui->tabMXtest) {
            ui->searchBoxMX->setFocus();
        } else if (ui->tabWidget->currentWidget() == ui->tabBackports) {
            ui->searchBoxBP->setFocus();
        } else if (ui->tabWidget->currentWidget() == ui->tabFlatpak) {
            ui->searchBoxFlatpak->setFocus();
        }
    }
}

void MainWindow::treePopularApps_itemChanged(QTreeWidgetItem *item)
{
    // Set current item if checked
    if (item->checkState(PopCol::Check) == Qt::Checked) {
        ui->treePopularApps->setCurrentItem(item);
    }

    // Scan tree to determine state
    bool hasCheckedItems = false;
    bool allCheckedAreInstalled = true;

    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
            hasCheckedItems = true;
            if ((*it)->icon(PopCol::Check).isNull()) {
                allCheckedAreInstalled = false;
                break;
            }
        }
    }

    // Update UI state
    ui->pushInstall->setEnabled(hasCheckedItems);
    ui->pushUninstall->setEnabled(hasCheckedItems && allCheckedAreInstalled);
    ui->pushInstall->setText(hasCheckedItems && allCheckedAreInstalled ? tr("Reinstall") : tr("Install"));
}

void MainWindow::pushRemoveUnused_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();
    setCursor(QCursor(Qt::BusyCursor));
    enableOutput();
    if (cmd.run("socat SYSTEM:'flatpak uninstall --unused -y',pty STDIO")) {
        displayFlatpaks(true);
        setCursor(QCursor(Qt::ArrowCursor));
        QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
        QMessageBox::critical(this, tr("Error"),
                              tr("Problem detected during last operation, please inspect the console output."));
    }
    enableTabs(true);
}

QString MainWindow::getMXTestRepoUrl()
{
    // Try to get test repo URL directly
    if (cmd.run("apt-get update --print-uris | tac | grep -m1 -oP 'https?://.*/mx/testrepo/dists/(?=" + verName
                + "/test/)'")) {
        return cmd.readAllOutput();
    }

    // Fall back to deriving from main repo URL
    if (cmd.run("apt-get update --print-uris | tac | grep -m1 -oE 'https?://.*/mx/repo/dists/" + verName
                + "/main/' | sed -e 's:/mx/repo/dists/" + verName
                + "/main/:/mx/testrepo/dists/:' | grep -oE 'https?://.*/mx/testrepo/dists/'")) {
        return cmd.readAllOutput();
    }

    // Return default URL if nothing else works
    return "https://mxrepo.com/mx/testrepo/dists/";
}

void MainWindow::pushRemoveAutoremovable_clicked()
{
    QString names = cmd.getOutAsRoot(R"(apt-get --dry-run autoremove |grep -Po '^Remv \K[^ ]+' |tr '\n' ' ')");
    QMessageBox::warning(this, tr("Warning"),
                         tr("Potentially dangerous operation.\nPlease make sure you check "
                            "carefully the list of packages to be removed."));
    showOutput();
    bool success = uninstall(names);
    setDirty();
    buildPackageLists();
    refreshPopularApps();
    if (success) {
        QMessageBox::information(this, tr("Success"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(currentTree->parentWidget());
    } else {
        QMessageBox::critical(this, tr("Error"), tr("We encountered a problem uninstalling the program"));
    }
    enableTabs(true);
    ui->tabWidget->setCurrentIndex(Tab::EnabledRepos);
}

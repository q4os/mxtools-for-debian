/**********************************************************************
 *  MainWindow.cpp
 **********************************************************************
 * Copyright (C) 2017-2023 MX Authors
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
#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QtGlobal>
#include <QtXml/QtXml>

#include "about.h"
#include "aptcache.h"
#include "versionnumber.h"
#include <algorithm>
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(const QCommandLineParser &arg_parser, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow),
      dictionary("/usr/share/mx-packageinstaller-pkglist/category.dict", QSettings::IniFormat),
      args {arg_parser},
      reply(nullptr)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << QCoreApplication::applicationVersion();
    ui->setupUi(this);
    setProgressDialog();

    connect(&timer, &QTimer::timeout, this, &MainWindow::updateBar);
    connect(&cmd, &Cmd::started, this, &MainWindow::cmdStart);
    connect(&cmd, &Cmd::done, this, &MainWindow::cmdDone);
    connect(&cmd, &Cmd::outputAvailable, [](const QString &out) { qDebug() << out.trimmed(); });
    connect(&cmd, &Cmd::errorAvailable, [](const QString &out) { qWarning() << out.trimmed(); });
    setWindowFlags(Qt::Window); // For the close, min and max buttons

    // Start displayPackage and displayFlatpaks in the background
    QTimer::singleShot(0, this, [this] {
        setup();
        QApplication::processEvents();
        {
            AptCache cache;
            enabled_list = cache.getCandidates();
            displayPackages();
            ui->tabWidget->setTabEnabled(Tab::Test, true);
            ui->tabWidget->setTabEnabled(Tab::Backports, true);
        }
        if (arch != "i386" && checkInstalled("flatpak")) {
            if (!Cmd().run("flatpak remote-list --system --columns=name | grep -qw flathub", true)) {
                Cmd().runAsRoot(
                    "flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo");
            }
            if (!Cmd().run("flatpak remote-list --system --columns=name | grep -qw flathub-verified", true)) {
                Cmd().runAsRoot("flatpak remote-add --if-not-exists --subset=verified flathub-verified "
                                "https://flathub.org/repo/flathub.flatpakrepo");
            }
            displayFlatpaks();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    //hide mx test tab, if mxtools not installed
    if(! pm_files.get_q4os_mxtools_installed())
      ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tabMXtest), false);
    //hide mx test tab, if "mx/testrepo" repository doesn't exist in sources.list
    if(! pm_files.get_has_mx_repo())
      ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tabMXtest), false);
    //hide debian backports tab if mxtools not installed and backports not activated
    if((! pm_files.get_q4os_mxtools_installed()) and (! pm_files.get_has_debian_backports_repo()))
      ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tabBackports), false);
    //hide debian backports tab for ubuntu based systems
    if(pm_files.get_is_ubuntu_based())
      ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(ui->tabBackports), false);

    ui->checkBoxInstallRecommends->setChecked(true);
    //ui->checkBoxInstallRecommendsMX->setChecked(true);
    //ui->checkBoxInstallRecommendsMXBP->setChecked(true);

    ui->tabWidget->blockSignals(true);
    ui->tabWidget->setCurrentWidget(ui->tabPopular);
    ui->tabWidget->setTabEnabled(Tab::Test, false);
    ui->tabWidget->setTabEnabled(Tab::Backports, false);
    ui->pushRemoveAutoremovable->setHidden(true);

    QFont font("monospace");
    font.setStyleHint(QFont::Monospace);
    ui->outputBox->setFont(font);

    QString defaultFSUser = settings.value("FlatpakUser", tr("For all users")).toString();
    FPuser = defaultFSUser == tr("For all users") ? "--system " : "--user ";
    ui->comboUser->blockSignals(true);
    ui->comboUser->setCurrentText(defaultFSUser);
    ui->comboUser->blockSignals(false);

    arch = AptCache::getArch();
    ver_name = getDebianVerName();

    ui->tabWidget->setTabVisible(Tab::Flatpak, arch != "i386");
    ui->tabWidget->setTabVisible(Tab::Test, QFile::exists("/etc/apt/sources.list.d/mx.list"));

    test_initially_enabled
        = cmd.run("apt-get update --print-uris | grep -m1 -qE '/mx/testrepo/dists/" + ver_name + "/test/'");

    setWindowTitle(tr("Debian Package Installer"));
    hideColumns();
    setIcons();
    loadPmFiles();
    refreshPopularApps();
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
        if (lock_file.isLockedGUI()) {
            return false;
        }
        success = cmd.runAsRoot(preuninstall);
    }

    if (success) {
        enableOutput();
        if (lock_file.isLockedGUI()) {
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
        if (lock_file.isLockedGUI()) {
            return false;
        }
        success = cmd.runAsRoot(postuninstall);
    }
    return success;
}

bool MainWindow::updateApt()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (lock_file.isLockedGUI()) {
        return false;
    }
    ui->tabOutput->isVisible() // Don't display in output if calling to refresh from tabs
        ? ui->tabWidget->setTabText(Tab::Output, tr("Refreshing sources..."))
        : progress->show();
    if (!timer.isActive()) {
        timer.start(100ms);
    }

    enableOutput();
    if (cmd.runAsRoot("apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o "
                      "Acquire::ftp:Timeout=10")) {
        qDebug() << "sources updated OK";
        updated_once = true;
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
    QString number = size.section(QChar(160), 0, 0);
    QString unit = size.section(QChar(160), 1).toUpper();
    double value = number.toDouble();
    if (unit == "KB") {
        return static_cast<quint64>(value * KiB);
    } else if (unit == "MB") {
        return static_cast<quint64>(value * MiB);
    } else if (unit == "GB") {
        return static_cast<quint64>(value * GiB);
    } else { // Bytes
        return static_cast<quint64>(value);
    }
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

    QStringList list = cmd.getOut("flatpak list " + FPuser + "--columns app,size").split('\n', Qt::SkipEmptyParts);
    auto total = std::accumulate(list.cbegin(), list.cend(), quint64(0),
                                 [](quint64 acc, const QString &item) { return acc + convert(item.section('\t', 1)); });
    ui->labelNumSize->setText(convert(total));
}

// Block interface while updating Flatpak list
void MainWindow::blockInterfaceFP(bool block)
{
    ui->tabWidget->widget(Tab::Flatpak)->setEnabled(!block);
    ui->comboRemote->setDisabled(block);
    ui->comboFilterFlatpak->setDisabled(block);
    ui->comboUser->setDisabled(block);
    ui->searchBoxFlatpak->setDisabled(block);
    ui->treeFlatpak->setDisabled(block);
    ui->frameFP->setDisabled(block);
    ui->iconInstalledPackages_4->setDisabled(block);
    ui->labelRepo->setDisabled(block);
    block ? setCursor(QCursor(Qt::BusyCursor)) : setCursor(QCursor(Qt::ArrowCursor));
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
    int upgr_count = 0;
    int inst_count = 0;

    for (QTreeWidgetItemIterator it(currentTree); *it; ++it) {
        auto userData = (*it)->data(TreeCol::Status, Qt::UserRole).toInt();
        switch (userData) {
        case Status::Upgradable:
            ++upgr_count;
            break;
        case Status::Installed:
            ++inst_count;
            break;
        }
        (*it)->setHidden(false);
    }

    auto updateLabelsAndFocus = [&](QLabel *labelNumApps, QLabel *labelNumUpgr, QLabel *labelNumInst,
                                    QPushButton *pushForceUpdate, QLineEdit *searchBox) {
        labelNumApps->setText(QString::number(currentTree->topLevelItemCount()));
        labelNumUpgr->setText(QString::number(upgr_count));
        labelNumInst->setText(QString::number(inst_count + upgr_count));
        pushForceUpdate->setEnabled(true);
        searchBox->setFocus();
    };

    switch (ui->tabWidget->currentIndex()) {
    case Tab::EnabledRepos:
        ui->pushUpgradeAll->setVisible(upgr_count > 0);
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
    QStringList list;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        list = line.split('.');
        file.close();
    } else {
        qCritical() << "Could not open /etc/debian_version:" << file.errorString() << "Assumes Bullseye";
        return Release::Bullseye;
    }
    bool ok = false;
    int ver = list.at(0).toInt(&ok);
    if (ok) {
        return ver;
    } else {
        QString verName = list.at(0).split('/').at(0);
        if (verName == "bullseye") {
            return Release::Bullseye;
        } else if (verName == "bookworm") {
            return Release::Bookworm;
        } else {
            qCritical() << "Unknown Debian version:" << ver << "Assumes Bullseye";
            return Release::Bullseye;
        }
    }
}

QString MainWindow::getDebianVerName()
{
    switch (getDebianVerNum()) {
    case Release::Jessie:
        return "jessie";
    case Release::Stretch:
        return "stretch";
    case Release::Buster:
        return "buster";
    case Release::Bullseye:
        return "bullseye";
    case Release::Bookworm:
        return "bookworm";
    case Release::Trixie:
        return "trixie";
    default:
        qWarning() << "Error: Invalid Debian version, assumes Bullseye";
        return "bullseye";
    }
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
    if (locale.name() == "en_US") {
        return item; // No need for translation
    }
    QStringList tagCandidates = {locale.name(), locale.name().section('_', 0, 0)};
    for (const auto &tag : tagCandidates) {
        dictionary.beginGroup(item);
        QString translation = dictionary.value(tag).toString().toLatin1();
        dictionary.endGroup();

        if (!translation.isEmpty()) {
            return translation;
        }
    }
    return item; // Return original item if no translation found
}

QString MainWindow::getArchOption() const
{
    static const QMap<QString, QString> archMap = {
        {"amd64", "--arch=x86_64 "}, {"i386", "--arch=i386 "}, {"armhf", "--arch=arm "}, {"arm64", "--arch=aarch64 "}};
    return archMap.value(arch, QString());
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
    ui->outputBox->moveCursor(QTextCursor::End);
    if (output.contains('\r')) {
        ui->outputBox->moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
        ui->outputBox->moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    ui->outputBox->insertPlainText(output);
    ui->outputBox->verticalScrollBar()->setValue(ui->outputBox->verticalScrollBar()->maximum());
}

void MainWindow::loadPmFiles()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    const QString pmFolderPath {"/usr/share/mx-packageinstaller-pkglist"};
    const QStringList pmFileList = QDir(pmFolderPath).entryList({"*.pm"});

    QDomDocument doc;
    for (const QString &fileName : pmFileList) {
        QFile file(pmFolderPath + '/' + fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Could not open file:" << file.fileName();
        } else if (!pm_files.checkpm(fileName)) {
            qDebug() << "Not loading file: " << fileName << "-- not enabled.";
        } else if (!doc.setContent(&file)) {
            qDebug() << "Could not load document:" << fileName << "-- not valid XML?";
        } else {
            processDoc(doc);
        }
        file.close();
    }
}

// Process dom documents (from .pm files)
void MainWindow::processDoc(const QDomDocument &doc)
{
    PopularInfo info;
    QDomElement root = doc.firstChildElement("app");
    QDomElement element = root.firstChildElement();
    while (!element.isNull()) {
        const QString tagName = element.tagName();
        QString trimmedText = element.text().trimmed();

        if (tagName == "category") {
            info.category = categoryTranslation(trimmedText);
        } else if (tagName == "name") {
            info.name = trimmedText;
        } else if (tagName == "description") {
            info.description = getLocalizedName(element);
        } else if (tagName == "installable") {
            info.installable = trimmedText;
        } else if (tagName == "screenshot") {
            info.screenshot = trimmedText;
        } else if (tagName == "preinstall") {
            info.preInstall = trimmedText;
        } else if (tagName == "install_package_names") {
            info.installNames = trimmedText.replace('\n', ' ');
        } else if (tagName == "postinstall") {
            info.postInstall = trimmedText;
        } else if (tagName == "uninstall_package_names") {
            info.uninstallNames = trimmedText;
        } else if (tagName == "postuninstall") {
            info.postUninstall = trimmedText;
        } else if (tagName == "preuninstall") {
            info.preUninstall = trimmedText;
        } else if (tagName == QLatin1String("qdistro")) {
            info.qDistro = trimmedText;
        }
        element = element.nextSiblingElement();
    }
    const QString modArch = mapArchToFormat(arch);
    if (!isPackageInstallable(info.installable, modArch)) {
        return;
    }
    popular_apps.append(info);
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

void MainWindow::refreshPopularApps()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    disableOutput();
    ui->treePopularApps->clear();
    ui->searchPopular->clear();
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);
    installed_packages = listInstalled();
    displayPopularApps();
}

// In case of duplicates add extra name to disambiguate
void MainWindow::removeDuplicatesFP() const
{
    QTreeWidgetItemIterator it(ui->treeFlatpak);
    QTreeWidgetItem *prevItem = nullptr;
    QSet<QString> namesSet;
    ui->treeFlatpak->setUpdatesEnabled(false);

    // Find and mark duplicates
    while (*it) {
        QString currentName = (*it)->text(FlatCol::Name);
        if (namesSet.contains(currentName)) {
            // Mark both occurrences as duplicate
            if (prevItem) {
                prevItem->setData(FlatCol::Duplicate, Qt::UserRole, true);
            }
            (*it)->setData(FlatCol::Duplicate, Qt::UserRole, true);
        } else {
            namesSet.insert(currentName);
        }
        prevItem = *it;
        ++it;
    }

    it = QTreeWidgetItemIterator(ui->treeFlatpak); // Rewind iterator
    // Rename duplicates to use more context
    while (*it) {
        if ((*it)->data(FlatCol::Duplicate, Qt::UserRole).toBool()) {
            QString longName = (*it)->text(FlatCol::LongName);
            (*it)->setText(FlatCol::Name, longName.section('.', -2));
        }
        ++it;
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
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);
    bar->setMaximum(bar->maximum());
    pushCancel = new QPushButton(tr("Cancel"));
    connect(pushCancel, &QPushButton::clicked, this, &MainWindow::cancelDownload);
    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
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
    switch (ui->tabWidget->currentIndex()) {
    case Tab::EnabledRepos:
        ui->searchBoxEnabled->setFocus();
        break;
    case Tab::Test:
        ui->searchBoxMX->setFocus();
        break;
    case Tab::Backports:
        ui->searchBoxBP->setFocus();
        break;
    case Tab::Flatpak:
        ui->searchBoxFlatpak->setFocus();
        break;
    default:
        break;
    }
}

void MainWindow::displayPopularApps() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    QMap<QString, QTreeWidgetItem *> categoryMap;
    ui->treePopularApps->setUpdatesEnabled(false);

    for (const auto &item : popular_apps) {
        QTreeWidgetItem *topLevelItem = nullptr;

        // Check if the category already exists, if not, create it
        if (!categoryMap.contains(item.category)) {
            topLevelItem = new QTreeWidgetItem();
            topLevelItem->setText(PopCol::Icon, item.category);
            ui->treePopularApps->addTopLevelItem(topLevelItem);

            QFont font;
            font.setBold(true);
            topLevelItem->setFont(PopCol::Icon, font);
            topLevelItem->setIcon(PopCol::Icon, QIcon::fromTheme("folder"));
            topLevelItem->setFirstColumnSpanned(true);

            categoryMap.insert(item.category, topLevelItem);
        } else {
            topLevelItem = categoryMap.value(item.category);
        }

        // Add package name as childItem to treePopularApps
        auto *childItem = new QTreeWidgetItem(topLevelItem);
        childItem->setText(PopCol::Name, item.name);
        childItem->setIcon(PopCol::Info, QIcon::fromTheme("dialog-information"));
        childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
        childItem->setCheckState(PopCol::Check, Qt::Unchecked);
        childItem->setText(PopCol::Description, item.description);
        childItem->setText(PopCol::InstallNames, item.installNames);

        childItem->setData(PopCol::UninstallNames, Qt::UserRole, item.uninstallNames);
        childItem->setData(PopCol::Screenshot, Qt::UserRole, item.screenshot);
        childItem->setData(PopCol::PostUninstall, Qt::UserRole, item.postUninstall);
        childItem->setData(PopCol::PreUninstall, Qt::UserRole, item.preUninstall);
        childItem->setData(PopCol::QDistro, Qt::UserRole, item.qDistro);
        if (checkInstalled(item.uninstallNames)) {
            childItem->setIcon(PopCol::Check, qicon_installed);
        }
    }

    for (int i = 0; i < ui->treePopularApps->columnCount(); ++i) {
        ui->treePopularApps->resizeColumnToContents(i);
    }

    ui->treePopularApps->sortItems(PopCol::Icon, Qt::AscendingOrder);
    connect(ui->treePopularApps, &QTreeWidget::itemClicked, this, &MainWindow::displayPopularInfo,
            Qt::UniqueConnection);
    ui->treePopularApps->setUpdatesEnabled(true);
}

void MainWindow::displayFilteredFP(QStringList list, bool raw)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->treeFlatpak->blockSignals(true);
    ui->treeFlatpak->setUpdatesEnabled(false);

    QMutableStringListIterator i(list);
    if (raw) { // Raw format that needs to be edited
        while (i.hasNext()) {
            i.setValue(i.next().section('\t', 1, 1).section('/', 1)); // Remove version and size
        }
    }
    uint total = 0;
    for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
        if (list.contains((*it)->data(FlatCol::FullName, Qt::UserRole).toString())) {
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
                change_list.removeOne((*it)->data(FlatCol::FullName, Qt::UserRole).toString());
            }
        }
        if (change_list.isEmpty()) { // Reset comboFilterFlatpak if nothing is selected
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
}

void MainWindow::displayPackages()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    displayPackagesIsRunning = true;

    auto *newtree = getCurrentTree();
    if (!newtree) {
        displayPackagesIsRunning = false;
        return;
    }

    QMap<QString, PackageInfo> *list = getCurrentList();
    if (!list) {
        displayPackagesIsRunning = false;
        return;
    }

    newtree->blockSignals(true);
    newtree->setUpdatesEnabled(false);
    newtree->clear();

    auto items = createTreeItemsList(list);

    newtree->addTopLevelItems(items);
    newtree->sortItems(TreeCol::Name, Qt::AscendingOrder);

    updateTreeItems(newtree);
    displayAutoremovable(newtree);

    newtree->blockSignals(false);
    newtree->setUpdatesEnabled(true);
    displayPackagesIsRunning = false;
    emit displayPackagesFinished();
}

void MainWindow::displayAutoremovable(const QTreeWidget *newtree)
{
    if (newtree != ui->treePopularApps && newtree != ui->treeFlatpak) {
        QStringList names = cmd.getOut(R"(apt-get --dry-run autoremove | grep -Po '^Remv \K[^ ]+' | tr '\n' ' ')")
                                .split(' ', Qt::SkipEmptyParts);
        if (!names.isEmpty()) {
            ui->pushRemoveAutoremovable->setVisible(true);
            for (const QString &name : names) {
                auto matchingItems = newtree->findItems(name, Qt::MatchExactly | Qt::MatchRecursive, TreeCol::Name);
                for (QTreeWidgetItem *item : matchingItems) {
                    item->setData(TreeCol::Status, Qt::UserRole, Status::Autoremovable);
                }
            }
        } else {
            ui->pushRemoveAutoremovable->setVisible(false);
        }
    }
}

QTreeWidget *MainWindow::getCurrentTree()
{
    if (currentTree == ui->treeMXtest) {
        if (!dirtyTest) {
            return nullptr;
        }
        dirtyTest = false;
        return ui->treeMXtest;
    } else if (currentTree == ui->treeBackports) {
        if (!dirtyBackports) {
            return nullptr;
        }
        dirtyBackports = false;
        return ui->treeBackports;
    } else {
        if (!dirtyEnabledRepos) {
            return nullptr;
        }
        dirtyEnabledRepos = false;
        return ui->treeEnabled;
    }
}

QMap<QString, PackageInfo> *MainWindow::getCurrentList()
{
    if (currentTree == ui->treeMXtest) {
        return &mx_list;
    } else if (currentTree == ui->treeBackports) {
        return &backports_list;
    } else {
        return &enabled_list;
    }
}

QList<QTreeWidgetItem *> MainWindow::createTreeItemsList(QMap<QString, PackageInfo> *list) const
{
    QList<QTreeWidgetItem *> items;
    items.reserve(list->size() + installed_packages.size());

    for (auto it = list->constBegin(); it != list->constEnd(); ++it) {
        items.append(createTreeItem(it.key(), it.value().version, it.value().description));
    }

    for (auto it = installed_packages.constBegin(); it != installed_packages.constEnd(); ++it) {
        if (!list->contains(it.key())) {
            items.append(createTreeItem(it.key(), QString(), it.value().description));
        }
    }

    return items;
}

void MainWindow::updateTreeItems(QTreeWidget *tree)
{
    int upgr_count = 0;
    int inst_count = 0;

    auto hashInstalled = listInstalledVersions();

    for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
        const QString app_name = (*it)->text(TreeCol::Name);
        if (ui->checkHideLibs->isChecked() && isFilteredName(app_name)) {
            (*it)->setHidden(true);
        }
        const QString app_ver = (*it)->text(TreeCol::RepoVersion);
        const VersionNumber installed = hashInstalled.value(app_name);
        (*it)->setText(TreeCol::InstalledVersion, installed.toString());
        const VersionNumber repo_candidate {app_ver};

        (*it)->setIcon(TreeCol::Check, QIcon());

        if (installed.toString().isEmpty()) {
            (*it)->setData(TreeCol::Status, Qt::UserRole, Status::NotInstalled);
        } else {
            ++inst_count;
            if (installed >= repo_candidate) {
                (*it)->setIcon(TreeCol::Check, qicon_installed);
                (*it)->setData(TreeCol::Status, Qt::UserRole, Status::Installed);
            } else {
                ++upgr_count;
                (*it)->setIcon(TreeCol::Check, qicon_upgradable);
                (*it)->setData(TreeCol::Status, Qt::UserRole, Status::Upgradable);
            }
        }
    }

    for (int i = 0; i < tree->columnCount(); ++i) {
        tree->resizeColumnToContents(i);
    }
}

void MainWindow::displayFlatpaks(bool force_update)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->treeFlatpak->setUpdatesEnabled(false);
    displayFlatpaksIsRunning = true;
    lastItemClicked = nullptr;
    if (flatpaks.isEmpty() || force_update) {
        if (ui->tabWidget->currentIndex() == Tab::Flatpak) {
            setCursor(QCursor(Qt::BusyCursor));
        }
        listFlatpakRemotes();
        ui->treeFlatpak->blockSignals(true);
        ui->treeFlatpak->clear();
        change_list.clear();

        if (ui->tabWidget->currentIndex() == Tab::Flatpak) {
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
            }
        }
        blockInterfaceFP(true);
        flatpaks = listFlatpaks(ui->comboRemote->currentText());
        flatpaks_apps.clear();
        flatpaks_runtimes.clear();

        // List installed packages
        installed_apps_fp = listInstalledFlatpaks("--app");

        // Add runtimes (needed for older flatpak versions)
        installed_runtimes_fp = listInstalledFlatpaks("--runtime");
        uint total_count = 0;
        QTreeWidgetItem *widget_item {nullptr};
        for (QString item : qAsConst(flatpaks)) {
            QString size = item.section('\t', -1);
            QString version = item.section('\t', 0, 0);
            item = item.section('\t', 1, 1).section('/', 1);
            if (version.isEmpty()) {
                version = item.section('/', -1);
            }
            QString long_name = item.section('/', 0, 0);
            QString short_name = long_name.section('.', -1);
            if (short_name == QLatin1String("Locale") || short_name == QLatin1String("Sources")
                || short_name == QLatin1String("Debug")) { // Skip Locale, Sources, Debug
                continue;
            }
            ++total_count;
            widget_item = new QTreeWidgetItem(ui->treeFlatpak);
            widget_item->setCheckState(FlatCol::Check, Qt::Unchecked);
            widget_item->setText(FlatCol::Name, short_name);
            widget_item->setText(FlatCol::LongName, long_name);
            widget_item->setText(FlatCol::Version, version);
            widget_item->setText(FlatCol::Size, size);
            widget_item->setData(FlatCol::FullName, Qt::UserRole, item); // Full string
            QStringList installed_all {installed_apps_fp + installed_runtimes_fp};
            if (installed_all.contains(item)) {
                widget_item->setIcon(FlatCol::Check, QIcon::fromTheme("package-installed-updated",
                                                                      QIcon(":/icons/package-installed-updated.png")));
                widget_item->setData(FlatCol::Status, Qt::UserRole, Status::Installed);
            } else {
                widget_item->setData(FlatCol::Status, Qt::UserRole, Status::NotInstalled);
            }
            widget_item->setData(0, Qt::UserRole, true); // all items are displayed till filtered
        }
        // Add sizes for the installed packages for older flatpak that doesn't list size for all the packages
        listSizeInstalledFP();
        ui->labelNumAppFP->setText(QString::number(total_count));

        uint total = 0;
        if (installed_apps_fp != QStringList("")) {
            total = installed_apps_fp.count();
        }
        ui->labelNumInstFP->setText(QString::number(total));
        ui->treeFlatpak->sortByColumn(FlatCol::Name, Qt::AscendingOrder);
        removeDuplicatesFP();
        for (int i = 0; i < ui->treeFlatpak->columnCount(); ++i) {
            ui->treeFlatpak->resizeColumnToContents(i);
        }
    }
    ui->treeFlatpak->blockSignals(false);
    if (ui->tabWidget->currentIndex() == Tab::Flatpak && !ui->comboFilterFlatpak->currentText().isEmpty()) {
        filterChanged(ui->comboFilterFlatpak->currentText());
    }
    if (ui->tabWidget->currentIndex() == Tab::Flatpak) {
        ui->searchBoxFlatpak->setFocus();
    }
    displayFlatpaksIsRunning = false;
    firstRunFP = false;
    blockInterfaceFP(false);
    ui->treeFlatpak->setUpdatesEnabled(true);
}

void MainWindow::displayWarning(const QString &repo)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    bool *displayed = nullptr;
    QString msg;
    QString key;

    if (repo == "test") {
        displayed = &warning_test;
        key = "NoWarningTest";
        msg = tr("You are about to use the MX Test repository, whose packages are provided for "
                 "testing purposes only. It is possible that they might break your system, so it "
                 "is suggested that you back up your system and install or update only one package "
                 "at a time. Please provide feedback in the Forum so the package can be evaluated "
                 "before moving up to Main.");

    } else if (repo == "backports") {
        displayed = &warning_backports;
        key = "NoWarningBackports";
        msg = tr("You are about to use Debian Backports, which contains packages taken from the next "
                 "Debian release (called 'testing'), adjusted and recompiled for usage on Debian stable. "
                 "They cannot be tested as extensively as in the stable releases of Debian and MX Linux, "
                 "and are provided on an as-is basis, with risk of incompatibilities with other components "
                 "in Debian stable. Use with care!");
    } else if (repo == "flatpaks") {
        displayed = &warning_flatpaks;
        key = "NoWarningFlatpaks";
        msg = tr("Debian includes this repository of flatpaks for the users' convenience only, and "
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
    ui->tabWidget->setCurrentWidget(ui->tabPopular);
}

void MainWindow::listFlatpakRemotes() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QString currentRemote = ui->comboRemote->currentText();
    ui->comboRemote->blockSignals(true);
    ui->comboRemote->clear();
    Cmd shell;
    QStringList list = shell.getOut("flatpak remote-list " + FPuser + "| cut -f1").remove(' ').split('\n');
    if (shell.exitCode() != 0) {
        return;
    }
    ui->comboRemote->addItems(list);
    QString savedRemote = firstRunFP ? settings.value("FlatpakRemote", "flathub").toString() : currentRemote;
    ui->comboRemote->setCurrentText(savedRemote.isEmpty() ? "flathub" : savedRemote);
    ui->comboRemote->blockSignals(false);
}

bool MainWindow::confirmActions(const QString &names, const QString &action)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    qDebug() << "names" << names << "and" << change_list;
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
        detailed_installed_names = change_list;
    } else if (currentTree == ui->treeBackports) {
        recommends = (ui->checkBoxInstallRecommendsMXBP->isChecked()) ? "--install-recommends " : "";
        recommends_aptitude
            = (ui->checkBoxInstallRecommendsMXBP->isChecked()) ? "--with-recommends " : "--without-recommends ";
        detailed_names = cmd.getOutAsRoot(
            frontend + aptget + action + ' ' + recommends + "-t " + ver_name + "-backports --reinstall " + names
            + R"lit(|grep 'Inst\|Remv' | awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info = cmd.getOutAsRoot(frontend + aptitude + action + ' ' + recommends_aptitude + "-t " + ver_name
                                         + "-backports " + names + " |tail -2 |head -1");
    } else if (currentTree == ui->treeMXtest) {
        recommends = (ui->checkBoxInstallRecommendsMX->isChecked()) ? "--install-recommends " : "";
        recommends_aptitude
            = (ui->checkBoxInstallRecommendsMX->isChecked()) ? "--with-recommends " : "--without-recommends ";
        detailed_names = cmd.getOutAsRoot(
            frontend + aptget + action + " -t mx " + recommends + "--reinstall " + names
            + R"lit(|grep 'Inst\|Remv' | awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info = cmd.getOutAsRoot(frontend + aptitude + action + " -t mx " + recommends_aptitude + names
                                         + " |tail -2 |head -1");
    } else {
        recommends = (ui->checkBoxInstallRecommends->isChecked()) ? "--install-recommends " : "";
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
            detailed_removed_names = change_list.join('\n');
            detailed_to_install.clear();
        }
        if (action == QLatin1String("install")) {
            detailed_to_install = change_list.join('\n');
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

    // find Detailed Info box and set heigth, set box height between 100 - 400 depending on length of content
    const auto min = 100;
    const auto max = 400;
    auto *const detailedInfo = msgBox.findChild<QTextEdit *>();
    const auto recommended = qMax(msgBox.detailedText().length() / 2, min); // half of length is just guesswork
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

    if (lock_file.isLockedGUI()) {
        return false;
    }
    QString recommends;
    bool success = false;
    if (currentTree == ui->treeBackports) {
        recommends = (ui->checkBoxInstallRecommendsMXBP->isChecked()) ? "--install-recommends " : "";
        success = cmd.runAsRoot(frontend + aptget + recommends + "-t " + ver_name + "-backports --reinstall " + names);
    } else if (currentTree == ui->treeMXtest) {
        recommends = (ui->checkBoxInstallRecommendsMX->isChecked()) ? "--install-recommends " : "";
        success = cmd.runAsRoot(frontend + aptget + recommends + " -t mx " + names);
    } else {
        recommends = (ui->checkBoxInstallRecommends->isChecked()) ? "--install-recommends " : "";
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
        for (const auto &item : qAsConst(popular_apps)) {
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
        if (lock_file.isLockedGUI()) {
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
    for (const auto &item : qAsConst(popular_apps)) {
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
        if (lock_file.isLockedGUI()) {
            return false;
        }
        if (!cmd.runAsRoot(preinstall)) {
            if (QFile::exists(temp_list)) {
                QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"};
                Cmd().run(elevate + " /usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", true);
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
        if (lock_file.isLockedGUI()) {
            return false;
        }
        cmd.runAsRoot(postinstall);
    }
    if (QFile::exists(temp_list)) {
        QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"};
        Cmd().run(elevate + " /usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", true);
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
    if (!updated_once) {
        updateApt();
    }

    // Make a list of apps to be installed together
    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
            QString name = (*it)->text(2);
            for (const auto &item : qAsConst(popular_apps)) {
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
    return result;
}

bool MainWindow::installSelected()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setTabEnabled(Tab::Output, true);
    QString names = change_list.join(' ');

    // Change sources as needed
    if (currentTree == ui->treeMXtest) {
        // Add testrepo unless already enabled
        if (!test_initially_enabled) {
            QString suite = ver_name;
            if (arch == "amd64") {
                cmd.runAsRoot("apt-get update --print-uris | tac | "
                              "grep -m1 -oE 'https?://.*/mx/repo/dists/"
                              + suite + "/main' | sed 's:^:deb :; s:/repo/dists/:/testrepo :; s:/main: test:' > "
                              + temp_list);
            } else {
                cmd.runAsRoot("apt-get update --print-uris | tac | "
                              "grep -m1 -oE 'https?://.*/mx/repo/dists/"
                              + suite
                              + "/main' | sed 's:^:deb [arch='$(dpkg --print-architecture)'] :; "
                                "s:/repo/dists/:/testrepo :; s:/main: test:' > "
                              + temp_list);
            }
        }
        updateApt();
    } else if (currentTree == ui->treeBackports) {
        cmd.runAsRoot("echo deb http://ftp.debian.org/debian " + ver_name + "-backports main contrib non-free > "
                      + temp_list);
        updateApt();
    }
    bool result = install(names);
    if (currentTree == ui->treeBackports || currentTree == ui->treeMXtest) {
        if (QFile::exists(temp_list)) {
            QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"};
            Cmd().run(elevate + " /usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", true);
            updateApt();
        }
    }
    change_list.clear();
    installed_packages = listInstalled();
    return result;
}

bool MainWindow::markKeep()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setTabEnabled(Tab::Output, true);
    QString names = change_list.join(' ');
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
        QMessageBox::warning(
            this, tr("Error"),
            tr("There was an error writing file: %1. Please check if you have enough free space on your drive")
                .arg(file.fileName()));
        qDebug() << "There was an error downloading the file:" << url << "Error:" << reply->errorString();
        file.remove();
        return false;
    }
    return true;
}

bool MainWindow::downloadAndUnzip(const QString &url, QFile &file)
{
    if (!downloadFile(url, file)) {
        file.remove();
        QFile::remove(QFileInfo(file.fileName()).path() + '/'
                      + QFileInfo(file.fileName()).baseName()); // rm unzipped file
        return false;
    }

    QString unzipCommand = (QFileInfo(file).suffix() == "gz") ? "gunzip -f " : "unxz -f ";
    if (!cmd.run(unzipCommand + file.fileName())) {
        qDebug() << "Could not unzip file:" << file.fileName();
        return false;
    }
    return true;
}

bool MainWindow::downloadAndUnzip(const QString &url, const QString &repo_name, const QString &branch,
                                  const QString &format, QFile &file)
{
    return downloadAndUnzip(url + repo_name + branch + "/binary-" + arch + "/Packages." + format, file);
}

bool MainWindow::buildPackageLists(bool force_download)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (force_download) {
        setDirty();
    }
    clearUi();
    if (!downloadPackageList(force_download)) {
        ifDownloadFailed();
        return false;
    }
    if (!readPackageList(force_download)) {
        ifDownloadFailed();
        return false;
    }
    displayPackages();
    return true;
}

// Download the Packages.gz from sources
bool MainWindow::downloadPackageList(bool force_download)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (!isOnline()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Internet is not available, won't be able to download the list of packages"));
        return false;
    }
    if (!tmp_dir.isValid()) {
        qDebug() << "Can't create temp folder";
        return false;
    }
    QDir::setCurrent(tmp_dir.path());
    progress->setLabelText(tr("Downloading package info..."));
    pushCancel->setEnabled(true);

    if (enabled_list.isEmpty() || force_download) {
        if (force_download) {
            if (!updateApt()) {
                return false;
            }
        }
        progress->show();
        if (!timer.isActive()) {
            timer.start(100ms);
        }
        AptCache cache;
        enabled_list = cache.getCandidates();
        if (enabled_list.isEmpty()) {
            updateApt();
            AptCache cache2;
            enabled_list = cache2.getCandidates();
        }
    }

    if (currentTree == ui->treeMXtest) {
        if (!QFile::exists(tmp_dir.path() + "/mxPackages") || force_download) {
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
            }

            QFile file(tmp_dir.path() + "/mxPackages.gz");
            QString url {"http://mxrepo.com/mx/testrepo/dists/"};
            if (!cmd.run("apt-get update --print-uris | tac | grep -m1 -oP 'https?://.*/mx/testrepo/dists/(?="
                         + ver_name + "/test/)'")) {
                cmd.run("apt-get update --print-uris | tac | grep -m1 -oE 'https?://.*/mx/repo/dists/" + ver_name
                        + "/main/' | sed -e 's:/mx/repo/dists/" + ver_name
                        + "/main/:/mx/testrepo/dists/:' | grep -oE 'https?://.*/mx/testrepo/dists/'");
            }
            url = cmd.readAllOutput();
            QString branch {"/test"};
            QString format {"gz"};
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }
        }
    } else if (currentTree == ui->treeBackports) {
        if (!QFile::exists(tmp_dir.path() + "/mainPackages") || !QFile::exists(tmp_dir.path() + "/contribPackages")
            || !QFile::exists(tmp_dir.path() + "/nonfreePackages") || force_download) {
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
            }

            QFile file(tmp_dir.path() + "/mainPackages.xz");
            QString url = "http://deb.debian.org/debian/dists/";
            QString branch {"-backports/main"};
            QString format {"xz"};
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }

            file.setFileName(tmp_dir.path() + "/contribPackages.xz");
            branch = "-backports/contrib";
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }

            file.setFileName(tmp_dir.path() + "/nonfreePackages.xz");
            branch = "-backports/non-free";
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }

            pushCancel->setDisabled(true);
            QFile outputFile("allPackages");
            if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qWarning() << "Could not open: " << outputFile.fileName();
            }

            QTextStream outStream(&outputFile);
            QFile inputFile1("mainPackages");
            QFile inputFile2("contribPackages");
            QFile inputFile3("nonfreePackages");

            if (!inputFile1.open(QIODevice::ReadOnly | QIODevice::Text)
                || !inputFile2.open(QIODevice::ReadOnly | QIODevice::Text)
                || !inputFile3.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning() << "Could not read file";
            }

            outStream << inputFile1.readAll();
            outStream << inputFile2.readAll();
            outStream << inputFile3.readAll();
            outputFile.close();
        }
    }
    return true;
}

void MainWindow::enableTabs(bool enable) const
{
    for (int tab = 0; tab < ui->tabWidget->count() - 1; ++tab) { // Enable all except last (Console)
        ui->tabWidget->setTabEnabled(tab, enable);
    }
    ui->tabWidget->setTabVisible(Tab::Test, QFile::exists("/etc/apt/sources.list.d/mx.list"));
    ui->tabWidget->setTabVisible(Tab::Flatpak, arch != "i386");
}

void MainWindow::hideColumns() const
{
    ui->tabWidget->setCurrentIndex(Tab::Popular);
    ui->treeEnabled->hideColumn(TreeCol::Status); // Status of the package: installed, upgradable, etc
    ui->treeMXtest->hideColumn(TreeCol::Status);
    ui->treeBackports->hideColumn(TreeCol::Status);
    ui->treeFlatpak->hideColumn(FlatCol::Status);
    ui->treeFlatpak->hideColumn(FlatCol::Duplicate);
    ui->treeFlatpak->hideColumn(FlatCol::FullName);
}

void MainWindow::hideLibs() const
{
    currentTree->setUpdatesEnabled(false);
    if (currentTree != ui->treeFlatpak && ui->checkHideLibs->isChecked()) {
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            if (isFilteredName((*it)->text(TreeCol::Name))) {
                (*it)->setHidden(true);
            }
        }
    }
    currentTree->setUpdatesEnabled(true);
}

// Process downloaded *Packages.gz files
bool MainWindow::readPackageList(bool force_download)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    pushCancel->setDisabled(true);

    // Early return if lists are already populated and not forced to download
    if (!force_download
        && ((currentTree == ui->treeEnabled && !enabled_list.isEmpty())
            || (currentTree == ui->treeMXtest && !mx_list.isEmpty())
            || (currentTree == ui->treeBackports && !backports_list.isEmpty()))) {
        return true;
    }

    // Determine the file name based on the current tree
    QString fileName;
    if (currentTree == ui->treeMXtest) {
        fileName = tmp_dir.path() + "/mxPackages";
    } else if (currentTree == ui->treeBackports) {
        fileName = tmp_dir.path() + "/allPackages";
    } else if (currentTree == ui->treeEnabled) {
        // treeEnabled is updated at downloadPackageList
        return true;
    }

    // Read the content of the file
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Could not open file: " << file.fileName();
        return false;
    }
    QString file_content = file.readAll();
    file.close();

    // Select the appropriate list to populate
    QMap<QString, PackageInfo> *packageMap = (currentTree == ui->treeMXtest) ? &mx_list : &backports_list;
    packageMap->clear();

    // Parse the file content and populate the map
    QStringList lines = file_content.split('\n');
    QString package, version, description;
    for (const QString &line : lines) {
        if (line.startsWith("Package: ")) {
            package = line.section(' ', 1);
        } else if (line.startsWith("Version: ")) {
            version = line.section(' ', 1);
        } else if (line.startsWith("Description: ")) {
            description = line.section(' ', 1);
            packageMap->insert(package, {version, description});
            package.clear();
            version.clear();
            description.clear();
        }
    }
    return true;
}

void MainWindow::cancelDownload()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
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
    ui->pushCancel->setEnabled(true);
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);

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
    ui->comboFilterBP->setCurrentIndex(savedComboIndex);
    ui->comboFilterMX->setCurrentIndex(savedComboIndex);
    ui->comboFilterEnabled->setCurrentIndex(savedComboIndex);
    ui->comboFilterFlatpak->setCurrentIndex(0);
    blockSignals(false);
}

void MainWindow::cleanup()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (cmd.state() != QProcess::NotRunning) {
        qDebug() << "Command" << cmd.program() << cmd.arguments() << "terminated" << cmd.terminateAndKill();
    }
    QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"};
    if (QFile::exists(temp_list)) {
        Cmd().run(elevate + " /usr/lib/mx-packageinstaller/mxpi-lib cleanup_temp", true);
        Cmd().runAsRoot("apt-get update&");
    }
    Cmd().run(elevate + " /usr/lib/mx-packageinstaller/mxpi-lib copy_log", true);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("FlatpakRemote", ui->comboRemote->currentText());
    settings.setValue("FlatpakUser", ui->comboUser->currentText());
}

QString MainWindow::getVersion(const QString &name) const
{
    return Cmd().getOut("dpkg-query -f '${Version}' -W " + name);
}

// Return true if all the packages listed are installed
bool MainWindow::checkInstalled(const QVariant &names) const
{
    QStringList name_list
        = names.canConvert<QString>() ? names.toString().split('\n', Qt::SkipEmptyParts) : names.toStringList();

    return !name_list.isEmpty() && std::all_of(name_list.cbegin(), name_list.cend(), [this](const QString &name) {
        return installed_packages.contains(name.trimmed());
    });
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

QMap<QString, PackageInfo> MainWindow::listInstalled() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    Cmd shell;
    QString list = shell.getOut("dpkg-query -W -f='${db:Status-Abbrev} ${Package} ${Version} ${binary:Synopsis}\n'");
    if (shell.exitStatus() != QProcess::NormalExit || shell.exitCode() != 0) {
        QMessageBox::critical(nullptr, tr("Error"),
                              tr("dpkg-query command returned an error. Please run 'dpkg-query -W' in terminal "
                                 "and check the output."));
        exit(EXIT_FAILURE);
    }

    QMap<QString, PackageInfo> installedPackages;
    const auto lines = list.split('\n', Qt::SkipEmptyParts);
    const QString statusPrefix = "ii ";

    for (const QString &line : lines) {
        if (line.startsWith(statusPrefix)) {
            QStringList parts = line.mid(statusPrefix.length()).split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString packageName = parts.takeFirst();
                QString version = parts.takeFirst();
                QString description = parts.join(' ');
                installedPackages.insert(packageName, {version, description});
            }
        }
    }

    return installedPackages;
}

QStringList MainWindow::listFlatpaks(const QString &remote, const QString &type) const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    static bool updated = false;

    QString arch_fp = getArchOption();
    if (arch_fp.isEmpty()) {
        return {};
    }

    Cmd shell;
    // Update appstream only once per session
    if (!updated) {
        if (!shell.run("flatpak update --appstream")) {
            qDebug() << "Failed to update flatpak appstream.";
        }
        updated = true;
    }

    // Construct the base command for listing flatpaks
    QString baseCommand = "flatpak remote-ls " + FPuser + remote + ' ' + arch_fp + "--columns=ver,ref,installed-size ";

    // Append the type to the base command if specified
    if (type == QLatin1String("--app")) {
        baseCommand.append("--app ");
    } else if (type == QLatin1String("--runtime")) {
        baseCommand.append("--runtime ");
    }
    baseCommand.append("2>/dev/null");

    QStringList list;
    // Execute the command and process the output
    if (shell.run(baseCommand)) {
        list = shell.readAllOutput().split('\n', Qt::SkipEmptyParts);
    }

    if (list.isEmpty()) {
        qDebug() << QString("Could not list packages from %1 remote, or remote doesn't contain packages").arg(remote);
    }

    return list;
}

// List installed flatpaks by type: apps, runtimes, or all (if no type is provided)
QStringList MainWindow::listInstalledFlatpaks(const QString &type)
{
    QStringList list {cmd.getOut("flatpak list " + FPuser + "2>/dev/null " + type + " --columns=ref")
                          .split('\n', Qt::SkipEmptyParts)};
    if (list == QStringList("")) {
        return {};
    }
    return list;
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
    const QList list({ui->treePopularApps, ui->treeEnabled, ui->treeMXtest, ui->treeBackports, ui->treeFlatpak});

    auto it = std::find_if(list.cbegin(), list.cend(), [](const auto &item) { return item->isVisible(); });
    if (it != list.end()) {
        currentTree = *it;
        updateInterface();
        return;
    }
}

void MainWindow::setDirty()
{
    dirtyBackports = dirtyEnabledRepos = dirtyTest = true;
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

    qicon_installed = force_backup_icon ? backup_icon_installed : theme_icon_installed;
    qicon_upgradable = force_backup_icon ? backup_icon_upgradable : theme_icon_upgradable;

    ui->iconUpgradable->setIcon(qicon_upgradable);
    ui->iconUpgradable_2->setIcon(ui->iconUpgradable->icon());
    ui->iconUpgradable_3->setIcon(ui->iconUpgradable->icon());
    ui->iconInstalledPackages->setIcon(qicon_installed);
    ui->iconInstalledPackages_2->setIcon(ui->iconInstalledPackages->icon());
    ui->iconInstalledPackages_3->setIcon(ui->iconInstalledPackages->icon());
    ui->iconInstalledPackages_4->setIcon(ui->iconInstalledPackages->icon());
    ui->iconInstalledPackages_5->setIcon(ui->iconInstalledPackages->icon());
}

QHash<QString, VersionNumber> MainWindow::listInstalledVersions()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QHash<QString, VersionNumber> result;
    Cmd shell;
    const QStringList &list = shell.getOut("dpkg -l | grep '^ii'", true).split('\n');
    if (shell.exitStatus() != QProcess::NormalExit || shell.exitCode() != 0) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("dpkg command returned an error, please run 'dpkg --list' in terminal and check the output."));
        return result;
    }
    for (const QString &line : list) {
        QStringList item = line.split(QRegularExpression("\\s{2,}"));
        if (item.size() < 3) {
            continue;
        }
        QString name = item.at(1);
        name.remove(":i386").remove(":amd64");
        QString ver_str = item.at(2);
        ver_str.remove(" amd64");
        result.insert(name, VersionNumber(ver_str));
    }
    return result;
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
    progress->hide();
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
    QString file_name = (tree == ui->treeMXtest) ? tmp_dir.path() + "/mxPackages" : tmp_dir.path() + "/allPackages";

    QFile file(file_name);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Could not open: " << file.fileName();
        return;
    }
    QString msg;
    QString item_name = item->text(TreeCol::Name);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line == "Package: " + item_name) {
            msg += line + '\n';
            line.clear();
            while (!in.atEnd()) {
                line = in.readLine();
                if (line.startsWith(QLatin1String("Package: "))) {
                    break;
                }
                msg += line + '\n';
            }
        }
    }
    auto msg_list = msg.split('\n');
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

        if (reply->error() != 0) {
            qDebug() << "Download of " << url.url() << " failed: " << qPrintable(reply->errorString());
        } else {
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

    ui->treePopularApps->setUpdatesEnabled(false);

    if (word.isEmpty()) {
        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            QTreeWidgetItem *item = *it;
            item->setExpanded(false);
            item->setHidden(false);
            if (!item->parent()) {
                item->setFirstColumnSpanned(true);
            }
        }
    } else {
        QSet<QTreeWidgetItem *> foundItems;
        for (int column : {PopCol::Name, PopCol::Icon, PopCol::Description}) {
            const auto items = ui->treePopularApps->findItems(word, Qt::MatchContains | Qt::MatchRecursive, column);
            for (QTreeWidgetItem *item : items) {
                foundItems.insert(item);
                QTreeWidgetItem *parent = item->parent();
                while (parent) {
                    foundItems.insert(parent);
                    parent = parent->parent();
                }
            }
        }

        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            QTreeWidgetItem *item = *it;
            const bool isFound = foundItems.contains(item);
            item->setHidden(!isFound);
            if (isFound && !item->parent()) {
                item->setExpanded(true);
                item->setFirstColumnSpanned(true);
            }
        }
    }

    for (int i = 1; i < ui->treePopularApps->columnCount(); ++i) {
        ui->treePopularApps->resizeColumnToContents(i);
    }

    ui->treePopularApps->setUpdatesEnabled(true);
}

void MainWindow::findPackage()
{
    // Retrieve the search word from the appropriate search box based on the current tree
    QString word = currentTree == ui->treeEnabled     ? ui->searchBoxEnabled->text()
                   : currentTree == ui->treeMXtest    ? ui->searchBoxMX->text()
                   : currentTree == ui->treeBackports ? ui->searchBoxBP->text()
                                                      : ui->searchBoxFlatpak->text(); // Default to flatpak

    // Return early if the search word is a single character
    if (word.length() == 1) {
        return;
    }
    currentTree->setUpdatesEnabled(false);

    // Use a set to track found items for faster lookup
    QSet<QTreeWidgetItem *> foundItems;
    QList<QTreeWidgetItem *> items;

    // Define the columns to search based on the current tree
    QVector<int> searchColumns = currentTree != ui->treeFlatpak ? QVector<int>({TreeCol::Name, TreeCol::Description})
                                                                : QVector<int>({FlatCol::LongName});

    // Find items in the specified columns
    for (int column : searchColumns) {
        items.append(currentTree->findItems(word, Qt::MatchContains | Qt::MatchRecursive, column));
    }

    // Insert found items and their ancestors into the set
    for (QTreeWidgetItem *item : qAsConst(items)) {
        while (item) {
            foundItems.insert(item);
            item = item->parent();
        }
    }

    // Iterate through all items and hide those not in the found set
    for (QTreeWidgetItemIterator it(currentTree); *it; ++it) {
        QTreeWidgetItem *item = *it;
        bool shouldHide = item->data(0, Qt::UserRole) == false;
        item->setHidden(!foundItems.contains(item) || shouldHide);
    }

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
        if (!confirmActions(change_list.join(' '), "install")) {
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
        if (cmd.run("socat SYSTEM:'flatpak install -y " + FPuser + ui->comboRemote->currentText() + ' '
                    + change_list.join(' ') + "',stderr STDIO")) {
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
            + tr("Package Installer for Debian")
            + R"(</h3></p><p align="center"><a href="http://debian.org">http://debian.org</a><br /></p><p align="center">)"
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
            }
        }
    } else if (currentTree == ui->treeFlatpak) {
        bool success = true;

        // Confirmation dialog
        if (!confirmActions(change_list.join(' '), "remove")) {
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
        for (const QString &app : qAsConst(change_list)) {
            enableOutput();
            if (!cmd.run("socat SYSTEM:'flatpak uninstall " + FPuser + "-y " + app
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
        names = change_list.join(' ');
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
    currentTree->blockSignals(true);

    resetCheckboxes();
    QString search_str;
    saveSearchText(search_str, savedComboIndex);

    auto setTabsEnabled = [this](bool enable) {
        for (auto tab : {Tab::Popular, Tab::EnabledRepos, Tab::Test, Tab::Backports, Tab::Flatpak}) {
            if (tab != ui->tabWidget->currentIndex()) {
                ui->tabWidget->setTabEnabled(tab, enable);
            }
        }
    };

    setTabsEnabled(false);
    switch (index) {
    case Tab::Popular: {
        bool tempFlag = false;
        handleTab(search_str, ui->searchPopular, "", tempFlag);
    } break;
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
    if (currentTree != ui->treePopularApps) {
        currentTree->clearSelection();
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            (*it)->setCheckState(0, Qt::Unchecked);
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
    // enableTabs(true);
    setCurrentTree();
    change_list.clear();
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
        findPackage();
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
    setCurrentTree();
    if (!warningMessage.isEmpty()) {
        displayWarning(warningMessage);
    }
    change_list.clear();
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
    if (!search_str.isEmpty()) {
        currentTree == ui->treePopularApps ? findPopular() : findPackage();
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
            findPackage();
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
            progress->show();
            if (!timer.isActive()) {
                timer.start(100ms);
                qApp->processEvents();
            }
        }
        ui->searchBoxBP->setText(search_str);
        if (!search_str.isEmpty()) {
            findPackage();
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
    installed_packages = listInstalled();
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
    Cmd().runAsRoot("flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo");
    Cmd().runAsRoot("flatpak remote-add --if-not-exists --subset=verified flathub-verified "
                    "https://flathub.org/repo/flathub.flatpakrepo");
    enableOutput();
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
    ui->searchPopular->clear();
    ui->searchBoxEnabled->clear();
    ui->searchBoxMX->clear();
    ui->searchBoxBP->clear();
    ui->pushInstall->setDisabled(true);
    ui->pushUninstall->setDisabled(true);
}

void MainWindow::filterChanged(const QString &arg1)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    currentTree->blockSignals(true);
    currentTree->setUpdatesEnabled(false);

    auto resetTree = [this]() {
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            (*it)->setData(0, Qt::UserRole, true);
            (*it)->setHidden(false);
            (*it)->setCheckState(TreeCol::Check, Qt::Unchecked);
        }
        findPackage();
        setSearchFocus();
        ui->pushInstall->setEnabled(false);
        ui->pushUninstall->setEnabled(false);
    };

    auto uncheckAllItems = [this]() {
        for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
            (*it)->setCheckState(TreeCol::Check, Qt::Unchecked);
        }
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
        change_list.clear();
    };

    auto blockSignalsForAll = [this](bool block) {
        ui->checkHideLibs->blockSignals(block);
        ui->checkHideLibsBP->blockSignals(block);
        ui->checkHideLibsMX->blockSignals(block);
    };

    bool isAutoremovable = (arg1 == tr("Autoremovable"));
    bool shouldHideLibs = !isAutoremovable && hideLibsChecked;
    if (currentTree == ui->treeFlatpak) {
        if (arg1 == tr("Installed runtimes")) {
            handleFlatpakFilter(installed_runtimes_fp, false);
            clearChangeListAndButtons();
        } else if (arg1 == tr("Installed apps")) {
            handleFlatpakFilter(installed_apps_fp, false);
            clearChangeListAndButtons();
        } else if (arg1 == tr("All apps")) {
            if (flatpaks_apps.isEmpty()) {
                flatpaks_apps = listFlatpaks(ui->comboRemote->currentText(), "--app");
            }
            handleFlatpakFilter(flatpaks_apps);
            clearChangeListAndButtons();
        } else if (arg1 == tr("All runtimes")) {
            if (flatpaks_runtimes.isEmpty()) {
                flatpaks_runtimes = listFlatpaks(ui->comboRemote->currentText(), "--runtime");
            }
            handleFlatpakFilter(flatpaks_runtimes);
            clearChangeListAndButtons();
        } else if (arg1 == tr("All available")) {
            resetTree();
            ui->labelNumAppFP->setText(QString::number(currentTree->topLevelItemCount()));
            clearChangeListAndButtons();
        } else if (arg1 == tr("All installed")) {
            displayFilteredFP(installed_apps_fp + installed_runtimes_fp);
        } else if (arg1 == tr("Not installed")) {
            for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
                bool isNotInstalled = (*it)->data(FlatCol::Status, Qt::UserRole) == Status::NotInstalled;
                if (!isNotInstalled) {
                    (*it)->setHidden(true);
                    (*it)->setCheckState(FlatCol::Check, Qt::Unchecked);
                    change_list.removeOne((*it)->data(FlatCol::FullName, Qt::UserRole).toString());
                }
                (*it)->setData(0, Qt::UserRole, isNotInstalled);
            }
            ui->pushUninstall->setEnabled(false);
        }
        findPackage();
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
            for (QTreeWidgetItemIterator it(currentTree); (*it) != nullptr; ++it) {
                int itemStatus = (*it)->data(TreeCol::Status, Qt::UserRole).toInt();
                bool shouldShow = (itStatus.value() == Status::Installed && itemStatus == Status::Upgradable)
                                  || (itemStatus == itStatus.value());
                (*it)->setHidden(!shouldShow);
                (*it)->setData(0, Qt::UserRole, shouldShow);
            }
        }
        uncheckAllItems();
        findPackage();
        setSearchFocus();
        clearChangeListAndButtons();
    }

    currentTree->setUpdatesEnabled(true);
    currentTree->blockSignals(false);
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

// Build the change_list when selecting on item in the tree
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
        if (change_list.isEmpty()
            && indexFilterFP.isEmpty()) { // remember the Flatpak combo location first time this is called
            indexFilterFP = ui->comboFilterFlatpak->currentText();
        }
        newapp = (item->data(FlatCol::FullName, Qt::UserRole).toString());
    } else {
        newapp = (item->text(TreeCol::Name));
    }

    if (item->checkState(0) == Qt::Checked) {
        ui->pushInstall->setEnabled(true);
        change_list.append(newapp);
    } else {
        change_list.removeOne(newapp);
    }

    if (currentTree != ui->treeFlatpak) {
        ui->pushUninstall->setEnabled(checkInstalled(change_list));
        ui->pushInstall->setText(checkUpgradable(change_list) ? tr("Upgrade") : tr("Install"));
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
        if (change_list.isEmpty()) { // Reset comboFilterFlatpak if nothing is selected
            ui->comboFilterFlatpak->setCurrentText(indexFilterFP);
            indexFilterFP.clear();
        }
        ui->treeFlatpak->setFocus();
    }

    if (change_list.isEmpty()) {
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
                                    "state.<p><b>Are you sure you want to exit Package Installer?</b>"),
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
    if (cmd.run("socat SYSTEM:'flatpak update " + FPuser + "',pty STDIO")) {
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
    auto *dialog = new ManageRemotes(this, FPuser);
    dialog->exec();
    if (dialog->isChanged()) {
        listFlatpakRemotes();
        displayFlatpaks(true);
    }
    if (!dialog->getInstallRef().isEmpty()) {
        showOutput();
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        if (cmd.run("socat SYSTEM:'flatpak install -y " + dialog->getUser() + "--from "
                    + dialog->getInstallRef().replace(':', "\\:") + "',stderr STDIO\"")) {
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
        FPuser = "--system ";
    } else {
        FPuser = "--user ";
        static bool updated = false;
        if (!updated) {
            setCursor(QCursor(Qt::BusyCursor));
            enableOutput();
            cmd.run("flatpak --user remote-add --if-not-exists flathub "
                    "https://flathub.org/repo/flathub.flatpakrepo");
            cmd.run("flatpak --user remote-add --if-not-exists --subset=verified flathub-verified "
                    "https://flathub.org/repo/flathub.flatpakrepo");
            cmd.run("flatpak update --appstream");
            setCursor(QCursor(Qt::ArrowCursor));
            updated = true;
        }
    }
    lastItemClicked = nullptr;
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
    }
}

void MainWindow::treePopularApps_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(PopCol::Check) == Qt::Checked) {
        ui->treePopularApps->setCurrentItem(item);
    }
    bool checked = false;
    bool installed = true;

    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
            checked = true;
            if ((*it)->icon(PopCol::Check).isNull()) {
                installed = false;
            }
        }
    }
    ui->pushInstall->setEnabled(checked);
    ui->pushUninstall->setEnabled(checked && installed);
    if (checked && installed) {
        ui->pushInstall->setText(tr("Reinstall"));
    } else {
        ui->pushInstall->setText(tr("Install"));
    }
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

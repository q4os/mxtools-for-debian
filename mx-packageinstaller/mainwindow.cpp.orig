/**********************************************************************
 *  MainWindow.cpp
 **********************************************************************
 * Copyright (C) 2017 MX Authors
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
      dictionary(QStringLiteral("/usr/share/mx-packageinstaller-pkglist/category.dict"), QSettings::IniFormat),
      args {arg_parser},
      reply(nullptr)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << QCoreApplication::applicationVersion();

    ui->setupUi(this);
    setProgressDialog();

    connect(&timer, &QTimer::timeout, this, &MainWindow::updateBar);
    connect(&cmd, &Cmd::started, this, &MainWindow::cmdStart);
    connect(&cmd, &Cmd::finished, this, &MainWindow::cmdDone);
    conn = connect(&cmd, &Cmd::outputAvailable, [](const QString &out) { qDebug() << out.trimmed(); });
    connect(&cmd, &Cmd::errorAvailable, [](const QString &out) { qWarning() << out.trimmed(); });
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->blockSignals(true);
    ui->pushRemoveOrphan->setHidden(true);

    QFont font(QStringLiteral("monospace"));
    font.setStyleHint(QFont::Monospace);
    ui->outputBox->setFont(font);

    fp_ver = getVersion(QStringLiteral("flatpak"));
    user = QStringLiteral("--system ");

    arch = AptCache::getArch();
    ver_name = getDebianVerName();

    if (arch == "i386") {
        ui->tabWidget->setTabEnabled(Tab::Flatpak, false); // setTabVisible is available only on Qt >= 5.15
        ui->tabWidget->setTabToolTip(Tab::Flatpak, tr("Flatpak tab is disabled on 32-bit."));
    }

    if (!QFile::exists("/etc/apt/sources.list.d/mx.list")) {
        ui->tabWidget->setTabEnabled(Tab::Test, false); // setTabVisible is available only on Qt >= 5.15
        ui->tabWidget->setTabToolTip(Tab::Test, tr("Could not find MX sources."));
    }

    lock_file = new LockFile(QStringLiteral("/var/lib/dpkg/lock"));
    connect(QApplication::instance(), &QApplication::aboutToQuit, this, &MainWindow::cleanup, Qt::QueuedConnection);

    test_initially_enabled
        = cmd.run("apt-get update --print-uris | grep -m1 -qE '/mx/testrepo/dists/" + ver_name + "/test/'");

    this->setWindowTitle(tr("MX Package Installer"));
    ui->tabWidget->setCurrentIndex(Tab::Popular);
    ui->treeEnabled->hideColumn(TreeCol::Status); // Status of the package: installed, upgradable, etc
    ui->treeMXtest->hideColumn(TreeCol::Status);
    ui->treeBackports->hideColumn(TreeCol::Status);
    ui->treeFlatpak->hideColumn(FlatCol::Status);
    ui->treeFlatpak->hideColumn(FlatCol::Duplicate);
    ui->treeFlatpak->hideColumn(FlatCol::FullName);

    const QString icon_upgradable = QStringLiteral("package-installed-outdated");
    const QIcon backup_icon_upgradable = QIcon(":/icons/package-installed-outdated.png");
    ui->iconUpgradable->setIcon(QIcon::fromTheme(icon_upgradable, backup_icon_upgradable));
    ui->iconUpgradable_2->setIcon(QIcon::fromTheme(icon_upgradable, backup_icon_upgradable));
    ui->iconUpgradable_3->setIcon(QIcon::fromTheme(icon_upgradable, backup_icon_upgradable));
    const QString icon_installed = QStringLiteral("package-installed-updated");
    const QIcon backup_icon_installed = QIcon(":/icons/package-installed-updated.png");
    ui->iconInstalledPackages->setIcon(QIcon::fromTheme(icon_installed, backup_icon_installed));
    ui->iconInstalledPackages_2->setIcon(QIcon::fromTheme(icon_installed, backup_icon_installed));
    ui->iconInstalledPackages_3->setIcon(QIcon::fromTheme(icon_installed, backup_icon_installed));
    ui->iconInstalledPackages_4->setIcon(QIcon::fromTheme(icon_installed, backup_icon_installed));
    ui->iconInstalledPackages_5->setIcon(QIcon::fromTheme(icon_installed, backup_icon_installed));

    loadPmFiles();
    refreshPopularApps();

    // connect search boxes
    connect(ui->searchPopular, &QLineEdit::textChanged, this, &MainWindow::findPopular);
    connect(ui->searchBoxEnabled, &QLineEdit::textChanged, this, &MainWindow::findPackageOther);
    connect(ui->searchBoxMX, &QLineEdit::textChanged, this, &MainWindow::findPackageOther);
    connect(ui->searchBoxBP, &QLineEdit::textChanged, this, &MainWindow::findPackageOther);
    connect(ui->searchBoxFlatpak, &QLineEdit::textChanged, this, &MainWindow::findPackageOther);

    // connect combo filters
    connect(ui->comboFilterEnabled, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);
    connect(ui->comboFilterMX, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);
    connect(ui->comboFilterBP, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);
    connect(ui->comboFilterFlatpak, &QComboBox::currentTextChanged, this, &MainWindow::filterChanged);

    ui->searchPopular->setFocus();
    updated_once = false;
    warning_backports = false;
    warning_flatpaks = false;
    warning_test = false;
    tree = ui->treePopularApps;

    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tabOutput), false);
    ui->tabWidget->blockSignals(false);
    ui->pushUpgradeAll->setVisible(false);

    const QSize size = this->size();
    if (settings.contains(QStringLiteral("geometry"))) {
        restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
        if (this->isMaximized()) { // add option to resize if maximized
            this->resize(size);
            centerWindow();
        }
    }

    // check/uncheck tree items spacebar press or double-click
    auto *shortcutToggle = new QShortcut(Qt::Key_Space, this);
    connect(shortcutToggle, &QShortcut::activated, this, &MainWindow::checkUnckeckItem);

    QList list_tree {ui->treePopularApps, ui->treeEnabled, ui->treeMXtest, ui->treeBackports, ui->treeFlatpak};
    for (const auto &tree : list_tree) {
        if (tree != ui->treeFlatpak) {
            tree->setContextMenuPolicy(Qt::CustomContextMenu);
        }
        connect(tree, &QTreeWidget::itemDoubleClicked, [tree](QTreeWidgetItem *item) { tree->setCurrentItem(item); });
        connect(tree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::checkUnckeckItem);
        connect(tree, &QTreeWidget::customContextMenuRequested, this,
                [this, tree](QPoint pos) { displayPackageInfo(tree, pos); });
    }
}

bool MainWindow::uninstall(const QString &names, const QString &preuninstall, const QString &postuninstall)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setCurrentWidget(ui->tabOutput);

    bool success = true;
    // simulate install of selections and present for confirmation
    // if user selects cancel, break routine but return success to avoid error message
    if (!confirmActions(names, QStringLiteral("remove"))) {
        return true;
    }

    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Uninstalling packages..."));
    enableOutput();

    if (!preuninstall.isEmpty()) {
        qDebug() << "Pre-uninstall";
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Running pre-uninstall operations..."));
        enableOutput();
        lock_file->unlock();
        success = cmd.run(preuninstall);
        lock_file->lock();
    }

    if (success) {
        enableOutput();
        lock_file->unlock();
        success = cmd.run("DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null | grep -sq ^i "
                          "&& echo kde || echo gnome) "
                          "apt-get -o=Dpkg::Use-Pty=0 remove -y "
                          + names); // use -y since there is a confirm dialog already
        lock_file->lock();
    }

    if (success && !postuninstall.isEmpty()) {
        qDebug() << "Post-uninstall";
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Running post-uninstall operations..."));
        enableOutput();
        lock_file->unlock();
        success = cmd.run(postuninstall);
        lock_file->lock();
    }

    return success;
}

bool MainWindow::updateApt()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    lock_file->unlock();
    ui->tabOutput->isVisible() // don't display in output if calling to refresh from tabs
        ? ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Refreshing sources..."))
        : progress->show();

    enableOutput();
    if (cmd.run(QStringLiteral("apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o "
                               "Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10"))) {
        lock_file->lock();
        qDebug() << "sources updated OK";
        updated_once = true;
        return true;
    }
    lock_file->lock();
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
    QString unit = size.section(QChar(160), 1);
    double value = number.toDouble();
    if (unit == QLatin1String("KB")) {
        return value * KiB;
    } else if (unit == QLatin1String("MB")) {
        return value * MiB;
    } else if (unit == QLatin1String("GB")) {
        return value * GiB;
    } else { // for "bytes"
        return value;
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

    QString name;
    QString size;
    QStringList list;
    QStringList runtimes;
    quint64 total {0};
    if (fp_ver < VersionNumber(QStringLiteral("1.0.1"))) { // older version doesn't display all apps
                                                           // and runtimes without specifying them
        list = cmd.getCmdOut(
                      "runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak -d list --app "
                      + user + "|tr -s ' ' |cut -f1,5,6 -d' '\"")
                   .split(QStringLiteral("\n"));
        runtimes
            = cmd.getCmdOut(
                     "runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak -d list --runtime "
                     + user + "|tr -s ' '|cut -f1,5,6 -d' '\"")
                  .split(QStringLiteral("\n"));
        if (!runtimes.isEmpty()) {
            list << runtimes;
        }
        for (QTreeWidgetItemIterator it(ui->treeFlatpak); (*it) != nullptr; ++it) {
            for (const QString &item : qAsConst(list)) {
                name = item.section(QStringLiteral(" "), 0, 0);
                size = item.section(QStringLiteral(" "), 1);
                if (name == (*it)->text(FlatCol::FullName)) {
                    (*it)->setText(FlatCol::Size, size);
                }
            }
        }
    } else if (fp_ver < VersionNumber(QStringLiteral("1.2.4"))) {
        list = cmd.getCmdOut("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak -d list "
                             + user + "|tr -s ' '|cut -f1,5\"")
                   .split(QStringLiteral("\n"));
    } else {
        list = cmd.getCmdOut("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak list " + user
                             + "--columns app,size\"")
                   .split(QStringLiteral("\n"));
    }

    total = std::accumulate(list.cbegin(), list.cend(), 0, [](quint64 acc, const QString &item) {
        return acc + convert(item.section(QStringLiteral("\t"), 1));
    });
    ui->labelNumSize->setText(convert(total));
}

// Block interface while updating Flatpak list
void MainWindow::blockInterfaceFP(bool block)
{
    for (int tab = 0; tab < 4; ++tab) {
        ui->tabWidget->setTabEnabled(tab, !block);
    }

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

void MainWindow::updateInterface()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    auto upgr_list = tree->findItems(QStringLiteral("upgradable"), Qt::MatchExactly, TreeCol::Status);
    auto inst_list = tree->findItems(QStringLiteral("installed"), Qt::MatchExactly, TreeCol::Status);
    for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
        (*it)->setHidden(false);
    }

    if (tree == ui->treeEnabled) {
        ui->labelNumApps->setText(QString::number(tree->topLevelItemCount()));
        ui->labelNumUpgr->setText(QString::number(upgr_list.count()));
        ui->labelNumInst->setText(QString::number(inst_list.count() + upgr_list.count()));
        ui->pushUpgradeAll->setVisible(!upgr_list.isEmpty());
        ui->pushForceUpdateEnabled->setEnabled(true);
        ui->searchBoxEnabled->setFocus();
    } else if (tree == ui->treeMXtest) {
        ui->labelNumApps_2->setText(QString::number(tree->topLevelItemCount()));
        ui->labelNumUpgrMX->setText(QString::number(upgr_list.count()));
        ui->labelNumInstMX->setText(QString::number(inst_list.count() + upgr_list.count()));
        ui->pushForceUpdateMX->setEnabled(true);
        ui->searchBoxMX->setFocus();
    } else if (tree == ui->treeBackports) {
        ui->labelNumApps_3->setText(QString::number(tree->topLevelItemCount()));
        ui->labelNumUpgrBP->setText(QString::number(upgr_list.count()));
        ui->labelNumInstBP->setText(QString::number(inst_list.count() + upgr_list.count()));
        ui->pushForceUpdateBP->setEnabled(true);
        ui->searchBoxBP->setFocus();
    }

    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    progress->hide();
}

int MainWindow::getDebianVerNum()
{
    QFile file("/etc/debian_version");
    QStringList list;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        list = line.split(".");
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
        QString verName = list.at(0).split(QStringLiteral("/")).at(0);
        if (verName == QLatin1String("bullseye")) {
            return Release::Bullseye;
        } else if (verName == QLatin1String("bookworm")) {
            return Release::Bookworm;
        } else {
            qCritical() << "Unknown Debian version:" << ver << "Assumes Bullseye";
            return Release::Bullseye;
        }
    }
}

QString MainWindow::getDebianVerName()
{
    int ver = getDebianVerNum();
    QHash<int, QString> releaseNames {{Release::Jessie, QStringLiteral("jessie")},
                                      {Release::Stretch, QStringLiteral("stretch")},
                                      {Release::Buster, QStringLiteral("buster")},
                                      {Release::Bullseye, QStringLiteral("bullseye")},
                                      {Release::Bookworm, QStringLiteral("bookworm")}};
    if (!releaseNames.contains(ver)) {
        qWarning() << "Error: Invalid Debian version, assumes Bullseye";
        return "bullseye";
    }
    return releaseNames.value(ver);
}

QString MainWindow::getLocalizedName(const QDomElement &element) const
{
    // pass one, find fully localized string, e.g. "pt_BR"
    for (auto child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        if (child.tagName() == locale.name() && !child.text().trimmed().isEmpty()) {
            return child.text().trimmed();
        }
    }

    // pass two, find language, e.g. "pt"
    for (auto child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        if (child.tagName() == locale.name().section(QStringLiteral("_"), 0, 0) && !child.text().trimmed().isEmpty()) {
            return child.text().trimmed();
        }
    }

    // pass three, return "en" or "en_US"
    for (auto child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        if ((child.tagName() == QLatin1String("en") || child.tagName() == QLatin1String("en_US"))
            && !child.text().trimmed().isEmpty()) {
            return child.text().trimmed();
        }
    }

    auto child = element.firstChildElement();
    if (child.isNull()) {
        return element.text().trimmed(); // if no language tags are present
    } else {
        return child.text().trimmed(); // return first language tag if neither the specified locale
                                       // nor "en" is found.
    }
}

QString MainWindow::categoryTranslation(const QString &item)
{
    if (locale.name() == QLatin1String("en_US")) { // no need for translation
        return item;
    }

    dictionary.beginGroup(item);

    QString trans = dictionary.value(locale.name()).toString().toLatin1(); // try pt_BR format
    if (trans.isEmpty()) {
        trans
            = dictionary.value(locale.name().section(QStringLiteral("_"), 0, 0)).toString().toLatin1(); // try pt format
        if (trans.isEmpty()) {
            dictionary.endGroup();
            return item; // return original item if no translation found
        }
    }
    dictionary.endGroup();
    return trans;
}

void MainWindow::updateBar()
{
    QApplication::processEvents();
    bar->setValue((bar->value() + 1) % bar->maximum());
}

void MainWindow::checkUnckeckItem()
{
    if (const auto &t_widget = qobject_cast<QTreeWidget *>(focusWidget())) {
        if (t_widget->currentItem() == nullptr || t_widget->currentItem()->childCount() > 0) {
            return;
        }
        int col
            = (t_widget == ui->treePopularApps) ? static_cast<int>(PopCol::Check) : static_cast<int>(TreeCol::Check);
        auto new_state = (t_widget->currentItem()->checkState(col) == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
        t_widget->currentItem()->setCheckState(col, new_state);
    }
}

void MainWindow::outputAvailable(const QString &output)
{
    ui->outputBox->moveCursor(QTextCursor::End);
    if (output.contains(QLatin1String("\r"))) {
        ui->outputBox->moveCursor(QTextCursor::Up, QTextCursor::KeepAnchor);
        ui->outputBox->moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    ui->outputBox->insertPlainText(output);
    ui->outputBox->verticalScrollBar()->setValue(ui->outputBox->verticalScrollBar()->maximum());
}

void MainWindow::loadPmFiles()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QDomDocument doc;

    const QStringList filter(QStringLiteral("*.pm"));
    const QDir dir(QStringLiteral("/usr/share/mx-packageinstaller-pkglist"));
    const QStringList pmfilelist = dir.entryList(filter);

    for (const QString &file_name : pmfilelist) {
        QFile file(dir.absolutePath() + "/" + file_name);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qDebug() << "Could not open: " << file.fileName();
        } else if (!doc.setContent(&file)) {
            qDebug() << "Could not load document: " << file_name << "-- not valid XML?";
        } else {
            processDoc(doc);
        }
        file.close();
    }
}

// Process dom documents (from .pm files)
void MainWindow::processDoc(const QDomDocument &doc)
{
    /*  Items order in list:
            0 "category"
            1 "name"
            2 "description"
            3 "installable"
            4 "screenshot"
            5 "preinstall"
            6 "install_package_names"
            7 "postinstall"
            8 "uninstall_package_names"
            9 "postuninstall"
           10 "preuninstall"
    */

    QString category;
    QString name;
    QString description;
    QString installable;
    QString screenshot;
    QString preinstall;
    QString postinstall;
    QString preuninstall;
    QString postuninstall;
    QString install_names;
    QString uninstall_names;
    QStringList list;

    QDomElement root = doc.firstChildElement(QStringLiteral("app"));
    QDomElement element = root.firstChildElement();

    while (!element.isNull()) {
        if (element.tagName() == QLatin1String("category")) {
            category = categoryTranslation(element.text().trimmed());
        } else if (element.tagName() == QLatin1String("name")) {
            name = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("description")) {
            description = getLocalizedName(element);
        } else if (element.tagName() == QLatin1String("installable")) {
            installable = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("screenshot")) {
            screenshot = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("preinstall")) {
            preinstall = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("install_package_names")) {
            install_names = element.text().trimmed();
            install_names.replace(QLatin1String("\n"), QLatin1String(" "));
        } else if (element.tagName() == QLatin1String("postinstall")) {
            postinstall = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("uninstall_package_names")) {
            uninstall_names = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("postuninstall")) {
            postuninstall = element.text().trimmed();
        } else if (element.tagName() == QLatin1String("preuninstall")) {
            preuninstall = element.text().trimmed();
        }
        element = element.nextSiblingElement();
    }

    QString mod_arch; // modified arch to match .pm files format
    if (arch == QLatin1String("amd64")) {
        mod_arch = QStringLiteral("64");
    } else if (arch == QLatin1String("i386")) {
        mod_arch = QStringLiteral("32");
    } else if (arch == QLatin1String("armhf")) {
        mod_arch = QStringLiteral("armhf");
    } else {
        return;
    }

    // skip non-installable packages
    if (!installable.contains(mod_arch) && installable != QLatin1String("all")) {
        return;
    }

    list << category << name << description << installable << screenshot << preinstall << postinstall << install_names
         << uninstall_names << postuninstall << preuninstall;
    popular_apps << list;
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
void MainWindow::removeDuplicatesFP()
{
    // find and mark duplicates
    QTreeWidgetItemIterator it(ui->treeFlatpak);
    QString current;
    QString next;
    while ((*it) != nullptr) {
        current = ((*it))->text(FlatCol::Name);
        if ((*(++it)) != nullptr) {
            next = ((*it))->text(FlatCol::Name);
            if (next == current) {
                --it;
                (*(it))->setText(FlatCol::Duplicate, QStringLiteral("true"));
                ++it;
                (*it)->setText(FlatCol::Duplicate, QStringLiteral("true"));
            }
        }
    }
    // rename duplicate to use more context
    for (QTreeWidgetItemIterator it(ui->treeFlatpak); (*it) != nullptr; ++it) {
        if ((*(it))->text(FlatCol::Duplicate) == QLatin1String("true")) {
            (*it)->setText(FlatCol::Name, (*it)->text(FlatCol::LongName).section(QStringLiteral("."), -2));
        }
    }
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

void MainWindow::setSearchFocus()
{
    if (ui->tabEnabled->isVisible()) {
        ui->searchBoxEnabled->setFocus();
    } else if (ui->tabMXtest->isVisible()) {
        ui->searchBoxMX->setFocus();
    } else if (ui->tabBackports->isVisible()) {
        ui->searchBoxBP->setFocus();
    } else if (ui->tabFlatpak->isVisible()) {
        ui->searchBoxFlatpak->setFocus();
    }
}

void MainWindow::displayPopularApps() const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QTreeWidgetItem *topLevelItem {nullptr};

    for (const QStringList &list : popular_apps) {
        const QString &category = list.at(Popular::Category);
        const QString &name = list.at(Popular::Name);
        const QString &description = list.at(Popular::Description);
        const QString &screenshot = list.at(Popular::Screenshot);
        const QString &install_names = list.at(Popular::InstallNames);
        const QString &uninstall_names = list.at(Popular::UninstallNames);
        const QString &postuninstall = list.at(Popular::PostUninstall);
        const QString &preuninstall = list.at(Popular::PreUninstall);

        // add package category if treePopularApps doesn't already have it
        if (ui->treePopularApps->findItems(category, Qt::MatchFixedString, PopCol::Icon).isEmpty()) {
            topLevelItem = new QTreeWidgetItem();
            topLevelItem->setText(PopCol::Icon, category);
            ui->treePopularApps->addTopLevelItem(topLevelItem);
            // topLevelItem look
            QFont font;
            font.setBold(true);
            topLevelItem->setFont(PopCol::Icon, font);
            topLevelItem->setIcon(PopCol::Icon, QIcon::fromTheme(QStringLiteral("folder")));
            topLevelItem->setFirstColumnSpanned(true);
        } else {
            topLevelItem = ui->treePopularApps->findItems(category, Qt::MatchFixedString, PopCol::Icon)
                               .at(0); // find first match; add the child there
        }
        // add package name as childItem to treePopularApps
        QTreeWidgetItem *childItem {nullptr};
        childItem = new QTreeWidgetItem(topLevelItem);
        childItem->setText(PopCol::Name, name);
        childItem->setIcon(PopCol::Info, QIcon::fromTheme(QStringLiteral("dialog-information")));
        childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
        childItem->setCheckState(PopCol::Check, Qt::Unchecked);
        childItem->setText(PopCol::Description, description);
        childItem->setText(PopCol::InstallNames, install_names);
        childItem->setText(PopCol::UninstallNames, uninstall_names); // not displayed
        childItem->setText(PopCol::Screenshot, screenshot);          // not displayed
        childItem->setText(PopCol::PostUninstall, postuninstall);    // not displayed
        childItem->setText(PopCol::PreUninstall, preuninstall);      // not displayed

        if (checkInstalled(uninstall_names)) {
            childItem->setIcon(PopCol::Check, QIcon::fromTheme(QStringLiteral("package-installed-updated"),
                                                               QIcon(":/icons/package-installed-updated.png")));
        }
    }
    for (int i = 0; i < ui->treePopularApps->columnCount(); ++i) {
        ui->treePopularApps->resizeColumnToContents(i);
    }

    ui->treePopularApps->sortItems(PopCol::Icon, Qt::AscendingOrder);
    connect(ui->treePopularApps, &QTreeWidget::itemClicked, this, &MainWindow::displayPopularInfo,
            Qt::UniqueConnection);
}

void MainWindow::displayFilteredFP(QStringList list, bool raw)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->treeFlatpak->blockSignals(true);

    QMutableStringListIterator i(list);
    if (raw) { // raw format that needs to be edited
        if (fp_ver < VersionNumber(QStringLiteral("1.2.4"))) {
            while (i.hasNext()) {
                i.setValue(i.next().section(QStringLiteral("\t"), 0, 0)); // remove size
            }
        } else {
            while (i.hasNext()) {
                i.setValue(i.next()
                               .section(QStringLiteral("\t"), 1, 1)
                               .section(QStringLiteral("/"), 1)); // remove version and size
            }
        }
    }

    int total = 0;
    for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
        if (list.contains((*it)->text(FlatCol::FullName))) {
            ++total;
            (*it)->setHidden(false);
            (*it)->setData(0, Qt::UserRole, true); // Displayed flag
            if ((*it)->checkState(FlatCol::Check) == Qt::Checked
                && (*it)->text(FlatCol::Status) == QLatin1String("installed")) {
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
                (*it)->setCheckState(FlatCol::Check, Qt::Unchecked); // uncheck hidden item
                change_list.removeOne((*it)->text(FlatCol::FullName));
            }
        }
        if (change_list.isEmpty()) { // reset comboFilterFlatpak if nothing is selected
            ui->pushUninstall->setEnabled(false);
            ui->pushInstall->setEnabled(false);
        }
    }
    ui->labelNumAppFP->setText(QString::number(total));
    ui->treeFlatpak->blockSignals(false);
}

void MainWindow::displayPackages()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    QTreeWidget *newtree {nullptr}; // new pointer to avoid overwriting current "tree"

    QMap<QString, QStringList> list;
    if (tree == ui->treeMXtest) {
        list = mx_list;
        newtree = ui->treeMXtest;
        dirtyTest = false;
    } else if (tree == ui->treeBackports) {
        list = backports_list;
        newtree = ui->treeBackports;
        dirtyBackports = false;
    } else { // for ui-treeEnabled, ui->treePopularApps, ui->treeFlatpak
        list = enabled_list;
        newtree = ui->treeEnabled;
        dirtyEnabledRepos = false;
        if (tree == ui->treeEnabled) {
            newtree->setColumnHidden(TreeCol::Displayed, true);
        }
    }

    newtree->blockSignals(true);

    // create a list of apps, create a hash with app_name, app_info
    auto hashInstalled = listInstalledVersions();
    for (auto i = list.constBegin(); i != list.constEnd(); ++i) {
        auto *widget_item = new QTreeWidgetItem(newtree);
        widget_item->setCheckState(TreeCol::Check, Qt::Unchecked);
        widget_item->setText(TreeCol::Name, i.key());
        widget_item->setText(TreeCol::Version, i.value().at(0));
        widget_item->setText(TreeCol::Description, i.value().at(1));
        widget_item->setData(0, Qt::UserRole, true); // all items are displayed till filtered
    }

    // process the entire list of apps and count upgradable and installable
    int upgr_count = 0;
    int inst_count = 0;

    QString app_name;
    QString app_ver;
    VersionNumber installed;
    VersionNumber candidate;
    // update tree
    for (QTreeWidgetItemIterator it(newtree); (*it) != nullptr; ++it) {
        app_name = (*it)->text(TreeCol::Name);
        if (ui->checkHideLibs->isChecked() && isFilteredName(app_name)) {
            (*it)->setHidden(true);
        }
        app_ver = (*it)->text(TreeCol::Version);
        installed = hashInstalled.value(app_name);
        VersionNumber repo_candidate(app_ver); // candidate from the selected repo, might be
                                               // different than the one from Enabled

        (*it)->setIcon(TreeCol::Check, QIcon()); // reset update icon
        if (installed.toString().isEmpty()) {
            for (int i = 0; i < newtree->columnCount(); ++i) {
                if (enabled_list.contains(app_name)) {
                    (*it)->setToolTip(i, tr("Version ") + enabled_list.value(app_name).at(0)
                                             + tr(" in the enabled repos"));
                } else {
                    (*it)->setToolTip(i, tr("Not available in the enabled repos"));
                }
            }
            (*it)->setText(TreeCol::Status, QStringLiteral("not installed"));
        } else {
            ++inst_count;
            if (installed >= repo_candidate) {
                (*it)->setIcon(TreeCol::Check, QIcon::fromTheme(QStringLiteral("package-installed-updated"),
                                                                QIcon(":/icons/package-installed-updated.png")));
                for (int i = 0; i < newtree->columnCount(); ++i) {
                    (*it)->setToolTip(i, tr("Latest version ") + installed.toString() + tr(" already installed"));
                }
                (*it)->setText(TreeCol::Status, QStringLiteral("installed"));
            } else {
                (*it)->setIcon(TreeCol::Check, QIcon::fromTheme(QStringLiteral("package-installed-outdated"),
                                                                QIcon(":/icons/package-installed-outdated.png")));
                for (int i = 0; i < newtree->columnCount(); ++i) {
                    (*it)->setToolTip(i, tr("Version ") + installed.toString() + tr(" installed"));
                }
                ++upgr_count;
                (*it)->setText(TreeCol::Status, QStringLiteral("upgradable"));
            }
        }
    }
    for (int i = 0; i < newtree->columnCount(); ++i) {
        newtree->resizeColumnToContents(i);
    }
    updateInterface();
    newtree->blockSignals(false);
}

void MainWindow::displayFlatpaks(bool force_update)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    setCursor(QCursor(Qt::BusyCursor));
    ui->treeFlatpak->clear();
    ui->treeFlatpak->blockSignals(true);
    change_list.clear();

    if (flatpaks.isEmpty() || force_update) {
        progress->show();
        blockInterfaceFP(true);
        flatpaks = listFlatpaks(ui->comboRemote->currentText());
        flatpaks_apps.clear();
        flatpaks_runtimes.clear();

        // list installed packages
        installed_apps_fp = listInstalledFlatpaks(QStringLiteral("--app"));

        // add runtimes (needed for older flatpak versions)
        installed_runtimes_fp = listInstalledFlatpaks(QStringLiteral("--runtime"));
    }
    int total_count = 0;
    QTreeWidgetItem *widget_item {nullptr};

    QString short_name;
    QString long_name;
    QString version;
    QString size;
    for (QString item : qAsConst(flatpaks)) {
        if (fp_ver < VersionNumber(QStringLiteral("1.2.4"))) {
            size = item.section(QStringLiteral("\t"), 1, 1);
            item = item.section(QStringLiteral("\t"), 0, 0); // strip size
            version = item.section(QStringLiteral("/"), -1);
        } else { // Buster and higher versions
            size = item.section(QStringLiteral("\t"), -1);
            version = item.section(QStringLiteral("\t"), 0, 0);
            item = item.section(QStringLiteral("\t"), 1, 1).section(QStringLiteral("/"), 1);
        }
        long_name = item.section(QStringLiteral("/"), 0, 0);
        short_name = long_name.section(QStringLiteral("."), -1);
        if (short_name == QLatin1String("Locale") || short_name == QLatin1String("Sources")
            || short_name == QLatin1String("Debug")) { // skip Locale, Sources, Debug
            continue;
        }
        ++total_count;
        widget_item = new QTreeWidgetItem(ui->treeFlatpak);
        widget_item->setCheckState(FlatCol::Check, Qt::Unchecked);
        widget_item->setText(FlatCol::Name, short_name);
        widget_item->setText(FlatCol::LongName, long_name);
        widget_item->setText(FlatCol::Version, version);
        widget_item->setText(FlatCol::Size, size);
        widget_item->setText(FlatCol::FullName, item); // Full string
        QStringList installed_all {installed_apps_fp + installed_runtimes_fp};
        if (installed_all.contains(item)) {
            widget_item->setIcon(FlatCol::Check, QIcon::fromTheme(QStringLiteral("package-installed-updated"),
                                                                  QIcon(":/icons/package-installed-updated.png")));
            widget_item->setText(FlatCol::Status, QStringLiteral("installed"));
        } else {
            widget_item->setText(FlatCol::Status, QStringLiteral("not installed"));
        }
        widget_item->setData(0, Qt::UserRole, true); // all items are displayed till filtered
    }

    // add sizes for the installed packages for older flatpak that doesn't list size for all the
    // packages
    listSizeInstalledFP();

    ui->labelNumAppFP->setText(QString::number(total_count));

    int total = 0;
    if (installed_apps_fp != QStringList(QLatin1String(""))) {
        total = installed_apps_fp.count();
    }

    ui->labelNumInstFP->setText(QString::number(total));

    ui->treeFlatpak->sortByColumn(FlatCol::Name, Qt::AscendingOrder);

    removeDuplicatesFP();

    for (int i = 0; i < ui->treeFlatpak->columnCount(); ++i) {
        ui->treeFlatpak->resizeColumnToContents(i);
    }

    ui->treeFlatpak->blockSignals(false);
    filterChanged(ui->comboFilterFlatpak->currentText());
    blockInterfaceFP(false);
    ui->searchBoxFlatpak->setFocus();
    progress->hide();
}

void MainWindow::displayWarning(const QString &repo)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    bool *displayed = nullptr;
    QString msg;
    QString key;

    if (repo == QLatin1String("test")) {
        displayed = &warning_test;
        key = QStringLiteral("NoWarningTest");
        msg = tr("You are about to use the MX Test repository, whose packages are provided for "
                 "testing purposes only. It is possible that they might break your system, so it "
                 "is suggested that you back up your system and install or update only one package "
                 "at a time. Please provide feedback in the Forum so the package can be evaluated "
                 "before moving up to Main.");

    } else if (repo == QLatin1String("backports")) {
        displayed = &warning_backports;
        key = QStringLiteral("NoWarningBackports");
        msg = tr("You are about to use Debian Backports, which contains packages taken from the next "
                 "Debian release (called 'testing'), adjusted and recompiled for usage on Debian stable. "
                 "They cannot be tested as extensively as in the stable releases of Debian and MX Linux, "
                 "and are provided on an as-is basis, with risk of incompatibilities with other components "
                 "in Debian stable. Use with care!");
    } else if (repo == QLatin1String("flatpaks")) {
        displayed = &warning_flatpaks;
        key = QStringLiteral("NoWarningFlatpaks");
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

void MainWindow::ifDownloadFailed()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    progress->hide();
    ui->tabWidget->setCurrentWidget(ui->tabPopular);
}

void MainWindow::listFlatpakRemotes()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->comboRemote->blockSignals(true);
    ui->comboRemote->clear();
    QStringList list
        = cmd.getCmdOut("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak remote-list "
                        + user + "| cut -f1\"")
              .remove(QStringLiteral(" "))
              .split(QStringLiteral("\n"));
    ui->comboRemote->addItems(list);
    // set flathub default
    ui->comboRemote->setCurrentIndex(ui->comboRemote->findText(QStringLiteral("flathub")));
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

    const QString frontend = QStringLiteral("DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null | grep -sq ^i "
                                            "&& echo kde || echo gnome) LANG=C ");
    const QString aptget = QStringLiteral("apt-get -s -V -o=Dpkg::Use-Pty=0 ");
    const QString aptitude = QStringLiteral("aptitude -sy -V -o=Dpkg::Use-Pty=0 ");
    lock_file->unlock();
    if (tree == ui->treeFlatpak) {
        detailed_installed_names = change_list;
    } else if (tree == ui->treeBackports) {
        recommends = (ui->checkBoxInstallRecommendsMXBP->isChecked()) ? QStringLiteral("--install-recommends ")
                                                                      : QLatin1String("");
        recommends_aptitude = (ui->checkBoxInstallRecommendsMXBP->isChecked())
                                  ? QStringLiteral("--with-recommends ")
                                  : QStringLiteral("--without-recommends ");
        detailed_names = cmd.getCmdOut(
            frontend + aptget + action + " " + recommends + "-t " + ver_name + "-backports --reinstall " + names
            + R"lit(|grep 'Inst\|Remv' | awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info = cmd.getCmdOut(frontend + aptitude + action + QStringLiteral(" ") + recommends_aptitude
                                      + QStringLiteral("-t ") + ver_name + QStringLiteral("-backports ") + names
                                      + QStringLiteral(" |tail -2 |head -1"));
    } else if (tree == ui->treeMXtest) {
        recommends = (ui->checkBoxInstallRecommendsMX->isChecked()) ? QStringLiteral("--install-recommends ")
                                                                    : QLatin1String("");
        recommends_aptitude = (ui->checkBoxInstallRecommendsMX->isChecked()) ? QStringLiteral("--with-recommends ")
                                                                             : QStringLiteral("--without-recommends ");
        detailed_names = cmd.getCmdOut(
            frontend + aptget + action + " -t mx " + recommends + "--reinstall " + names
            + R"lit(|grep 'Inst\|Remv' | awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info = cmd.getCmdOut(frontend + aptitude + action + " -t mx " + recommends_aptitude + names
                                      + " |tail -2 |head -1");
    } else {
        recommends = (ui->checkBoxInstallRecommends->isChecked()) ? QStringLiteral("--install-recommends ")
                                                                  : QLatin1String("");
        recommends_aptitude = (ui->checkBoxInstallRecommends->isChecked()) ? QStringLiteral("--with-recommends ")
                                                                           : QStringLiteral("--without-recommends ");
        detailed_names = cmd.getCmdOut(
            frontend + aptget + action + " " + recommends + "--reinstall " + names
            + R"lit(|grep 'Inst\|Remv'| awk '{V=""; P="";}; $3 ~ /^\[/ { V=$3 }; $3 ~ /^\(/ { P=$3 ")"}; $4 ~ /^\(/ {P=" => " $4 ")"};  {print $2 ";" V  P ";" $1}')lit");
        aptitude_info
            = cmd.getCmdOut(frontend + aptitude + action + " " + recommends_aptitude + names + " |tail -2 |head -1");
    }
    lock_file->lock();

    if (tree != ui->treeFlatpak) {
        detailed_installed_names = detailed_names.split(QStringLiteral("\n"));
    }

    detailed_installed_names.sort();
    qDebug() << "detailed installed names sorted " << detailed_installed_names;
    QStringListIterator iterator(detailed_installed_names);

    if (tree != ui->treeFlatpak) {
        while (iterator.hasNext()) {
            QString value = iterator.next();
            if (value.contains(QLatin1String("Remv"))) {
                value = value.section(QStringLiteral(";"), 0, 0) + " " + value.section(QStringLiteral(";"), 1, 1);
                detailed_removed_names = detailed_removed_names + value + "\n";
            }
            if (value.contains(QLatin1String("Inst"))) {
                value = value.section(QStringLiteral(";"), 0, 0) + " " + value.section(QStringLiteral(";"), 1, 1);
                detailed_to_install = detailed_to_install + value + "\n";
            }
        }
        if (!detailed_removed_names.isEmpty()) {
            detailed_removed_names.prepend(tr("Remove") + "\n");
        }
        if (!detailed_to_install.isEmpty()) {
            detailed_to_install.prepend(tr("Install") + "\n");
        }
    } else {
        if (action == QLatin1String("remove")) {
            detailed_removed_names = change_list.join(QStringLiteral("\n"));
            detailed_to_install.clear();
        }
        if (action == QLatin1String("install")) {
            detailed_to_install = change_list.join(QStringLiteral("\n"));
            detailed_removed_names.clear();
        }
    }

    msg = "<b>" + tr("The following packages were selected. Click Show Details for list of changes.") + "</b>";

    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.setInformativeText("\n" + names + "\n\n" + aptitude_info);

    if (action == QLatin1String("install")) {
        msgBox.setDetailedText(detailed_to_install + "\n" + detailed_removed_names);
    } else {
        msgBox.setDetailedText(detailed_removed_names + "\n" + detailed_to_install);
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
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Installing packages..."));

    bool success = false;

    QString recommends;

    // simulate install of selections and present for confirmation
    // if user selects cancel, break routine but return success to avoid error message
    if (!confirmActions(names, QStringLiteral("install"))) {
        return true;
    }

    enableOutput();
    QString frontend = QStringLiteral("DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null | "
                                      "grep -sq ^i && echo kde || echo gnome) ");
    QString aptget = QStringLiteral("apt-get -o=Dpkg::Use-Pty=0 install -y ");

    lock_file->unlock();
    if (tree == ui->treeBackports) {
        recommends = (ui->checkBoxInstallRecommendsMXBP->isChecked()) ? QStringLiteral("--install-recommends ")
                                                                      : QLatin1String("");
        success = cmd.run(frontend + aptget + recommends + "-t " + ver_name + "-backports --reinstall " + names);
    } else if (tree == ui->treeMXtest) {
        recommends = (ui->checkBoxInstallRecommendsMX->isChecked()) ? QStringLiteral("--install-recommends ")
                                                                    : QLatin1String("");
        success = cmd.run(frontend + aptget + recommends + " -t mx " + names);
    } else {
        recommends = (ui->checkBoxInstallRecommends->isChecked()) ? QStringLiteral("--install-recommends ")
                                                                  : QLatin1String("");
        success = cmd.run(frontend + aptget + recommends + "--reinstall " + names);
    }
    lock_file->lock();

    return success;
}

// install a list of application and run postprocess for each of them.
bool MainWindow::installBatch(const QStringList &name_list)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    QString postinstall;
    QString install_names;
    bool result = true;

    // load all the
    for (const QString &name : name_list) {
        for (const QStringList &list : qAsConst(popular_apps)) {
            if (list.at(Popular::Name) == name) {
                postinstall += list.at(Popular::Postinstall) + QStringLiteral("\n");
                install_names += list.at(Popular::InstallNames) + QStringLiteral(" ");
            }
        }
    }

    if (!install_names.isEmpty()) {
        if (!install(install_names)) {
            result = false;
        }
    }

    if (postinstall != QLatin1String("\n")) {
        qDebug() << "Post-install";
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Post-processing..."));
        lock_file->unlock();
        enableOutput();
        if (!cmd.run(postinstall)) {
            result = false;
        }
    }
    lock_file->lock();
    return result;
}

bool MainWindow::installPopularApp(const QString &name)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    bool result = true;
    QString preinstall;
    QString postinstall;
    QString install_names;

    // get all the app info
    for (const QStringList &list : qAsConst(popular_apps)) {
        if (list.at(Popular::Name) == name) {
            preinstall = list.at(Popular::Preinstall);
            postinstall = list.at(Popular::Postinstall);
            install_names = list.at(Popular::InstallNames);
        }
    }
    enableOutput();
    // preinstall
    if (!preinstall.isEmpty()) {
        qDebug() << "Pre-install";
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Pre-processing for ") + name);
        lock_file->unlock();
        if (!cmd.run(preinstall)) {
            QFile file(QStringLiteral("/etc/apt/sources.list.d/mxpitemp.list")); // remove temp source list if it exists
            if (file.exists()) {
                file.remove();
                updateApt();
            }
            lock_file->lock();
            return false;
        }
    }
    // install
    if (!install_names.isEmpty()) {
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Installing ") + name);
        result = install(install_names);
    }
    enableOutput();
    // postinstall
    if (!postinstall.isEmpty()) {
        qDebug() << "Post-install";
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Post-processing for ") + name);
        lock_file->unlock();
        cmd.run(postinstall);
    }
    lock_file->lock();
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

    // make a list of apps to be installed together
    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
            QString name = (*it)->text(2);
            for (const QStringList &list : qAsConst(popular_apps)) {
                if (list.at(Popular::Name) == name) {
                    const QString &preinstall = list.at(Popular::Preinstall);
                    if (preinstall.isEmpty()) { // add to batch processing if there is not
                                                // preinstall command
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

    // install the rest of the apps
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
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tabOutput), true);
    QString names = change_list.join(QStringLiteral(" "));

    // change sources as needed
    if (tree == ui->treeMXtest) {
        // add testrepo unless already enabled
        if (!test_initially_enabled) {
            QString suite = ver_name;
            if (ver_name == QLatin1String("jessie")) { // use 'mx15' for Stretch based MX, user
                                                       // version name for newer versions
                suite = QStringLiteral("mx15");
            }
            cmd.run("apt-get update --print-uris | tac | grep -m1 -oE 'https?://.*/mx/repo/dists/" + suite
                    + "/main' | sed 's:^:deb :; s:/repo/dists/:/testrepo :; s:/main: test:' > "
                      "/etc/apt/sources.list.d/mxpm-temp.list");
        }
        updateApt();
    } else if (tree == ui->treeBackports) {
        cmd.run("echo deb http://ftp.debian.org/debian " + ver_name
                + "-backports main contrib non-free>/etc/apt/sources.list.d/mxpm-temp.list");
        updateApt();
    }
    getDebianVerNum();
    bool result = install(names);
    if (tree == ui->treeBackports || (tree == ui->treeMXtest && !test_initially_enabled)) {
        if (QFile::remove(QStringLiteral("/etc/apt/sources.list.d/mxpm-temp.list"))) {
            updateApt();
        }
    }
    change_list.clear();
    installed_packages = listInstalled();
    return result;
}

bool MainWindow::isFilteredName(const QString &name)
{
    return ((name.startsWith(QLatin1String("lib")) && !name.startsWith(QLatin1String("libreoffice")))
            || name.endsWith(QLatin1String("-dev")) || name.endsWith(QLatin1String("-dbg"))
            || name.endsWith(QLatin1String("-dbgsym")) || name.endsWith(QLatin1String("-libs")));
}

bool MainWindow::isOnline()
{
    if (settings.value(QStringLiteral("skiponlinecheck"), false).toBool() || args.isSet("skip-online-check")) {
        return true;
    }

    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + "/"
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
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                [&error](QNetworkReply::NetworkError err) { error = err; }); // errorOccured only in Qt >= 5.15
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &loop, &QEventLoop::quit);
        auto timeout = settings.value(QStringLiteral("timeout"), 5000).toInt();
        QTimer::singleShot(timeout, &loop, [&loop, &error] {
            error = QNetworkReply::TimeoutError;
            loop.quit();
        }); // manager.setTransferTimeout(time) // only in Qt >= 5.15
        loop.exec();
        reply->disconnect();
        if (error == QNetworkReply::NoError) {
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

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + "/"
                                           + QApplication::applicationVersion().toUtf8() + " (linux-gnu)");

    reply = manager.get(request);
    QEventLoop loop;

    bool success = true;
    connect(reply, &QNetworkReply::readyRead, this,
            [this, &file, &success] { success = (file.write(reply->readAll()) != 0); });
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->disconnect();

    if (!success) {
        QMessageBox::warning(this, tr("Error"),
                             tr("There was an error writing file: %1. Please check if you have "
                                "enough free space on your drive")
                                 .arg(file.fileName()));
        exit(EXIT_FAILURE);
    }

    file.close();
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "There was an error downloading the file:" << url;
        return false;
    }
    return true;
}

bool MainWindow::downloadAndUnzip(const QString &url, QFile &file)
{
    if (!downloadFile(url, file)) {
        file.remove();
        QFile::remove(QFileInfo(file.fileName()).path() + "/"
                      + QFileInfo(file.fileName()).baseName()); // rm unzipped file
        return false;
    } else {
        QString unzip = (QFileInfo(file).suffix() == QLatin1String("gz")) ? QStringLiteral("gunzip -f ")
                                                                          : QStringLiteral("unxz -f ");

        if (!cmd.run(unzip + file.fileName())) {
            qDebug() << "Could not unzip file:" << file.fileName();
            return false;
        }
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
            progress->show();
            if (!updateApt()) {
                return false;
            }
        }
        progress->show();
        AptCache cache;
        enabled_list = cache.getCandidates();
        if (enabled_list.isEmpty()) {
            updateApt();
            AptCache cache2;
            enabled_list = cache2.getCandidates();
        }
    }

    if (tree == ui->treeMXtest) {
        if (!QFile::exists(tmp_dir.path() + "/mxPackages") || force_download) {
            progress->show();

            QFile file(tmp_dir.path() + "/mxPackages.gz");
            QString url = QStringLiteral("http://mxrepo.com/mx/testrepo/dists/");
            QString testrepo_url = url;
            if (!cmd.run("apt-get update --print-uris | tac | grep -m1 -oP 'https?://.*/mx/testrepo/dists/(?="
                             + ver_name + "/test/)'",
                         &testrepo_url)) {
                cmd.run("apt-get update --print-uris | tac | grep -m1 -oE 'https?://.*/mx/repo/dists/" + ver_name
                            + "/main/' | sed -e 's:/mx/repo/dists/" + ver_name
                            + "/main/:/mx/testrepo/dists/:' | grep -oE 'https?://.*/mx/testrepo/dists/'",
                        &testrepo_url);
            }
            url = testrepo_url;

            QString branch = QStringLiteral("/test");
            QString format = QStringLiteral("gz");
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }
        }
    } else if (tree == ui->treeBackports) {
        if (!QFile::exists(tmp_dir.path() + "/mainPackages") || !QFile::exists(tmp_dir.path() + "/contribPackages")
            || !QFile::exists(tmp_dir.path() + "/nonfreePackages") || force_download) {
            progress->show();

            QFile file(tmp_dir.path() + "/mainPackages.xz");
            QString url = QStringLiteral("http://deb.debian.org/debian/dists/");
            QString branch = QStringLiteral("-backports/main");
            QString format = QStringLiteral("xz");
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }

            file.setFileName(tmp_dir.path() + "/contribPackages.xz");
            branch = QStringLiteral("-backports/contrib");
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }

            file.setFileName(tmp_dir.path() + "/nonfreePackages.xz");
            branch = QStringLiteral("-backports/non-free");
            if (!downloadAndUnzip(url, ver_name, branch, format, file)) {
                return false;
            }

            pushCancel->setDisabled(true);
            QFile outputFile("allPackages");
            if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qWarning() << "Could not open: " << outputFile;
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

void MainWindow::enableTabs(bool enable)
{
    for (int tab = 0; tab < ui->tabWidget->count() - 1; ++tab) { // enable all except last (Console)
        ui->tabWidget->setTabEnabled(tab, enable);
    }
    if (arch == "i386") {
        ui->tabWidget->setTabEnabled(Tab::Flatpak, false);
    }
}

// Process downloaded *Packages.gz files
bool MainWindow::readPackageList(bool force_download)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    pushCancel->setDisabled(true);
    // Don't process if the lists are already populated
    if (!((tree == ui->treeEnabled && enabled_list.isEmpty()) || (tree == ui->treeMXtest && mx_list.isEmpty())
          || (tree == ui->treeBackports && backports_list.isEmpty()) || force_download)) {
        return true;
    }

    QFile file;
    if (tree == ui->treeMXtest) { // read MX Test list
        file.setFileName(tmp_dir.path() + "/mxPackages");
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "Could not open file: " << file.fileName();
            return false;
        }
    } else if (tree == ui->treeBackports) { // read Backports list
        file.setFileName(tmp_dir.path() + "/allPackages");
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "Could not open file: " << file.fileName();
            return false;
        }
    } else if (tree == ui->treeEnabled) { // treeEnabled is updated at downloadPackageList
        return true;
    }

    QString file_content = file.readAll();
    file.close();

    QMap<QString, QStringList> *map {};
    map = (tree == ui->treeMXtest) ? &mx_list : &backports_list;
    map->clear();
    QString package;
    QString version;
    QString description;

    const QStringList list = file_content.split(QStringLiteral("\n"));
    for (const QString &line : list) {
        if (line.startsWith(QLatin1String("Package: "))) {
            package = line.mid(9);
        } else if (line.startsWith(QLatin1String("Version: "))) {
            version = line.mid(9);
        } else if (line.startsWith(QLatin1String("Description: "))) {
            description = line.mid(13);
            map->insert(package, {version, description});
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
    auto screenGeometry = QApplication::primaryScreen()->geometry();
    auto x = (screenGeometry.width() - this->width()) / 2;
    auto y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

void MainWindow::clearUi()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    blockSignals(true);
    ui->pushCancel->setEnabled(true);
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);

    if (tree == ui->treeEnabled) {
        ui->labelNumApps->clear();
        ui->labelNumInst->clear();
        ui->labelNumUpgr->clear();
        ui->treeEnabled->clear();
        ui->pushUpgradeAll->setHidden(true);
    } else if (tree == ui->treeMXtest) {
        ui->labelNumApps_2->clear();
        ui->labelNumInstMX->clear();
        ui->labelNumUpgrMX->clear();
        ui->treeMXtest->clear();
    } else if (tree == ui->treeBackports) {
        ui->labelNumApps_3->clear();
        ui->labelNumInstBP->clear();
        ui->labelNumUpgrBP->clear();
        ui->treeBackports->clear();
    }
    ui->comboFilterBP->setCurrentIndex(0);
    ui->comboFilterFlatpak->setCurrentIndex(0);
    ui->comboFilterMX->setCurrentIndex(0);
    ui->comboFilterEnabled->setCurrentIndex(0);
    blockSignals(false);
}

void MainWindow::cleanup()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    cmd.close();
    bool changed = false;
    if (QFile::remove(QStringLiteral("/etc/apt/sources.list.d/mxpm-temp.list"))) {
        changed = true;
    }

    if (changed) {
        QProcess::startDetached("apt-get", {"update"});
    }

    lock_file->unlock();
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
}

QString MainWindow::getVersion(const QString &name)
{
    return cmd.getCmdOut("dpkg-query -f '${Version}' -W " + name);
}

// Return true if all the packages listed are installed
bool MainWindow::checkInstalled(const QString &names) const
{
    if (names.isEmpty()) {
        return false;
    }

    auto names_list = names.split(QStringLiteral("\n"));
    return std::all_of(names_list.cbegin(), names_list.cend(),
                       [&](const QString &name) { return installed_packages.contains(name.trimmed()); });
}

// Return true if all the packages in the list are installed
bool MainWindow::checkInstalled(const QStringList &name_list) const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (name_list.isEmpty()) {
        return false;
    }

    for (const QString &name : name_list) {
        if (!installed_packages.contains(name)) {
            return false;
        }
    }
    return true;
}

// return true if all the items in the list are upgradable
bool MainWindow::checkUpgradable(const QStringList &name_list) const
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    if (name_list.isEmpty()) {
        return false;
    }

    QList<QTreeWidgetItem *> item_list;
    for (const QString &name : name_list) {
        item_list = tree->findItems(name, Qt::MatchExactly, TreeCol::Name);
        if (item_list.isEmpty() || item_list.at(0)->text(TreeCol::Status) != QLatin1String("upgradable")) {
            return false;
        }
    }
    return true;
}

QStringList MainWindow::listInstalled()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    disconnect(conn);
    QString str = cmd.getCmdOut(QStringLiteral("dpkg --get-selections | grep -v deinstall | cut -f1"));
    conn = connect(&cmd, &Cmd::outputAvailable, [](const QString &out) { qDebug() << out.trimmed(); });
    str.remove(QStringLiteral(":i386"));
    str.remove(QStringLiteral(":amd64"));
    return str.split(QStringLiteral("\n"));
}

QStringList MainWindow::listFlatpaks(const QString &remote, const QString &type)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    static bool updated = false;

    bool success = false;
    QString out;
    QStringList list;

    // need to specify arch for older version (flatpak takes different format than dpkg)
    QString arch_fp;
    if (arch == QLatin1String("amd64")) {
        arch_fp = QStringLiteral("--arch=x86_64 ");
    } else if (arch == QLatin1String("i386")) {
        arch_fp = QStringLiteral("--arch=i386 ");
    } else if (arch == QLatin1String("armhf")) {
        arch_fp = QStringLiteral("--arch=arm ");
    } else {
        return {};
    }

    disconnect(conn);
    if (fp_ver < VersionNumber(QStringLiteral("1.0.1"))) {
        // list packages, strip first part remote/ or app/ no size for old flatpak
        success = cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"set -o pipefail; "
                          "flatpak -d remote-ls "
                              + user + remote + " " + arch_fp + type
                              + R"(2>/dev/null| cut -f1 | tr -s ' ' | cut -f1 -d' '|sed 's/^[^\/]*\///g' ")",
                          &out);
        list = QString(out).split(QStringLiteral("\n"));
    } else if (fp_ver < VersionNumber(QStringLiteral("1.2.4"))) { // lower than Buster version
        // list size too
        success = cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"set -o pipefail; "
                          "flatpak -d remote-ls "
                              + user + remote + " " + arch_fp + type
                              + R"(2>/dev/null| cut -f1,3 |tr -s ' ' | sed 's/^[^\/]*\///g' ")",
                          &out);
        list = QString(out).split(QStringLiteral("\n"));
    } else { // Buster version and above
        if (!updated) {
            success = cmd.run(QStringLiteral(
                "runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak update --appstream\""));
            updated = true;
        }
        // list version too, unfortunatelly the resulting string structure is different depending on
        // type option
        if (type == QLatin1String("--app") || type.isEmpty()) {
            success
                = cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"set -o pipefail; "
                          "flatpak remote-ls "
                              + user + remote + " " + arch_fp + "--app --columns=ver,ref,installed-size 2>/dev/null\"",
                          &out);
            list = QString(out).split(QStringLiteral("\n"));
            if (list == QStringList(QLatin1String(""))) {
                list = QStringList();
            }
        }
        if (type == QLatin1String("--runtime") || type.isEmpty()) {
            success = cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"set -o pipefail; "
                              "flatpak remote-ls "
                                  + user + remote + " " + arch_fp
                                  + "--runtime --columns=branch,ref,installed-size 2>/dev/null\"",
                              &out);
            list += QString(out).split(QStringLiteral("\n"));
            if (list == QStringList(QLatin1String(""))) {
                list = QStringList();
            }
        }
    }
    conn = connect(&cmd, &Cmd::outputAvailable, [](const QString &out) { qDebug() << out.trimmed(); });

    if (!success || list == QStringList(QLatin1String(""))) {
        qDebug()
            << QStringLiteral("Could not list packages from %1 remote, or remote doesn't contain packages").arg(remote);
        return {};
    }
    return list;
}

// list installed flatpaks by type: apps, runtimes, or all (if no type is provided)
QStringList MainWindow::listInstalledFlatpaks(const QString &type)
{
    QStringList list;
    if (fp_ver < VersionNumber(QStringLiteral("1.2.4"))) {
        list << cmd.getCmdOut("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak -d list "
                              "2>/dev/null "
                              + user + type + "|cut -f1|cut -f1 -d' '\"")
                    .remove(QStringLiteral(" "))
                    .split(QStringLiteral("\n"));
    } else {
        list << cmd.getCmdOut(
                       "runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak list 2>/dev/null "
                       + user + type + " --columns=ref\"")
                    .remove(QStringLiteral(" "))
                    .split(QStringLiteral("\n"));
    }
    if (list == QStringList(QLatin1String(""))) {
        list = QStringList();
    }
    return list;
}

void MainWindow::setCurrentTree()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    const QList list({ui->treePopularApps, ui->treeEnabled, ui->treeMXtest, ui->treeBackports, ui->treeFlatpak});

    auto it = std::find_if(list.cbegin(), list.cend(), [](const auto &item) { return item->isVisible(); });
    if (it != list.end()) {
        tree = *it;
        updateInterface();
        return;
    }
}

void MainWindow::setDirty()
{
    dirtyBackports = dirtyEnabledRepos = dirtyTest = true;
}

QHash<QString, VersionNumber> MainWindow::listInstalledVersions()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    disconnect(conn);
    QString out = cmd.getCmdOut(QStringLiteral("dpkg -l | grep '^ii'"), true);
    conn = connect(&cmd, &Cmd::outputAvailable, [](const QString &out) { qDebug() << out.trimmed(); });

    QString name;
    QString ver_str;
    QStringList item;
    QHash<QString, VersionNumber> result;

    const QStringList list = out.split(QStringLiteral("\n"));
    for (const QString &line : list) {
        item = line.split(QRegularExpression(QStringLiteral("\\s{2,}")));
        name = item.at(1);
        name.remove(QLatin1String(":i386")).remove(QLatin1String(":amd64"));
        ver_str = item.at(2);
        ver_str.remove(QLatin1String(" amd64"));
        result.insert(name, VersionNumber(ver_str));
    }
    return result;
}

void MainWindow::cmdStart()
{
    timer.start(100ms);
    setCursor(QCursor(Qt::BusyCursor));
    ui->lineEdit->setFocus();
}

void MainWindow::cmdDone()
{
    timer.stop();
    setCursor(QCursor(Qt::ArrowCursor));
    disableOutput();
    bar->setValue(bar->maximum());
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
        if (line == QStringLiteral("Package: ") + item_name) {
            msg += line + QStringLiteral("\n");
            line.clear();
            while (!in.atEnd()) {
                line = in.readLine();
                if (line.startsWith(QLatin1String("Package: "))) {
                    break;
                }
                msg += line + QStringLiteral("\n");
            }
        }
    }
    auto msg_list = msg.split(QStringLiteral("\n"));
    auto max_no_chars = 2000;        // around 15-17 lines
    if (msg.size() > max_no_chars) { // split msg into details if too large
        auto max_no_lines = 20;      // cut message after these many lines
        msg = msg_list.mid(0, max_no_lines).join(QStringLiteral("\n"));
    }

    QMessageBox info(QMessageBox::NoIcon, tr("Package info"), msg, QMessageBox::Close);

    // make it wider
    auto *horizontalSpacer = new QSpacerItem(this->width(), 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto *layout = qobject_cast<QGridLayout *>(info.layout());
    layout->addItem(horizontalSpacer, 0, 1);
    info.exec();
}

void MainWindow::displayPackageInfo(const QTreeWidget *tree, QPoint pos)
{
    auto *t_widget = qobject_cast<QTreeWidget *>(focusWidget());
    auto *action = new QAction(QIcon::fromTheme(QStringLiteral("dialog-information")), tr("More &info..."), this);
    if (tree == ui->treePopularApps) {
        if (t_widget->currentItem()->parent() == nullptr) { // skip categories
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
    action->deleteLater();
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

    QUrl url = item->text(PopCol::Screenshot); // screenshot url

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
                msg += QStringLiteral("<p><img src='data:image/png;base64, %0'>").arg(QString(data.toBase64()));
            }
        }
    }
    QMessageBox info(QMessageBox::NoIcon, tr("Package info"), msg, QMessageBox::Close);
    info.exec();
}

void MainWindow::displayPackageInfo(const QTreeWidgetItem *item)
{
    QString msg = cmd.getCmdOut("aptitude show " + item->text(TreeCol::Name));
    // remove first 5 lines from aptitude output "Reading package..."
    QString details
        = cmd.getCmdOut("DEBIAN_FRONTEND=$(dpkg -l debconf-kde-helper 2>/dev/null "
                        "| grep -sq ^i && echo kde || echo gnome) aptitude -sy -V -o=Dpkg::Use-Pty=0 install "
                        + item->text(TreeCol::Name) + " |tail -5");

    auto detail_list = details.split(QStringLiteral("\n"));
    auto msg_list = msg.split(QStringLiteral("\n"));
    auto max_no_chars = 2000;        // around 15-17 lines
    if (msg.size() > max_no_chars) { // split msg into details if too large
        auto max_no_lines = 17;      // cut message after these many lines
        msg = msg_list.mid(0, max_no_lines).join(QStringLiteral("\n"));
        detail_list = msg_list.mid(max_no_lines, msg_list.length()) + QStringList {} + detail_list;
        details = detail_list.join(QStringLiteral("\n"));
    }
    msg += "\n\n" + detail_list.at(detail_list.size() - 2); // add info about space needed/freed

    QMessageBox info(QMessageBox::NoIcon, tr("Package info"), msg.trimmed(), QMessageBox::Close);
    info.setDetailedText(details.trimmed());

    // make it wider
    auto *horizontalSpacer = new QSpacerItem(this->width(), 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto *layout = qobject_cast<QGridLayout *>(info.layout());
    layout->addItem(horizontalSpacer, 0, 1);
    info.exec();
}

void MainWindow::findPopular() const
{
    QString word = ui->searchPopular->text();
    if (word.length() == 1) {
        return;
    }

    if (word.isEmpty()) {
        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            (*it)->setExpanded(false);
        }
        ui->treePopularApps->reset();
        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            if ((*it)->parent() == nullptr) {
                (*it)->setFirstColumnSpanned(true);
            }
        }
        for (int i = 1; i < ui->treePopularApps->columnCount(); ++i) {
            ui->treePopularApps->resizeColumnToContents(i);
        }
        return;
    }
    auto found_items = ui->treePopularApps->findItems(word, Qt::MatchContains | Qt::MatchRecursive, PopCol::Name);
    found_items << ui->treePopularApps->findItems(word, Qt::MatchContains | Qt::MatchRecursive,
                                                  PopCol::Icon); // category
    found_items << ui->treePopularApps->findItems(word, Qt::MatchContains | Qt::MatchRecursive, PopCol::Description);

    // hide/unhide items
    for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
        if ((*it)->parent() != nullptr) { // if child
            if (found_items.contains(*it)) {
                (*it)->setHidden(false);
            } else {
                (*it)->parent()->setHidden(true);
                (*it)->setHidden(true);
            }
        }
    }

    // process found items
    for (const auto &item : qAsConst(found_items)) {
        if (item->parent() != nullptr) { // if child, expand parent
            item->parent()->setExpanded(true);
            item->parent()->setHidden(false);
        } else { // if parent, expand children
            item->setFirstColumnSpanned(true);
            item->setExpanded(true);
            item->setHidden(false);
            for (int i = 0; i < item->childCount(); ++i) {
                item->child(i)->setHidden(false);
            }
        }
    }
    for (int i = 1; i < ui->treePopularApps->columnCount(); ++i) {
        ui->treePopularApps->resizeColumnToContents(i);
    }
}

void MainWindow::findPackageOther()
{
    QString word;
    if (tree == ui->treeEnabled) {
        word = ui->searchBoxEnabled->text();
    } else if (tree == ui->treeMXtest) {
        word = ui->searchBoxMX->text();
    } else if (tree == ui->treeBackports) {
        word = ui->searchBoxBP->text();
    } else if (tree == ui->treeFlatpak) {
        word = ui->searchBoxFlatpak->text();
    }
    if (word.length() == 1) {
        return;
    }

    auto found_items = tree->findItems(word, Qt::MatchContains, TreeCol::Name);
    if (tree != ui->treeFlatpak) { // not for treeFlatpak as it has a different column structure
        found_items << tree->findItems(word, Qt::MatchContains, TreeCol::Description);
    }

    for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
        (*it)->setHidden((*it)->data(0, Qt::UserRole) == false || !found_items.contains(*it));
        // Hide libs
        if (tree != ui->treeFlatpak && ui->checkHideLibs->isChecked() && isFilteredName((*it)->text(TreeCol::Name))) {
            (*it)->setHidden(true);
        }
    }
}

void MainWindow::showOutput()
{
    ui->outputBox->clear();
    ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tabOutput), true);
    ui->tabWidget->setCurrentWidget(ui->tabOutput);
    enableTabs(false);
}

void MainWindow::on_pushInstall_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    // qDebug() << "change list"  << .join(" ");
    showOutput();

    if (tree == ui->treePopularApps) {
        bool success = installPopularApps();
        if (!enabled_list.isEmpty()) { // clear cache to update list if it already exists
            buildPackageLists();
        }
        if (success) {
            setDirty();
            refreshPopularApps();
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(tree->parentWidget());
        } else {
            refreshPopularApps();
            QMessageBox::critical(this, tr("Error"),
                                  tr("Problem detected while installing, please inspect the console output."));
        }
    } else if (tree == ui->treeFlatpak) {
        // confirmation dialog
        if (!confirmActions(change_list.join(QStringLiteral(" ")), QStringLiteral("install"))) {
            displayFlatpaks(true);
            indexFilterFP.clear();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            ui->tabWidget->blockSignals(false);
            enableTabs(true);
            return;
        }
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        if (cmd.run(
                "runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"socat SYSTEM:'flatpak install -y "
                + user + ui->comboRemote->currentText() + " " + change_list.join(QStringLiteral(" "))
                + "',stderr STDIO\"")) {
            displayFlatpaks(true);
            indexFilterFP.clear();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            ui->tabWidget->blockSignals(false);
        } else {
            setCursor(QCursor(Qt::ArrowCursor));
            QMessageBox::critical(this, tr("Error"),
                                  tr("Problem detected while installing, please inspect the console output."));
        }
    } else {
        bool success = installSelected();
        setDirty();
        buildPackageLists();
        refreshPopularApps();
        if (success) {
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->setCurrentWidget(tree->parentWidget());
        } else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Problem detected while installing, please inspect the console output."));
        }
    }
    enableTabs(true);
}

void MainWindow::on_pushAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(this->windowTitle()),
        "<p align=\"center\"><b><h2>" + this->windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + QCoreApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Package Installer for MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-packageinstaller/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void MainWindow::on_pushHelp_clicked()
{
    QString lang = locale.bcp47Name();
    QString url {"/usr/share/doc/mx-packageinstaller/mx-package-installer.html"};

    if (lang.startsWith(QLatin1String("fr"))) {
        url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-mx-installateur-de-paquets");
    }
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

// Resize columns when expanding
void MainWindow::on_treePopularApps_expanded()
{
    ui->treePopularApps->resizeColumnToContents(PopCol::Name);
    ui->treePopularApps->resizeColumnToContents(PopCol::Description);
}

void MainWindow::on_treePopularApps_itemExpanded(QTreeWidgetItem *item)
{
    item->setIcon(PopCol::Icon, QIcon::fromTheme(QStringLiteral("folder-open")));
    ui->treePopularApps->resizeColumnToContents(PopCol::Name);
    ui->treePopularApps->resizeColumnToContents(PopCol::Description);
}

void MainWindow::on_treePopularApps_itemCollapsed(QTreeWidgetItem *item)
{
    item->setIcon(PopCol::Icon, QIcon::fromTheme(QStringLiteral("folder")));
    ui->treePopularApps->resizeColumnToContents(PopCol::Name);
    ui->treePopularApps->resizeColumnToContents(PopCol::Description);
}

void MainWindow::on_pushUninstall_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";

    showOutput();

    QString names;
    QString preuninstall;
    QString postuninstall;
    if (tree == ui->treePopularApps) {
        for (QTreeWidgetItemIterator it(ui->treePopularApps); (*it) != nullptr; ++it) {
            if ((*it)->checkState(PopCol::Check) == Qt::Checked) {
                names += (*it)->text(PopCol::UninstallNames).replace(QLatin1String("\n"), QLatin1String(" ")) + " ";
                postuninstall += (*it)->text(PopCol::PostUninstall) + "\n";
                preuninstall += (*it)->text(PopCol::PreUninstall) + "\n";
            }
        }
    } else if (tree == ui->treeFlatpak) {
        bool success = true;

        // new version of flatpak takes a "-y" confirmation
        QString conf = QStringLiteral("-y ");
        if (fp_ver < VersionNumber(QStringLiteral("1.0.1"))) {
            conf = QString();
        }
        // confirmation dialog
        if (!confirmActions(change_list.join(QStringLiteral(" ")), QStringLiteral("remove"))) {
            displayFlatpaks(true);
            indexFilterFP.clear();
            listFlatpakRemotes();
            ui->comboRemote->setCurrentIndex(0);
            on_comboRemote_activated();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            ui->tabWidget->blockSignals(false);
            enableTabs(true);
            return;
        }

        setCursor(QCursor(Qt::BusyCursor));
        for (const QString &app : qAsConst(change_list)) {
            enableOutput();
            if (!cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"socat SYSTEM:'flatpak "
                         "uninstall "
                         + conf + user + app + "',stderr STDIO\"")) { // success if all processed successfuly, failure
                                                                      // if one failed
                success = false;
            }
        }
        if (success) { // success if all processed successfuly, failure if one failed
            displayFlatpaks(true);
            indexFilterFP.clear();
            listFlatpakRemotes();
            ui->comboRemote->setCurrentIndex(0);
            on_comboRemote_activated();
            ui->comboFilterFlatpak->setCurrentIndex(0);
            QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            ui->tabWidget->blockSignals(false);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("We encountered a problem uninstalling, please check output"));
        }
        enableTabs(true);
        return;
    } else {
        names = change_list.join(QStringLiteral(" "));
    }

    setDirty();
    if (uninstall(names, preuninstall, postuninstall)) {
        if (!enabled_list.isEmpty()) { // update list if it already exists
            buildPackageLists();
        }
        refreshPopularApps();
        QMessageBox::information(this, tr("Success"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(tree->parentWidget());
    } else {
        if (!enabled_list.isEmpty()) { // update list if it already exists
            buildPackageLists();
        }
        refreshPopularApps();
        QMessageBox::critical(this, tr("Error"), tr("We encountered a problem uninstalling the program"));
    }
    enableTabs(true);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Console Output"));
    ui->pushInstall->setEnabled(false);
    ui->pushUninstall->setEnabled(false);

    // reset checkboxes when tab changes
    if (tree != ui->treePopularApps) {
        tree->blockSignals(true);
        tree->clearSelection();

        for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
            (*it)->setCheckState(0, Qt::Unchecked);
        }
        tree->blockSignals(false);
    }

    // save the search text
    QString search_str;
    int filter_idx = 0;
    if (tree == ui->treePopularApps) {
        search_str = ui->searchPopular->text();
    } else if (tree == ui->treeEnabled) {
        search_str = ui->searchBoxEnabled->text();
        filter_idx = ui->comboFilterEnabled->currentIndex();
    } else if (tree == ui->treeMXtest) {
        search_str = ui->searchBoxMX->text();
        filter_idx = ui->comboFilterMX->currentIndex();
    } else if (tree == ui->treeBackports) {
        search_str = ui->searchBoxBP->text();
        filter_idx = ui->comboFilterBP->currentIndex();
    } else if (tree == ui->treeFlatpak) {
        search_str = ui->searchBoxFlatpak->text();
    }

    Cmd shell;
    switch (bool success = false; index) {
    case Tab::Popular:
        ui->searchPopular->setText(search_str);
        enableTabs(true);
        setCurrentTree();
        if (!ui->searchPopular->text().isEmpty()) {
            findPopular();
        }
        ui->searchPopular->setFocus();
        break;
    case Tab::EnabledRepos:
        ui->searchBoxEnabled->setText(search_str);
        ui->pushRemoveOrphan->setVisible(
            shell.run(R"lit(test -n "$(apt-get --dry-run autoremove |grep -Po '^Remv \K[^ ]+' )")lit"));
        enableTabs(true);
        setCurrentTree();
        change_list.clear();
        if (tree->topLevelItemCount() == 0 || dirtyEnabledRepos) {
            if (!buildPackageLists()) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Could not download the list of packages. Please check your APT sources."));
                return;
            }
        }
        ui->comboFilterEnabled->setCurrentIndex(filter_idx);
        if (!ui->searchBoxEnabled->text().isEmpty()) {
            findPackageOther();
        }
        ui->searchBoxEnabled->setFocus();
        break;
    case Tab::Test:
        ui->searchBoxMX->setText(search_str);
        enableTabs(true);
        setCurrentTree();
        displayWarning(QStringLiteral("test"));
        change_list.clear();
        if (tree->topLevelItemCount() == 0 || dirtyTest) {
            if (!buildPackageLists()) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Could not download the list of packages. Please check your APT sources."));
                return;
            }
        }
        ui->comboFilterMX->setCurrentIndex(filter_idx);
        if (!ui->searchBoxMX->text().isEmpty()) {
            findPackageOther();
        }
        ui->searchBoxMX->setFocus();
        break;
    case Tab::Backports:
        ui->searchBoxBP->setText(search_str);
        enableTabs(true);
        setCurrentTree();
        displayWarning(QStringLiteral("backports"));
        change_list.clear();
        if (tree->topLevelItemCount() == 0 || dirtyBackports) {
            if (!buildPackageLists()) {
                QMessageBox::critical(this, tr("Error"),
                                      tr("Could not download the list of packages. Please check your APT sources."));
                return;
            }
        }
        ui->comboFilterBP->setCurrentIndex(filter_idx);
        if (!ui->searchBoxBP->text().isEmpty()) {
            findPackageOther();
        }
        ui->searchBoxBP->setFocus();
        break;
    case Tab::Flatpak:
        ui->searchBoxFlatpak->setText(search_str);
        enableTabs(true);
        setCurrentTree();
        displayWarning(QStringLiteral("flatpaks"));
        blockInterfaceFP(true);

        if (!checkInstalled(QStringLiteral("flatpak"))) {
            int ans = QMessageBox::question(this, tr("Flatpak not installed"),
                                            tr("Flatpak is not currently installed.\nOK to go ahead and install it?"));
            if (ans == QMessageBox::No) {
                ui->tabWidget->setCurrentIndex(Tab::Popular);
                break;
            }
            ui->tabWidget->setTabEnabled(ui->tabWidget->indexOf(ui->tabOutput), true);
            ui->tabWidget->setCurrentWidget(ui->tabOutput);
            setCursor(QCursor(Qt::BusyCursor));
            install(QStringLiteral("flatpak"));
            installed_packages = listInstalled();
            if (!checkInstalled(QStringLiteral("flatpak"))) {
                QMessageBox::critical(this, tr("Flatpak not installed"), tr("Flatpak was not installed"));
                ui->tabWidget->setCurrentIndex(Tab::Popular);
                setCursor(QCursor(Qt::ArrowCursor));
                break;
            }
            if (ui->treeEnabled != nullptr) { // mark flatpak installed in enabled tree
                auto hashInstalled = listInstalledVersions();
                VersionNumber installed = hashInstalled.value(QStringLiteral("flatpak"));
                const auto found_items
                    = ui->treeEnabled->findItems(QStringLiteral("flatpak"), Qt::MatchExactly, TreeCol::Name);
                for (const auto &item : found_items) {
                    for (int i = 0; i < ui->treeEnabled->columnCount(); ++i) {
                        item->setIcon(FlatCol::Check, QIcon::fromTheme(QStringLiteral("package-installed-updated"),
                                                                       QIcon(":/icons/package-installed-updated.png")));
                        item->setToolTip(i, tr("Latest version ") + installed.toString() + tr(" already installed"));
                    }
                    item->setText(TreeCol::Status, QStringLiteral("installed"));
                }
            }
            enableOutput();
            success = cmd.run(QStringLiteral("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c "
                                             "\"flatpak remote-add --if-not-exists "
                                             "flathub https://flathub.org/repo/flathub.flatpakrepo\""));
            if (!success) {
                QMessageBox::critical(this, tr("Flathub remote failed"), tr("Flathub remote could not be added"));
                ui->tabWidget->setCurrentIndex(Tab::Popular);
                setCursor(QCursor(Qt::ArrowCursor));
                break;
            }
            listFlatpakRemotes();
            displayFlatpaks(false);
            setCursor(QCursor(Qt::ArrowCursor));
            QMessageBox::warning(this, tr("Needs re-login"),
                                 tr("You might need to logout/login to see installed items in the menu"));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
            ui->tabWidget->blockSignals(false);
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tabOutput), tr("Console Output"));
            break;
        }
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        success = cmd.run(QStringLiteral("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak "
                                         "remote-add --if-not-exists flathub "
                                         "https://flathub.org/repo/flathub.flatpakrepo\""));
        if (!success) {
            QMessageBox::critical(this, tr("Flathub remote failed"), tr("Flathub remote could not be added"));
            ui->tabWidget->setCurrentIndex(Tab::Popular);
            setCursor(QCursor(Qt::ArrowCursor));
            break;
        }
        setCursor(QCursor(Qt::ArrowCursor));
        if (ui->comboRemote->currentText().isEmpty()) {
            listFlatpakRemotes();
        }
        displayFlatpaks(false);
        ui->searchBoxBP->setText(search_str);
        break;
    case Tab::Output:
        ui->searchPopular->clear();
        ui->searchBoxEnabled->clear();
        ui->searchBoxMX->clear();
        ui->searchBoxBP->clear();
        ui->pushInstall->setDisabled(true);
        ui->pushUninstall->setDisabled(true);
        break;
    }
    ui->pushUpgradeAll->setVisible((tree == ui->treeEnabled) && (ui->labelNumUpgr->text().toInt() > 0));
}

void MainWindow::filterChanged(const QString &arg1)
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    tree->blockSignals(true);

    QList<QTreeWidgetItem *> found_items;
    // filter for Flatpak
    if (tree == ui->treeFlatpak) {
        if (arg1 == tr("Installed runtimes")) {
            displayFilteredFP(installed_runtimes_fp);
        } else if (arg1 == tr("Installed apps")) {
            displayFilteredFP(installed_apps_fp);
        } else if (arg1 == tr("All apps")) {
            if (flatpaks_apps.isEmpty()) {
                flatpaks_apps = listFlatpaks(ui->comboRemote->currentText(), QStringLiteral("--app"));
            }
            displayFilteredFP(flatpaks_apps, true);
        } else if (arg1 == tr("All runtimes")) {
            if (flatpaks_runtimes.isEmpty()) {
                flatpaks_runtimes = listFlatpaks(ui->comboRemote->currentText(), QStringLiteral("--runtime"));
            }
            displayFilteredFP(flatpaks_runtimes, true);
        } else if (arg1 == tr("All available")) {
            int total = 0;
            for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
                ++total;
                (*it)->setData(0, Qt::UserRole, true);
                (*it)->setHidden(false);
            }
            ui->labelNumAppFP->setText(QString::number(total));
        } else if (arg1 == tr("All installed")) {
            displayFilteredFP(installed_apps_fp + installed_runtimes_fp);
        } else if (arg1 == tr("Not installed")) {
            found_items = tree->findItems(QStringLiteral("not installed"), Qt::MatchExactly, FlatCol::Status);
            QStringList new_list;
            for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
                if (found_items.contains(*it)) {
                    new_list << (*it)->text(FlatCol::FullName);
                }
            }
            displayFilteredFP(new_list);
        }
        setSearchFocus();
        findPackageOther();
        tree->blockSignals(false);
        return;
    }

    if (arg1 == tr("All packages")) {
        for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
            (*it)->setData(0, Qt::UserRole, true);
            (*it)->setHidden(false);
        }
        findPackageOther();
        setSearchFocus();
        tree->blockSignals(false);
        return;
    }

    if (arg1 == tr("Upgradable")) {
        found_items = tree->findItems(QStringLiteral("upgradable"), Qt::MatchExactly, TreeCol::Status);
    } else if (arg1 == tr("Installed")) {
        found_items = tree->findItems(QStringLiteral("installed"), Qt::MatchExactly, TreeCol::Status);
        found_items += tree->findItems(QStringLiteral("upgradable"), Qt::MatchExactly, TreeCol::Status);
    } else if (arg1 == tr("Not installed")) {
        found_items = tree->findItems(QStringLiteral("not installed"), Qt::MatchExactly, TreeCol::Status);
    }

    change_list.clear();
    ui->pushUninstall->setEnabled(false);
    ui->pushInstall->setEnabled(false);

    for (QTreeWidgetItemIterator it(tree); (*it) != nullptr; ++it) {
        (*it)->setCheckState(TreeCol::Check, Qt::Unchecked); // uncheck all items
        if (found_items.contains(*it)) {
            (*it)->setHidden(false);
            (*it)->setData(0, Qt::UserRole, true); // Displayed
        } else {
            (*it)->setHidden(true);
            (*it)->setData(0, Qt::UserRole, false);
        }
    }
    findPackageOther();
    setSearchFocus();
    tree->blockSignals(false);
}

void MainWindow::on_treeEnabled_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(TreeCol::Check) == Qt::Checked) {
        ui->treeEnabled->setCurrentItem(item);
    }
    buildChangeList(item);
}

void MainWindow::on_treeMXtest_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(TreeCol::Check) == Qt::Checked) {
        ui->treeMXtest->setCurrentItem(item);
    }
    buildChangeList(item);
}

void MainWindow::on_treeBackports_itemChanged(QTreeWidgetItem *item)
{
    if (item->checkState(TreeCol::Check) == Qt::Checked) {
        ui->treeBackports->setCurrentItem(item);
    }
    buildChangeList(item);
}

void MainWindow::on_treeFlatpak_itemChanged(QTreeWidgetItem *item)
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

    QString newapp;
    if (tree == ui->treeFlatpak) {
        if (change_list.isEmpty()
            && indexFilterFP.isEmpty()) { // remember the Flatpak combo location first time this is called
            indexFilterFP = ui->comboFilterFlatpak->currentText();
        }
        newapp = (item->text(FlatCol::FullName));
    } else {
        newapp = (item->text(TreeCol::Name));
    }

    if (item->checkState(0) == Qt::Checked) {
        ui->pushInstall->setEnabled(true);
        change_list.append(newapp);
    } else {
        change_list.removeOne(newapp);
    }

    if (tree != ui->treeFlatpak) {
        ui->pushUninstall->setEnabled(checkInstalled(change_list));
        ui->pushInstall->setText(checkUpgradable(change_list) ? tr("Upgrade") : tr("Install"));
    } else { // for Flatpaks allow selection only of installed or not installed items so one clicks
             // on an installed item only installed items should be displayed and the other way
             // round
        ui->pushInstall->setText(tr("Install"));
        if (item->text(FlatCol::Status) == QLatin1String("installed")) {
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
        if (change_list.isEmpty()) { // reset comboFilterFlatpak if nothing is selected
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
void MainWindow::on_pushForceUpdateEnabled_clicked()
{
    ui->searchBoxEnabled->clear();
    ui->comboFilterEnabled->setCurrentIndex(0);
    buildPackageLists(true);
}

void MainWindow::on_pushForceUpdateMX_clicked()
{
    ui->searchBoxMX->clear();
    ui->comboFilterMX->setCurrentIndex(0);
    buildPackageLists(true);
}

void MainWindow::on_pushForceUpdateBP_clicked()
{
    ui->searchBoxBP->clear();
    ui->comboFilterBP->setCurrentIndex(0);
    buildPackageLists(true);
}

// Hide/unhide lib/-dev packages
void MainWindow::on_checkHideLibs_toggled(bool checked)
{
    ui->checkHideLibsMX->setChecked(checked);
    ui->checkHideLibsBP->setChecked(checked);

    for (QTreeWidgetItemIterator it(ui->treeEnabled); (*it) != nullptr; ++it) {
        (*it)->setHidden(isFilteredName((*it)->text(TreeCol::Name)) && checked);
    }
    filterChanged(ui->comboFilterEnabled->currentText());
}

void MainWindow::on_pushUpgradeAll_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();

    auto found_items = ui->treeEnabled->findItems(QStringLiteral("upgradable"), Qt::MatchExactly, TreeCol::Status);

    QString names;
    for (QTreeWidgetItemIterator it(ui->treeEnabled); (*it) != nullptr; ++it) {
        if (found_items.contains(*it)) {
            names += (*it)->text(TreeCol::Name) + " ";
        }
    }

    if (install(names)) {
        buildPackageLists();
        QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(tree->parentWidget());
    } else {
        buildPackageLists();
        QMessageBox::critical(this, tr("Error"),
                              tr("Problem detected while installing, please inspect the console output."));
    }

    enableTabs(true);
}

// Pressing Enter or buttonEnter will do the same thing
void MainWindow::on_pushEnter_clicked()
{
    if (tree == ui->treeFlatpak
        && ui->lineEdit->text().isEmpty()) { // add "Y" as default response for flatpacks to work like apt-get
        cmd.write("y");
    }
    on_lineEdit_returnPressed();
}

void MainWindow::on_lineEdit_returnPressed()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    cmd.write(ui->lineEdit->text().toUtf8() + "\n");
    ui->outputBox->appendPlainText(ui->lineEdit->text() + "\n");
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

void MainWindow::on_pushCancel_clicked()
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

void MainWindow::on_checkHideLibsMX_clicked(bool checked)
{
    ui->checkHideLibs->setChecked(checked);
    ui->checkHideLibsBP->setChecked(checked);
}

void MainWindow::on_checkHideLibsBP_clicked(bool checked)
{
    ui->checkHideLibs->setChecked(checked);
    ui->checkHideLibsMX->setChecked(checked);
}

// on change flatpack remote
void MainWindow::on_comboRemote_activated()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    displayFlatpaks(true);
}

void MainWindow::on_pushUpgradeFP_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();
    setCursor(QCursor(Qt::BusyCursor));
    enableOutput();
    if (cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"socat SYSTEM:'flatpak update "
                + user.trimmed() + "',pty STDIO\"")) {
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

void MainWindow::on_pushRemotes_clicked()
{
    auto *dialog = new ManageRemotes(this);
    dialog->exec();
    if (dialog->isChanged()) {
        listFlatpakRemotes();
        displayFlatpaks(true);
    }
    if (!dialog->getInstallRef().isEmpty()) {
        showOutput();
        setCursor(QCursor(Qt::BusyCursor));
        enableOutput();
        if (cmd.run(
                "runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"socat SYSTEM:'flatpak install -y "
                + dialog->getUser() + "--from "
                + dialog->getInstallRef().replace(QLatin1String(":"), QLatin1String("\\:")) + "',stderr STDIO\"")) {
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

void MainWindow::on_comboUser_activated(int index)
{
    if (index == 0) {
        user = QStringLiteral("--system ");
    } else {
        user = QStringLiteral("--user ");
        static bool updated = false;
        if (!updated) {
            setCursor(QCursor(Qt::BusyCursor));
            enableOutput();
            cmd.run(QStringLiteral("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak "
                                   "--user remote-add --if-not-exists "
                                   "flathub https://flathub.org/repo/flathub.flatpakrepo\""));
            if (fp_ver >= VersionNumber(QStringLiteral("1.2.4"))) {
                enableOutput();
                cmd.run(QStringLiteral("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"flatpak "
                                       "update --appstream\""));
            }
            setCursor(QCursor(Qt::ArrowCursor));
            updated = true;
        }
    }
    listFlatpakRemotes();
    displayFlatpaks(true);
}

void MainWindow::on_treePopularApps_customContextMenuRequested(QPoint pos)
{
    auto *t_widget = qobject_cast<QTreeWidget *>(focusWidget());
    if (t_widget->currentItem()->parent() == nullptr) { // skip categories
        return;
    }
    auto *action = new QAction(QIcon::fromTheme(QStringLiteral("dialog-information")), tr("More &info..."), this);
    QMenu menu(this);
    menu.addAction(action);
    connect(action, &QAction::triggered, this, [this, t_widget] { displayPopularInfo(t_widget->currentItem(), 3); });
    menu.exec(ui->treePopularApps->mapToGlobal(pos));
    action->deleteLater();
}

// process keystrokes
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        on_pushCancel_clicked();
    }
}

void MainWindow::on_treePopularApps_itemChanged(QTreeWidgetItem *item)
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

void MainWindow::on_pushRemoveUnused_clicked()
{
    qDebug() << "+++" << __PRETTY_FUNCTION__ << "+++";
    showOutput();
    setCursor(QCursor(Qt::BusyCursor));
    enableOutput();
    // new version of flatpak takes a "-y" confirmation
    QString conf = QStringLiteral("-y ");
    if (fp_ver < VersionNumber(QStringLiteral("1.0.1"))) {
        conf = QString();
    }
    if (cmd.run("runuser -l $(logname) --whitelist-environment LANG -s /bin/bash -c \"socat SYSTEM:'flatpak uninstall "
                "--unused "
                + conf + user + "',pty STDIO\"")) {
        displayFlatpaks(true);
        setCursor(QCursor(Qt::ArrowCursor));
        QMessageBox::information(this, tr("Done"), tr("Processing finished successfully."));
        ui->tabWidget->blockSignals(true);
        ui->tabWidget->setCurrentWidget(ui->tabFlatpak);
        ui->tabWidget->blockSignals(false);
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
        QMessageBox::critical(this, tr("Error"),
                              tr("Problem detected during last operation, please inspect the console output."));
    }
    enableTabs(true);
}

void MainWindow::on_pushRemoveOrphan_clicked()
{
    QString names
        = cmd.getCmdOut(QStringLiteral(R"(apt-get --dry-run autoremove |grep -Po '^Remv \K[^ ]+' |tr '\n' ' ')"));
    QMessageBox::warning(this, tr("Warning"),
                         tr("Potentially dangerous operation.\nPlease make sure you check "
                            "carefully the list of packages to be removed."));
    showOutput();
    if (uninstall(names)) {
        if (!enabled_list.isEmpty()) { // update list if it already exists
            buildPackageLists();
        }
        refreshPopularApps();
        QMessageBox::information(this, tr("Success"), tr("Processing finished successfully."));
        ui->tabWidget->setCurrentWidget(tree->parentWidget());
    } else {
        if (!enabled_list.isEmpty()) { // update list if it already exists
            buildPackageLists();
        }
        refreshPopularApps();
        QMessageBox::critical(this, tr("Error"), tr("We encountered a problem uninstalling the program"));
    }
    enableTabs(true);
    ui->tabWidget->setCurrentIndex(Tab::EnabledRepos);
}

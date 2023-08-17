/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-repo-manager.
 *
 * mx-repo-manager is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-repo-manager is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-repo-manager.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QMetaEnum>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QProgressBar>
#include <QRadioButton>
#include <QScreen>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextEdit>

#include "about.h"
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    qDebug().noquote() << QApplication::applicationName() << "version:" << QApplication::applicationVersion();
    ui->setupUi(this);
    setConnections();
    setWindowFlags(Qt::Window);
    if (ui->pushOk->icon().isNull()) {
        ui->pushOk->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok"), QIcon(":/icons/dialog-ok.svg")));
    }

    if (ui->pushFastestMX->icon().isNull()) {
        ui->pushFastestMX->setIcon(QIcon::fromTheme(QStringLiteral("cursor-arrow"), QIcon(":/icons/cursor-arrow.svg")));
    }

    if (ui->pushFastestDebian->icon().isNull()) {
        ui->pushFastestDebian->setIcon(
            QIcon::fromTheme(QStringLiteral("cursor-arrow"), QIcon(":/icons/cursor-arrow.svg")));
    }

    shell = new Cmd(this);

    connect(shell, &Cmd::started, this, &MainWindow::procStart);
    connect(shell, &Cmd::finished, this, &MainWindow::procDone);

    setProgressBar();

    ui->pushOk->setDisabled(true);

    this->setWindowTitle(tr("MX Repo Manager"));
    ui->tabWidget->setCurrentWidget(ui->tabMX);
    refresh();
    displayMXRepos(readMXRepos(), QString());

    QSize size = this->size();
    if (settings.contains(QStringLiteral("geometry"))) {
        restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
        if (this->isMaximized()) { // add option to resize if maximized
            this->resize(size);
            centerWindow();
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if (sources_changed) {
        QProcess::startDetached("apt-get", {"update"});
    }
}

// refresh repo info
void MainWindow::refresh()
{
    getCurrentRepo();
    displayAllRepos(listAptFiles());
    ui->lineSearch->clear();
    ui->lineSearch->setFocus();
    ui->pushOk->setDisabled(true);
}

// replace default Debian repos
void MainWindow::replaceDebianRepos(QString url)
{
    // Apt source files that are present by default in MX and /etc/apt/sources.list
    // which might be the default in some Debian releases
    const QStringList files {"/etc/apt/sources.list.d/debian.list",
                             "/etc/apt/sources.list.d/debian-stable-updates.list", "/etc/apt/sources.list"};
    const QDir backupDir(QStringLiteral("/etc/apt/sources.list.d/backups"));

    // make backup folder
    if (!backupDir.exists()) {
        QDir().mkdir(QStringLiteral("/etc/apt/sources.list.d/backups"));
    }

    for (const QString &filePath : files) {
        if (!QFile::exists(filePath)) {
            continue;
        }
        QFileInfo fileInfo(filePath);
        const QString &backupFilePath = backupDir.absoluteFilePath(
            fileInfo.fileName() + "." + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
        if (!QFile::copy(filePath, backupFilePath)) {
            qWarning() << "Failed to backup" << filePath;
            continue;
        }
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Count not open file: " << file.fileName() << file.errorString();
            continue;
        }

        url.remove(QRegularExpression("/$"));
        QRegularExpression re {"(ftp|http[s]?://.*/debian)"};

        QString content;
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            QRegularExpressionMatch match = re.match(line);
            // Don't replace security line since it might not be available on the mirror
            if (!line.contains("security") && match.hasMatch()) {
                line.replace(match.captured(1), url);
            }
            content.append(line).append("\n");
        }
        file.close();
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Count not open file: " << file.fileName() << file.errorString();
            continue;
        }
        QTextStream out(&file);
        out << content;
        file.close();
    }
    sources_changed = true;
    QMessageBox::information(this, tr("Success"),
                             tr("Your new selection will take effect the next time sources are updated."));
}

// List available repos
QStringList MainWindow::readMXRepos()
{
    QFile file(QStringLiteral("/usr/share/mx-repo-list/repos.txt"));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Count not open file: " << file.fileName() << file.errorString();
    }

    QString file_content = file.readAll().trimmed();
    file.close();

    QStringList file_content_list = file_content.split(QStringLiteral("\n"));
    file_content_list.sort();

    QStringList list;
    std::remove_copy_if(file_content_list.begin(), file_content_list.end(), std::back_inserter(list),
                        [](const QString &line) { return line.startsWith(QLatin1String("#")); });
    extractUrls(list);
    repos = list;
    return list;
}

// List current repo
void MainWindow::getCurrentRepo()
{
    QFile file("/etc/apt/sources.list.d/mx.list");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file:" << file.fileName() << file.errorString();
        return;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.contains(QRegularExpression("^#?\\s?deb\\s+.*/repo[/]?"))) {
            QRegularExpression re {"//(.*?)/"};
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                current_repo = match.captured(1);
                break;
            }
        }
    }
    file.close();
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
        return Version::Bullseye;
    }
    bool ok = false;
    int ver = list.at(0).toInt(&ok);
    if (ok) {
        return ver;
    } else {
        QString verName = list.at(0).split(QStringLiteral("/")).at(0);
        if (verName == QLatin1String("bullseye")) {
            return Version::Bullseye;
        } else if (verName == QLatin1String("bookworm")) {
            return Version::Bookworm;
        } else {
            qCritical() << "Unknown Debian version:" << ver << "Assumes Bullseye";
            return Version::Bullseye;
        }
    }
}

QString MainWindow::getDebianVerName(int ver)
{
    QHash<int, QString> versionNames {{Version::Jessie, QStringLiteral("jessie")},
                                      {Version::Stretch, QStringLiteral("stretch")},
                                      {Version::Buster, QStringLiteral("buster")},
                                      {Version::Bullseye, QStringLiteral("bullseye")},
                                      {Version::Bookworm, QStringLiteral("bookworm")}};
    if (!versionNames.contains(ver)) {
        qWarning() << "Error: Invalid Debian version, assumes Bullseye";
        return "bullseye";
    }
    return versionNames.value(ver);
}

// display available repos
void MainWindow::displayMXRepos(const QStringList &repos, const QString &filter)
{
    ui->listWidget->clear();
    for (const QString &repo : repos) {
        if (!filter.isEmpty() && !repo.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }
        QString country = repo.section(QStringLiteral("-"), 0, 0).trimmed().section(QStringLiteral(","), 0, 0);
        auto *item = new QListWidgetItem(ui->listWidget);
        auto *radio = new QRadioButton(repo);
        radio->setIcon(getFlag(country));
        ui->listWidget->setItemWidget(item, radio);
        if (repo.contains(current_repo)) {
            radio->setChecked(true);
            ui->listWidget->scrollToItem(item);
        }
        connect(radio, &QRadioButton::clicked, ui->pushOk, &QPushButton::setEnabled);
    }
}

void MainWindow::displayAllRepos(const QFileInfoList &apt_files)
{
    ui->treeWidget->clear();
    ui->treeWidgetDeb->clear();
    ui->treeWidget->blockSignals(true);
    ui->treeWidgetDeb->blockSignals(true);

    const QStringList columnNames {tr("Lists"), tr("Sources (checked sources are enabled)")};
    ui->treeWidget->setHeaderLabels(columnNames);
    ui->treeWidgetDeb->setHeaderLabels(columnNames);

    QTreeWidgetItem *topLevelItem = nullptr;
    QTreeWidgetItem *topLevelItemDeb = nullptr;

    for (const QFileInfo &file_info : apt_files) {
        QString file_name = file_info.fileName();
        QString file = file_info.absoluteFilePath();
        topLevelItem = new QTreeWidgetItem;
        topLevelItem->setText(0, file_name);
        topLevelItemDeb = new QTreeWidgetItem;
        topLevelItemDeb->setText(0, file_name);
        ui->treeWidget->addTopLevelItem(topLevelItem);
        if (file_name.contains(QLatin1String("debian")) || file_name == "sources.list") {
            ui->treeWidgetDeb->addTopLevelItem(topLevelItemDeb);
        }
        // topLevelItem look
        topLevelItem->setForeground(0, QBrush(Qt::darkGreen));
        topLevelItemDeb->setForeground(0, QBrush(Qt::darkGreen));
        // load file entries
        const QStringList entries = loadAptFile(file);

        for (const QString &item : entries) {
            // add entries as childItem to treeWidget
            auto *childItem = new QTreeWidgetItem(topLevelItem);
            auto *childItemDeb = new QTreeWidgetItem(topLevelItemDeb);
            childItem->setText(1, item);
            childItemDeb->setText(1, item);
            // add checkboxes
            childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
            childItemDeb->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
            if (item.startsWith(QLatin1String("#"))) {
                childItem->setCheckState(1, Qt::Unchecked);
                childItemDeb->setCheckState(1, Qt::Unchecked);
            } else {
                childItem->setCheckState(1, Qt::Checked);
                childItemDeb->setCheckState(1, Qt::Checked);
            }
        }
    }
    for (int i = 0; i < ui->treeWidget->columnCount(); i++) {
        ui->treeWidget->resizeColumnToContents(i);
    }

    for (int i = 0; i < ui->treeWidgetDeb->columnCount(); i++) {
        ui->treeWidgetDeb->resizeColumnToContents(i);
    }

    ui->treeWidget->expandAll();
    ui->treeWidgetDeb->expandAll();
    ui->treeWidget->blockSignals(false);
    ui->treeWidgetDeb->blockSignals(false);
}

QStringList MainWindow::loadAptFile(const QString &file)
{
    QFile aptFile(file);
    if (!aptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file: " << aptFile << aptFile.errorString();
        return {};
    }

    QStringList entries;
    QTextStream in(&aptFile);
    QRegularExpression re("^#*\\s*deb ");
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (re.match(line).hasMatch()) {
            entries.append(line);
        }
    }

    aptFile.close();
    return entries;
}

void MainWindow::cancelOperation()
{
    shell->close();
    procDone();
}

void MainWindow::centerWindow()
{
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

void MainWindow::closeEvent(QCloseEvent * /*unused*/)
{
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
}

// displays the current repo by selecting the item
void MainWindow::displaySelected(const QString &repo)
{
    for (int row = 0; row < ui->listWidget->count(); ++row) {
        auto *radio = qobject_cast<QRadioButton *>(ui->listWidget->itemWidget(ui->listWidget->item(row)));
        if (radio->text().contains(repo)) {
            radio->setChecked(true);
            ui->listWidget->scrollToItem(ui->listWidget->item(row));
        }
    }
}

// extract the URLs from the list of repos that contain country names and description
void MainWindow::extractUrls(const QStringList &repos)
{
    for (const QString &line : repos) {
        QStringList linelist = line.split(QStringLiteral("-"));
        if (linelist.size() > 1) {
            linelist.pop_front();
            QString joinedLine = linelist.join(QStringLiteral("-")).trimmed();
            if (!joinedLine.isEmpty()) {
                listMXurls += joinedLine + " ";
            }
        }
    }
}

// set the selected repo
void MainWindow::setSelected()
{
    QString url;
    for (int row = 0; row < ui->listWidget->count(); ++row) {
        auto *radio = qobject_cast<QRadioButton *>(ui->listWidget->itemWidget(ui->listWidget->item(row)));
        if (radio->isChecked()) {
            url = radio->text().section(QStringLiteral(" - "), 1, 1).trimmed();
            replaceRepos(url);
        }
    }
}

void MainWindow::procTime()
{
    bar->setValue((bar->value() + 1) % bar->maximum());
    //    qApp->processEvents();
}

void MainWindow::procStart()
{
    bar->setValue(0);
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    timer.start(100ms);
    connect(&timer, &QTimer::timeout, this, &MainWindow::procTime);
}

void MainWindow::procDone()
{
    bar->setValue(bar->maximum());
    timer.stop();
    timer.disconnect();
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

// replaces the lines in the APT file in mx.list file
void MainWindow::replaceRepos(const QString &url)
{
    QString mx_file {QStringLiteral("/etc/apt/sources.list.d/mx.list")};
    QString repo_line_mx = "deb " + url + "/mx/repo/ ";
    QString test_line_mx = "deb " + url + "/mx/testrepo/ ";
    QString cmd_mx = QStringLiteral("sed -i 's;deb.*/repo/ ;%1;' %2 && ").arg(repo_line_mx, mx_file)
                     + QStringLiteral("sed -i 's;deb.*/testrepo/ ;%1;' %2").arg(test_line_mx, mx_file);
    shell->run(cmd_mx) ? QMessageBox::information(
        this, tr("Success"), tr("Your new selection will take effect the next time sources are updated."))
                       : QMessageBox::critical(this, tr("Error"), tr("Could not change the repo."));
    sources_changed = true;
}

void MainWindow::setConnections()
{
    connect(ui->lineSearch, &QLineEdit::textChanged, this, &MainWindow::lineSearch_textChanged);
    connect(ui->pb_restoreSources, &QPushButton::clicked, this, &MainWindow::pb_restoreSources_clicked);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushCancel, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->pushFastestDebian, &QPushButton::clicked, this, &MainWindow::pushFastestDebian_clicked);
    connect(ui->pushFastestMX, &QPushButton::clicked, this, &MainWindow::pushFastestMX_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushOk, &QPushButton::clicked, this, &MainWindow::pushOk_clicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &MainWindow::treeWidget_itemChanged);
    connect(ui->treeWidgetDeb, &QTreeWidget::itemChanged, this, &MainWindow::treeWidget_itemChanged);
}

void MainWindow::setProgressBar()
{
    progress = new QProgressDialog(this);
    bar = new QProgressBar(progress);
    progCancel = new QPushButton(tr("Cancel"));
    connect(progCancel, &QPushButton::clicked, this, &MainWindow::cancelOperation);
    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                             | Qt::WindowStaysOnTopHint);
    progress->setCancelButton(progCancel);
    progress->setLabelText(tr("Please wait..."));
    progress->setAutoClose(false);
    progress->setBar(bar);
    bar->setTextVisible(false);
    progress->reset();
}

QFileInfoList MainWindow::listAptFiles()
{
    const QDir apt_dir(QStringLiteral("/etc/apt/sources.list.d"));
    QFileInfoList list {apt_dir.entryInfoList(QStringList("*.list"))};
    QFile file(QStringLiteral("/etc/apt/sources.list"));
    if (file.size() != 0) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QRegularExpression re(QStringLiteral("^#*\\s*deb"));
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (re.match(line).hasMatch()) {
                    list << file;
                    break;
                }
            }
            file.close();
        }
    }
    return list;
}

// Submit button clicked
void MainWindow::pushOk_clicked()
{
    if (!queued_changes.empty()) {
        for (const QStringList &changes : qAsConst(queued_changes)) {
            QString text;
            QString new_text;
            QString file_name;
            text = QRegularExpression::escape(changes.at(0));
            new_text = changes.at(1);
            file_name = changes.at(2);
            QString cmd = QStringLiteral("sed -i 's;%1;%2;g' %3").arg(text, new_text, file_name);
            shell->run(cmd);
            sources_changed = true;
        }
        queued_changes.clear();
    }
    setSelected();
    refresh();
}

// About button clicked
void MainWindow::pushAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(this->windowTitle()),
        "<p align=\"center\"><b><h2>" + this->windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Program for choosing the default APT repository")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-repo-manager/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

// Help button clicked
void MainWindow::pushHelp_clicked()
{
    QLocale locale;
    QString lang = locale.bcp47Name();
    QString url = QStringLiteral("/usr/share/doc/mx-repo-manager/mx-repo-manager.html");

    if (lang.startsWith(QLatin1String("fr"))) {
        url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-mx-gestionnaire-de-d%C3%A9p%C3%B4ts");
    }

    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    ui->pushOk->setEnabled(true);
    ui->treeWidget->blockSignals(true);
    ui->treeWidgetDeb->blockSignals(true);
    if (item->text(column).contains(QLatin1String("/mx/testrepo/")) && item->checkState(column) == Qt::Checked) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("You have selected MX Test Repo. It's not recommended to leave it enabled or to "
                                "upgrade all the packages from it.")
                                 + "\n\n"
                                 + tr("A safer option is to install packages individually with MX Package "
                                      "Installer."));
    }

    QFile file;
    QString new_text;
    const QString file_name {item->parent()->text(0)};
    const QString text {item->text(column)};
    QStringList changes;
    if (file_name == QLatin1String("sources.list")) {
        file.setFileName("/etc/apt/" + file_name);
    } else {
        file.setFileName("/etc/apt/sources.list.d/" + file_name);
    }

    if (item->checkState(column) == Qt::Checked) {
        new_text = text;
        new_text.remove(QRegularExpression(QStringLiteral("#\\s*")));
        item->setText(column, new_text);
    } else {
        new_text = "# " + text;
        item->setText(column, new_text);
    }
    changes << text << new_text << file.fileName();
    queued_changes << changes;
    ui->treeWidgetDeb->blockSignals(false);
    ui->treeWidget->blockSignals(false);
}

void MainWindow::tabWidget_currentChanged()
{
    if (ui->tabWidget->currentWidget() == ui->tabMX) {
        ui->label->setText(tr("Select the APT repository that you want to use:"));
    } else {
        ui->label->setText(tr("Select the APT repository and sources that you want to use:"));
    }
}

// Transform "country" name to 2-3 letter ISO 3166 country code and provide the QIcon for it
QIcon MainWindow::getFlag(QString country)
{
    QMetaObject metaObject = QLocale::staticMetaObject;
    QMetaEnum metaEnum = metaObject.enumerator(metaObject.indexOfEnumerator("Country"));
    // fix flag of the Netherlands: QLocale::Netherlands
    if (country == QLatin1String("The Netherlands")) {
        country = QStringLiteral("Netherlands");
    }
    // fix flag of the United States of America: QLocale::UnitedStates
    if (country == QLatin1String("USA")) {
        country = QStringLiteral("UnitedStates");
    }
    if (country == QLatin1String("Anycast") || country == QLatin1String("Any") || country == QLatin1String("World")) {
        return QIcon("/usr/share/flags-common/any.png");
    }

    // QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Country>(); -- not in older Qt versions
    int index = metaEnum.keyToValue(country.remove(QStringLiteral(" ")).toUtf8());
    QList<QLocale> locales
        = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, static_cast<QLocale::Country>(index));
    // qDebug() << "etFlag county: " << country << " locales: " << locales;
    if (locales.length() > 0) {
        QString short_name = locales.at(0).name().section(QStringLiteral("_"), 1, 1).toLower();
        return QIcon("/usr/share/flags-common/" + short_name + ".png");
    }
    return {};
}

// detect fastest Debian repo
void MainWindow::pushFastestDebian_clicked()
{
    progress->show();
    QTemporaryFile tmpfile;
    if (!tmpfile.open()) {
        qDebug() << "Could not create temp file";
        return;
    }

    QString ver_name {getDebianVerName(getDebianVerNum())};
    if (ver_name == QLatin1String("buster") || ver_name == QLatin1String("bullseye")) {
        ver_name = QString(); // netselect-apt doesn't like name buster/bullseye for some reason,
                              // maybe it expects "stable"
    }

    bool success = shell->run("netselect-apt " + ver_name + " -o " + tmpfile.fileName(), false);
    progress->hide();

    if (!success) {
        QMessageBox::critical(this, tr("Error"), tr("netselect-apt could not detect fastest repo."));
        return;
    }
    QString repo = shell->getCmdOut("set -o pipefail; grep -m1 '^deb ' " + tmpfile.fileName() + "| cut -d' ' -f2");
    this->blockSignals(false);

    if (checkRepo(repo)) {
        replaceDebianRepos(repo);
        refresh();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not detect fastest repo."));
    }
}

// detect and select the fastest MX repo
void MainWindow::pushFastestMX_clicked()
{
    progress->show();
    QString out;
    bool success = shell->run(
        "set -o pipefail; netselect -D -I " + listMXurls + " |tr -s ' ' |sed 's/^ //' |cut -d' ' -f2", &out);
    qDebug() << listMXurls;
    qDebug() << "FASTEST " << success << out;
    progress->hide();
    if (success && !out.isEmpty()) {
        displaySelected(out);
        pushOk_clicked();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not detect fastest repo."));
    }
}

void MainWindow::lineSearch_textChanged(const QString &arg1)
{
    displayMXRepos(repos, arg1);
}

void MainWindow::pb_restoreSources_clicked()
{
    // check if running on antiX/MX
    if (!QFileInfo::exists(QStringLiteral("/etc/antix-version"))
        && !QFileInfo::exists(QStringLiteral("/etc/mx-version"))) {
        QMessageBox::critical(this, tr("Error"), tr("Can't figure out if this app is running on antiX or MX"));
        return;
    }

    bool ok = true;
    int mx_version
        = shell->getCmdOut(QStringLiteral("grep -oP '(?<=DISTRIB_RELEASE=).*' /etc/lsb-release")).leftRef(2).toInt(&ok);
    if (!ok || mx_version < 18) {
        QMessageBox::critical(this, tr("Error"),
                              tr("MX version not detected or out of range: ") + QString::number(mx_version));
        return;
    }

    QTemporaryDir tmpdir;
    if (!tmpdir.isValid()) {
        qDebug() << "Could not create temp dir";
        return;
    }
    QDir::setCurrent(tmpdir.path());

    // download source files from
    const QString url("https://codeload.github.com/MX-Linux/mx-sources/zip/mx" + QString::number(mx_version));
    QFileInfo fi(url);
    QFile tofile(tmpdir.path() + "/" + fi.fileName() + ".zip");
    if (!downloadFile(url, &tofile)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not download original APT files."));
        return;
    }
    // extract master.zip to temp folder
    QString cmd = ("unzip -q " + tofile.fileName() + " -d %1/").arg(tmpdir.path());
    if (!tofile.exists() || !shell->run(cmd)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not unzip downloaded file."));
        return;
    }
    // move the sources list files from the temporary directory to /etc/apt/sources.list.d/
    cmd = ("mv -b %1/mx-sources-mx" + QString::number(mx_version) + "/*.list /etc/apt/sources.list.d/")
              .arg(tmpdir.path());
    shell->run(cmd);

    // for 64-bit OS check if user wants AHS repo
    if (mx_version >= 19 && shell->getCmdOut(QStringLiteral("uname -m"), true) == QLatin1String("x86_64")) {
        if (QMessageBox::Yes
            == QMessageBox::question(this, tr("Enabling AHS"), tr("Do you use AHS (Advanced Hardware Stack) repo?"))) {
            shell->run(QStringLiteral("sed -i '/^\\s*#*\\s*deb.*ahs\\s*/s/^#*\\s*//' /etc/apt/sources.list.d/mx.list"),
                       true);
        }
    }
    refresh();
    displayMXRepos(readMXRepos(), QString());
    QMessageBox::information(this, tr("Success"),
                             tr("Original APT sources have been restored to the release status. User added source "
                                "files in /etc/apt/sources.list.d/ have not been touched.")
                                 + "\n\n"
                                 + tr("Your new selection will take effect the next time sources are updated."));
    sources_changed = true;
}

bool MainWindow::checkRepo(const QString &repo)
{
    QNetworkProxyQuery query {QUrl(repo)};
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
    QNetworkAccessManager manager;
    if (!proxies.isEmpty()) {
        manager.setProxy(proxies.first());
    }

    QNetworkRequest request;
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + "/"
                                           + QApplication::applicationVersion().toUtf8() + " (linux-gnu)");
    request.setUrl(QUrl(repo));
    QNetworkReply *reply = manager.head(request);

    auto error {QNetworkReply::NoError};
    QEventLoop loop;
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [&error](QNetworkReply::NetworkError err) { error = err; }); // errorOccured only in Qt >= 5.15
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5s, &loop, [&loop, &error]() {
        error = QNetworkReply::TimeoutError;
        loop.quit();
    }); // manager.setTransferTimeout(time) // only in Qt >= 5.15
    loop.exec();
    if (error == QNetworkReply::NoError) {
        return true;
    }
    qDebug() << "No reponse from repo:" << reply->url() << error;
    return false;
}

bool MainWindow::downloadFile(const QString &url, QFile *file)
{
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file:" << file->fileName() << file->errorString();
        return false;
    }

    QNetworkProxyQuery query {QUrl(url)};
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
    QNetworkAccessManager manager;
    if (!proxies.isEmpty()) {
        manager.setProxy(proxies.first());
    }

    QNetworkRequest request;
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + "/"
                                           + QApplication::applicationVersion().toUtf8() + " (linux-gnu)");
    request.setUrl(QUrl(url));

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;

    bool success = true;
    connect(reply, &QNetworkReply::readyRead, this,
            [&reply, file, &success]() { success = file->write(reply->readAll()) > 0; });
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (!success) {
        QMessageBox::warning(this, tr("Error"),
                             tr("There was an error writing file: %1. Please check if you have "
                                "enough free space on your drive")
                                 .arg(file->fileName()));
        exit(EXIT_FAILURE);
    }

    file->close();
    return (reply->error() == QNetworkReply::NoError);
}

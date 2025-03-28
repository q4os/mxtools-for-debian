/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2015-2025 MX Authors
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
    initializeIcons();
    shell = new Cmd(this);

    connect(shell, &Cmd::started, this, &MainWindow::procStart);
    connect(shell, &Cmd::done, this, &MainWindow::procDone);

    setProgressBar();

    ui->pushRestoreSources->hide(); //permanently hide restore original repositories button
    ui->pushOk->setDisabled(true);

    setWindowTitle(tr("MX Repo Manager"));
    ui->tabWidget->setCurrentWidget(ui->tabMX);
    refresh(true);

    restoreWindowGeometry();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (sources_changed) {
        Cmd().runAsRoot("apt-get update&");
    }
}

void MainWindow::initializeIcons()
{
    setIconIfNull(ui->pushOk, "dialog-ok", ":/icons/dialog-ok.svg");
    setIconIfNull(ui->pushFastestMX, "cursor-arrow", ":/icons/cursor-arrow.svg");
    setIconIfNull(ui->pushFastestDebian, "cursor-arrow", ":/icons/cursor-arrow.svg");
}

void MainWindow::setIconIfNull(QPushButton *button, const QString &themeIcon, const QString &fallbackIcon)
{
    if (button->icon().isNull()) {
        button->setIcon(QIcon::fromTheme(themeIcon, QIcon(fallbackIcon)));
    }
}

void MainWindow::restoreWindowGeometry()
{
    QSize size = this->size();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        if (isMaximized()) {
            resize(size);
            centerWindow();
        }
    }
}

void MainWindow::refresh(bool force)
{
    getCurrentRepo(force);
    displayMXRepos(repos, {});
    displayAllRepos(listAptFiles());
    ui->lineSearch->clear();
    ui->lineSearch->setFocus();
    ui->pushOk->setDisabled(true);
    radioSelectionChanged = false;
}

void MainWindow::replaceDebianRepos(const QString &url)
{
    const QStringList files {"/etc/apt/sources.list.d/debian.list", "/etc/apt/sources.list.d/debian.sources",
                             "/etc/apt/sources.list.d/debian-stable-updates.list",
                             "/etc/apt/sources.list.d/debian-stable-updates.sources", "/etc/apt/sources.list"};
    const QDir backupDir("/etc/apt/sources.list.d/backups");

    if (!backupDir.exists() && !Cmd().runAsRoot("mkdir -p " + backupDir.path())) {
        qWarning() << "Failed to create backup directory:" << backupDir.path();
        return;
    }

    for (const QString &filePath : files) {
        if (!QFile::exists(filePath)) {
            continue;
        }

        QFileInfo fileInfo(filePath);
        const QString backupFilePath = backupDir.absoluteFilePath(
            fileInfo.fileName() + "." + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

        if (!Cmd().runAsRoot("cp " + filePath + " " + backupFilePath)) {
            qWarning() << "Failed to backup" << filePath;
            continue;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open file:" << file.fileName() << file.errorString();
            continue;
        }

        QString content;
        QTextStream in(&file);
        QRegularExpression re {"(ftp|http[s]?://.*/debian)"};
        QString trimmedUrl = url.trimmed().remove(QRegularExpression("/$"));

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            // Don't replace security line since it might not be available on the mirror
            if (!line.contains("security")) {
                line.replace(re, trimmedUrl);
            }
            content.append(line + '\n');
        }
        file.close();

        if (!writeUpdatedFile(filePath, content)) {
            continue;
        }
    }
    sources_changed = true;
    QMessageBox::information(this, tr("Success"),
                             tr("Your new selection will take effect the next time sources are updated."));
}

bool MainWindow::writeUpdatedFile(const QString &filePath, const QString &content)
{
    QTemporaryFile tmpFile(QDir::tempPath() + "/XXXXXX.list");
    if (!tmpFile.open()) {
        qWarning() << "Could not open temporary file:" << tmpFile.fileName() << tmpFile.errorString();
        return false;
    }

    {
        QTextStream out(&tmpFile);
        out << content;
    } // Ensuring the QTextStream is flushed and closed before moving the file

    QString cmd = QStringLiteral("mv -f %1 %2 && chown root: %2 && chmod +r %2").arg(tmpFile.fileName(), filePath);
    if (!Cmd().runAsRoot(cmd)) {
        qWarning() << "Failed to replace the file and update permissions for" << filePath;
        return false;
    }
    return true;
}

QStringList MainWindow::readMXRepos()
{
    QTemporaryDir tmpdir;
    QFile file(tmpdir.path() + "/repos.txt");
    const QString remoteUrl = "https://raw.githubusercontent.com/MX-Linux/mx-repo-list/master/repos.txt";
    const QString localPath = "/usr/share/mx-repo-list/repos.txt";

    if (!downloadFile(remoteUrl, &file, 3s)) {
        qWarning() << "Could not download 'repos.txt' from GitHub, falling back to local file.";
        file.setFileName(localPath);
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file:" << file.fileName() << file.errorString();
        return {};
    }

    QTextStream in(&file);
    QStringList fileContentList = in.readAll().trimmed().split('\n');
    file.close();

    fileContentList.sort(Qt::CaseInsensitive);

    QStringList list;
    std::copy_if(fileContentList.cbegin(), fileContentList.cend(), std::back_inserter(list),
                 [](const QString &line) { return !line.startsWith('#'); });

    extractUrls(list);
    repos = list;
    return list;
}

void MainWindow::getCurrentRepo(bool force)
{
    QFile file("/etc/apt/sources.list.d/mx.list");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.setFileName("/etc/apt/sources.list.d/mx.sources");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open either mx.list or mx.sources file:" << file.errorString();
            return;
        }
    }

    QTextStream in(&file);

    QRegularExpression repoRegex {R"(^#?\s?deb\s+.*/repo[/]?)"};
    QRegularExpression urlRegex {"//(.*?)/"};
    QRegularExpression sourcesRegex {R"(URIs:\s*(.*?)(?:\s|$))"};

    QString line;
    while (in.readLineInto(&line)) {
        line = line.trimmed();

        if (file.fileName().endsWith("mx.sources")) {
            QRegularExpressionMatch match = sourcesRegex.match(line);
            if (match.hasMatch()) {
                current_repo = urlRegex.match(match.captured(1)).captured(1);
                break;
            }
        } else {
            if (repoRegex.match(line).hasMatch()) {
                QRegularExpressionMatch match = urlRegex.match(line);
                if (match.hasMatch()) {
                    current_repo = match.captured(1);
                    break;
                }
            }
        }
    }
    file.close();

    if (force || repos.isEmpty()) {
        readMXRepos();
    }

    bool repoFound = std::find_if(repos.cbegin(), repos.cend(),
                                  [this](const QString &item) { return item.contains(current_repo); })
                     != repos.cend();

    if (!repoFound) {
        qDebug() << "Current repo not found in the up-to-date list of repos, it might have been removed as "
                    "non-working; selecting mxrepo.com as default";
        const QString defaultRepo = "http://mxrepo.com";
        if (replaceRepos(defaultRepo, true)) {
            current_repo = defaultRepo;
        }
    }
}

int MainWindow::getDebianVerNum()
{
    QFile file("/etc/debian_version");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open /etc/debian_version:" << file.errorString() << ". Defaulting to Bullseye.";
        return Version::Bullseye;
    }

    QTextStream in(&file);
    QString line = in.readLine().trimmed();
    file.close();

    static const QHash<QString, Version> versionMap {
        {"bullseye", Version::Bullseye}, {"bookworm", Version::Bookworm}, {"trixie", Version::Trixie}};

    for (auto it = versionMap.cbegin(); it != versionMap.cend(); ++it) {
        if (line.contains(it.key(), Qt::CaseInsensitive)) {
            return it.value();
        }
    }

    qCritical() << "Unrecognized or unparseable Debian version:" << line << ". Defaulting to Bullseye.";
    return Version::Bullseye;
}

QString MainWindow::getDebianVerName(int ver)
{
    static const QHash<int, QString> versionNames {
        {Version::Bullseye, "bullseye"}, {Version::Bookworm, "bookworm"}, {Version::Trixie, "trixie"}};

    if (versionNames.contains(ver)) {
        return versionNames.value(ver);
    } else {
        qWarning() << "Error: Invalid Debian version. Defaulting to Bullseye";
        return "bullseye";
    }
}

void MainWindow::displayMXRepos(const QStringList &repos, const QString &filter)
{
    ui->listWidget->clear();
    for (const QString &repo : repos) {
        if (!filter.isEmpty() && !repo.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }
        QString country = repo.section('-', 0, 0).trimmed().section(',', 0, 0);
        auto *item = new QListWidgetItem(ui->listWidget);
        auto *radio = new QRadioButton(repo);
        radio->setIcon(getFlag(country));
        ui->listWidget->setItemWidget(item, radio);
        if (repo.contains(current_repo)) {
            radio->setChecked(true);
            ui->listWidget->scrollToItem(item);
        }
        connect(radio, &QRadioButton::clicked, this, [this]() {
            radioSelectionChanged = true;
            ui->pushOk->setEnabled(true);
        });
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

    auto processEntries = [](QTreeWidgetItem *parentItem, const auto &entries) {
        for (const auto &item : entries) {
            auto *childItem = new QTreeWidgetItem(parentItem, {QString(), item.first});
            childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable);
            childItem->setCheckState(1, item.second ? Qt::Checked : Qt::Unchecked);
        }
    };

    for (const QFileInfo &file_info : apt_files) {
        QString file_name = file_info.fileName();
        QString file = file_info.absoluteFilePath();
        auto *topLevelItem = new QTreeWidgetItem(ui->treeWidget, {file_name});
        topLevelItem->setForeground(0, QBrush(Qt::darkGreen));
        QTreeWidgetItem *topLevelItemDeb = nullptr;

        if (file_name.contains("debian") || file_name == "sources.list") {
            topLevelItemDeb = new QTreeWidgetItem(ui->treeWidgetDeb, {file_name});
            topLevelItemDeb->setForeground(0, QBrush(Qt::darkGreen));
        }

        auto entries = loadAptFile(file);
        processEntries(topLevelItem, entries);
        if (topLevelItemDeb) {
            processEntries(topLevelItemDeb, entries);
        }
    }

    for (auto *treeWidget : {ui->treeWidget, ui->treeWidgetDeb}) {
        for (int i = 0; i < treeWidget->columnCount(); ++i) {
            treeWidget->resizeColumnToContents(i);
        }
        treeWidget->expandAll();
        treeWidget->blockSignals(false);
    }
}

QVector<QPair<QString, bool>> MainWindow::loadAptFile(const QString &file)
{
    QFile aptFile(file);
    if (!aptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file: " << file << " - " << aptFile.errorString();
        return {};
    }

    QVector<QPair<QString, bool>> entries;
    QTextStream in(&aptFile);
    bool isSources = QFileInfo(file).suffix() == "sources";
    if (isSources) {
        QString block;
        QRegularExpression enabledRegex("Enabled:\\s*no", QRegularExpression::CaseInsensitiveOption);

        auto processBlock = [&entries, &enabledRegex](const QString &block) {
            if (!block.isEmpty()) {
                bool blockEnabled = !block.contains(enabledRegex);
                entries.append({block.trimmed(), blockEnabled});
            }
        };

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.trimmed().isEmpty()) {
                processBlock(block);
                block.clear();
            } else {
                block.append(line + "\n");
            }
        }
        processBlock(block); // Process final block
    } else {
        QRegularExpression re(R"(^\s*(?:#)?\s*deb\s+)");
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (re.match(line).hasMatch()) {
                bool enabled = !line.startsWith('#');
                entries.append({line, enabled});
            }
        }
    }

    return entries;
}

void MainWindow::cancelOperation()
{
    auto processId = shell->processId();
    if (processId != 0) {
        Cmd cmd;
        cmd.runAsRoot(QString("kill %1").arg(processId));
        if (!shell->waitForFinished(1000)) {
            cmd.runAsRoot(QString("kill -9 %1").arg(processId));
        }
    }
    procDone();
}

void MainWindow::centerWindow()
{
    auto screenGeometry = QApplication::primaryScreen()->geometry();
    auto x = (screenGeometry.width() - width()) / 2;
    auto y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::closeEvent([[maybe_unused]] QCloseEvent *event)
{
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::displaySelected(const QString &repo)
{
    for (int row = 0; row < ui->listWidget->count(); ++row) {
        auto *item = ui->listWidget->item(row);
        auto *radio = qobject_cast<QRadioButton *>(ui->listWidget->itemWidget(item));
        if (radio && radio->text().contains(repo, Qt::CaseInsensitive)) {
            radio->setChecked(true);
            ui->listWidget->scrollToItem(item);
            break;
        }
    }
}

void MainWindow::extractUrls(const QStringList &repos)
{
    QStringList extractedUrls;
    extractedUrls.reserve(repos.size());
    for (const QString &line : repos) {
        QStringList linelist = line.split('-');
        if (linelist.size() > 1) {
            linelist.pop_front();
            QString joinedLine = linelist.join('-').trimmed();
            if (!joinedLine.isEmpty()) {
                extractedUrls.append(joinedLine);
            }
        }
    }
    listMXurls = extractedUrls.join(' ');
}

void MainWindow::setSelected()
{
    if (!radioSelectionChanged) {
        return;
    }
    for (int row = 0; row < ui->listWidget->count(); ++row) {
        auto *item = ui->listWidget->item(row);
        if (auto *radio = qobject_cast<QRadioButton *>(ui->listWidget->itemWidget(item)); radio && radio->isChecked()) {
            QString url = radio->text().section(" - ", 1, 1).trimmed();
            replaceRepos(url);
            break;
        }
    }
}

void MainWindow::procTime()
{
    bar->setValue((bar->value() + 1) % bar->maximum());
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

bool MainWindow::replaceRepos(const QString &url, bool quiet)
{
    const QString mx_list {"/etc/apt/sources.list.d/mx.list"};
    const QString mx_sources {"/etc/apt/sources.list.d/mx.sources"};

    // Try mx.list first
    const QString cmd_mx_list
        = QString(
              R"(sed -i -E 's=(deb|deb-src)[[:space:]]+(\[.*\][[:space:]]+)?\S*(/mx/([.]?/)*repo/?|/mx/([.]?/)*testrepo/?)=\1 \2%1\3=g; s/[[:space:]]{2,}/ /g; s/[[:space:]]+$//g' %2)")
              .arg(url, mx_list);

    // Try mx.sources if mx.list doesn't exist or fails
    const QString cmd_mx_sources
        = QString(
              R"(sed -i -E 's=URIs:[[:space:]]*\S*=URIs: %1=; s/[[:space:]]{2,}/ /g; s/[[:space:]]+$//' %2)")
              .arg(url, mx_sources);

    sources_changed = true;
    bool result = false;

    if (QFile::exists(mx_list)) {
        result = shell->runAsRoot(cmd_mx_list);
    }
    if (!result && QFile::exists(mx_sources)) {
        result = shell->runAsRoot(cmd_mx_sources);
    }

    if (quiet) {
        return result;
    } else {
        if (result) {
            QMessageBox::information(this, tr("Success"),
                                     tr("Your new selection will take effect the next time sources are updated."));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Could not change the repo."));
        }
        return result;
    }
}

void MainWindow::setConnections()
{
    connect(ui->lineSearch, &QLineEdit::textChanged, this, &MainWindow::lineSearch_textChanged);
    connect(ui->pushRestoreSources, &QPushButton::clicked, this, &MainWindow::pushRestoreSources_clicked);
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
    QFileInfoList list;
    const QDir apt_dir("/etc/apt/sources.list.d");
    list = apt_dir.entryInfoList(QStringList({"*.list", "*.sources"}), QDir::Files | QDir::Readable, QDir::Name);

    QFile file("/etc/apt/sources.list");
    if (file.exists() && file.size() > 0 && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QRegularExpression re(R"(^\s*deb)");
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (re.match(line).hasMatch()) {
                list << QFileInfo(file);
                break;
            }
        }
        file.close();
    }

    return list;
}

void MainWindow::pushOk_clicked()
{
    if (!queued_changes.isEmpty()) {
        for (const QStringList &changes : std::as_const(queued_changes)) {
            const QString &text = changes.at(0);
            const QString &new_text = changes.at(1);
            const QString &file_name = changes.at(2);

            QFile file(file_name);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning() << "Could not open file:" << file_name;
                continue;
            }

            QString content;
            QTextStream in(&file);
            content = in.readAll();
            file.close();

            content.replace(text, new_text);

            // Create temp file
            QTemporaryFile tempFile;
            if (!tempFile.open()) {
                qWarning() << "Could not create temp file";
                continue;
            }

            QTextStream out(&tempFile);
            out << content;
            out.flush();
            tempFile.close();

            shell->runAsRoot(QString("mv %1 %2").arg(tempFile.fileName(), file_name));
            sources_changed = true;
        }
        queued_changes.clear();
    }
    setSelected();
    refresh();
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About %1").arg(windowTitle()),
        "<p align=\"center\"><b><h2>" + windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Program for choosing the default APT repository")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-repo-manager/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    QLocale locale;
    QString lang = locale.bcp47Name();
    QString url = "/usr/share/doc/mx-repo-manager/mx-repo-manager.html";

    if (lang.startsWith("fr")) {
        url = "https://mxlinux.org/wiki/help-files/help-mx-gestionnaire-de-d%C3%A9p%C3%B4ts";
    }

    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    ui->pushOk->setEnabled(true);
    ui->treeWidget->blockSignals(true);
    ui->treeWidgetDeb->blockSignals(true);

    const QString text = item->text(column);
    const QString file_name = item->parent()->text(0);
    const QString file_path
        = (file_name == "sources.list") ? "/etc/apt/" + file_name : "/etc/apt/sources.list.d/" + file_name;
    QString new_text = text;
    QStringList changes;

    if (text.contains("/mx/testrepo/") && item->checkState(column) == Qt::Checked) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("You have selected MX Test Repo. It's not recommended to leave it enabled or to "
                                "upgrade all the packages from it.")
                                 + "\n\n"
                                 + tr("A safer option is to install packages individually with MX Package "
                                      "Installer."));
    }

    if (file_name.endsWith(".sources")) {
        if (item->checkState(column) == Qt::Checked) {
            new_text.replace(QRegularExpression("Enabled:\\s*no", QRegularExpression::CaseInsensitiveOption),
                             "Enabled: yes");
        } else {
            if (!new_text.contains(QRegularExpression("Enabled:", QRegularExpression::CaseInsensitiveOption))) {
                new_text.append("\nEnabled: no");
            } else {
                new_text.replace(QRegularExpression("Enabled:\\s*yes", QRegularExpression::CaseInsensitiveOption),
                                 "Enabled: no");
            }
        }
    } else {
        if (item->checkState(column) == Qt::Checked) {
            new_text.remove(QRegularExpression("#\\s*"));
        } else {
            new_text.prepend("# ");
        }
    }

    item->setText(column, new_text);
    changes << text << new_text << file_path;
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
    static const QMap<QString, QString> specialCases = {{"The Netherlands", "Netherlands"},
                                                        {"USA", "UnitedStates"},
                                                        {"Anycast", "any"},
                                                        {"Any", "any"},
                                                        {"World", "any"}};

    if (specialCases.contains(country)) {
        if (country == "Anycast" || country == "Any" || country == "World") {
            return QIcon("/usr/share/flags-common/any.png");
        }
        country = specialCases[country];
    }

    QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Country>();
    int index = metaEnum.keyToValue(country.remove(' ').toUtf8());
    if (index == -1) {
        return {};
    }

    QList<QLocale> locales
        = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, static_cast<QLocale::Country>(index));
    if (!locales.isEmpty()) {
        QString short_name = locales.at(0).name().section('_', 1, 1).toLower();
        return QIcon("/usr/share/flags-common/" + short_name + ".png");
    }
    return {};
}

// Detect fastest Debian repo
void MainWindow::pushFastestDebian_clicked()
{
    progress->show();
    QTemporaryFile tmpfile;
    if (!tmpfile.open()) {
        qDebug() << "Could not create temp file";
        progress->hide();
        return;
    }

    QString ver_name = getDebianVerName(getDebianVerNum());
    if (ver_name == "buster" || ver_name == "bullseye") {
        ver_name.clear(); // netselect-apt doesn't like name buster/bullseye for some reason,
                          // maybe it expects "stable"
    }

    bool success = shell->runAsRoot("netselect-apt " + ver_name + " -o " + tmpfile.fileName(), false);
    progress->hide();

    if (!success) {
        QMessageBox::critical(this, tr("Error"), tr("netselect-apt could not detect fastest repo."));
        return;
    }
    QString repo = shell->getOut("grep -m1 '^deb ' " + tmpfile.fileName() + "| cut -d' ' -f2").trimmed();
    blockSignals(false);

    if (checkRepo(repo)) {
        replaceDebianRepos(repo);
        refresh();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not detect fastest repo."));
    }
}

// Detect and select the fastest MX repo
void MainWindow::pushFastestMX_clicked()
{
    progress->show();
    QString command = QString("netselect -D -I %1 | tr -s ' ' | sed 's/^[[:space:]]//' | cut -d' ' -f2").arg(listMXurls);
    bool success = shell->runAsRoot(command);
    qDebug() << listMXurls;
    QString out = shell->readAllStandardOutput().trimmed();
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

void MainWindow::pushRestoreSources_clicked()
{
    // Check if running on antiX/MX
    if (!QFileInfo::exists("/etc/antix-version") && !QFileInfo::exists("/etc/mx-version")) {
        QMessageBox::critical(this, tr("Error"), tr("Can't figure out if this app is running on antiX or MX"));
        return;
    }

    bool ok = true;
    int mx_version = shell->getOut("grep -oP '(?<=DISTRIB_RELEASE=).*' /etc/lsb-release").leftRef(2).toInt(&ok);
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

    // Download source files from
    const QString url = QString("https://codeload.github.com/MX-Linux/mx-sources/zip/mx%1").arg(mx_version);
    QFile tofile(tmpdir.path() + "/" + QFileInfo(url).fileName() + ".zip");
    if (!downloadFile(url, &tofile)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not download original APT files."));
        return;
    }

    // Extract master.zip to temp folder
    QString cmd = QString("unzip -q %1 -d %2/").arg(tofile.fileName(), tmpdir.path());
    if (!tofile.exists() || !shell->run(cmd)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not unzip downloaded file."));
        return;
    }

    bool enable_ahs = false;
    // For newer versions and 64-bit OS check if AHS was enabled
    if (mx_version >= 19 && shell->getOut("uname -m", true).trimmed() == "x86_64") {
        if (shell->run("apt-get update --print-uris | grep -q -E '/mx/([.]?/)*repo/.*/ahs/binary-amd64/Packages'",
                       true)) {
            enable_ahs = true;
            qDebug() << "AHS repo detected:" << enable_ahs;
        }
    }

    bool enable_mx = false;
    if (shell->run("apt-get update --print-uris | grep -q -E '/mx/([.]?/)*repo/.*/main/binary-amd64/Packages'", true)) {
        enable_mx = true;
    }

    QString mx_list = QString("%1/mx-sources-mx%2/mx.list").arg(tmpdir.path(), QString::number(mx_version));
    QString mx_sources = QString("%1/mx-sources-mx%2/mx.sources").arg(tmpdir.path(), QString::number(mx_version));

    auto enableAHS = [&](const QString &file) {
        if (QFile::exists(file)) {
            if (file.endsWith(".list")) {
                cmd = QString("sed -i -r 's/^[[:space:]]*#[[:space:]#]*(deb.*[[:space:]]ahs)[[:space:]]*/\\1/' %1")
                          .arg(file);
            } else {
                cmd = QString("sed -i -r '/Components:.*ahs/!s/^[[:space:]]*Components:[[:space:]]*\\[([^]]*?)\\][[:space:]]*$/Components: [\\1 ahs]/' %1")
                          .arg(file);
            }
            shell->run(cmd);
        }
    };

    QString &file = QFile::exists(mx_list) ? mx_list : mx_sources;

    if (enable_ahs) {
        enableAHS(file);
    } else if (mx_version >= 19 && shell->getOut("uname -m", true).trimmed() == "x86_64" && !enable_mx) {
        if (QMessageBox::Yes
            == QMessageBox::question(this, tr("Enabling AHS"), tr("Do you use AHS (Advanced Hardware Stack) repo?"))) {
            enableAHS(file);
        }
    }

    // Move the sources list files from the temporary directory to /etc/apt/sources.list.d/
    cmd = QString("mv -b %1/mx-sources-mx%2/*.{list,sources} /etc/apt/sources.list.d/ && chown 0:0 /etc/apt/sources.list.d/* && "
                  "chmod 644 /etc/apt/sources.list.d/*")
              .arg(tmpdir.path(), QString::number(mx_version));
    shell->runAsRoot(cmd);

    refresh(true);
    QMessageBox::information(this, tr("Success"),
                             tr("Original APT sources have been restored to the release status. User added source "
                                "files in /etc/apt/sources.list.d/ have not been touched.")
                                 + "\n\n"
                                 + tr("Your new selection will take effect the next time sources are updated."));
    sources_changed = true;
}

bool MainWindow::checkRepo(const QString &repo)
{
    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader(
        "User-Agent",
        QString("%1/%2 (linux-gnu)").arg(QApplication::applicationName(), QApplication::applicationVersion()).toUtf8());

    QNetworkProxyQuery query {QUrl(repo)};
    QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
    QNetworkAccessManager manager;
    if (!proxies.isEmpty()) {
        manager.setProxy(proxies.first());
    }

    request.setUrl(QUrl(repo));
    QNetworkReply *reply = manager.head(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);
    manager.setTransferTimeout(5000);
    loop.exec();
    reply->disconnect();

    if (reply->error() == QNetworkReply::NoError) {
        return true;
    }

    qDebug() << "No response from repo:" << reply->url() << reply->error();
    return false;
}

bool MainWindow::downloadFile(const QString &url, QFile *file, std::chrono::seconds timeout)
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

    QNetworkRequest request {QUrl(url)};
    request.setRawHeader(
        "User-Agent",
        QString("%1/%2 (linux-gnu)").arg(QApplication::applicationName(), QApplication::applicationVersion()).toUtf8());

    manager.setTransferTimeout(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;

    connect(reply, &QNetworkReply::readyRead, this, [&file, reply]() {
        if (file->write(reply->readAll()) == -1) {
            qDebug() << "Failed to write data to file:" << file->fileName();
            reply->abort();
            file->close();
            file->remove();
        }
    });
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);

    loop.exec();
    file->close();

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, tr("Error"),
                             tr("There was an error writing file: %1. Please check if you have "
                                "enough free space on your drive")
                                 .arg(file->fileName()));
        file->remove();
        return false;
    }
    return true;
}

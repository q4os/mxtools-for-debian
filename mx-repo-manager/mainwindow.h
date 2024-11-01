/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2015-2024 MX Authors
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
#pragma once

#include <QDir>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QTimer>
#include <QTreeWidget>

#include "cmd.h"

namespace Ui
{
class MainWindow;
}

using namespace std::chrono_literals;

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void cancelOperation();
    void closeEvent(QCloseEvent * /*unused*/) override;
    void procDone();
    void procTime();
    void procStart();

    void lineSearch_textChanged(const QString &arg1);
    void pushRestoreSources_clicked();
    void pushAbout_clicked();
    void pushFastestDebian_clicked();
    void pushFastestMX_clicked();
    void pushHelp_clicked();
    void pushOk_clicked();
    void tabWidget_currentChanged();
    void treeWidget_itemChanged(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;
    Cmd *shell;
    QHash<QString, QIcon> flags;
    QList<QStringList> queued_changes;
    QProgressBar *bar {};
    QProgressDialog *progress {};
    QPushButton *progCancel {};
    QSettings settings;
    QString current_repo;
    QString listMXurls;
    QStringList repos;
    QTimer timer;
    bool sources_changed {};

    QStringList readMXRepos();
    bool downloadFile(const QString &url, QFile *file, std::chrono::seconds timeout = 10s);
    bool replaceRepos(const QString &url, bool quiet = false);
    bool writeUpdatedFile(const QString &filePath, const QString &content);
    enum Version { Bullseye = 11, Bookworm, Trixie };
    static QFileInfoList listAptFiles();
    static QIcon getFlag(QString country);
    static QString getDebianVerName(int ver);
    static QStringList loadAptFile(const QString &file);
    static bool checkRepo(const QString &repo);
    static int getDebianVerNum();
    void centerWindow();
    void displayAllRepos(const QFileInfoList &apt_files);
    void displayMXRepos(const QStringList &repos, const QString &filter);
    void displaySelected(const QString &repo);
    void extractUrls(const QStringList &repos);
    void getCurrentRepo(bool force = false);
    void initializeIcons();
    void refresh(bool force = false);
    void replaceDebianRepos(const QString &url);
    void restoreWindowGeometry();
    void setConnections();
    void setIconIfNull(QPushButton *button, const QString &themeIcon, const QString &fallbackIcon);
    void setProgressBar();
    void setSelected();
};

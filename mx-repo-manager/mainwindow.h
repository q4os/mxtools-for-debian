/**********************************************************************
 *  mainwindow.h
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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QSettings>
#include <QTimer>
#include <QTreeWidget>

#include "cmd.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum Version {Jessie = 8, Stretch, Buster, Bullseye, Bookworm};
    QFileInfoList listAptFiles();
    static QIcon getFlag(QString country);
    QList<QStringList> queued_changes;
    static QString getDebianVerName(int ver);
    QString listMXurls;
    QString version;
    QStringList loadAptFile(const QString &file);
    QStringList readMXRepos();
    int getDebianVerNum();
    void centerWindow();
    void displayAllRepos(const QFileInfoList &apt_files);
    void displayMXRepos(const QStringList &repos, const QString &filter);
    void displaySelected(const QString &repo);
    void extractUrls(const QStringList &repos);
    void getCurrentRepo();
    void refresh();
    void replaceDebianRepos(const QString &url);
    void replaceRepos(const QString &url);
    void setConnections();
    void setProgressBar();
    void setSelected();

private slots:
    void cancelOperation();
    void closeEvent(QCloseEvent *);
    void procDone();
    void procTime();
    void procStart();

    void lineSearch_textChanged(const QString &arg1);
    void pb_restoreSources_clicked();
    void pushAbout_clicked();
    void pushFastestDebian_clicked();
    void pushFastestMX_clicked();
    void pushHelp_clicked();
    void pushOk_clicked();
    void tabWidget_currentChanged();
    void treeWidgetDeb_itemChanged(QTreeWidgetItem *item, int column);
    void treeWidget_itemChanged(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;
    Cmd *shell;
    QHash<QString, QIcon> flags;
    QProgressBar *bar{};
    QProgressDialog *progress{};
    QPushButton *progCancel{};
    QSettings settings;
    QString current_repo;
    QStringList repos;
    QTimer timer;

    QNetworkAccessManager manager;
    QNetworkReply* reply{};
    bool checkRepo(const QString &repo);
    bool downloadFile(const QString &url, QFile &file);

};


#endif // MAINWINDOW_H


/**********************************************************************
 *  mxpackageinstaller.h
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
#pragma once

#include <QCommandLineParser>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeWidgetItem>

#include "aptcache.h"
#include "cmd.h"
#include "lockfile.h"
#include "remotes.h"
#include "versionnumber.h"
#include "pmfiles.h"

namespace Ui
{
class MainWindow;
}

namespace Status
{
enum { NotInstalled, Installed, Upgradable };
}

namespace Tab
{
enum { Popular, EnabledRepos, Test, Backports, Flatpak, Output };
}

namespace PopCol
{
enum {
    Icon,
    Check,
    Name,
    Info,
    Description,
    InstallNames,
    UninstallNames,
    Screenshot,
    PostUninstall,
    PreUninstall,
    QDistro,
    MAX
};
} // namespace PopCol

namespace TreeCol
{
enum { Check, Name, Version, Description, Status };
}

namespace FlatCol
{
enum { Check, Name, LongName, Version, Size, Status, Duplicate, FullName };
}

namespace Release
{
enum { Jessie = 8, Stretch, Buster, Bullseye, Bookworm, Trixie };
}

struct PopularInfo {
    QString category;
    QString name;
    QString description;
    QString installable;
    QString screenshot;
    QString preInstall;
    QString postInstall;
    QString installNames;
    QString uninstallNames;
    QString postUninstall;
    QString preUninstall;
    QString qDistro;
};

constexpr uint KiB = 1024;
constexpr uint MiB = KiB * 1024;
constexpr uint GiB = MiB * 1024;

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QCommandLineParser &arg_parser, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void checkUnckeckItem();
    void cleanup();
    void cmdDone();
    void cmdStart();
    void disableOutput();
    void displayInfoTestOrBackport(const QTreeWidget *tree, const QTreeWidgetItem *item);
    void displayPackageInfo(const QTreeWidget *tree, QPoint pos);
    void displayPackageInfo(const QTreeWidgetItem *item);
    void displayPopularInfo(const QTreeWidgetItem *item, int column);
    void enableOutput();
    void filterChanged(const QString &arg1);
    void findPackageOther();
    void findPopular() const;
    void outputAvailable(const QString &output);
    void showOutput();
    void updateBar();

    void on_checkHideLibsBP_clicked(bool checked);
    void on_checkHideLibsMX_clicked(bool checked);
    void on_checkHideLibs_toggled(bool checked);
    void on_comboRemote_activated();
    void on_comboUser_activated(int index);
    void on_lineEdit_returnPressed();
    void on_pushAbout_clicked();
    void on_pushCancel_clicked();
    void on_pushEnter_clicked();
    void on_pushForceUpdateBP_clicked();
    void on_pushForceUpdateMX_clicked();
    void on_pushForceUpdateEnabled_clicked();
    void on_pushHelp_clicked();
    void on_pushInstall_clicked();
    void on_pushRemotes_clicked();
    void on_pushRemoveOrphan_clicked();
    void on_pushRemoveUnused_clicked();
    void on_pushUninstall_clicked();
    void on_pushUpgradeAll_clicked();
    void on_pushUpgradeFP_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_treeBackports_itemChanged(QTreeWidgetItem *item);
    void on_treeFlatpak_itemChanged(QTreeWidgetItem *item);
    void on_treeMXtest_itemChanged(QTreeWidgetItem *item);
    void on_treePopularApps_customContextMenuRequested(QPoint pos);
    void on_treePopularApps_expanded();
    void on_treePopularApps_itemChanged(QTreeWidgetItem *item);
    void on_treePopularApps_itemCollapsed(QTreeWidgetItem *item);
    void on_treePopularApps_itemExpanded(QTreeWidgetItem *item);
    void on_treeEnabled_itemChanged(QTreeWidgetItem *item);

private:
    Ui::MainWindow *ui;

    QString indexFilterFP;
    bool dirtyBackports {true};
    bool dirtyEnabledRepos {true};
    bool dirtyTest {true};
    bool displayFlatpaksIsRunning {false};
    bool displayPackagesIsRunning {false};
    bool firstRunFP {true};
    bool test_initially_enabled {false};
    bool updated_once {false};
    bool warning_backports {false};
    bool warning_flatpaks {false};
    bool warning_test {false};

    Cmd cmd;
    LockFile *lock_file {};
    QHash<QString, VersionNumber> listInstalledVersions();
    QList<PopularInfo> popular_apps;
    QLocale locale;
    QMap<QString, PackageInfo> backports_list;
    QMap<QString, PackageInfo> mx_list;
    QMap<QString, PackageInfo> enabled_list;
    QProgressBar *bar {};
    QProgressDialog *progress {};
    QPushButton *pushCancel {};
    QSettings dictionary;
    QSettings settings;
    QString arch;
    QString FPuser;
    QString ver_name;
    QString temp_list {"/etc/apt/sources.list.d/mxpm-temp.list"};
    QStringList change_list;
    QStringList flatpaks;
    QStringList flatpaks_apps;
    QStringList flatpaks_runtimes;
    QStringList installed_apps_fp;
    QStringList installed_packages;
    QStringList installed_runtimes_fp;
    QTemporaryDir tmp_dir;
    QTimer timer;
    QTreeWidget *currentTree {}; // current/calling tree
    QTreeWidgetItem *lastItemClicked {};
    VersionNumber fp_ver;
    const QCommandLineParser &args;

    QNetworkAccessManager manager;
    QNetworkReply *reply;

    [[nodiscard]] QString categoryTranslation(const QString &item);
    [[nodiscard]] QString getArchOption() const;
    [[nodiscard]] QString getLocalizedName(const QDomElement &element) const;
    [[nodiscard]] QString getVersion(const QString &name) const;
    [[nodiscard]] QString mapArchToFormat(const QString &arch) const;
    [[nodiscard]] QStringList listFlatpaks(const QString &remote, const QString &type = QLatin1String("")) const;
    [[nodiscard]] QStringList listInstalled() const;
    [[nodiscard]] QStringList listInstalledFlatpaks(const QString &type = QLatin1String(""));
    [[nodiscard]] bool checkInstalled(const QString &names) const;
    [[nodiscard]] bool checkInstalled(const QStringList &name_list) const;
    [[nodiscard]] bool checkUpgradable(const QStringList &name_list) const;
    [[nodiscard]] bool isOnline();
    [[nodiscard]] bool isPackageInstallable(const QString &installable, const QString &modArch) const;
    [[nodiscard]] static QString getDebianVerName();
    [[nodiscard]] static bool isFilteredName(const QString &name);
    [[nodiscard]] static uchar getDebianVerNum();
    bool buildPackageLists(bool force_download = false);
    bool confirmActions(const QString &names, const QString &action);
    bool downloadAndUnzip(const QString &url, QFile &file);
    bool downloadAndUnzip(const QString &url, const QString &repo_name, const QString &branch, const QString &format,
                          QFile &file);
    bool downloadFile(const QString &url, QFile &file);
    bool downloadPackageList(bool force_download = false);
    bool install(const QString &names);
    bool installBatch(const QStringList &name_list);
    bool installPopularApp(const QString &name);
    bool installPopularApps();
    bool installSelected();
    bool readPackageList(bool force_download = false);
    bool uninstall(const QString &names, const QString &preuninstall = QLatin1String(""),
                   const QString &postuninstall = QLatin1String(""));
    bool updateApt();
    static QString convert(quint64 bytes);
    static quint64 convert(const QString &size);
    void blockInterfaceFP(bool block);
    void buildChangeList(QTreeWidgetItem *item);
    void cancelDownload();
    void centerWindow();
    void clearUi();
    void displayFilteredFP(QStringList list, bool raw = false);
    void displayFlatpaks(bool force_update = false);
    void displayPackages();
    void displayPopularApps() const;
    void displayWarning(const QString &repo);
    void enableTabs(bool enable) const;
    void hideColumns() const;
    void hideLibs() const;
    void ifDownloadFailed() const;
    void listFlatpakRemotes() const;
    void listSizeInstalledFP();
    void loadPmFiles();
    void processDoc(const QDomDocument &doc);
    void refreshPopularApps();
    void removeDuplicatesFP() const;
    void setConnections() const;
    void setCurrentTree();
    void setDirty();
    void setIcons() const;
    void setProgressDialog();
    void setSearchFocus() const;
    void setup();
    void updateInterface() const;

    PMFiles pm_files;
};

/**********************************************************************
 *  mxpackageinstaller.h
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
#pragma once

#include <QCommandLineParser>
#include <QDomDocument>
#include <QFile>
#include <QHash>
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
#include "checkableheaderview.h"
#include "lockfile.h"
#include "remotes.h"
#include "versionnumber.h"

namespace Ui
{
class MainWindow;
}

namespace Status
{
enum { Installed = 1, Upgradable, NotInstalled, Autoremovable }; // Also used for filter combo index
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
    MAX
};
} // Namespace PopCol

namespace TreeCol
{
enum { Check, Name, RepoVersion, InstalledVersion, Description, Status };
}

namespace FlatCol
{
enum { Check, Name, LongName, Version, Branch, Size, Status, Duplicate, FullName };
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
};

constexpr uint KiB = 1024;
constexpr uint MiB = KiB * 1024;
constexpr uint GiB = MiB * 1024;

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QCommandLineParser &argParser, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void displayPackagesFinished();

private slots:
    void checkUncheckItem();
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
    void findPackage();
    void findPopular() const;
    void outputAvailable(const QString &output);
    void showOutput();
    void updateBar();

    void checkHideLibsBP_clicked(bool checked);
    void checkHideLibsMX_clicked(bool checked);
    void checkHideLibs_toggled(bool checked);
    void selectAllUpgradable_toggled(bool checked);
    void comboRemote_activated(int index = 0);
    void comboUser_currentIndexChanged(int index);
    void lineEdit_returnPressed();
    void pushAbout_clicked();
    void pushCancel_clicked();
    void pushEnter_clicked();
    void pushForceUpdateBP_clicked();
    void pushForceUpdateEnabled_clicked();
    void pushForceUpdateMX_clicked();
    void pushForceUpdateFP_clicked();
    void pushHelp_clicked();
    void pushInstall_clicked();
    void pushRemotes_clicked();
    void pushRemoveAutoremovable_clicked();
    void pushRemoveUnused_clicked();
    void pushUninstall_clicked();
    void pushUpgradeAll_clicked();
    void pushUpgradeFP_clicked();
    void tabWidget_currentChanged(int index);
    void treeBackports_itemChanged(QTreeWidgetItem *item);
    void treeEnabled_itemChanged(QTreeWidgetItem *item);
    void treeFlatpak_itemChanged(QTreeWidgetItem *item);
    void treeMXtest_itemChanged(QTreeWidgetItem *item);
    void treePopularApps_customContextMenuRequested(QPoint pos);
    void treePopularApps_expanded();
    void treePopularApps_itemChanged(QTreeWidgetItem *item);
    void treePopularApps_itemCollapsed(QTreeWidgetItem *item);
    void treePopularApps_itemExpanded(QTreeWidgetItem *item);

private:
    Ui::MainWindow *ui;

    QString indexFilterFP;
    bool dirtyBackports {true};
    bool dirtyEnabledRepos {true};
    bool dirtyTest {true};
    bool displayFlatpaksIsRunning {false};
    bool displayPackagesIsRunning {false};
    bool firstRunFP {true};
    bool hideLibsChecked {true};
    bool testInitiallyEnabled {false};
    bool updatedOnce {false};
    bool warningBackports {false};
    bool warningFlatpaks {false};
    bool warningTest {false};
    int savedComboIndex {0};

    Cmd cmd;
    LockFile lockFile {"/var/lib/dpkg/lock"};
    QHash<QString, VersionNumber> listInstalledVersions();
    QIcon qiconInstalled;
    QIcon qiconUpgradable;
    QList<PopularInfo> popularApps;
    QLocale locale;
    QHash<QString, PackageInfo> backportsList;
    QHash<QString, PackageInfo> enabledList;
    QHash<QString, PackageInfo> installedPackages;
    QHash<QString, PackageInfo> mxList;
    QProgressBar *bar {};
    QProgressDialog *progress {};
    QPushButton *pushCancel {};
    QSettings dictionary;
    QSettings settings;
    QString arch;
    QString fpUser;
    uchar debianVersion {Release::Bookworm};
    QString verName;
    QStringList changeList;
    QStringList flatpaks;
    QStringList flatpaksApps;
    QStringList flatpaksRuntimes;
    QStringList installedAppsFP;
    QStringList installedRuntimesFP;
    QStringList cachedInstalledFlatpaks; // Raw lines from flatpak list --columns=ref,size
    QString cachedInstalledScope;
    QHash<QString, QString> cachedInstalledSizeMap; // canonical ref -> size string
    mutable QStringList cachedFlatpakRemotes;
    mutable QString cachedFlatpakRemotesScope;
    mutable bool cachedFlatpakRemotesFetched {false};
    bool cachedInstalledFetched {false};
    bool holdProgressForAptRefresh {false};
    bool holdProgressForFlatpakRefresh {false};
    bool flatpakCancelHidden {false};
    bool flatpakUiBlocked {false};
    bool suppressCmdOutput {false};
    QTemporaryDir tempDir;
    QTimer timer;
    QTreeWidget *currentTree {}; // current/calling tree
    QTreeWidgetItem *lastItemClicked {};
    QUrl getScreenshotUrl(const QString &name);
    const QCommandLineParser &args;
    const QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec " : "/usr/bin/gksu "};
    const QString tempList {"/etc/apt/sources.list.d/mxpitemp.list"};

    QNetworkAccessManager manager;
    QNetworkReply *reply;

    [[nodiscard]] QHash<QString, PackageInfo> listInstalled();
    [[nodiscard]] QString categoryTranslation(const QString &item);
    [[nodiscard]] QString getMXTestRepoUrl();
    [[nodiscard]] QString getArchOption() const;
    [[nodiscard]] QString getLocalizedName(const QDomElement &element) const;
    [[nodiscard]] QString getVersion(const QString &name) const;
    [[nodiscard]] QString mapArchToFormat(const QString &arch) const;
    [[nodiscard]] QStringList listFlatpaks(const QString &remote, const QString &type = QLatin1String("")) const;
    [[nodiscard]] QStringList listInstalledFlatpaks(const QString &type = QLatin1String(""));
    [[nodiscard]] QTreeWidgetItem *createFlatpakItem(const QString &item, const QStringList &installedAll) const;
    [[nodiscard]] QTreeWidgetItem *createTreeItem(const QString &name, const QString &version,
                                                  const QString &description) const;
    [[nodiscard]] bool checkInstalled(const QVariant &names) const;
    [[nodiscard]] bool checkUpgradable(const QStringList &nameList) const;
    [[nodiscard]] bool isOnline();
    [[nodiscard]] bool isPackageInstallable(const QString &installable, const QString &modArch) const;
    [[nodiscard]] static QString getDebianVerName(uchar version = 0);
    [[nodiscard]] static bool isFilteredName(const QString &name);
    [[nodiscard]] static uchar getDebianVerNum();
    [[nodiscard]] static uchar showVersionDialog(const QString &message);
    [[nodiscard]] QList<QTreeWidgetItem *> createTreeItemsList(QHash<QString, PackageInfo> *list) const;
    [[nodiscard]] QHash<QString, PackageInfo> *getCurrentList();
    [[nodiscard]] QTreeWidget *getCurrentTree();

    bool buildPackageLists(bool forceDownload = false);
    bool confirmActions(const QString &names, const QString &action);

    bool downloadAndUnzip(const QString &url, QFile &file);
    bool downloadAndUnzip(const QString &url, const QString &repoName, const QString &branch, const QString &format,
                          QFile &file);
    bool downloadFile(const QString &url, QFile &file);
    bool downloadPackageList(bool forceDownload = false);
    bool install(const QString &names);
    bool installBatch(const QStringList &nameList);
    bool installPopularApp(const QString &name);
    bool installPopularApps();
    bool installSelected();
    bool markKeep();
    bool readPackageList(bool forceDownload = false);
    bool uninstall(const QString &names, const QString &preUninstall = QLatin1String(""),
                   const QString &postUninstall = QLatin1String(""));
    bool updateApt();
    [[nodiscard]] static QString convert(quint64 bytes);
    [[nodiscard]] static quint64 convert(const QString &size);
    void blockInterfaceFP(bool block);
    void buildChangeList(QTreeWidgetItem *item);
    void cancelDownload();
    void centerWindow();
    void clearUi();
    void displayAutoremovable(const QTreeWidget *newTree);
    void displayFilteredFP(QStringList list, bool raw = false);
    void displayFlatpaks(bool forceUpdate = false);
    void displayPackages();
    void displayPopularApps() const;
    void displayWarning(const QString &repo);
    void enableTabs(bool enable);
    void finalizeFlatpakDisplay();
    void formatFlatpakTree() const;
    void handleEnabledReposTab(const QString &searchStr);
    void handleFlatpakTab(const QString &searchStr);
    void handleOutputTab();
    void handleTab(const QString &searchStr, QLineEdit *searchBox, const QString &warningMessage, bool dirtyFlag);
    void hideColumns() const;
    void hideLibs() const;
    void ifDownloadFailed() const;
    void installFlatpak();
    void invalidateFlatpakRemoteCache();
    void listFlatpakRemotes() const;
    void listSizeInstalledFP();
    void loadFlatpakData();
    void loadPmFiles();
    void populateFlatpakTree();
    void processDoc(const QDomDocument &doc);
    void refreshPopularApps();
    void removeDuplicatesFP() const;
    void resetCheckboxes();
    void saveSearchText(QString &searchStr, int &filterIdx);
    void setConnections() const;
    void setCurrentTree();
    void setDirty();
    void setIcons();
    void setProgressDialog();
    void setSearchFocus() const;
    void setup();
    void setupFlatpakDisplay();
    void updateFlatpakCounts(uint totalCount);
    void updateInterface() const;
    void updateTreeItems(QTreeWidget *tree);
    // Header checkbox helpers
    CheckableHeaderView *headerEnabled {nullptr};
    CheckableHeaderView *headerMX {nullptr};
    CheckableHeaderView *headerBP {nullptr};
};

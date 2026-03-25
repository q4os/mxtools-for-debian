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

#include <QCloseEvent>
#include <QCommandLineParser>
#include <QDomDocument>
#include <QFile>
#include <QHash>
#include <QMessageBox>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QStandardItem>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeView>

#include "aptcache.h"
#include "cmd.h"
#include "checkableheaderview.h"
#include "lockfile.h"
#include "models/flatpakfilterproxy.h"
#include "models/flatpakmodel.h"
#include "models/packagefilterproxy.h"
#include "models/packagemodel.h"
#include "models/popularfilterproxy.h"
#include "models/popularmodel.h"
#include "remotes.h"
#include "versionnumber.h"
#include "pmfiles.h"

namespace Ui
{
class MainWindow;
}

namespace Tab
{
enum { Popular, EnabledRepos, Test, Backports, Flatpak, Output };
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
    explicit MainWindow(const QCommandLineParser &argParser, QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void displayPackagesFinished();

private slots:
    void checkUncheckItem();
    void cleanup();
    void cmdDone();
    void cmdStart();
    void disableOutput();
    void displayInfoTestOrBackport(QTreeView *tree, const QModelIndex &index);
    void displayPackageInfo(QTreeView *tree, QPoint pos);
    void displayPackageInfo(const QModelIndex &index);
    void displayPopularInfo(const QModelIndex &index);
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
    void checkRepoOnlyMX_clicked(bool checked);
    void checkRepoOnlyBP_clicked(bool checked);
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
    void onPackageCheckStateChanged(const QString &packageName, Qt::CheckState state);
    void onFlatpakCheckStateChanged(const QString &fullName, Qt::CheckState state, int status);
    void onPopularItemChanged(const QModelIndex &index);
    void treePopularApps_customContextMenuRequested(QPoint pos);
    void treePopularApps_expanded();
    void treePopularApps_itemCollapsed(const QModelIndex &index);
    void treePopularApps_itemExpanded(const QModelIndex &index);

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
    bool operationInProgress {false};
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
    QTreeView *currentTree {}; // current/calling tree
    QModelIndex lastIndexClicked {};
    QUrl getScreenshotUrl(const QString &name);
    const QCommandLineParser &args;
    const QString tempList {QStringLiteral("/etc/apt/sources.list.d/mxpitemp.list")};

    // Models
    PackageModel *enabledModel {nullptr};
    PackageModel *mxtestModel {nullptr};
    PackageModel *backportsModel {nullptr};
    FlatpakModel *flatpakModel {nullptr};
    PopularModel *popularModel {nullptr};

    // Filter proxies
    PackageFilterProxy *enabledProxy {nullptr};
    PackageFilterProxy *mxtestProxy {nullptr};
    PackageFilterProxy *backportsProxy {nullptr};
    FlatpakFilterProxy *flatpakProxy {nullptr};
    PopularFilterProxy *popularProxy {nullptr};

    QNetworkAccessManager manager;
    QNetworkReply *reply;

    [[nodiscard]] QHash<QString, PackageInfo> listInstalled();
    [[nodiscard]] QString categoryTranslation(const QString &item);
    [[nodiscard]] QString getMXTestRepoUrl();
    [[nodiscard]] QString getArchOption() const;
    [[nodiscard]] QString getLocalizedName(const QDomElement &element) const;
    [[nodiscard]] QString getVersion(const QString &name) const;
    [[nodiscard]] QString mapArchToFormat(const QString &arch) const;
    [[nodiscard]] QStringList listFlatpaks(const QString &remote, const QString &type = QString()) const;
    [[nodiscard]] QStringList listInstalledFlatpaks(const QString &type = QString());
    [[nodiscard]] FlatpakData createFlatpakData(const QString &item, const QStringList &installedAll) const;
    [[nodiscard]] PackageData createPackageData(const QString &name, const QString &version,
                                                const QString &description) const;
    [[nodiscard]] bool checkInstalled(const QVariant &names) const;
    [[nodiscard]] bool checkUpgradable(const QStringList &nameList) const;
    [[nodiscard]] bool isOnline();
    [[nodiscard]] bool isPackageInstallable(const QString &installable, const QString &modArch) const;
    [[nodiscard]] static QString getDebianVerName(uchar version = 0);
    [[nodiscard]] static bool isFilteredName(const QString &name);
    [[nodiscard]] static uchar getDebianVerNum();
    [[nodiscard]] static uchar showVersionDialog(const QString &message);
    [[nodiscard]] QVector<PackageData> createPackageDataList(QHash<QString, PackageInfo> *list) const;
    [[nodiscard]] QHash<QString, PackageInfo> *getCurrentList();
    [[nodiscard]] PackageModel *getCurrentModel();
    [[nodiscard]] PackageFilterProxy *getCurrentProxy();

    bool buildPackageLists(bool forceDownload = false);
    [[nodiscard]] bool confirmActions(const QString &names, const QString &action);

    [[nodiscard]] bool downloadAndUnzip(const QString &url, QFile &file);
    [[nodiscard]] bool downloadAndUnzip(const QString &url, const QString &repoName, const QString &branch,
                                        const QString &format, QFile &file);
    [[nodiscard]] bool downloadFile(const QString &url, QFile &file);
    [[nodiscard]] bool downloadPackageList(bool forceDownload = false);
    bool install(const QString &names);
    [[nodiscard]] bool installBatch(const QStringList &nameList);
    [[nodiscard]] bool installPopularApp(const QString &name);
    [[nodiscard]] bool installPopularApps();
    [[nodiscard]] bool installSelected();
    [[nodiscard]] bool markKeep();
    [[nodiscard]] bool readPackageList(bool forceDownload = false);
    [[nodiscard]] bool uninstall(const QString &names, const QStringList &preUninstall = {},
                                 const QStringList &postUninstall = {});
    bool updateApt();
    [[nodiscard]] static QString convert(quint64 bytes);
    [[nodiscard]] static quint64 convert(const QString &size);
    void blockInterfaceFP(bool block);
    void buildChangeList(const QString &packageName, Qt::CheckState state);
    void buildFlatpakChangeList(const QString &fullName, Qt::CheckState state, int status);
    void cancelDownload();
    void centerWindow();
    void clearUi();
    void displayAutoremovable();
    void displayFilteredFP(QStringList list, bool raw = false);
    void displayFlatpaks(bool forceUpdate = false);
    void displayPackages();
    void displayPopularApps();
    void displayWarning(const QString &repo);
    void enableTabs(bool enable);
    void finalizeFlatpakDisplay();
    void formatFlatpakTree();
    void removeDuplicatesFP();
    void applyPopularCategorySpanning();
    void handleEnabledReposTab(const QString &searchStr);
    void handleFlatpakTab(const QString &searchStr);
    void handleOutputTab();
    void handleTab(const QString &searchStr, QLineEdit *searchBox, const QString &warningMessage, bool dirtyFlag);
    void resizeCurrentColumns();
    [[nodiscard]] bool shouldRefreshFilters(const QString &searchStr);
    void hideColumns();
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
    void resetCheckboxes();
    void saveSearchText(QString &searchStr, int &filterIdx);
    void setConnections();
    void setCurrentTree();
    void setDirty();
    void rebuildPackageViews();
    void setIcons();
    void setProgressDialog();
    void setSearchFocus() const;
    void setup();
    void setupFlatpakDisplay();
    void showFlatpakProgress(const QString &label);
    void setupModels();
    void updateFlatpakCounts(uint totalCount);
    void updateInterface();
    void updatePackageStatuses();
    // Header checkbox helpers
    CheckableHeaderView *headerEnabled {nullptr};
    CheckableHeaderView *headerMX {nullptr};
    CheckableHeaderView *headerBP {nullptr};
    //added by q4os
    PMFiles pm_files;
};

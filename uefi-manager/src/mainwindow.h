/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2024-2025 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#pragma once

#include <QCommandLineParser>
#include <QListWidget>
#include <QMap>
#include <QMessageBox>
#include <QSettings>

#include "cmd.h"

namespace Ui
{
class MainWindow;
}

struct Tab {
    enum Values { Entries, StubInstall, Frugal };
};

struct Page {
    enum Values { Location, Options };
};

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QCommandLineParser &argParser, QWidget *parent = nullptr);
    ~MainWindow() override;
    void centerWindow();
    void setup();

private slots:
    void cmdDone();
    void cmdStart();
    void pushAboutClicked();
    void pushHelpClicked();
    void pushBackClicked();
    void pushNextClicked();
    void setConnections();
    void tabWidgetCurrentChanged();

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QString distro = getDistroName();
    QString espMountPoint;
    QString frugalDir;
    QString rootDrive;
    QString rootPartition;
    QString rootDevicePath;
    QSettings settings;
    QStringList driveList;
    QStringList espList;
    QStringList partitionList;
    QStringList linuxPartitionList;
    QStringList frugalPartitionList;
    QStringList newDirectories;
    QStringList newLuksDevices;
    QStringList newMounts;

    static const QMap<QString, QString> PERSISTENCE_TYPES;

    struct Options {
        QString entryName;
        QString uuid;
        QString bdir;
        QString stringOptions;
        QString persistenceType;
    } options;

    [[nodiscard]] QString getBootLocation();
    [[nodiscard]] QString getBootLocation(const QString &mountPoint);
    [[nodiscard]] QString getDistroName(bool pretty = false, const QString &mountPoint = "/",
                                        const QString &releaseFile = "initrd_release") const;
    [[nodiscard]] QString getLuksUUID(const QString &part);
    [[nodiscard]] QString getMountPoint(const QString &part);
    [[nodiscard]] QString mountPartition(QString part);
    [[nodiscard]] QString openLuks(const QString &part);
    [[nodiscard]] QString selectESP();
    [[nodiscard]] QString selectFrugalDirectory(const QString &part);
    [[nodiscard]] bool checkSizeEsp();
    [[nodiscard]] bool copyKernel();
    [[nodiscard]] bool installEfiStub(const QString &esp);
    [[nodiscard]] bool isLuks(const QString &part);
    [[nodiscard]] bool readGrubEntry();
    bool renameUefiEntry(const QString &oldLabel, const QString &newLabel, const QString &oldBootNum = QString());
    static void removeUefiEntry(QListWidget *listEntries, QWidget *uefiDialog);
    static void setUefiBootNext(QListWidget *listEntries, QLabel *textBootNext);
    static void setUefiTimeout(QWidget *uefiDialog, QLabel *textTimeout);
    static void sortUefiBootOrder(const QStringList &order, QListWidget *list);
    static void toggleUefiActive(QListWidget *listEntries);
    void addDevToList();
    void addUefiEntry(QListWidget *listEntries, QWidget *dialogUefi);
    void checkDoneStub();
    void clearEntryWidget();
    void filterDrivePartitions();
    void getGrubOptions(const QString &mountPoint = "/");
    void getKernelOptions(const QString &mountPoint);
    void getKernelOptions(const QString &mountPoint, const QString &rootDir, const QString &kernel);
    void guessPartition();
    void listDevices();
    void loadStubOption();
    void promptFrugalStubInstall();
    void readBootEntries(QListWidget *listEntries, QLabel *textTimeout, QLabel *textBootNext, QLabel *textBootCurrent,
                         QStringList *bootorder);
    void refreshEntries();
    void refreshFrugal();
    void refreshStubInstall();
    void saveBootOrder(const QListWidget *list);
    void selectKernel(const QString &mountPoint);
    void validateAndLoadOptions(const QString &frugalDir);
    QStringList sortKernelVersions(const QStringList &kernelFiles, bool reverse = true) const;
    bool isSystemd() const;
    bool isShimSystemd(const QString &rootPath = "/") const;
};

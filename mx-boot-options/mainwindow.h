/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian, Dolphin Oracle
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

#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTimer>

#include <cmd.h>
#include <dialog.h>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void cleanup();
    void cmdDone();
    void cmdStart();
    void procTime();
    void setConnections();

    void btnBgFileClicked();
    void btnThemeFileClicked();
    void comboBootsplashClicked(bool checked);
    void comboBootsplashToggled(bool checked);
    void comboEnableFlatmenusClicked(bool checked);
    void comboGrubThemeToggled(bool checked);
    void comboMenuEntryCurrentIndexChanged();
    void comboSaveDefaultClicked();
    void comboThemeActivated(int);
    void comboThemeCurrentIndexChanged(int index);
    void lineEditKernelTextEdited();
    void pushAboutClicked();
    void pushApplyClicked();
    void pushHelpClicked();
    void pushLogClicked();
    void pushPreviewClicked();
    void pushUefiClicked();
    void radioDetailedMsgToggled(bool checked);
    void radioLimitedMsgToggled(bool checked);
    void radioVeryDetailedMsgToggled(bool checked);
    void spinBoxTimeoutValueChanged(int val);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QProgressBar *bar {};
    QTimer timer;

    bool grubInstalled {};
    bool installedMode = true;
    bool justInstalled {};
    bool kernelOptionsChanged {};
    bool live = isLive();
    bool messagesChanged {};
    bool optionsChanged {};
    bool splashChanged {};

    QString bootLocation;
    QString chroot;
    QString user;
    QStringList defaultGrub;
    QStringList grubCfg;
    QTemporaryDir tempDir;
    const QString kernelOptions {readKernelOpts()};
    const QStringList requiredPackages {"plymouth", "plymouth-x11", "plymouth-themes", "plymouth-themes-mx"};

    [[nodiscard]] QString readKernelOpts();
    [[nodiscard]] QString selectPartition(const QStringList &list);
    [[nodiscard]] QStringList getLinuxPartitions();
    [[nodiscard]] bool inVirtualMachine();
    [[nodiscard]] bool isInstalled(const QString &package);
    [[nodiscard]] bool isInstalled(const QStringList &packages);
    [[nodiscard]] bool isLive();
    [[nodiscard]] bool isLuks(const QString &part);
    [[nodiscard]] bool mountBoot(const QString &path);
    [[nodiscard]] bool openLuks(const QString &part, const QString &path);
    [[nodiscard]] static bool isUefi();
    bool replaceGrubArg(const QString &key, const QString &item);
    static void removeUefiEntry(QListWidget *listEntries, QDialog *uefiDialog);
    static void sendMouseEvents();
    static void setUefiBootNext(QListWidget *listEntries, QLabel *textBootNext);
    static void setUefiTimeout(QDialog *uefiDialog, QLabel *textTimeout);
    static void sortUefiBootOrder(const QStringList &order, QListWidget *list);
    static void toggleUefiActive(QListWidget *listEntries);
    void addGrubLine(const QString &item);
    void addUefiEntry(QListWidget *listEntries, QDialog *dialogUefi);
    void appendLogWithColors(QTextEdit *textEdit, const QString &logContent);
    void createChrootEnv(const QString &root);
    void disableGrubLine(const QString &item);
    void enableGrubLine(const QString &item);
    void handleLiveSystem();
    void handleSpecialFilesystems();
    void installSplash();
    void loadPlymouthThemes();
    void processGrubDefault(const QString &line);
    void processGrubTheme(const QString &line);
    void processKernelCommandLine(QString line);
    void readBootEntries(QListWidget *listEntries, QLabel *textTimeout, QLabel *textBootNext, QLabel *textBootCurrent,
                         QStringList *bootorder);
    void readDefaultGrub();
    void readGrubCfg();
    void replaceSyslinuxArgs(const QString &args);
    void replaceLiveGrubArgs(const QString &args);
    void setupUiElements();
    void saveBootOrder(const QListWidget *list);
    void setGeneralConnections();
    void setup();
    void setupGrubSettings();
    void unmountAndClean(const QStringList &mountList);
    void writeDefaultGrub();
};

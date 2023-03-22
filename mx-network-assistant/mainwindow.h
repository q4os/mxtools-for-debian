/*
   Copyright (C) 2003-2010 by Warren Woodford
   Copyright (C) 2014 by Adrian adrian@mxlinux.org

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


   With big modifications made by Adrian adrian@mxlinux.org
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include <QDir>

#include "cmd.h"

enum Tab {Status, LinuxDrivers, Diagnostics}; // WindowsDrivers is removed in constructor
enum Col {Enabled, Interface, Driver, Description, Vendor, Product};

class MainWindow : public QDialog, public Ui::MainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

    QString getIP();
    QString getIPfromRouter();
    static bool checkSysFileExists(const QDir& searchPath, const QString& fileName, Qt::CaseSensitivity cs);
    bool checkWifiAvailable();
    bool checkWifiEnabled();
    static bool replaceStringInFile(const QString& oldtext, const QString& newtext, const QString& filepath);
    void refresh();

public slots:
    virtual void aptUpdateFinished();
    virtual void hwListFullToClipboard();
    virtual void hwListToClipboard();
    virtual void installFinished(int);
    virtual void linuxDrvListFullToClipboard();
    virtual void linuxDrvListToClipboard();
    virtual void on_buttonAbout_clicked();
    virtual void on_buttonCancel_clicked();
    virtual void on_cancelPing_clicked();
    virtual void on_cancelTrace_clicked();
    virtual void on_clearPingOutput_clicked();
    virtual void on_clearTraceOutput_clicked();
    virtual void on_generalHelpPushButton_clicked();
    virtual void on_hwDiagnosePushButton_clicked();
    virtual void on_linuxDrvBlockPushButton_clicked();
    virtual void on_linuxDrvDiagnosePushButton_clicked();
    virtual void on_linuxDrvList_currentRowChanged(int currentRow );
    virtual void on_pingButton_clicked();
    virtual void on_tabWidget_currentChanged();
    virtual void on_tracerouteButton_clicked();
    virtual void on_windowsDrvAddPushButton_clicked() ;
    virtual void on_windowsDrvDiagnosePushButton_clicked();
    virtual void on_windowsDrvList_currentRowChanged(int row);
    virtual void on_windowsDrvRemovePushButton_clicked();
    virtual void pingFinished();
    virtual void show();
    virtual void showContextMenuForHw(QPoint pos);
    virtual void showContextMenuForLinuxDrv(QPoint pos);
    virtual void showContextMenuForWindowsDrv(QPoint pos);
    virtual void tracerouteFinished();
    virtual void uninstallNdisFinished(int);
    virtual void windowsDrvListFullToClipboard();
    virtual void windowsDrvListToClipboard();
    virtual void writeInstallOutput();
    virtual void writePingOutput();
    virtual void writeTraceOutput();

protected:
    QProcess *installProc;
    QProcess *pingProc;
    QProcess *traceProc;
    QStringList blockedModules;
    QStringList broadcomModules;
    QStringList loadedModules;
    QStringList unloadedModules;
    QTextEdit *installOutputEdit;
    Tab currentTab;
    bool driverBlocklisted{};
    bool internetConnection{};
    bool ndiswrapBlocklisted{};

    bool blockModule(const QString& module);
    bool installModule(const QString& module);
    bool loadModule(const QString& module);
    static bool removable(const QString& module);
    bool removeModule(const QString& module);
    static bool removeStart(const QString& module);
    void updateDriverStatus();

private slots:
    void on_hwUnblock_clicked();
    void on_installNdiswrapper_clicked();
    void on_linuxDrvLoad_clicked();
    void on_linuxDrvUnload_clicked();
    void on_pushDisable_clicked();
    void on_pushEnable_clicked();
    void on_uninstallNdiswrapper_clicked();

private:
    Cmd cmd;

};

#endif


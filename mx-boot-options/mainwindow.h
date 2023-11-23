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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QTemporaryDir>
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

    void btn_bg_file_clicked();
    void btn_theme_file_clicked();
    void pushAbout_clicked();
    void pushApply_clicked();
    void pushHelp_clicked();
    void pushLog_clicked();
    void pushUefi_clicked();
    void push_preview_clicked();
    void combo_bootsplash_clicked(bool checked);
    void combo_bootsplash_toggled(bool checked);
    void combo_enable_flatmenus_clicked(bool checked);
    void combo_grub_theme_toggled(bool checked);
    void combo_save_default_clicked();
    void combo_menu_entry_currentIndexChanged();
    void combo_theme_activated(int);
    void combo_theme_currentIndexChanged(int index);
    void lineEdit_kernel_textEdited();
    void radio_detailed_msg_toggled(bool checked);
    void radio_limited_msg_toggled(bool checked);
    void radio_very_detailed_msg_toggled(bool checked);
    void spinBoxTimeout_valueChanged(int val);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QProgressBar *bar {};
    QTimer timer;

    bool grub_installed {};
    bool just_installed {};
    bool kernel_options_changed {};
    bool messages_changed {};
    bool options_changed {};
    bool splash_changed {};

    QString chroot;
    QString kernel_options;
    QString user;
    QStringList default_grub;
    QStringList grub_cfg;
    QTemporaryDir tmpdir;

    QString selectPartiton(const QStringList &list);
    QStringList getLinuxPartitions();
    bool inVirtualMachine();
    bool installSplash();
    bool isInstalled(const QString &package);
    bool isInstalled(const QStringList &packages);
    bool replaceGrubArg(const QString &key, const QString &item);
    static bool isUefi();
    static void removeUefiEntry(QListWidget *listEntries, QDialog *uefiDialog);
    static void sendMouseEvents();
    static void setUefiBootNext(QListWidget *listEntries, QLabel *textBootNext);
    static void setUefiTimeout(QDialog *uefiDialog, QLabel *textTimeout);
    static void sortUefiBootOrder(const QStringList &order, QListWidget *list);
    static void toggleUefiActive(QListWidget *listEntries);
    void addGrubLine(const QString &item);
    void addUefiEntry(QListWidget *listEntries, QDialog *dialogUefi);
    void createChrootEnv(const QString &root);
    void disableGrubLine(const QString &item);
    void enableGrubLine(const QString &item);
    void loadPlymouthThemes();
    void readBootEntries(QListWidget *listEntries, QLabel *textTimeout, QLabel *textBootNext, QLabel *textBootCurrent,
                         QStringList *bootorder);
    void readDefaultGrub();
    void readGrubCfg();
    void readKernelOpts();
    void saveBootOrder(const QListWidget *list);
    void setGeneralConnections();
    void setup();
    void unmountAndClean(const QStringList &mount_list);
    void writeDefaultGrub();
};

#endif

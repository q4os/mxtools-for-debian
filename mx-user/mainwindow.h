/*
   Copyright (C) 2003-2010 by Warren Woodford
   Copyright (C) 2014 by Timothy E. Harris
   for modifications applicable to the MX Linux project.

   Heavily modified by Adrian adrian@mxlinux.org

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>

#include "cmd.h"

class MainWindow : public QDialog, public Ui::MEConfig
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // helpers
    static bool replaceStringInFile(const QString &oldtext, const QString &newtext, const QString &filepath);
    // common
    void refresh();
    // special
    static void restartPanel(const QString &user);
    void applyAdd();
    void applyChangePass();
    void applyDelete();
    void applyDesktop();
    void applyGroup();
    void applyMembership();
    void applyOptions();
    void applyRename();
    void buildListGroups();
    void buildListGroupsToRemove();
    void refreshAdd();
    void refreshChangePass();
    void refreshCopy();
    void refreshDelete();
    void refreshGroups();
    void refreshMembership();
    void refreshOptions();
    void refreshRename();

public slots:
    void progress();
    void syncDone(bool success);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void closeApp();
    void on_buttonAbout_clicked();
    void on_buttonApply_clicked();
    void on_buttonCancel_clicked();
    void on_buttonHelp_clicked();
    void on_checkGroups_stateChanged(int);
    void on_checkMozilla_stateChanged(int);
    void on_comboChangePass_activated(const QString &);
    void on_comboDeleteUser_activated(const QString &);
    void on_comboRenameUser_activated(const QString &);
    void on_copyRadioButton_clicked();
    void on_docsRadioButton_clicked();
    void on_entireRadioButton_clicked();
    void on_fromUserComboBox_activated(const QString &);
    void on_groupNameEdit_textEdited();
    void on_lineEditChangePassConf_textChanged(const QString &arg1);
    void on_lineEditChangePass_textChanged();
    void on_mozillaRadioButton_clicked();
    void on_radioAutologinNo_clicked();
    void on_radioAutologinYes_clicked();
    void on_sharedRadioButton_clicked();
    void on_syncRadioButton_clicked();
    void on_tabWidget_currentChanged();
    void on_toUserComboBox_activated(const QString &arg1);
    void on_userComboBox_activated(const QString &);
    void on_userComboMembership_activated(const QString &);
    void on_userNameEdit_textEdited();
    void on_userPassword2Edit_textChanged(const QString &arg1);
    void on_userPasswordEdit_textChanged();

private:
    Cmd *shell;
    QSettings settings;
    QStringList users;
};

#endif

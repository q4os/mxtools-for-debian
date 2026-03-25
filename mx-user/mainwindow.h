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
#pragma once

#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>

#include "cmd.h"

namespace Tab
{
enum { Administration, Options, Copy, AddRemoveGroup, GroupMembership, MAX };
}

class MainWindow : public QDialog, public Ui::MEConfig
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

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
    void refresh();
    void refreshAdd();
    void refreshChangePass();
    void refreshCopy();
    void refreshDelete();
    void refreshGroups();
    void refreshMembership();
    void refreshOptions();
    void refreshRename();

public slots:
    void syncDone(bool success);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void buttonAbout_clicked();
    void buttonApply_clicked();
    void buttonCancel_clicked();
    void buttonHelp_clicked();
    void checkGroups_stateChanged(Qt::CheckState);
    void checkMozilla_stateChanged(Qt::CheckState);
    void closeApp();
    void comboChangePass_activated(const QString &);
    void comboDeleteUser_activated(const QString &);
    void comboRenameUser_activated(const QString &);
    void copyRadioButton_clicked();
    void docsRadioButton_clicked();
    void entireRadioButton_clicked();
    void fromUserComboBox_activated(const QString &);
    void groupNameEdit_textEdited();
    void mozillaRadioButton_clicked();
    void radioAutologinNo_clicked();
    void radioAutologinYes_clicked();
    void sharedRadioButton_clicked();
    void syncRadioButton_clicked();
    void tabWidget_currentChanged();
    void toUserComboBox_activated(const QString &arg1);
    void userComboBox_activated(const QString &);
    void userComboMembership_activated(const QString &);
    void userNameEdit_textEdited();

private:
    Cmd *shell;
    QSettings settings;
    QStringList users;
    class PassEdit *passChange;
    class PassEdit *passUser;
    void setConnections();
    [[nodiscard]] QString adminGroupName() const;
    [[nodiscard]] QString defaultShellPath() const;
    [[nodiscard]] QStringList defaultExtraGroups() const;
    [[nodiscard]] bool commandExists(const QString &command) const;
    [[nodiscard]] QString currentLogname() const;
};

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
#include "mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QScreen>
#include <QTextEdit>
#include <QTimer>

#include "about.h"
#include "version.h"
#include <chrono>

using namespace std::chrono_literals;

enum Tab { Administration, Options, Copy, AddRemoveGroup, GroupMembership, MAX };

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setWindowIcon(QApplication::windowIcon());

    shell = new Cmd(this);
    tabWidget->blockSignals(true);
    tabWidget->setCurrentIndex(Tab::Administration);
    tabWidget->blockSignals(false);
    QSize size = this->size();
    restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    if (this->isMaximized()) { // if started maximized give option to resize to normal window size
        this->resize(size);
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - this->width()) / 2;
        int y = (screenGeometry.height() - this->height()) / 2;
        this->move(x, y);
    }
    refresh();
}

MainWindow::~MainWindow() { settings.setValue(QStringLiteral("geometry"), saveGeometry()); }

bool MainWindow::replaceStringInFile(const QString &oldtext, const QString &newtext, const QString &filepath)
{

    QString cmd = QStringLiteral("sed -i 's/%1/%2/g' %3").arg(oldtext, newtext, filepath);
    return system(cmd.toUtf8()) == 0;
}

void MainWindow::refresh()
{
    setCursor(QCursor(Qt::ArrowCursor));
    syncProgressBar->setValue(0);

    switch (tabWidget->currentIndex()) {
    case Tab::Options:
        refreshOptions();
        buttonApply->setEnabled(false);
        break;
    case Tab::Copy:
        refreshCopy();
        buttonApply->setEnabled(true);
        break;
    case Tab::AddRemoveGroup:
        refreshGroups();
        buttonApply->setEnabled(false);
        break;
    case Tab::GroupMembership:
        refreshMembership();
        buttonApply->setEnabled(false);
        break;
    default:
        refreshAdd();
        refreshDelete();
        refreshChangePass();
        refreshRename();
        users = shell->getCmdOut(QStringLiteral("lslogins --noheadings -u -o user |  grep -vw root"), true)
                    .split(QStringLiteral("\n"));
        users.sort();
        comboRenameUser->addItems(users);
        comboChangePass->addItems(users);
        comboDeleteUser->addItems(users);
        buttonApply->setEnabled(false);
        break;
    }
}

void MainWindow::refreshOptions()
{
    userComboBox->clear();
    userComboBox->addItem(tr("none"));
    userComboBox->addItems(users);
    checkGroups->setChecked(false);
    checkMozilla->setChecked(false);
    radioAutologinNo->setAutoExclusive(false);
    radioAutologinNo->setChecked(false);
    radioAutologinNo->setAutoExclusive(true);
    radioAutologinYes->setAutoExclusive(false);
    radioAutologinYes->setChecked(false);
    radioAutologinYes->setAutoExclusive(true);
    if (system("pgrep lightdm\\|sddm") != 0) // if unknown desktop manager
        groupBox->setVisible(false);
}

void MainWindow::refreshCopy()
{
    fromUserComboBox->clear();
    fromUserComboBox->addItems(users);
    const QString logname = shell->getCmdOut(QStringLiteral("logname"), true);
    fromUserComboBox->setCurrentIndex(fromUserComboBox->findText(logname));
    copyRadioButton->setChecked(true);
    entireRadioButton->setChecked(true);
    QStringList items = users;
    items.removeAll(logname);
    items.removeAll(fromUserComboBox->currentText());
    items.sort();
    toUserComboBox->clear();
    toUserComboBox->addItems(items);
    if (items.isEmpty())
        toUserComboBox->addItem(QString());
    toUserComboBox->addItem(tr("browse..."));
}

void MainWindow::refreshAdd()
{
    userNameEdit->clear();
    userPasswordEdit->clear();
    userPassword2Edit->clear();
    addUserBox->setEnabled(true);
}

void MainWindow::refreshDelete()
{
    comboDeleteUser->clear();
    comboDeleteUser->addItem(tr("none"));
    deleteHomeCheckBox->setChecked(false);
    deleteUserBox->setEnabled(true);
}

void MainWindow::refreshChangePass()
{
    comboChangePass->clear();
    comboChangePass->addItem(tr("none"));
    changePasswordBox->setEnabled(true);
    comboChangePass->addItem(QStringLiteral("root"));
    lineEditChangePass->clear();
    lineEditChangePassConf->clear();
}

void MainWindow::refreshGroups()
{
    groupNameEdit->clear();
    addBox->setEnabled(true);
    deleteBox->setEnabled(true);
    buildListGroupsToRemove();
}

void MainWindow::refreshMembership()
{
    userComboMembership->clear();
    userComboMembership->addItem(tr("none"));
    listGroups->clear();
    userComboMembership->addItems(users);
}

void MainWindow::refreshRename()
{
    renameUserNameEdit->clear();
    comboRenameUser->clear();
    comboRenameUser->addItem(tr("none"));
    renameUserBox->setEnabled(true);
}

// Apply but do not close
void MainWindow::applyOptions()
{
    const QString user = userComboBox->currentText();
    if (user == (tr("none"))) // no user selected
        return;
    QString home = user;
    if (user != QLatin1String("root"))
        home = QStringLiteral("/home/%1").arg(user);

    if (checkGroups->isChecked() || checkMozilla->isChecked()) {
        int ans = QMessageBox::warning(
            this, windowTitle(),
            tr("The user configuration will be repaired. Please close all other applications now. When finished, "
               "please logout or reboot. Are you sure you want to repair now?"),
            QMessageBox::Yes, QMessageBox::No);
        if (ans != QMessageBox::Yes)
            return;
    }
    setCursor(QCursor(Qt::WaitCursor));

    QString cmd;
    // restore groups
    if (checkGroups->isChecked() && user != QLatin1String("root")) {
        buildListGroups();
        cmd = QStringLiteral(
            "sed -n '/^EXTRA_GROUPS=/s/^EXTRA_GROUPS=//p' /etc/adduser.conf | sed  -e 's/ /,/g' -e 's/\"//g'");
        QStringList extra_groups_list = shell->getCmdOut(cmd).split(QStringLiteral(","));
        QStringList new_group_list;
        for (const QString &group : extra_groups_list)
            if (!listGroups->findItems(group, Qt::MatchExactly).isEmpty())
                new_group_list << group;
        cmd = "usermod -G '' " + user + "; usermod -G " + new_group_list.join(QStringLiteral(",")) + " "
              + user; // reset group and add extra from /etc/adduser.conf
        system(cmd.toUtf8());
        QMessageBox::information(this, windowTitle(), tr("User group membership was restored."));
    }
    // restore Mozilla configs
    if (checkMozilla->isChecked()) {
        cmd = QStringLiteral("rm -r %1/.mozilla").arg(home);
        system(cmd.toUtf8());
        QMessageBox::information(this, windowTitle(), tr("Mozilla settings were reset."));
    }
    if (radioAutologinNo->isChecked()) {
        if (QFile::exists(QStringLiteral("/etc/lightdm/lightdm.conf"))) {
            cmd = QStringLiteral("sed -i -r '/^autologin-user=%1/d' /etc/lightdm/lightdm.conf").arg(user);
            system(cmd.toUtf8());
        }
        if (QFile::exists(QStringLiteral("/etc/sddm.conf")))
            system(QStringLiteral("sed -i 's/^User=%1/User=/' /etc/sddm.conf").arg(user).toUtf8());
        QMessageBox::information(this, tr("Autologin options"),
                                 (tr("Autologin has been disabled for the '%1' account.").arg(user)));
    } else if (radioAutologinYes->isChecked()) {
        if (QFile::exists(QStringLiteral("/etc/lightdm/lightdm.conf"))) {
            cmd = QStringLiteral("sed -i -r '/^autologin-user=/d; /^[[]SeatDefaults[]]/aautologin-user=%1' "
                                 "/etc/lightdm/lightdm.conf")
                      .arg(user);
            system(cmd.toUtf8());
        }
        if (QFile::exists(QStringLiteral("/etc/sddm.conf")))
            system(QStringLiteral("sed -i 's/^User=.*/User=%1/' /etc/sddm.conf").arg(user).toUtf8());
        QMessageBox::information(this, tr("Autologin options"),
                                 (tr("Autologin has been enabled for the '%1' account.").arg(user)));
    }
    setCursor(QCursor(Qt::ArrowCursor));
    buttonApply->setEnabled(false);
}

void MainWindow::applyDesktop()
{

    if (toUserComboBox->currentText().isEmpty()) {
        QMessageBox::information(
            this, windowTitle(),
            tr("You must specify a 'copy to' destination. You cannot copy to the desktop you are logged in to."));
        return;
    }
    // verify
    if (QMessageBox::Yes
        != QMessageBox::critical(
            this, windowTitle(),
            tr("Before copying, close all other applications. Be sure the copy to destination is large enough to "
               "contain the files you are copying. Copying between desktops may overwrite or delete your files or "
               "preferences on the destination desktop. Are you sure you want to proceed?"),
            QMessageBox::Yes, QMessageBox::No))
        return;

    QString fromDir = QStringLiteral("/home/%1").arg(fromUserComboBox->currentText());
    QString toDir = QStringLiteral("/home/%1").arg(toUserComboBox->currentText());
    if (toUserComboBox->currentText().contains(QLatin1String("/"))) // if a directory rather than a user name
        toDir = toUserComboBox->currentText();
    if (docsRadioButton->isChecked()) {
        fromDir.append("/Documents");
        toDir.append("/Documents");
    } else if (mozillaRadioButton->isChecked()) {
        fromDir.append("/.mozilla");
        toDir.append("/.mozilla");
    } else if (sharedRadioButton->isChecked()) {
        fromDir.append("/Shared");
        toDir.append("/Shared");
    }
    fromDir.append("/");

    setCursor(QCursor(Qt::WaitCursor));
    if (syncRadioButton->isChecked())
        syncStatusEdit->setText(tr("Synchronizing desktop..."));
    else
        syncStatusEdit->setText(tr("Copying desktop..."));
    QString cmd = QStringLiteral("rsync -qa ");
    if (syncRadioButton->isChecked())
        cmd.append("--delete-after ");
    cmd.append(fromDir);
    cmd.append(" ");
    cmd.append(toDir);
    QTimer timer;
    timer.start(100ms);
    connect(&timer, &QTimer::timeout, this, &MainWindow::progress);

    for (int tab = 0; tab < Tab::MAX; ++tab) {
        if (tab == Tab::Copy)
            continue;
        tabWidget->setTabEnabled(tab, false);
    }
    syncDone(shell->run(cmd));
    for (int tab = 0; tab < Tab::MAX; ++tab)
        tabWidget->setTabEnabled(tab, true);
}

void MainWindow::applyAdd()
{
    // validate data before proceeding
    // see if username is reasonable length
    if (userNameEdit->text().length() < 2) {
        QMessageBox::critical(
            this, windowTitle(),
            tr("The user name needs to be at least 2 characters long. Please select a longer name before proceeding."));
        return;
    } else if (!userNameEdit->text().contains(QRegularExpression(QStringLiteral("^[A-Za-z_][A-Za-z0-9_-]*[$]?$")))) {
        QMessageBox::critical(this, windowTitle(),
                              tr("The user name cannot contain special characters or spaces.\n"
                                 "Please choose another name before proceeding."));
        return;
    }
    // check that user name is not already used
    QString cmd = QStringLiteral("grep -w '^%1' /etc/passwd >/dev/null").arg(userNameEdit->text());
    if (system(cmd.toUtf8()) == 0) {
        QMessageBox::critical(this, windowTitle(), tr("Sorry, this name is in use. Please enter a different name."));
        return;
    }
    if (userPasswordEdit->text() != userPassword2Edit->text()) {
        QMessageBox::critical(this, windowTitle(), tr("Password entries do not match. Please try again."));
        return;
    }
    if (userPasswordEdit->text().length() < 2) {
        QMessageBox::critical(
            this, windowTitle(),
            tr("Password needs to be at least 2 characters long. Please enter a longer password before proceeding."));
        return;
    }

    cmd = QStringLiteral("adduser --disabled-login --force-badname --gecos %1 %1").arg(userNameEdit->text());
    system(cmd.toUtf8());

    QProcess proc;
    proc.start(QStringLiteral("passwd"), QStringList {userNameEdit->text()}, QIODevice::ReadWrite);
    proc.waitForStarted();
    proc.write(userPasswordEdit->text().toUtf8() + "\n");
    proc.write(userPasswordEdit->text().toUtf8() + "\n");
    proc.waitForFinished();

    if (proc.exitCode() == 0) {
        QMessageBox::information(this, windowTitle(), tr("The user was added ok."));
        refresh();
    } else {
        QMessageBox::critical(this, windowTitle(), tr("Failed to add the user."));
    }
}

// change user password
void MainWindow::applyChangePass()
{
    if (lineEditChangePass->text() != lineEditChangePassConf->text()) {
        QMessageBox::critical(this, windowTitle(), tr("Password entries do not match. Please try again."));
        return;
    }
    if (lineEditChangePass->text().length() < 2) {
        QMessageBox::critical(
            this, windowTitle(),
            tr("Password needs to be at least 2 characters long. Please enter a longer password before proceeding."));
        return;
    }

    QProcess proc;
    proc.start(QStringLiteral("passwd"), QStringList {comboChangePass->currentText()}, QIODevice::ReadWrite);
    proc.waitForStarted();
    proc.write(lineEditChangePass->text().toUtf8() + "\n");
    proc.write(lineEditChangePass->text().toUtf8() + "\n");
    proc.waitForFinished();

    if (proc.exitCode() == 0) {
        QMessageBox::information(this, windowTitle(), tr("Password successfully changed."));
        refresh();
    } else {
        QMessageBox::critical(this, windowTitle(), tr("Failed to change password."));
    }
}

void MainWindow::applyDelete()
{
    QString cmd = QString(tr("This action cannot be undone. Are you sure you want to delete user %1?"))
                      .arg(comboDeleteUser->currentText());
    if (QMessageBox::Yes == QMessageBox::warning(this, windowTitle(), cmd, QMessageBox::Yes, QMessageBox::No)) {
        if (deleteHomeCheckBox->isChecked()) {
            cmd = QStringLiteral("killall -u %1").arg(comboDeleteUser->currentText());
            system(cmd.toUtf8());
            cmd = QStringLiteral("deluser --force --remove-home %1").arg(comboDeleteUser->currentText());
        } else {
            cmd = QStringLiteral("deluser %1").arg(comboDeleteUser->currentText());
        }
        if (system(cmd.toUtf8()) == 0)
            QMessageBox::information(this, windowTitle(), tr("The user has been deleted."));
        else
            QMessageBox::critical(this, windowTitle(), tr("Failed to delete the user."));
        refresh();
    }
}

void MainWindow::applyGroup()
{
    // checks if adding or removing groups
    if (addBox->isEnabled()) {
        // validate data before proceeding
        // see if groupname is reasonable length
        if (groupNameEdit->text().length() < 2) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("The group name needs to be at least 2 characters long. Please select a longer "
                                     "name before proceeding."));
            return;
        } else if (!groupNameEdit->text().contains(QRegularExpression(QStringLiteral("^[a-z_][a-z0-9_-]*[$]?$")))) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("The group name needs to be lower case and it \n"
                                     "cannot contain special characters or spaces.\n"
                                     "Please choose another name before proceeding."));
            return;
        }
        // check that group name is not already used
        if (QProcess::execute("grep", {"-w", "^" + groupNameEdit->text(), "/etc/group"}) == 0) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("Sorry, that group name already exists. Please enter a different name."));
            return;
        }
        // run addgroup command
        QString group_user_level = checkGroupUserLevel->checkState() == Qt::Checked
                                       ? "--quiet" // --quiet because it fails if ""
                                       : "--system";
        if (QProcess::execute("addgroup", {groupNameEdit->text(), group_user_level}) == 0)
            QMessageBox::information(this, windowTitle(), tr("The system group was added ok."));
        else
            QMessageBox::critical(this, windowTitle(), tr("Failed to add the system group."));
    } else { // deleting group if addBox disabled
        QStringList groups;
        for (auto i = 0; i < listGroupsToRemove->count(); ++i) {
            if (listGroupsToRemove->item(i)->checkState() == Qt::Checked)
                groups << listGroupsToRemove->item(i)->text();
        }
        if (groups.isEmpty()) {
            refresh();
            return;
        }
        QString msg
            = groups.count() == 1
                  ? QString(tr("This action cannot be undone. Are you sure you want to delete group %1?"))
                        .arg(groups.at(0))
                  : QString(
                        tr("This action cannot be undone. Are you sure you want to delete the following groups: %1?"))
                        .arg(groups.join(" "));
        int ans = QMessageBox::warning(this, windowTitle(), msg, QMessageBox::Yes, QMessageBox::No);
        if (ans == QMessageBox::Yes) {
            for (const auto &group : qAsConst(groups)) {
                if (QProcess::execute("delgroup", {group}) != 0) {
                    QMessageBox::critical(this, windowTitle(),
                                          tr("Failed to delete the group.") + "\n" + tr("Group: %1").arg(group));
                    refresh();
                    return;
                }
            }
            msg = groups.count() == 1 ? tr("The group has been deleted.") : tr("The groups have been deleted.");
            QMessageBox::information(this, windowTitle(), msg);
        }
    }
    refresh();
}

void MainWindow::applyMembership()
{
    QString cmd;
    for (auto i = 0; i < listGroups->count(); ++i) {
        if (listGroups->item(i)->checkState() == Qt::Checked)
            cmd += listGroups->item(i)->text() + ",";
    }
    cmd.chop(1);
    cmd = QStringLiteral("usermod -G %1 %2").arg(cmd, userComboMembership->currentText());
    if (shell->run(cmd))
        QMessageBox::information(this, windowTitle(), tr("The changes have been applied."));
    else
        QMessageBox::critical(this, windowTitle(), tr("Failed to apply group changes"));
}

void MainWindow::applyRename()
{
    const QString old_name = comboRenameUser->currentText();
    const QString new_name = renameUserNameEdit->text();

    // validate data before proceeding
    // check if selected user is in use
    if (shell->getCmdOut(QStringLiteral("logname"), true) == old_name) {
        QMessageBox::critical(
            this, windowTitle(),
            tr("The selected user name is currently in use.") + "\n\n"
                + tr("To rename this user, please log out and log back in using another user account."));
        refresh();
        return;
    }

    // see if username is reasonable length
    if (new_name.length() < 2) {
        QMessageBox::critical(
            this, windowTitle(),
            tr("The user name needs to be at least 2 characters long. Please select a longer name before proceeding."));
        return;
    } else if (!new_name.contains(QRegularExpression(QStringLiteral("^[a-z_][a-z0-9_-]*[$]?$")))) {
        QMessageBox::critical(this, windowTitle(),
                              tr("The user name needs to be lower case and it\n"
                                 "cannot contain special characters or spaces.\n"
                                 "Please choose another name before proceeding."));
        return;
    }
    // check that user name is not already used
    QString cmd = QStringLiteral("grep -w '^%1' /etc/passwd >/dev/null").arg(new_name);
    if (system(cmd.toUtf8()) == 0) {
        QMessageBox::critical(this, windowTitle(),
                              tr("Sorry, this name already exists on your system. Please enter a different name."));
        return;
    }

    // rename user
    bool success = shell->run("usermod --login " + new_name + " --move-home --home /home/" + new_name + " " + old_name);
    if (!success) {
        QMessageBox::critical(this, windowTitle(),
                              tr("Failed to rename the user. Please make sure that the user is not logged in, you "
                                 "might need to restart"));
        return;
    }

    // rename other instances of the old name, like "Finger" name if present
    shell->run("sed -i 's/\\b" + old_name + "\\b/" + new_name + "/g' /etc/passwd");

    // change group
    shell->run("groupmod --new-name " + new_name + " " + old_name);

    // fix "home/old_user" string in all ~/ files
    shell->run(
        QStringLiteral("grep -rl \"home/%1\" /home/%2 | xargs sed -i 's|home/%1|home/%2|g'").arg(old_name, new_name));

    // change autologin name (Xfce and KDE)
    if (QFile::exists(QStringLiteral("/etc/lightdm/lightdm.conf")))
        shell->run(QStringLiteral("sed -i 's/autologin-user=%1/autologin-user=%2/g' /etc/lightdm/lightdm.conf")
                       .arg(old_name, new_name));
    if (QFile::exists(QStringLiteral("/etc/sddm.conf")))
        shell->run(QStringLiteral("sed -i 's/User=%1/User=%2/g' /etc/sddm.conf").arg(old_name, new_name));

    QMessageBox::information(this, windowTitle(), tr("The user was renamed."));
    refresh();
}

void MainWindow::syncDone(bool success)
{
    if (success) {
        QString toDir = QStringLiteral("/home/%1").arg(toUserComboBox->currentText());

        // if a directory rather than a user name
        if (toUserComboBox->currentText().contains(QLatin1String("/"))) {
            if (syncRadioButton->isChecked())
                syncStatusEdit->setText(tr("Synchronizing desktop...ok"));
            else
                syncStatusEdit->setText(tr("Copying desktop...ok"));
            syncProgressBar->setValue(syncProgressBar->maximum());
            setCursor(QCursor(Qt::ArrowCursor));
            return;
        }

        // fix owner
        QString cmd = QStringLiteral("chown -R %1:%1 %2").arg(toUserComboBox->currentText(), toDir);
        system(cmd.toUtf8());

        // fix "home/old_user" string in all ~/ or ~/.mozilla files
        if (entireRadioButton->isChecked())
            cmd = QStringLiteral("grep -rl \"home/%1\" /home/%2 | xargs sed -i 's|home/%1|home/%2|g'")
                      .arg(fromUserComboBox->currentText(), toUserComboBox->currentText());
        else if (mozillaRadioButton->isChecked())
            cmd = QStringLiteral("grep -rl \"home/%1\" /home/%2/.mozilla | xargs sed -i 's|home/%1|home/%2|g'")
                      .arg(fromUserComboBox->currentText(), toUserComboBox->currentText());
        shell->run(cmd);

        if (entireRadioButton->isChecked()) {
            // delete some files
            cmd = QStringLiteral("rm -f %1/.recently-used >/dev/null").arg(toDir);
            system(cmd.toUtf8());
            cmd = QStringLiteral("rm -f %1/.openoffice.org/*/.lock >/dev/null").arg(toDir);
            system(cmd.toUtf8());
        }
        if (syncRadioButton->isChecked())
            syncStatusEdit->setText(tr("Synchronizing desktop...ok"));
        else
            syncStatusEdit->setText(tr("Copying desktop...ok"));
    } else {
        if (syncRadioButton->isChecked())
            syncStatusEdit->setText(tr("Synchronizing desktop...failed"));
        else
            syncStatusEdit->setText(tr("Copying desktop...failed"));
    }
    syncProgressBar->setValue(syncProgressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        closeApp();
}

void MainWindow::closeApp()
{
    if (shell->state() != QProcess::NotRunning) {
        setCursor(QCursor(Qt::ArrowCursor));
        if (QMessageBox::Yes
            != QMessageBox::question(this, tr("Confirmation"),
                                     tr("Process not done. Are you sure you want to quit the application?"),
                                     QMessageBox::Yes | QMessageBox::No)) {
            setCursor(QCursor(Qt::WaitCursor));
            return;
        }
    }
    close();
}

void MainWindow::on_fromUserComboBox_activated(const QString & /*unused*/)
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
    QStringList items = users;
    items.removeAll(shell->getCmdOut(QStringLiteral("logname"), true));
    items.removeAll(fromUserComboBox->currentText());
    items.sort();
    toUserComboBox->clear();
    toUserComboBox->addItems(items);
    if (items.isEmpty())
        toUserComboBox->addItem(QString());
    toUserComboBox->addItem(tr("browse..."));
}

void MainWindow::on_userComboBox_activated(const QString & /*unused*/)
{
    buttonApply->setDisabled(true);
    radioAutologinNo->setAutoExclusive(false);
    radioAutologinNo->setChecked(false);
    radioAutologinNo->setAutoExclusive(true);
    radioAutologinYes->setAutoExclusive(false);
    radioAutologinYes->setChecked(false);
    radioAutologinYes->setAutoExclusive(true);
    QString user = userComboBox->currentText();
    if (user == (tr("none")))
        return;
    if (system("pgrep lightdm") == 0) {
        const QString cmd = QStringLiteral("grep -qw ^autologin-user=%1 /etc/lightdm/lightdm.conf").arg(user);
        if (system(cmd.toUtf8()) == 0)
            radioAutologinYes->setChecked(true);
        else
            radioAutologinNo->setChecked(true);
    } else if (system("pgrep sddm") == 0) {
        QSettings sddm_settings(QStringLiteral("/etc/sddm.conf"), QSettings::NativeFormat);
        qDebug() << "TEST" << sddm_settings.value(QStringLiteral("Autologin/User")).toString();
        if (sddm_settings.value(QStringLiteral("Autologin/User")).toString() == user)
            radioAutologinYes->setChecked(true);
        else
            radioAutologinNo->setChecked(true);
    }
}

void MainWindow::on_comboDeleteUser_activated(const QString & /*unused*/)
{
    addUserBox->setEnabled(false);
    changePasswordBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (comboDeleteUser->currentText() == tr("none"))
        refresh();
}

void MainWindow::on_userNameEdit_textEdited()
{
    deleteUserBox->setEnabled(false);
    changePasswordBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (userNameEdit->text().isEmpty())
        refresh();
}

void MainWindow::on_groupNameEdit_textEdited()
{
    deleteBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (groupNameEdit->text().isEmpty())
        refresh();
}

void MainWindow::on_userComboMembership_activated(const QString & /*unused*/)
{
    buildListGroups();
    buttonApply->setEnabled(true);
    if (userComboMembership->currentText() == tr("none"))
        refresh();
}

void MainWindow::buildListGroups()
{
    listGroups->clear();
    // read /etc/group and add all the groups in the listGroups
    QStringList groups = shell->getCmdOut(QStringLiteral("cat /etc/group | cut -f 1 -d :")).split(QStringLiteral("\n"));
    groups.sort();
    for (const QString &group : groups) {
        auto *item = new QListWidgetItem;
        item->setText(group);
        item->setCheckState(Qt::Unchecked);
        listGroups->addItem(item);
    }
    // check the boxes for the groups that the current user belongs to
    const QString cmd = QStringLiteral("id -nG %1").arg(userComboMembership->currentText());
    const QString out = shell->getCmdOut(cmd);
    QStringList out_tok = out.split(QStringLiteral(" "));
    while (!out_tok.isEmpty()) {
        QString text = out_tok.takeFirst();
        auto list = listGroups->findItems(text, Qt::MatchExactly);
        while (!list.isEmpty())
            list.takeFirst()->setCheckState(Qt::Checked);
    }
}

void MainWindow::buildListGroupsToRemove()
{
    listGroupsToRemove->clear();
    QFile file("/etc/group");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Can't open /etc/group";
        exit(EXIT_FAILURE);
    }
    QStringList groups = QString(file.readAll()).split("\n");
    groups.sort();
    for (const auto &group : qAsConst(groups)) {
        if (!group.isEmpty()) {
            auto *item = new QListWidgetItem;
            item->setText(group.section(":", 0, 0));
            if (item->text() == "root")
                continue;
            item->setCheckState(Qt::Unchecked);
            listGroupsToRemove->addItem(item);
        }
    }
    disconnect(listGroupsToRemove, &QListWidget::itemChanged, nullptr, nullptr);
    connect(listGroupsToRemove, &QListWidget::itemChanged, [this](QListWidgetItem *item) {
        buttonApply->setEnabled(true);
        addBox->setDisabled(true);
        if (item->checkState() == Qt::Checked)
            return;
        QStringList items;
        for (auto i = 0; i < listGroupsToRemove->count(); ++i) {
            if (listGroupsToRemove->item(i)->checkState() == Qt::Checked)
                return;
        }
        refresh();
    });
}

// apply but do not close
void MainWindow::on_buttonApply_clicked()
{
    if (!buttonApply->isEnabled())
        return;

    switch (tabWidget->currentIndex()) {
    case Tab::Options:
        setCursor(QCursor(Qt::WaitCursor));
        applyOptions();
        setCursor(QCursor(Qt::ArrowCursor));
        buttonApply->setEnabled(false);
        break;
    case Tab::Copy:
        applyDesktop();
        buttonApply->setEnabled(false);
        break;
    case Tab::AddRemoveGroup:
        setCursor(QCursor(Qt::WaitCursor));
        applyGroup();
        setCursor(QCursor(Qt::ArrowCursor));
        buttonApply->setEnabled(false);
        break;
    case Tab::GroupMembership:
        setCursor(QCursor(Qt::WaitCursor));
        applyMembership();
        setCursor(QCursor(Qt::ArrowCursor));
        break;
    default:
        setCursor(QCursor(Qt::WaitCursor));
        if (addUserBox->isEnabled()) {
            applyAdd();
        } else if (deleteUserBox->isEnabled()) {
            applyDelete();
            buttonApply->setEnabled(false);
        } else if (changePasswordBox->isEnabled()) {
            applyChangePass();
        } else if (renameUserBox->isEnabled()) {
            applyRename();
        }
        setCursor(QCursor(Qt::ArrowCursor));
        break;
    }
}

void MainWindow::on_tabWidget_currentChanged() { refresh(); }

// close but do not apply
void MainWindow::on_buttonCancel_clicked() { closeApp(); }

void MainWindow::progress() { syncProgressBar->setValue((syncProgressBar->value() + 1) % syncProgressBar->maximum()); }

// show about
void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(this->windowTitle()),
        "<p align=\"center\"><b><h2>" + this->windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + VERSION + "</p><p align=\"center\"><h3>" + tr("Simple user configuration for MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-user/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

// Help button clicked
void MainWindow::on_buttonHelp_clicked()
{
    QLocale locale;
    const QString lang = locale.bcp47Name();

    QString url = QStringLiteral("/usr/share/doc/mx-user/mx-user.html");

    if (lang.startsWith(QLatin1String("fr")))
        url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-gestionnaire-des-utilisateurs");
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::restartPanel(const QString &user)
{
    const QString cmd = QStringLiteral("pkill xfconfd; sudo -Eu %1 bash -c 'xfce4-panel -r'").arg(user);
    system(cmd.toUtf8());
}

void MainWindow::on_comboChangePass_activated(const QString & /*unused*/)
{
    addUserBox->setEnabled(false);
    deleteUserBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (comboChangePass->currentText() == tr("none"))
        refresh();
}

void MainWindow::on_toUserComboBox_activated(const QString &arg1)
{
    if (arg1 == tr("browse...")) {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder to copy to"), QStringLiteral("/"),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            toUserComboBox->removeItem(toUserComboBox->currentIndex());
            if (toUserComboBox->findText(dir, Qt::MatchExactly | Qt::MatchCaseSensitive) == -1)
                toUserComboBox->addItem(dir);
            toUserComboBox->setCurrentIndex(toUserComboBox->findText(dir, Qt::MatchExactly | Qt::MatchCaseSensitive));
            toUserComboBox->addItem(tr("browse..."));
        } else {
            toUserComboBox->setCurrentIndex(toUserComboBox->currentIndex() - 1);
        }
    }
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_copyRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_syncRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_entireRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_docsRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_mozillaRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_sharedRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::on_userPassword2Edit_textChanged(const QString &arg1)
{
    QPalette pal = userPassword2Edit->palette();
    if (arg1 != userPasswordEdit->text())
        pal.setColor(QPalette::Base, QColor(255, 0, 0, 20));
    else
        pal.setColor(QPalette::Base, QColor(0, 255, 0, 10));
    userPasswordEdit->setPalette(pal);
    userPassword2Edit->setPalette(pal);
}

void MainWindow::on_lineEditChangePassConf_textChanged(const QString &arg1)
{
    QPalette pal = lineEditChangePassConf->palette();
    if (arg1 != lineEditChangePass->text())
        pal.setColor(QPalette::Base, QColor(255, 0, 0, 20));
    else
        pal.setColor(QPalette::Base, QColor(0, 255, 0, 10));
    lineEditChangePassConf->setPalette(pal);
    lineEditChangePass->setPalette(pal);
}

void MainWindow::on_userPasswordEdit_textChanged()
{
    userPassword2Edit->clear();
    userPasswordEdit->setPalette(QApplication::palette());
    userPassword2Edit->setPalette(QApplication::palette());
}

void MainWindow::on_lineEditChangePass_textChanged()
{
    lineEditChangePassConf->clear();
    lineEditChangePass->setPalette(QApplication::palette());
    lineEditChangePassConf->setPalette(QApplication::palette());
}

void MainWindow::on_comboRenameUser_activated(const QString & /*unused*/)
{
    addUserBox->setEnabled(false);
    changePasswordBox->setEnabled(false);
    deleteUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (comboRenameUser->currentText() == tr("none"))
        refresh();
}

void MainWindow::on_checkGroups_stateChanged(int /*unused*/)
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

void MainWindow::on_checkMozilla_stateChanged(int /*unused*/)
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

void MainWindow::on_radioAutologinYes_clicked() { buttonApply->setEnabled(userComboBox->currentText() != tr("none")); }

void MainWindow::on_radioAutologinNo_clicked() { buttonApply->setEnabled(userComboBox->currentText() != tr("none")); }

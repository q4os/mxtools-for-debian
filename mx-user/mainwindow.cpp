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
#include "passedit.h"

#include <QDebug>
#include <QFileDialog>
#include <QScreen>
#include <QTextEdit>
#include <QTimer>

#include "about.h"
#include "version.h"
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    setupUi(this);
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    setWindowIcon(QApplication::windowIcon());
    passUser = new PassEdit(userPasswordEdit, userPassword2Edit, 1, this);
    passChange = new PassEdit(lineEditChangePass, lineEditChangePassConf, 1, this);

    shell = new Cmd(this);
    tabWidget->blockSignals(true);
    tabWidget->setCurrentIndex(Tab::Administration);
    tabWidget->blockSignals(false);
    QSize size = this->size();
    restoreGeometry(settings.value("geometry").toByteArray());
    if (isMaximized()) { // If started maximized give option to resize to normal window size
        resize(size);
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
    refresh();
    setConnections();
}

MainWindow::~MainWindow()
{
    settings.setValue("geometry", saveGeometry());
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
        users = shell->getOut("lslogins --noheadings -u -o user | grep -vw root", true).trimmed().split('\n');
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
    userComboBox->addItems(users);
    checkGroups->setChecked(false);
    checkMozilla->setChecked(false);
    radioAutologinNo->setAutoExclusive(false);
    radioAutologinNo->setChecked(false);
    radioAutologinNo->setAutoExclusive(true);
    radioAutologinYes->setAutoExclusive(false);
    radioAutologinYes->setChecked(false);
    radioAutologinYes->setAutoExclusive(true);
    if ((QProcess::execute("pgrep", {"lightdm"}) != 0) && QProcess::execute("pgrep", {"sddm"}) != 0) {
        groupBox->setVisible(false);
    }
}

void MainWindow::refreshCopy()
{
    fromUserComboBox->clear();
    fromUserComboBox->addItems(users);
    QString logname = QString::fromUtf8(qgetenv("SUDO_USER")).trimmed();
    if (logname.isEmpty()) {
        logname = QString::fromUtf8(qgetenv("LOGNAME")).trimmed();
    }
    fromUserComboBox->setCurrentIndex(fromUserComboBox->findText(logname));
    copyRadioButton->setChecked(true);
    entireRadioButton->setChecked(true);
    QStringList items = users;
    items.removeAll(logname);
    items.removeAll(fromUserComboBox->currentText());
    items.sort();
    toUserComboBox->clear();
    toUserComboBox->addItems(items);
    if (items.isEmpty()) {
        toUserComboBox->addItem(QString());
    }
    toUserComboBox->addItem(tr("browse..."));
}

void MainWindow::refreshAdd()
{
    userNameEdit->clear();
    userPasswordEdit->clear();
    userPassword2Edit->clear();
    addUserBox->setEnabled(true);
    checkSudoGroup->setVisible(shell->run("grep -q '^EXTRA_GROUPS=.*\\<sudo\\>' /etc/adduser.conf", true));
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
    comboChangePass->addItem("root");
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
    listGroups->clear();
    userComboMembership->addItems(users);
    buildListGroups();
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
    QString home = user;
    if (user != "root") {
        home = QString("/home/%1").arg(user);
    }

    if (checkGroups->isChecked() || checkMozilla->isChecked()) {
        int ans = QMessageBox::warning(
            this, windowTitle(),
            tr("The user configuration will be repaired. Please close all other applications now. When finished, "
               "please logout or reboot. Are you sure you want to repair now?"),
            QMessageBox::Yes, QMessageBox::No);
        if (ans != QMessageBox::Yes) {
            return;
        }
    }
    setCursor(QCursor(Qt::WaitCursor));

    // Restore groups
    if (checkGroups->isChecked() && user != "root") {
        buildListGroups();
        QString cmd = "sed -n '/^EXTRA_GROUPS=/s/^EXTRA_GROUPS=//p' /etc/adduser.conf | sed  -e 's/ /,/g' -e 's/\"//g'";
        QStringList extra_groups_list = shell->getOutAsRoot(cmd).split(',');
        QStringList new_group_list;
        for (const QString &group : extra_groups_list) {
            if (!listGroups->findItems(group, Qt::MatchExactly).isEmpty()) {
                new_group_list << group;
            }
        }
        shell->runAsRoot("usermod -G '' " + user);
        shell->runAsRoot("usermod -G " + new_group_list.join(',') + ' ' + user);
        QMessageBox::information(this, windowTitle(), tr("User group membership was restored."));
    }
    // Reset Mozilla configs
    if (checkMozilla->isChecked()) {
        shell->runAsRoot("rm -r " + home + "/.mozilla");
        QMessageBox::information(this, windowTitle(), tr("Mozilla settings were reset."));
    }
    if (radioAutologinNo->isChecked()) {
        if (QFile::exists("/etc/lightdm/lightdm.conf")) {
            shell->runAsRoot("sed -i -E "
                             + QString(R"('/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=/d;'})")
                             + " /etc/lightdm/lightdm.conf");
        }

        if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
            shell->runAsRoot("sed -i " + QString("s/^User=%1/User=/").arg(user)
                             + " /etc/sddm.conf.d/kde_settings.conf");
        } else if (QFile::exists("/etc/sddm.conf")) {
            shell->runAsRoot("sed -i " + QString("s/^User=%1/User=/").arg(user) + " /etc/sddm.conf");
        }
        QMessageBox::information(this, tr("Autologin options"),
                                 (tr("Autologin has been disabled for the '%1' account.").arg(user)));
    } else if (radioAutologinYes->isChecked()) {
        if (QFile::exists("/etc/lightdm/lightdm.conf")) {
            shell->runAsRoot("sed -i -E -e "
                             + QString(R"('/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=/d;}')")
                             + " -e " + QString(R"('/^[[]Seat(Defaults|:[*])[]]/aautologin-user=%1')").arg(user)
                             + " /etc/lightdm/lightdm.conf");
        }
        if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
            shell->runAsRoot("sed -i " + QString("s/^User=.*/User=%1/").arg(user)
                             + " /etc/sddm.conf.d/kde_settings.conf");
            if (qEnvironmentVariable("XDG_CURRENT_DESKTOP") == "KDE") {
                QString sessionType;
                if (qEnvironmentVariable("XDG_SESSION_TYPE") == "wayland") {
                    sessionType = "plasma";
                } else if (qEnvironmentVariable("XDG_SESSION_TYPE") == "x11") {
                    sessionType = "plasmax11";
                }

                if (!sessionType.isEmpty()) {
                    shell->runAsRoot(QString("sed -i 's/^Session=.*/Session=%1/' /etc/sddm.conf.d/kde_settings.conf")
                                         .arg(sessionType));
                }
            }
        } else if (QFile::exists("/etc/sddm.conf")) {
            shell->runAsRoot(QString("sed -i 's/^User=.*/User=%1/' /etc/sddm.conf").arg(user));
            if (qEnvironmentVariable("XDG_CURRENT_DESKTOP") == "KDE") {
                shell->runAsRoot("sed -i 's/^Session=.*/Session=plasma.desktop/' /etc/sddm.conf");
            }
        }
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
    if (QMessageBox::Yes
        != QMessageBox::critical(
            this, windowTitle(),
            tr("Before copying, close all other applications. Be sure the copy to destination is large enough to "
               "contain the files you are copying. Copying between desktops may overwrite or delete your files or "
               "preferences on the destination desktop. Are you sure you want to proceed?"),
            QMessageBox::Yes, QMessageBox::No)) {
        return;
    }

    QString fromDir = QString("/home/%1").arg(fromUserComboBox->currentText());
    QString toDir = QString("/home/%1").arg(toUserComboBox->currentText());
    if (toUserComboBox->currentText().contains('/')) { // If a directory rather than a user name
        toDir = toUserComboBox->currentText();
    }
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
    fromDir.append('/');

    setCursor(QCursor(Qt::WaitCursor));
    if (syncRadioButton->isChecked()) {
        syncStatusEdit->setText(tr("Synchronizing desktop..."));
    } else {
        syncStatusEdit->setText(tr("Copying desktop..."));
    }
    QString cmd = "rsync -a --info=progress2 ";
    if (syncRadioButton->isChecked()) {
        cmd.append("--delete-after ");
    }
    cmd.append(fromDir);
    cmd.append(' ');
    cmd.append(toDir);
    for (int tab = 0; tab < Tab::MAX; ++tab) {
        if (tab == Tab::Copy) {
            continue;
        }
        tabWidget->setTabEnabled(tab, false);
    }
    connect(shell, &Cmd::outputAvailable, this, [this](const QString &out) {
        QRegularExpression regex("\\b(\\d+)%");
        QRegularExpressionMatch match = regex.match(out);
        if (match.hasMatch()) {
            bool ok = false;
            int percent = match.captured(1).toInt(&ok);
            if (ok && percent > syncProgressBar->value()) {
                syncProgressBar->setValue(percent);
            }
        }
    });
    syncDone(shell->runAsRoot(cmd));
    for (int tab = 0; tab < Tab::MAX; ++tab) {
        tabWidget->setTabEnabled(tab, true);
    }
}

void MainWindow::applyAdd()
{
    // Validate data before proceeding, see if username is reasonable length
    if (userNameEdit->text().length() < 2) {
        QMessageBox::critical(this, windowTitle(),
                              tr("The user name needs to be at least 2 characters long. Please select a longer "
                                 "name before proceeding."));
        return;
    } else if (!userNameEdit->text().contains(QRegularExpression("^[A-Za-z_][A-Za-z0-9_-]*[$]?$"))) {
        QMessageBox::critical(this, windowTitle(),
                              tr("The user name cannot contain special characters or spaces.\n"
                                 "Please choose another name before proceeding."));
        return;
    }
    // Check that user name is not already used
    if (QProcess::execute("grep", {"-E", "^" + userNameEdit->text() + ":", "/etc/passwd"}) == 0) {
        QMessageBox::critical(this, windowTitle(), tr("Sorry, this name is in use. Please enter a different name."));
        return;
    }
    if (!passUser->confirmed()) {
        QMessageBox::critical(this, windowTitle(), tr("Password entries do not match. Please try again."));
        return;
    }
    if (!passUser->lengthOK()) {
        QMessageBox::critical(this, windowTitle(),
                              tr("Password needs to be at least 2 characters long. Please enter a longer password "
                                 "before proceeding."));
        return;
    }

    QString cmd {"sed -n '/^DSHELL=/{ s///; s:^/bin/:/usr/bin/:; h}; ${x; p}' /etc/adduser.conf"};
    QString dshell = shell->getOutAsRoot(cmd, true).trimmed();
    if (!QFile::exists(dshell)) {
        dshell = "/usr/bin/bash";
    }

    cmd = "LC_ALL=C adduser --help 2>/dev/null | grep -m1 -o -- --comment | head -1";
    QString commentOption = shell->getOutAsRoot(cmd, true).trimmed();
    if (commentOption != "--comment") {
        commentOption = "--gecos";
    }

    cmd = "LC_ALL=C adduser --help 2>/dev/null | grep -m1 -o -- --allow-bad-names | head -1";
    QString allowBadNames = shell->getOutAsRoot(cmd, true).trimmed();
    if (allowBadNames != "--allow-bad-names") {
        allowBadNames = "--force-badname";
    }

    shell->runAsRoot("adduser --disabled-login " + allowBadNames + " --shell " + dshell + " " + commentOption + " "
                     + userNameEdit->text() + ' ' + userNameEdit->text());

    QProcess proc;
    QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"};
    QString helper {"/usr/lib/" + QApplication::applicationName() + "/helper"};
    QString userNameArg = userNameEdit->text();
    proc.start(elevate, {helper, "passwd", userNameArg}, QIODevice::ReadWrite);
    proc.waitForStarted();
    proc.write(userPasswordEdit->text().toUtf8() + '\n');
    proc.write(userPasswordEdit->text().toUtf8() + '\n');
    proc.waitForFinished();

    if (proc.exitCode() == 0) {
        if (!checkSudoGroup->isChecked()) {
            shell->runAsRoot("delgroup " + userNameArg + " sudo");
        }
        QMessageBox::information(this, windowTitle(), tr("The user was added ok."));
        refresh();
    } else {
        QMessageBox::critical(this, windowTitle(), tr("Failed to add the user."));
    }
}

// Change user password
void MainWindow::applyChangePass()
{
    if (!passChange->confirmed()) {
        QMessageBox::critical(this, windowTitle(), tr("Password entries do not match. Please try again."));
        return;
    }
    if (!passChange->lengthOK()) {
        QMessageBox::critical(this, windowTitle(),
                              tr("Password needs to be at least 2 characters long. Please enter a longer password "
                                 "before proceeding."));
        return;
    }

    QProcess proc;
    QString elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"};
    QString helper {"/usr/lib/" + QApplication::applicationName() + "/helper"};
    proc.start(elevate, {helper, "passwd", comboChangePass->currentText()}, QIODevice::ReadWrite);
    proc.waitForStarted();
    proc.write(lineEditChangePass->text().toUtf8() + '\n');
    proc.write(lineEditChangePass->text().toUtf8() + '\n');
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
    QString msg = tr("This action cannot be undone. Are you sure you want to delete user %1?")
                      .arg(comboDeleteUser->currentText());
    if (QMessageBox::Yes == QMessageBox::warning(this, windowTitle(), msg, QMessageBox::Yes, QMessageBox::No)) {
        QString cmd;
        if (deleteHomeCheckBox->isChecked()) {
            shell->runAsRoot("timeout 5s killall -w -u " + comboDeleteUser->currentText()
                             + "; timeout 5s killall -9 -w -u " + comboDeleteUser->currentText());
            cmd = QString("deluser --remove-home %1").arg(comboDeleteUser->currentText());
        } else {
            cmd = QString("deluser %1").arg(comboDeleteUser->currentText());
        }
        if (shell->runAsRoot(cmd)) {
            if (QFile::exists("/etc/lightdm/lightdm.conf")) {
                shell->runAsRoot(
                    "sed -i -E -e "
                    + QString(R"('/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=%1$/d;}')")
                          .arg(comboDeleteUser->currentText())
                    + " /etc/lightdm/lightdm.conf");
            }
            QMessageBox::information(this, windowTitle(), tr("The user has been deleted."));
        } else {
            QMessageBox::critical(this, windowTitle(), tr("Failed to delete the user."));
        }
        refresh();
    }
}

void MainWindow::applyGroup()
{
    // Checks if adding or removing groups
    if (addBox->isEnabled()) {
        // validate data before proceeding
        // see if groupname is reasonable length
        if (groupNameEdit->text().length() < 2) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("The group name needs to be at least 2 characters long. Please select a longer "
                                     "name before proceeding."));
            return;
        } else if (!groupNameEdit->text().contains(QRegularExpression("^[a-z_][a-z0-9_-]*[$]?$"))) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("The group name needs to be lower case and it \n"
                                     "cannot contain special characters or spaces.\n"
                                     "Please choose another name before proceeding."));
            return;
        }
        // Check that group name is not already used
        if (QProcess::execute("grep", {"-w", '^' + groupNameEdit->text(), "/etc/group"}) == 0) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("Sorry, that group name already exists. Please enter a different name."));
            return;
        }
        // Run addgroup command
        QString group_user_level = checkGroupUserLevel->checkState() == Qt::Checked
                                       ? "--quiet" // --quiet because it fails if ""
                                       : "--system";
        if (shell->runAsRoot("addgroup " + groupNameEdit->text() + " " + group_user_level)) {
            QMessageBox::information(this, windowTitle(), tr("The system group was added ok."));
        } else {
            QMessageBox::critical(this, windowTitle(), tr("Failed to add the system group."));
        }
    } else { // Deleting group if addBox disabled
        QStringList groups;
        for (auto i = 0; i < listGroupsToRemove->count(); ++i) {
            if (listGroupsToRemove->item(i)->checkState() == Qt::Checked) {
                groups << listGroupsToRemove->item(i)->text();
            }
        }
        if (groups.isEmpty()) {
            refresh();
            return;
        }
        QString msg
            = groups.count() == 1
                  ? tr("This action cannot be undone. Are you sure you want to delete group %1?").arg(groups.at(0))
                  : tr("This action cannot be undone. Are you sure you want to delete the following groups: %1?")
                        .arg(groups.join(' '));
        int ans = QMessageBox::warning(this, windowTitle(), msg, QMessageBox::Yes, QMessageBox::No);
        if (ans == QMessageBox::Yes) {
            auto it = std::find_if(groups.cbegin(), groups.cend(),
                                   [&](const auto &group) { return !shell->runAsRoot("delgroup " + group); });
            if (it != groups.cend()) {
                const auto &group = *it;
                QMessageBox::critical(this, windowTitle(),
                                      tr("Failed to delete the group.") + '\n' + tr("Group: %1").arg(group));
                refresh();
                return;
            }
            msg = groups.count() == 1 ? tr("The group has been deleted.") : tr("The groups have been deleted.");
            QMessageBox::information(this, windowTitle(), msg);
        }
    }
    refresh();
}

void MainWindow::applyMembership()
{
    QString groups;
    for (auto i = 0; i < listGroups->count(); ++i) {
        if (listGroups->item(i)->checkState() == Qt::Checked) {
            groups += listGroups->item(i)->text() + ',';
        }
    }
    groups.chop(1);
    if (shell->runAsRoot("usermod -G " + groups + ' ' + userComboMembership->currentText())) {
        QMessageBox::information(this, windowTitle(), tr("The changes have been applied."));
    } else {
        QMessageBox::critical(this, windowTitle(), tr("Failed to apply group changes"));
    }
}

void MainWindow::applyRename()
{
    const QString old_name = comboRenameUser->currentText();
    const QString new_name = renameUserNameEdit->text();

    // Validate data before proceeding
    // Check if selected user is in use
    QString logname = QString::fromUtf8(qgetenv("SUDO_USER")).trimmed();
    if (logname.isEmpty()) {
        logname = QString::fromUtf8(qgetenv("LOGNAME")).trimmed();
    }
    if (logname == old_name) {
        QMessageBox::critical(
            this, windowTitle(),
            tr("The selected user name is currently in use.") + "\n\n"
                + tr("To rename this user, please log out and log back in using another user account."));
        refresh();
        return;
    }

    // See if username is reasonable length
    if (new_name.length() < 2) {
        QMessageBox::critical(this, windowTitle(),
                              tr("The user name needs to be at least 2 characters long. Please select a longer "
                                 "name before proceeding."));
        return;
    } else if (!new_name.contains(QRegularExpression("^[A-Za-z_][A-Za-z0-9_-]*[$]?$"))) {
        QMessageBox::critical(this, windowTitle(),
                              tr("The user name cannot contain special characters or spaces.\n"
                                 "Please choose another name before proceeding."));
        return;
    }
    if (QProcess::execute("grep", {"-E", "^" + new_name + ":", "/etc/passwd"}) == 0) {
        QMessageBox::critical(this, windowTitle(),
                              tr("Sorry, this name already exists on your system. Please enter a different name."));
        return;
    }

    // Rename user
    bool success
        = shell->runAsRoot("usermod --login " + new_name + " --move-home --home /home/" + new_name + " " + old_name);
    if (!success) {
        QMessageBox::critical(this, windowTitle(),
                              tr("Failed to rename the user. Please make sure that the user is not logged in, you "
                                 "might need to restart"));
        return;
    }

    // Rename other instances of the old name, like "Finger" name if present
    QString StartRex = "([^-_[:alnum:]])";
    QString EndRex = "([^-_[:alnum:]$])";
    QString escapedNew = QRegularExpression::escape(new_name);
    QString escapedOld = QRegularExpression::escape(old_name);
    shell->runAsRoot("sed -i  -r '/^" + escapedNew + ":/s/" + StartRex + escapedOld + EndRex + "/\\1" + escapedNew
                     + "\\2/g' /etc/passwd");

    // Change group
    shell->runAsRoot("groupmod --new-name " + new_name + " " + old_name);

    // Fix "home/old_user" string in all ~/ files
    shell->runAsRoot(QString(R"(find /home/%1 -type f -exec sed -i 's|home/%1|home/%2|g' {} +)")
                         .arg(QRegularExpression::escape(old_name), QRegularExpression::escape(new_name)));

    // Change autologin name (Xfce and KDE)
    if (QFile::exists("/etc/lightdm/lightdm.conf")) {
        shell->runAsRoot(QString("sed -i 's/autologin-user=%1/autologin-user=%2/g' /etc/lightdm/lightdm.conf")
                             .arg(QRegularExpression::escape(old_name), QRegularExpression::escape(new_name)));
    }
    if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
        shell->runAsRoot(QString("sed -i 's/^User=%1$/User=%2/g' /etc/sddm.conf.d/kde_settings.conf")
                             .arg(QRegularExpression::escape(old_name), QRegularExpression::escape(new_name)));
    } else if (QFile::exists("/etc/sddm.conf")) {
        shell->runAsRoot(QString("sed -i 's/^User=%1$/User=%2/g' /etc/sddm.conf")
                             .arg(QRegularExpression::escape(old_name), QRegularExpression::escape(new_name)));
    }

    QMessageBox::information(this, windowTitle(), tr("The user was renamed."));
    refresh();
}

void MainWindow::syncDone(bool success)
{
    if (success) {
        QString toDir = QString("/home/%1").arg(toUserComboBox->currentText());

        // If a directory rather than a user name
        if (toUserComboBox->currentText().contains('/')) {
            if (syncRadioButton->isChecked()) {
                syncStatusEdit->setText(tr("Synchronizing desktop...ok"));
            } else {
                syncStatusEdit->setText(tr("Copying desktop...ok"));
            }
            syncProgressBar->setValue(syncProgressBar->maximum());
            setCursor(QCursor(Qt::ArrowCursor));
            return;
        }

        // Fix owner
        QString cmd = QString("chown -R %1:%1 %2").arg(toUserComboBox->currentText(), toDir);
        shell->runAsRoot(cmd);

        // Fix "home/old_user" string in all ~/ or ~/.mozilla files
        if (entireRadioButton->isChecked()) {
            cmd = QString("grep -rl \"home/%1\" /home/%2 | xargs sed -i 's|home/%1|home/%2|g'")
                      .arg(fromUserComboBox->currentText(), toUserComboBox->currentText());
        } else if (mozillaRadioButton->isChecked()) {
            cmd = QString("grep -rl \"home/%1\" /home/%2/.mozilla | xargs sed -i 's|home/%1|home/%2|g'")
                      .arg(fromUserComboBox->currentText(), toUserComboBox->currentText());
        }
        shell->runAsRoot(cmd);

        if (entireRadioButton->isChecked()) {
            shell->runAsRoot("rm -f " + toDir + "/.recently-used");
            shell->runAsRoot("rm -f " + toDir + "/.openoffice.org/*/.lock");
        }
        if (syncRadioButton->isChecked()) {
            syncStatusEdit->setText(tr("Synchronizing desktop...ok"));
        } else {
            syncStatusEdit->setText(tr("Copying desktop...ok"));
        }
    } else {
        if (syncRadioButton->isChecked()) {
            syncStatusEdit->setText(tr("Synchronizing desktop...failed"));
        } else {
            syncStatusEdit->setText(tr("Copying desktop...failed"));
        }
    }
    syncProgressBar->setValue(syncProgressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        closeApp();
    }
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

void MainWindow::setConnections()
{
    connect(buttonAbout, &QPushButton::clicked, this, &MainWindow::buttonAbout_clicked);
    connect(buttonApply, &QPushButton::clicked, this, &MainWindow::buttonApply_clicked);
    connect(buttonCancel, &QPushButton::clicked, this, &MainWindow::buttonCancel_clicked);
    connect(buttonHelp, &QPushButton::clicked, this, &MainWindow::buttonHelp_clicked);
    connect(checkGroups, &QCheckBox::stateChanged, this, &MainWindow::checkGroups_stateChanged);
    connect(checkMozilla, &QCheckBox::stateChanged, this, &MainWindow::checkMozilla_stateChanged);
    connect(comboChangePass, &QComboBox::textActivated, this, &MainWindow::comboChangePass_activated);
    connect(comboDeleteUser, &QComboBox::textActivated, this, &MainWindow::comboDeleteUser_activated);
    connect(comboRenameUser, &QComboBox::textActivated, this, &MainWindow::comboRenameUser_activated);
    connect(copyRadioButton, &QRadioButton::clicked, this, &MainWindow::copyRadioButton_clicked);
    connect(docsRadioButton, &QRadioButton::clicked, this, &MainWindow::docsRadioButton_clicked);
    connect(entireRadioButton, &QRadioButton::clicked, this, &MainWindow::entireRadioButton_clicked);
    connect(fromUserComboBox, &QComboBox::textActivated, this, &MainWindow::fromUserComboBox_activated);
    connect(groupNameEdit, &QLineEdit::textEdited, this, &MainWindow::groupNameEdit_textEdited);
    connect(listGroups, &QListWidget::itemChanged, this, [this] { buttonApply->setEnabled(true); });
    connect(mozillaRadioButton, &QRadioButton::clicked, this, &MainWindow::mozillaRadioButton_clicked);
    connect(radioAutologinNo, &QRadioButton::clicked, this, &MainWindow::radioAutologinNo_clicked);
    connect(radioAutologinYes, &QRadioButton::clicked, this, &MainWindow::radioAutologinYes_clicked);
    connect(sharedRadioButton, &QRadioButton::clicked, this, &MainWindow::sharedRadioButton_clicked);
    connect(syncRadioButton, &QRadioButton::clicked, this, &MainWindow::syncRadioButton_clicked);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabWidget_currentChanged);
    connect(toUserComboBox, &QComboBox::textActivated, this, &MainWindow::toUserComboBox_activated);
    connect(userComboBox, &QComboBox::textActivated, this, &MainWindow::userComboBox_activated);
    connect(userComboMembership, &QComboBox::textActivated, this, &MainWindow::userComboMembership_activated);
    connect(userNameEdit, &QLineEdit::textEdited, this, &MainWindow::userNameEdit_textEdited);
}

void MainWindow::fromUserComboBox_activated(const QString & /*unused*/)
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
    QStringList items = users;
    QString logname = QString::fromUtf8(qgetenv("SUDO_USER")).trimmed();
    if (logname.isEmpty()) {
        logname = QString::fromUtf8(qgetenv("LOGNAME")).trimmed();
    }
    items.removeAll(logname);
    items.removeAll(fromUserComboBox->currentText());
    items.sort();
    toUserComboBox->clear();
    toUserComboBox->addItems(items);
    if (items.isEmpty()) {
        toUserComboBox->addItem(QString());
    }
    toUserComboBox->addItem(tr("browse..."));
}

void MainWindow::userComboBox_activated(const QString & /*unused*/)
{
    buttonApply->setDisabled(true);
    radioAutologinNo->setAutoExclusive(false);
    radioAutologinNo->setChecked(false);
    radioAutologinNo->setAutoExclusive(true);
    radioAutologinYes->setAutoExclusive(false);
    radioAutologinYes->setChecked(false);
    radioAutologinYes->setAutoExclusive(true);
    QString user = userComboBox->currentText();
    if (QProcess::execute("pgrep", {"lightdm"}) == 0) {
        const QString cmd = QString("grep -qw ^autologin-user=%1 /etc/lightdm/lightdm.conf").arg(user);
        if (shell->run(cmd, true)) {
            radioAutologinYes->setChecked(true);
        } else {
            radioAutologinNo->setChecked(true);
        }
    } else if (QProcess::execute("pgrep", {"sddm"}) == 0) {
        QSettings sddm_settings("/etc/sddm.conf", QSettings::NativeFormat);
        if (sddm_settings.value("Autologin/User").toString() == user) {
            radioAutologinYes->setChecked(true);
        } else {
            radioAutologinNo->setChecked(true);
        }
    }
}

void MainWindow::comboDeleteUser_activated(const QString & /*unused*/)
{
    addUserBox->setEnabled(false);
    changePasswordBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (comboDeleteUser->currentText() == tr("none")) {
        refresh();
    }
}

void MainWindow::userNameEdit_textEdited()
{
    deleteUserBox->setEnabled(false);
    changePasswordBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (userNameEdit->text().isEmpty()) {
        refresh();
    }
}

void MainWindow::groupNameEdit_textEdited()
{
    deleteBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (groupNameEdit->text().isEmpty()) {
        refresh();
    }
}

void MainWindow::userComboMembership_activated(const QString & /*unused*/)
{
    buildListGroups();
    buttonApply->setEnabled(true);
    if (userComboMembership->currentText() == tr("none")) {
        refresh();
    }
}

void MainWindow::buildListGroups()
{
    listGroups->clear();
    // Read /etc/group and add all the groups in the listGroups
    QStringList groups = shell->getOut("cat /etc/group | cut -f 1 -d :").trimmed().split('\n');
    groups.sort();
    for (const QString &group : groups) {
        auto *item = new QListWidgetItem;
        item->setText(group);
        item->setCheckState(Qt::Unchecked);
        listGroups->addItem(item);
    }
    // Check the boxes for the groups that the current user belongs to
    const QString cmd = QString("id -nG %1").arg(userComboMembership->currentText());
    const QString out = shell->getOut(cmd);
    QStringList out_tok = out.split(' ');
    while (!out_tok.isEmpty()) {
        QString text = out_tok.takeFirst().trimmed();
        auto list = listGroups->findItems(text, Qt::MatchExactly);
        while (!list.isEmpty()) {
            list.takeFirst()->setCheckState(Qt::Checked);
        }
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
    QStringList groups = QString(file.readAll()).split('\n');
    groups.sort();
    for (const auto &group : std::as_const(groups)) {
        if (!group.isEmpty()) {
            auto *item = new QListWidgetItem;
            item->setText(group.section(':', 0, 0));
            if (item->text() == "root") {
                continue;
            }
            item->setCheckState(Qt::Unchecked);
            listGroupsToRemove->addItem(item);
        }
    }
    disconnect(listGroupsToRemove, &QListWidget::itemChanged, nullptr, nullptr);
    connect(listGroupsToRemove, &QListWidget::itemChanged, this, [this](QListWidgetItem *item) {
        buttonApply->setEnabled(true);
        addBox->setDisabled(true);
        if (item->checkState() == Qt::Checked) {
            return;
        }
        QStringList items;
        for (auto i = 0; i < listGroupsToRemove->count(); ++i) {
            if (listGroupsToRemove->item(i)->checkState() == Qt::Checked) {
                return;
            }
        }
        refresh();
    });
}

// apply but do not close
void MainWindow::buttonApply_clicked()
{
    if (!buttonApply->isEnabled()) {
        return;
    }

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

void MainWindow::tabWidget_currentChanged()
{
    refresh();
}

// Close but do not apply
void MainWindow::buttonCancel_clicked()
{
    closeApp();
}

void MainWindow::buttonAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About %1").arg(windowTitle()),
        "<p align=\"center\"><b><h2>" + windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ") + VERSION
            + "</p><p align=\"center\"><h3>" + tr("Simple user configuration for MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-user/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::buttonHelp_clicked()
{
    QLocale locale;
    const QString lang = locale.bcp47Name();

    QString url = "/usr/share/doc/mx-user/mx-user.html";

    if (lang.startsWith("fr")) {
        url = "https://mxlinux.org/wiki/help-files/help-gestionnaire-des-utilisateurs";
    }
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::comboChangePass_activated(const QString & /*unused*/)
{
    addUserBox->setEnabled(false);
    deleteUserBox->setEnabled(false);
    renameUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (comboChangePass->currentText() == tr("none")) {
        refresh();
    }
}

void MainWindow::toUserComboBox_activated(const QString &arg1)
{
    if (arg1 == tr("browse...")) {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder to copy to"), "/",
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            toUserComboBox->removeItem(toUserComboBox->currentIndex());
            if (toUserComboBox->findText(dir, Qt::MatchExactly | Qt::MatchCaseSensitive) == -1) {
                toUserComboBox->addItem(dir);
            }
            toUserComboBox->setCurrentIndex(toUserComboBox->findText(dir, Qt::MatchExactly | Qt::MatchCaseSensitive));
            toUserComboBox->addItem(tr("browse..."));
        } else {
            toUserComboBox->setCurrentIndex(toUserComboBox->currentIndex() - 1);
        }
    }
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::copyRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::syncRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::entireRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::docsRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::mozillaRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::sharedRadioButton_clicked()
{
    buttonApply->setEnabled(true);
    syncProgressBar->setValue(0);
}

void MainWindow::comboRenameUser_activated(const QString & /*unused*/)
{
    addUserBox->setEnabled(false);
    changePasswordBox->setEnabled(false);
    deleteUserBox->setEnabled(false);
    buttonApply->setEnabled(true);
    if (comboRenameUser->currentText() == tr("none")) {
        refresh();
    }
}

void MainWindow::checkGroups_stateChanged(int /*unused*/)
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

void MainWindow::checkMozilla_stateChanged(int /*unused*/)
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

void MainWindow::radioAutologinYes_clicked()
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

void MainWindow::radioAutologinNo_clicked()
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

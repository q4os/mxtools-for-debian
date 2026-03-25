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
#include "about.h"
#include "passedit.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QProcess>
#include <QRegularExpression>
#include <QScreen>
#include <QStandardPaths>
#include <QTextEdit>

#ifndef VERSION
#define VERSION "?.?.?.?"
#endif

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
    QSize savedSize = QDialog::size();
    restoreGeometry(settings.value("geometry").toByteArray());
    if (isMaximized()) { // If started maximized give option to resize to normal window size
        resize(savedSize);
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
        users = shell->getOut("lslogins --noheadings -u -o user | grep -vw root", QuietMode::Yes).trimmed().split('\n');
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
    if ((QProcess::execute("pgrep", {"lightdm"}) != 0) && QProcess::execute("pgrep", {"sddm"}) != 0
        && QProcess::execute("pgrep", {"plasmalogin"}) != 0) {
        groupBox->setVisible(false);
    }
}

void MainWindow::refreshCopy()
{
    fromUserComboBox->clear();
    fromUserComboBox->addItems(users);
    const QString logname = currentLogname();
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
    const QString adminGroup = adminGroupName();
    const QStringList extraGroups = defaultExtraGroups();
    const bool adminInDefaults = !adminGroup.isEmpty() && extraGroups.contains(adminGroup);
    checkSudoGroup->setVisible(!adminGroup.isEmpty());
    checkSudoGroup->setChecked(adminInDefaults);
    if (!adminGroup.isEmpty()) {
        checkSudoGroup->setText(tr("Add user to %1 group").arg(adminGroup));
    }
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
    if (user != QStringLiteral("root")) {
        home = QStringLiteral("/home/%1").arg(user);
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
    if (checkGroups->isChecked() && user != QStringLiteral("root")) {
        buildListGroups();
        QStringList extra_groups_list = defaultExtraGroups();
        if (extra_groups_list.isEmpty()) {
            QMessageBox::information(this, windowTitle(),
                                     tr("No default extra groups are configured on this system."));
        } else {
            QStringList new_group_list;
            for (const QString &group : extra_groups_list) {
                if (!listGroups->findItems(group, Qt::MatchExactly).isEmpty()) {
                    new_group_list << group;
                }
            }
            shell->procAsRoot("usermod", {"-G", "", user});
            if (!new_group_list.isEmpty()) {
                shell->procAsRoot("usermod", {"-G", new_group_list.join(','), user});
            }
            QMessageBox::information(this, windowTitle(), tr("User group membership was restored."));
        }
    }
    // Reset Mozilla configs
    if (checkMozilla->isChecked()) {
        shell->procAsRoot("rm", {"-r", home + "/.mozilla"});
        QMessageBox::information(this, windowTitle(), tr("Mozilla settings were reset."));
    }
    if (radioAutologinNo->isChecked()) {
        if (QFile::exists("/etc/lightdm/lightdm.conf")) {
            shell->procAsRoot("sed", {"-i", "-E",
                                      QString(R"(/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=/d;})"),
                                      "/etc/lightdm/lightdm.conf"});
        }

        if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
            shell->procAsRoot("sed",
                              {"-i", QStringLiteral("s/^User=%1/User=/").arg(user), "/etc/sddm.conf.d/kde_settings.conf"});
        } else if (QFile::exists("/etc/sddm.conf")) {
            shell->procAsRoot("sed", {"-i", QStringLiteral("s/^User=%1/User=/").arg(user), "/etc/sddm.conf"});
        }
        if (QFile::exists("/etc/plasmalogin.conf.d/autologin.conf")) {
            shell->procAsRoot("sed",
                              {"-i", QStringLiteral("s/^User=%1/User=/").arg(user), "/etc/plasmalogin.conf.d/autologin.conf"});
        }
        QMessageBox::information(this, tr("Autologin options"),
                                 (tr("Autologin has been disabled for the '%1' account.").arg(user)));
    } else if (radioAutologinYes->isChecked()) {
        if (QFile::exists("/etc/lightdm/lightdm.conf")) {
            shell->procAsRoot("sed",
                              {"-i",
                               "-E",
                               "-e",
                               QString(R"(/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=/d;})"),
                               "-e",
                               QString(R"(/^[[]Seat(Defaults|:[*])[]]/aautologin-user=%1)").arg(user),
                               "/etc/lightdm/lightdm.conf"});
        }
        if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
            shell->procAsRoot("sed", {"-i",
                                      QStringLiteral("s/^User=.*/User=%1/").arg(user),
                                      "/etc/sddm.conf.d/kde_settings.conf"});
            if (qEnvironmentVariable("XDG_CURRENT_DESKTOP") == QLatin1String("KDE")) {
                QString sessionType;
                if (qEnvironmentVariable("XDG_SESSION_TYPE") == QLatin1String("wayland")) {
                    sessionType = QStringLiteral("plasma");
                } else if (qEnvironmentVariable("XDG_SESSION_TYPE") == QLatin1String("x11")) {
                    sessionType = QStringLiteral("plasmax11");
                }

                if (!sessionType.isEmpty()) {
                    shell->procAsRoot("sed",
                                      {"-i",
                                       QString("s/^Session=.*/Session=%1/").arg(sessionType),
                                       "/etc/sddm.conf.d/kde_settings.conf"});
                }
            }
        } else if (QFile::exists("/etc/sddm.conf")) {
            shell->procAsRoot("sed", {"-i", QString("s/^User=.*/User=%1/").arg(user), "/etc/sddm.conf"});
            if (qEnvironmentVariable("XDG_CURRENT_DESKTOP") == QLatin1String("KDE")) {
                shell->procAsRoot("sed", {"-i", "s/^Session=.*/Session=plasma.desktop/", "/etc/sddm.conf"});
            }
        }
        if (QFile::exists("/etc/plasmalogin.conf.d/autologin.conf")) {
            shell->procAsRoot("sed",
                              {"-i", QStringLiteral("s/^User=.*/User=%1/").arg(user),
                               "/etc/plasmalogin.conf.d/autologin.conf"});
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

    QString fromDir = QStringLiteral("/home/%1").arg(fromUserComboBox->currentText());
    QString toDir = QStringLiteral("/home/%1").arg(toUserComboBox->currentText());
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
    QStringList rsyncArgs {"-a", "--info=progress2"};
    if (syncRadioButton->isChecked()) {
        rsyncArgs << "--delete-after";
    }
    rsyncArgs << fromDir << toDir;
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
    syncDone(shell->procAsRoot("rsync", rsyncArgs));
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

    const QString adminGroup = adminGroupName();
    QStringList extraGroups = defaultExtraGroups();
    if (!adminGroup.isEmpty()) {
        if (checkSudoGroup->isChecked() && !extraGroups.contains(adminGroup)) {
            extraGroups << adminGroup;
        } else if (!checkSudoGroup->isChecked()) {
            extraGroups.removeAll(adminGroup);
        }
    }

    const QString dshell = defaultShellPath();
    const bool hasAdduser = commandExists("adduser");
    const bool hasUseradd = commandExists("useradd");
    if (!hasAdduser && !hasUseradd) {
        QMessageBox::critical(this, windowTitle(),
                              tr("No suitable user management tools found (adduser or useradd)."));
        return;
    }

    bool success = false;
    if (hasAdduser) {
        const QString addUserHelp = shell->getOutAsRoot("adduser", {"--help"}, QuietMode::Yes);
        QString commentOption = addUserHelp.contains("--comment") ? QStringLiteral("--comment") : QStringLiteral("--gecos");
        QString allowBadNames
            = addUserHelp.contains("--allow-bad-names") ? QStringLiteral("--allow-bad-names") : QStringLiteral("--force-badname");

        success = shell->procAsRoot("adduser",
                                    {"--disabled-login", allowBadNames, "--shell", dshell, commentOption,
                                     userNameEdit->text(), userNameEdit->text()});
    } else {
        QStringList useraddArgs {"-m", "-s", dshell, "-c", userNameEdit->text()};
        if (!extraGroups.isEmpty()) {
            useraddArgs << "-G" << extraGroups.join(',');
        }
        useraddArgs << userNameEdit->text();
        success = shell->procAsRoot("useradd", useraddArgs);
    }

    if (!success) {
        QMessageBox::critical(this, windowTitle(), tr("Failed to add the user."));
        return;
    }

    QString userNameArg = userNameEdit->text();
    const QByteArray passwordInput = userPasswordEdit->text().toUtf8() + '\n' + userPasswordEdit->text().toUtf8() + '\n';
    if (shell->procAsRoot("passwd", {userNameArg}, nullptr, &passwordInput, QuietMode::Yes)) {
        if (!adminGroup.isEmpty()) {
            const QStringList groupArgs
                = checkSudoGroup->isChecked() ? QStringList {"-a", userNameArg, adminGroup}
                                             : QStringList {"-d", userNameArg, adminGroup};
            shell->procAsRoot("gpasswd", groupArgs, nullptr, nullptr, QuietMode::Yes);
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

    const QByteArray passwordInput
        = lineEditChangePass->text().toUtf8() + '\n' + lineEditChangePass->text().toUtf8() + '\n';
    if (shell->procAsRoot("passwd", {comboChangePass->currentText()}, nullptr, &passwordInput, QuietMode::Yes)) {
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
        const QString username = comboDeleteUser->currentText();
        const bool hasDeluser = commandExists("deluser");
        const bool hasUserdel = commandExists("userdel");
        if (!hasDeluser && !hasUserdel) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("No suitable user removal tools found (deluser or userdel)."));
            return;
        }

        if (deleteHomeCheckBox->isChecked()) {
            shell->procAsRoot("timeout", {"5s", "killall", "-w", "-u", username}, nullptr, nullptr, QuietMode::Yes);
            shell->procAsRoot("timeout", {"5s", "killall", "-9", "-w", "-u", username}, nullptr, nullptr, QuietMode::Yes);
        } else {
        }

        const bool deleted = hasDeluser
            ? shell->procAsRoot("deluser", deleteHomeCheckBox->isChecked() ? QStringList {"--remove-home", username}
                                                                           : QStringList {username})
            : shell->procAsRoot("userdel", deleteHomeCheckBox->isChecked() ? QStringList {"-r", username}
                                                                           : QStringList {username});
        if (deleted) {
            if (QFile::exists("/etc/lightdm/lightdm.conf")) {
                shell->procAsRoot("sed", {"-i",
                                          "-E",
                                          "-e",
                                          QString(R"(/^[[]Seat(Defaults|:[*])[]]/,/[[]/{/^[[:space:]]*autologin-user=%1$/d;})")
                                              .arg(comboDeleteUser->currentText()),
                                          "/etc/lightdm/lightdm.conf"});
            }
            if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
                shell->procAsRoot(
                    "sed",
                    {"-i", QStringLiteral("s/^User=%1$/User=/").arg(comboDeleteUser->currentText()),
                     "/etc/sddm.conf.d/kde_settings.conf"});
            } else if (QFile::exists("/etc/sddm.conf")) {
                shell->procAsRoot("sed",
                                  {"-i", QStringLiteral("s/^User=%1$/User=/").arg(comboDeleteUser->currentText()),
                                   "/etc/sddm.conf"});
            }
            if (QFile::exists("/etc/plasmalogin.conf.d/autologin.conf")) {
                shell->procAsRoot(
                    "sed",
                    {"-i", QStringLiteral("s/^User=%1$/User=/").arg(comboDeleteUser->currentText()),
                     "/etc/plasmalogin.conf.d/autologin.conf"});
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
        const bool hasAddgroup = commandExists("addgroup");
        const bool hasGroupadd = commandExists("groupadd");
        if (!hasAddgroup && !hasGroupadd) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("No suitable group creation tools found (addgroup or groupadd)."));
            return;
        }
        // Run addgroup command
        if (hasAddgroup) {
            const QStringList groupArgs = checkGroupUserLevel->checkState() == Qt::Checked
                ? QStringList {groupNameEdit->text(), "--quiet"}
                : QStringList {groupNameEdit->text(), "--system"};
            if (shell->procAsRoot("addgroup", groupArgs)) {
                QMessageBox::information(this, windowTitle(), tr("The system group was added ok."));
            } else {
                QMessageBox::critical(this, windowTitle(), tr("Failed to add the system group."));
            }
        } else {
            const QStringList groupArgs = checkGroupUserLevel->checkState() == Qt::Checked
                ? QStringList {groupNameEdit->text()}
                : QStringList {"--system", groupNameEdit->text()};
            if (shell->procAsRoot("groupadd", groupArgs)) {
                QMessageBox::information(this, windowTitle(), tr("The system group was added ok."));
            } else {
                QMessageBox::critical(this, windowTitle(), tr("Failed to add the system group."));
            }
        }
    } else { // Deleting group if addBox disabled
        QStringList groups;
        for (int i = 0; i < listGroupsToRemove->count(); ++i) {
            auto *item = listGroupsToRemove->item(i);
            if (item && item->checkState() == Qt::Checked) {
                groups << item->text();
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
            const bool hasDelgroup = commandExists("delgroup");
            const bool hasGroupdel = commandExists("groupdel");
            if (!hasDelgroup && !hasGroupdel) {
                QMessageBox::critical(this, windowTitle(),
                                      tr("No suitable group removal tools found (delgroup or groupdel)."));
                return;
            }
            auto it = std::find_if(groups.cbegin(), groups.cend(), [&](const auto &group) {
                return hasDelgroup ? !shell->procAsRoot("delgroup", {group}) : !shell->procAsRoot("groupdel", {group});
            });
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
    QStringList groups;
    for (int i = 0; i < listGroups->count(); ++i) {
        auto *item = listGroups->item(i);
        if (item && item->checkState() == Qt::Checked) {
            groups << item->text();
        }
    }
    if (shell->procAsRoot("usermod", {"-G", groups.join(','), userComboMembership->currentText()})) {
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
    const QString logname = currentLogname();
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

    shell->procAsRoot("timeout", {"5s", "killall", "-w", "-u", old_name}, nullptr, nullptr, QuietMode::Yes);
    shell->procAsRoot("timeout", {"5s", "killall", "-9", "-w", "-u", old_name}, nullptr, nullptr, QuietMode::Yes);

    // Rename user
    bool success = shell->procAsRoot("usermod", {"--login", new_name, "--move-home", "--home", "/home/" + new_name, old_name});
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
    shell->procAsRoot("sed",
                      {"-i", "-r",
                       QString("/^%1:/s/%2%3%4/\\1%5\\2/g")
                           .arg(escapedNew, StartRex, escapedOld, EndRex, escapedNew),
                       "/etc/passwd"});

    // Change group
    shell->procAsRoot("groupmod", {"--new-name", new_name, old_name});

    // Fix "home/old_user" string in all ~/ files
    shell->procAsRoot("find",
                      {QString("/home/%1").arg(old_name), "-type", "f", "-exec", "sed", "-i",
                       QString("s|home/%1|home/%2|g").arg(old_name, new_name), "{}", "+"});

    // Change autologin name (Xfce and KDE)
    if (QFile::exists("/etc/lightdm/lightdm.conf")) {
        shell->procAsRoot("sed", {"-i",
                                  QString("s/autologin-user=%1/autologin-user=%2/g").arg(escapedOld, escapedNew),
                                  "/etc/lightdm/lightdm.conf"});
    }
    if (QFile::exists("/etc/sddm.conf.d/kde_settings.conf")) {
        shell->procAsRoot("sed",
                          {"-i", QString("s/^User=%1$/User=%2/g").arg(escapedOld, escapedNew),
                           "/etc/sddm.conf.d/kde_settings.conf"});
    } else if (QFile::exists("/etc/sddm.conf")) {
        shell->procAsRoot("sed", {"-i", QString("s/^User=%1$/User=%2/g").arg(escapedOld, escapedNew), "/etc/sddm.conf"});
    }
    if (QFile::exists("/etc/plasmalogin.conf.d/autologin.conf")) {
        shell->procAsRoot("sed",
                          {"-i", QString("s/^User=%1$/User=%2/g").arg(escapedOld, escapedNew),
                           "/etc/plasmalogin.conf.d/autologin.conf"});
    }

    QMessageBox::information(this, windowTitle(), tr("The user was renamed."));
    refresh();
}

void MainWindow::syncDone(bool success)
{
    if (success) {
        QString toDir = QStringLiteral("/home/%1").arg(toUserComboBox->currentText());

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
        shell->procAsRoot("chown", {"-R", QString("%1:%1").arg(toUserComboBox->currentText()), toDir});

        // Fix "home/old_user" string in all ~/ or ~/.mozilla files
        QString scanPath;
        if (entireRadioButton->isChecked()) {
            scanPath = QString("/home/%1").arg(toUserComboBox->currentText());
        } else if (mozillaRadioButton->isChecked()) {
            scanPath = QString("/home/%1/.mozilla").arg(toUserComboBox->currentText());
        }
        if (!scanPath.isEmpty()) {
            shell->procAsRoot("find",
                              {scanPath, "-type", "f", "-exec", "sed", "-i",
                               QString("s|home/%1|home/%2|g")
                                   .arg(fromUserComboBox->currentText(), toUserComboBox->currentText()),
                               "{}", "+"});
        }

        if (entireRadioButton->isChecked()) {
            shell->procAsRoot("rm", {"-f", toDir + "/.recently-used"});
            shell->procAsRoot("find",
                              {toDir + "/.openoffice.org", "-type", "f", "-name", ".lock", "-delete"},
                              nullptr, nullptr, QuietMode::Yes);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(checkGroups, &QCheckBox::checkStateChanged, this, &MainWindow::checkGroups_stateChanged);
    connect(checkMozilla, &QCheckBox::checkStateChanged, this, &MainWindow::checkMozilla_stateChanged);
#else
    connect(checkGroups, &QCheckBox::stateChanged, this, [this](int state) {
        checkGroups_stateChanged(static_cast<Qt::CheckState>(state));
    });
    connect(checkMozilla, &QCheckBox::stateChanged, this, [this](int state) {
        checkMozilla_stateChanged(static_cast<Qt::CheckState>(state));
    });
#endif
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
    const QString logname = currentLogname();
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
        if (shell->run(cmd, nullptr, nullptr, QuietMode::Yes)) {
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
    } else if (QProcess::execute("pgrep", {"plasmalogin"}) == 0) {
        QSettings plasma_settings("/etc/plasmalogin.conf.d/autologin.conf", QSettings::NativeFormat);
        if (plasma_settings.value("Autologin/User").toString() == user) {
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
            if (item->text() == QStringLiteral("root")) {
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
        for (int i = 0; i < listGroupsToRemove->count(); ++i) {
            auto *listItem = listGroupsToRemove->item(i);
            if (listItem && listItem->checkState() == Qt::Checked) {
                return;
            }
        }
        refresh();
    });
}

bool MainWindow::commandExists(const QString &command) const
{
    if (!QStandardPaths::findExecutable(command).isEmpty()) {
        return true;
    }
    // Fallback: check sbin paths which may not be in $PATH when launched from desktop menu
    for (const QString &path : {"/usr/sbin/", "/sbin/"}) {
        if (QFileInfo::exists(path + command)) {
            return true;
        }
    }
    return false;
}

QString MainWindow::currentLogname() const
{
    QString logname = QString::fromUtf8(qgetenv("SUDO_USER")).trimmed();
    if (logname.isEmpty()) {
        logname = QString::fromUtf8(qgetenv("LOGNAME")).trimmed();
    }
    return logname;
}

QString MainWindow::adminGroupName() const
{
    const QStringList candidateGroups {"sudo", "wheel"};
    for (const auto &group : candidateGroups) {
        if (QProcess::execute("getent", {"group", group}) == 0) {
            return group;
        }
    }

    QFile groupFile("/etc/group");
    if (!groupFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString contents = QString::fromUtf8(groupFile.readAll());
    for (const auto &group : candidateGroups) {
        QRegularExpression rx(QStringLiteral("^%1:").arg(QRegularExpression::escape(group)), QRegularExpression::MultilineOption);
        if (rx.match(contents).hasMatch()) {
            return group;
        }
    }

    return {};
}

QString MainWindow::defaultShellPath() const
{
    auto normalizedShell = [](QString shellPath) {
        if (shellPath.startsWith("/bin/")) {
            shellPath.replace(0, 5, "/usr/bin/");
        } else if (shellPath.startsWith("/sbin/")) {
            shellPath.replace(0, 6, "/usr/sbin/");
        }
        // Prefer bash over sh
        if (shellPath.endsWith("/sh") && QFile::exists("/usr/bin/bash")) {
            shellPath = "/usr/bin/bash";
        }
        if (!QFile::exists(shellPath)) {
            shellPath = "/usr/bin/bash";
        }
        return shellPath;
    };

#if BUILD_FOR_ARCH
    // For Arch Linux, check /etc/default/useradd first
    QFile userAddDefault("/etc/default/useradd");
    if (userAddDefault.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!userAddDefault.atEnd()) {
            const QString line = QString::fromUtf8(userAddDefault.readLine()).trimmed();
            if (line.startsWith("SHELL=")) {
                return normalizedShell(line.section('=', 1).remove('"'));
            }
        }
    }

    // Fallback to /etc/adduser.conf
    QFile adduserConf("/etc/adduser.conf");
    if (adduserConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!adduserConf.atEnd()) {
            const QString line = QString::fromUtf8(adduserConf.readLine()).trimmed();
            if (line.startsWith("DSHELL=")) {
                return normalizedShell(line.section('=', 1).remove('"'));
            }
        }
    }
#else
    // For Debian-based systems, check /etc/adduser.conf first
    QFile adduserConf("/etc/adduser.conf");
    if (adduserConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!adduserConf.atEnd()) {
            const QString line = QString::fromUtf8(adduserConf.readLine()).trimmed();
            if (line.startsWith("DSHELL=")) {
                return normalizedShell(line.section('=', 1).remove('"'));
            }
        }
    }

    // Fallback to /etc/default/useradd
    QFile userAddDefault("/etc/default/useradd");
    if (userAddDefault.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!userAddDefault.atEnd()) {
            const QString line = QString::fromUtf8(userAddDefault.readLine()).trimmed();
            if (line.startsWith("SHELL=")) {
                return normalizedShell(line.section('=', 1).remove('"'));
            }
        }
    }
#endif

    return normalizedShell("/usr/bin/bash");
}

QStringList MainWindow::defaultExtraGroups() const
{
    QFile adduserConf("/etc/adduser.conf");
    if (!adduserConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    while (!adduserConf.atEnd()) {
        const QString line = QString::fromUtf8(adduserConf.readLine()).trimmed();
        if (!line.startsWith("EXTRA_GROUPS=")) {
            continue;
        }
        QString groups = line.section('=', 1).remove('"');
        return groups.split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);
    }

    return {};
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
    const QString lang = QLocale().bcp47Name();
    QString helpPath = "/usr/share/doc/mx-user/mx-user.html";

    if (lang.startsWith("fr")) {
        helpPath = "/usr/share/doc/mx-user/mx-user_fr.html";
    }

    displayHelpDoc(helpPath, tr("%1 Help").arg(windowTitle()));
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

void MainWindow::checkGroups_stateChanged(Qt::CheckState /*unused*/)
{
    buttonApply->setEnabled(userComboBox->currentText() != tr("none"));
}

void MainWindow::checkMozilla_stateChanged(Qt::CheckState /*unused*/)
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

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
    displayManager = detectDisplayManager();
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
        users = shell->getOut("lslogins --noheadings -u -o user", QuietMode::Yes)
                    .split('\n', Qt::SkipEmptyParts);
        users.removeAll(QStringLiteral("root"));
        users.sort();
        comboRenameUser->addItems(users);
        comboChangePass->addItems(users);
        comboDeleteUser->addItems(users);
        buttonApply->setEnabled(false);
        break;
    }
}

DisplayManager MainWindow::detectDisplayManager() const
{
    if (QProcess::execute("pgrep", {"lightdm"}) == 0) {
        return DisplayManager::Lightdm;
    }
    if (QProcess::execute("pgrep", {"sddm"}) == 0) {
        return DisplayManager::Sddm;
    }
    if (QProcess::execute("pgrep", {"plasmalogin"}) == 0) {
        return DisplayManager::Plasmalogin;
    }
    return DisplayManager::None;
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
    // Autologin options only make sense when a supported login manager is running.
    groupBox->setVisible(displayManager != DisplayManager::None);
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
            if (shell->runHelper({"set-groups", user, new_group_list.join(',')})) {
                QMessageBox::information(this, windowTitle(), tr("User group membership was restored."));
            }
        }
    }
    // Reset Mozilla configs
    if (checkMozilla->isChecked()) {
        if (shell->runHelper({"remove-home-path", user, home + "/.mozilla"})) {
            QMessageBox::information(this, windowTitle(), tr("Mozilla settings were reset."));
        }
    }
    if (radioAutologinNo->isChecked()) {
        if (shell->runHelper({"autologin-disable", user})) {
            QMessageBox::information(this, tr("Autologin options"),
                                     (tr("Autologin has been disabled for the '%1' account.").arg(user)));
        }
    } else if (radioAutologinYes->isChecked()) {
        // The helper edits whichever DM config exists; supply the desired session
        // names so it can set them on the sddm config it chooses.
        QString kdeSession;
        QString defaultSession;
        if (qEnvironmentVariable("XDG_CURRENT_DESKTOP") == QLatin1String("KDE")) {
            const QString sessionType = qEnvironmentVariable("XDG_SESSION_TYPE");
            if (sessionType == QLatin1String("wayland")) {
                kdeSession = QStringLiteral("plasma");
            } else if (sessionType == QLatin1String("x11")) {
                kdeSession = QStringLiteral("plasmax11");
            }
            defaultSession = QStringLiteral("plasma.desktop");
        }
        if (shell->runHelper({"autologin-enable", user, kdeSession, defaultSession})) {
            QMessageBox::information(this, tr("Autologin options"),
                                     (tr("Autologin has been enabled for the '%1' account.").arg(user)));
        }
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

    // Snapshot the operation now so the post-copy follow-ups in syncDone act on
    // exactly what was copied, independent of any later change to the controls.
    CopyJob job;
    job.fromUser = fromUserComboBox->currentText();
    job.toUser = toUserComboBox->currentText();
    job.toDir = toDir;
    job.toIsDir = toUserComboBox->currentText().contains('/');
    job.entire = entireRadioButton->isChecked();
    job.rewritePaths = entireRadioButton->isChecked() || mozillaRadioButton->isChecked();
    job.sync = syncRadioButton->isChecked();

    setCursor(QCursor(Qt::WaitCursor));
    if (syncRadioButton->isChecked()) {
        syncStatusEdit->setText(tr("Synchronizing desktop..."));
    } else {
        syncStatusEdit->setText(tr("Copying desktop..."));
    }
    for (int tab = 0; tab < Tab::MAX; ++tab) {
        if (tab == Tab::Copy) {
            continue;
        }
        tabWidget->setTabEnabled(tab, false);
    }
    const auto conn = connect(shell, &Cmd::outputAvailable, this, [this](const QString &out) {
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
    syncDone(shell->runHelper({"rsync-copy", job.fromUser, fromDir, toDir, job.sync ? "sync" : "copy"}), job);
    disconnect(conn);
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

    // The helper picks adduser/useradd, detects its option spelling, and applies
    // the groups; extra groups are only used by the useradd path, matching the
    // adduser path's reliance on the system EXTRA_GROUPS configuration.
    const bool success = shell->runHelper({"add-user", userNameEdit->text(), dshell, extraGroups.join(',')});

    if (!success) {
        if (!shell->elevationError()) {
            QMessageBox::critical(this, windowTitle(), tr("Failed to add the user."));
        }
        return;
    }

    QString userNameArg = userNameEdit->text();
    const QByteArray passwordInput = userPasswordEdit->text().toUtf8() + '\n' + userPasswordEdit->text().toUtf8() + '\n';
    if (shell->runHelper({"set-password", userNameArg}, nullptr, &passwordInput, QuietMode::Yes)) {
        if (!adminGroup.isEmpty()) {
            shell->runHelper({checkSudoGroup->isChecked() ? "group-add-member" : "group-remove-member", userNameArg,
                              adminGroup},
                             nullptr, nullptr, QuietMode::Yes);
        }
        QMessageBox::information(this, windowTitle(), tr("The user was added ok."));
        refresh();
    } else if (!shell->elevationError()) {
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
    if (shell->runHelper({"set-password", comboChangePass->currentText()}, nullptr, &passwordInput, QuietMode::Yes)) {
        QMessageBox::information(this, windowTitle(), tr("Password successfully changed."));
        refresh();
    } else if (!shell->elevationError()) {
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
            shell->runHelper({"kill-user", username}, nullptr, nullptr, QuietMode::Yes);
        }

        QStringList delUserArgs {"del-user", username};
        if (deleteHomeCheckBox->isChecked()) {
            delUserArgs << "--remove-home";
        }
        const bool deleted = shell->runHelper(delUserArgs);
        if (deleted) {
            shell->runHelper({"autologin-remove-user", username});
            QMessageBox::information(this, windowTitle(), tr("The user has been deleted."));
        } else if (!shell->elevationError()) {
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
        // Run the group-creation command via the helper
        const QString scope = checkGroupUserLevel->checkState() == Qt::Checked ? QStringLiteral("user")
                                                                               : QStringLiteral("system");
        if (shell->runHelper({"add-group", groupNameEdit->text(), scope})) {
            QMessageBox::information(this, windowTitle(), tr("The system group was added ok."));
        } else if (!shell->elevationError()) {
            QMessageBox::critical(this, windowTitle(), tr("Failed to add the system group."));
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
            auto it = std::find_if(groups.cbegin(), groups.cend(),
                                   [&](const auto &group) { return !shell->runHelper({"del-group", group}); });
            if (it != groups.cend()) {
                const auto &group = *it;
                if (!shell->elevationError()) {
                    QMessageBox::critical(this, windowTitle(),
                                          tr("Failed to delete the group.") + '\n' + tr("Group: %1").arg(group));
                }
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
    if (shell->runHelper({"set-groups", userComboMembership->currentText(), groups.join(',')})) {
        QMessageBox::information(this, windowTitle(), tr("The changes have been applied."));
    } else if (!shell->elevationError()) {
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

    shell->runHelper({"kill-user", old_name}, nullptr, nullptr, QuietMode::Yes);

    // Rename the user. The helper relocates the home directory, updates the
    // GECOS/full-name field, renames the personal group, and rewrites "home/<old>"
    // references inside the (new) home directory.
    bool success = shell->runHelper({"rename-user", old_name, new_name});
    if (!success) {
        if (!shell->elevationError()) {
            QMessageBox::critical(this, windowTitle(),
                                  tr("Failed to rename the user. Please make sure that the user is not logged in, you "
                                     "might need to restart"));
        }
        return;
    }

    // Change autologin name (Xfce and KDE)
    shell->runHelper({"autologin-rename", old_name, new_name});

    QMessageBox::information(this, windowTitle(), tr("The user was renamed."));
    refresh();
}

void MainWindow::syncDone(bool success, const CopyJob &job)
{
    if (success) {
        // Destination is a browsed directory, not a managed home: just report.
        if (job.toIsDir) {
            syncStatusEdit->setText(job.sync ? tr("Synchronizing desktop...ok") : tr("Copying desktop...ok"));
            syncProgressBar->setValue(syncProgressBar->maximum());
            setCursor(QCursor(Qt::ArrowCursor));
            return;
        }

        // Fix owner of exactly the copied subtree (job.toDir is what rsync wrote).
        shell->runHelper({"chown-home", job.toUser, job.toDir});

        // Fix "home/<from>" references for entire-home or .mozilla copies.
        if (job.rewritePaths) {
            shell->runHelper({"rewrite-home-paths", job.toUser, job.toDir, job.fromUser, job.toUser});
        }

        if (job.entire) {
            shell->runHelper({"remove-home-path", job.toUser, job.toDir + "/.recently-used"});
            shell->runHelper({"clean-openoffice-locks", job.toUser, job.toDir}, nullptr, nullptr, QuietMode::Yes);
        }
        syncStatusEdit->setText(job.sync ? tr("Synchronizing desktop...ok") : tr("Copying desktop...ok"));
    } else {
        syncStatusEdit->setText(job.sync ? tr("Synchronizing desktop...failed") : tr("Copying desktop...failed"));
    }
    syncProgressBar->setValue(syncProgressBar->maximum());
    setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        closeApp();
    } else {
        // Defer every other key to QDialog so standard handling (e.g. Enter
        // activating the default button) is not swallowed.
        QDialog::keyPressEvent(event);
    }
}

// All close paths (Cancel button, Escape, and the window-manager [X]) funnel
// through close() -> closeEvent(), so the "process not done" confirmation is
// applied consistently and the window can never be torn down silently.
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (shell->state() != QProcess::NotRunning) {
        setCursor(QCursor(Qt::ArrowCursor));
        if (QMessageBox::Yes
            != QMessageBox::question(this, tr("Confirmation"),
                                     tr("Process not done. Are you sure you want to quit the application?"),
                                     QMessageBox::Yes | QMessageBox::No)) {
            setCursor(QCursor(Qt::WaitCursor));
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::closeApp()
{
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
    const QString user = userComboBox->currentText();
    bool autologin = false;
    switch (displayManager) {
    case DisplayManager::Lightdm: {
        QFile lightdmConf(QStringLiteral("/etc/lightdm/lightdm.conf"));
        if (lightdmConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString needle = QStringLiteral("autologin-user=%1").arg(user);
            while (!lightdmConf.atEnd()) {
                if (QString::fromUtf8(lightdmConf.readLine()).trimmed() == needle) {
                    autologin = true;
                    break;
                }
            }
        }
        break;
    }
    case DisplayManager::Sddm: {
        QSettings sddm_settings("/etc/sddm.conf", QSettings::NativeFormat);
        autologin = sddm_settings.value("Autologin/User").toString() == user;
        break;
    }
    case DisplayManager::Plasmalogin: {
        QSettings plasma_settings("/etc/plasmalogin.conf.d/autologin.conf", QSettings::NativeFormat);
        autologin = plasma_settings.value("Autologin/User").toString() == user;
        break;
    }
    case DisplayManager::None:
        return;
    }
    radioAutologinYes->setChecked(autologin);
    radioAutologinNo->setChecked(!autologin);
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
    QStringList groups;
    QFile groupFile(QStringLiteral("/etc/group"));
    if (groupFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!groupFile.atEnd()) {
            const QString line = QString::fromUtf8(groupFile.readLine());
            const QString name = line.section(':', 0, 0).trimmed();
            if (!name.isEmpty()) {
                groups << name;
            }
        }
    }
    groups.sort();
    for (const QString &group : groups) {
        auto *item = new QListWidgetItem;
        item->setText(group);
        item->setCheckState(Qt::Unchecked);
        listGroups->addItem(item);
    }
    // Check the boxes for the groups that the current user belongs to. Run
    // via an argument list (not a shell string) so the username can't be
    // interpreted as shell syntax.
    const QString user = userComboMembership->currentText();
    QString out;
    if (!user.isEmpty()) {
        shell->proc(QStringLiteral("id"), {"-nG", user}, &out, nullptr, QuietMode::Yes);
    }
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

    // Start each operation with a clean elevation state so a refusal during this
    // Apply can be detected and the affected tab reverted afterwards.
    shell->resetElevationError();

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

    // Elevation was refused mid-operation: the privileged helper made no change,
    // so reload the current tab to discard any pending selections and reflect the
    // unchanged system state.
    if (shell->elevationError()) {
        refresh();
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

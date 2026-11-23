/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2021-2026 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          Dolphin_Oracle
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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>

#include <QDebug>
#include <QDir>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QMap>
#include <QPointer>
#include <QRadioButton>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <QScreen>
#include <QScrollBar>
#include <QSysInfo>
#include <QTextStream>
#include <QTimer>

#include "about.h"
#include "ui_editshare.h"

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // Enable close, minimize, and maximize buttons
    setConnections();

    const QSize &size = this->size();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        if (isMaximized()) { // Add option to resize if maximized
            resize(size);
            centerWindow();
        }
    }
    checkSambashareGroup();
    refreshUserList();
    refreshShareList();
    checksamba();
    checkHomesShare();
}

MainWindow::~MainWindow()
{
    settings.setValue("geometry", saveGeometry());
    delete ui;
}

void MainWindow::centerWindow()
{
    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 2);
}

void MainWindow::setConnections()
{
    const auto connectButton = [this](QPushButton* button, void (MainWindow::*slot)()) {
        connect(button, &QPushButton::clicked, this, [this, slot]() {
            if (!commandRunning) {
                (this->*slot)();
            }
        });
    };

    connectButton(ui->pushAbout, &MainWindow::pushAbout_clicked);
    connectButton(ui->pushAddShare, &MainWindow::pushAddShare_clicked);
    connectButton(ui->pushAddUser, &MainWindow::pushAddUser_clicked);
    connectButton(ui->pushEditShare, &MainWindow::pushEditShare_clicked);
    connectButton(ui->pushEnableDisableSamba, &MainWindow::pushEnableDisableSamba_clicked);
    connectButton(ui->pushHelp, &MainWindow::pushHelp_clicked);
    connectButton(ui->pushRemoveShare, &MainWindow::pushRemoveShare_clicked);
    connectButton(ui->pushRemoveUser, &MainWindow::pushRemoveUser_clicked);
    connectButton(ui->pushStartStopSamba, &MainWindow::pushStartStopSamba_clicked);
    connectButton(ui->pushUserPassword, &MainWindow::pushUserPassword_clicked);
}

void MainWindow::addEditShares(EditShare *editshare)
{
    editshare->adjustSize();
    if (editshare->exec() != QDialog::Accepted) {
        return;
    }

    const QString shareName = editshare->ui->textShareName->text();
    const QString sharePath = editshare->ui->textSharePath->text();
    const QString comment = editshare->ui->textComment->text();
    const QString guestOK = editshare->ui->comboGuestOK->currentText() == tr("Yes") ? "guest_ok=y" : "guest_ok=n";

    if (shareName.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Error, could not add share. Empty share name"));
        return;
    }

    if (!QFileInfo::exists(sharePath)) {
        QMessageBox::critical(this, tr("Error"), tr("Path: %1 does not exist.").arg(sharePath));
        return;
    }

    const QStringList permissions = editshare->permissions();

    if (permissions.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please set access for at least one user."));
        return;
    }

    const QStringList args {"usershare",
                            "add",
                            shareName,
                            sharePath,
                            comment.isEmpty() ? "" : comment,
                            permissions.join(','),
                            guestOK};

    if (run("net", args) != 0) {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Could not add share. Error message:\n\n%1").arg(QString(proc.readAllStandardError())));
        return;
    }

    refreshShareList();
}

QStringList MainWindow::listUsers()
{
    if (run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-list-users"}) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Error listing users"));
        return {};
    }

    const QStringList output = QString(proc.readAllStandardOutput().trimmed()).split('\n', Qt::SkipEmptyParts);
    if (output.isEmpty()) {
        return {};
    }

    QStringList userList;
    userList.reserve(output.size());
    for (const QString &item : output) {
        userList << item.section(':', 0, 0);
    }
    userList.sort();
    return userList;
}

void MainWindow::buildUserList(EditShare *editshare)
{
    auto *layout = editshare->ui->frameUsers->layout();
    // Offer the Samba accounts shown in the Users tab; principals that only
    // appear in an existing ACL are added when the share is loaded.
    QStringList userList {"Everyone"};
    for (int i = 0; i < ui->listWidgetUsers->count(); ++i) {
        const QString user = ui->listWidgetUsers->item(i)->text();
        if (!user.isEmpty() && !userList.contains(user)) {
            userList << user;
        }
    }

    for (const QString &user : userList) {
        editshare->addUser(user);
    }

    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Ignored, QSizePolicy::Expanding));
}

void MainWindow::refreshShareList()
{
    ui->treeWidgetShares->clear();
    ui->labelSambaSharesFound->hide();

    const int result = run("net", {"usershare", "info"});
    if (result != 0) {
        QString details = QString::fromLocal8Bit(proc.readAllStandardError()).trimmed();
        if (details.isEmpty()) {
            details = result == -1 ? proc.errorString() : tr("Command exited with status %1.").arg(result);
        }
        QMessageBox box(QMessageBox::Critical, tr("Error"), tr("Error listing shares"), QMessageBox::Ok, this);
        box.setInformativeText(tr("Could not read Samba usershares. Shares defined in /etc/samba/smb.conf "
                                  "are managed separately and are not listed here."));
        box.setDetailedText("net usershare info\n\n" + details);
        box.exec();
        return;
    }

    const QString output = proc.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        ui->labelSambaSharesFound->show();
        return;
    }

    const QStringList listShares = output.split("\n\n");
    qDebug() << listShares;

    for (const QString &share : listShares) {
        QStringList shareDetails = share.split('\n');
        if (shareDetails.size() < 5) {
            continue;
        }

        auto removePattern = [](QString &str, const QString &pattern) {
            if (!str.isEmpty()) {
                str.remove(QRegularExpression(pattern));
            }
        };

        shareDetails.first().remove(QRegularExpression("^\\[")).remove(QRegularExpression("]$"));
        removePattern(shareDetails[1], "^path=");
        removePattern(shareDetails[2], "^comment=");
        removePattern(shareDetails[3], "^usershare_acl=");
        shareDetails[3].remove(QRegularExpression(",$"));
        removePattern(shareDetails[4], "^guest_ok=");

        ui->treeWidgetShares->insertTopLevelItem(0, new QTreeWidgetItem(shareDetails));
    }

    for (int i = 0; i < ui->treeWidgetShares->columnCount(); ++i) {
        ui->treeWidgetShares->resizeColumnToContents(i);
    }

    connect(ui->treeWidgetShares, &QTreeWidget::itemDoubleClicked, this, &MainWindow::pushEditShare_clicked, Qt::UniqueConnection);
}

void MainWindow::refreshUserList()
{
    ui->listWidgetUsers->clear();
    ui->labelUserNotFound->hide();

    const QStringList &users = listUsers();
    if (users.isEmpty()) {
        ui->labelUserNotFound->show();
    } else {
        ui->listWidgetUsers->addItems(users);
    }
}

/* Convenience function for running an external command that takes a considerable amount of time
 *  -- returns when the process ends, but doesn't freeze the GUI (can update progress bars, etc)
 * For quick commands system() calls are probably more efficient, GUI freezes
 * For non-blocking commands proc.start() */
int MainWindow::run(const QString &cmd, const QStringList &args, const QByteArray &input)
{
    if (commandRunning) {
        return -1;
    }
    QScopedValueRollback<bool> commandGuard(commandRunning, true);
    const bool wasEnabled = isEnabled();
    const bool hadExplicitCursor = testAttribute(Qt::WA_SetCursor);
    const QCursor previousCursor = cursor();
    const QPointer<QWidget> previousFocus = QApplication::focusWidget();
    bool disabledForCommand = false;
    const auto restoreUi = qScopeGuard([this, wasEnabled, hadExplicitCursor, previousCursor, previousFocus,
                                      &disabledForCommand]() {
        setEnabled(wasEnabled);
        if (hadExplicitCursor) {
            setCursor(previousCursor);
        } else {
            unsetCursor();
        }
        if (disabledForCommand && previousFocus && previousFocus->isEnabled() && previousFocus->isVisible()) {
            previousFocus->setFocus();
        }
    });
    setCursor(QCursor(Qt::BusyCursor));
    QEventLoop loop;
    QTimer disableTimer(&loop);
    disableTimer.setSingleShot(true);
    connect(&disableTimer, &QTimer::timeout, this, [this, wasEnabled, &disabledForCommand]() {
        if (proc.state() != QProcess::NotRunning) {
            disabledForCommand = wasEnabled;
            setEnabled(false);
        }
    });
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    const QIODevice::OpenMode mode = input.isEmpty() ? QIODevice::ReadOnly : QIODevice::ReadWrite;
    proc.start(cmd, args, mode);
    if (!proc.waitForStarted()) {
        return -1;
    }
    disableTimer.start(150);
    if (!input.isEmpty()) {
        proc.write(input);
        proc.closeWriteChannel();
    }
    if (proc.state() != QProcess::NotRunning) {
        loop.exec();
    }
    return proc.exitStatus() == QProcess::NormalExit ? proc.exitCode() : -1;
}

// The [homes] section in smb.conf exports each Samba user's home folder to
// that user. It is not a usershare, so it never shows in the share list.
void MainWindow::checkHomesShare()
{
    if (run("testparm", {"-s", "--section-name=homes", "--parameter-name=read only"}) != 0) {
        return;
    }
    const bool readOnly = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed().compare("No", Qt::CaseInsensitive) != 0;
    const QString host = QSysInfo::machineHostName().section('.', 0, 0);
    ui->labelHomesNote->setText(
        (readOnly ? tr("Each Samba user can also open their own home folder, read-only, as \\\\%1\\<user name>.")
                  : tr("Each Samba user can also open their own home folder, with write access, as \\\\%1\\<user name>."))
            .arg(host)
        + ' ' + tr("This is set by the [homes] section in /etc/samba/smb.conf."));
    ui->labelHomesNote->show();
}

// The name Samba qualifies local accounts with in ACLs, e.g. "MX" in "MX\adrian"
QString MainWindow::netbiosName()
{
    if (run("testparm", {"-s", "--parameter-name=netbios name"}) == 0) {
        const QString configuredName = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed();
        if (!configuredName.isEmpty()) {
            return configuredName;
        }
    }
    return QSysInfo::machineHostName().section('.', 0, 0).left(15).toUpper();
}

void MainWindow::checkSambashareGroup()
{
    QProcess groups;
    groups.start("groups", {});
    groups.waitForFinished();
    const auto groupList = QString::fromLocal8Bit(groups.readAllStandardOutput()).split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (!groupList.contains("sambashare")) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Your user doesn't belong to 'sambashare' group  "
                                 "if you just installed the app you might need to restart the system first."));
        exit(EXIT_FAILURE);
    }
}

void MainWindow::checksamba()
{
    const QString sambaPath = "/usr/sbin/smbd";
    if (!QFileInfo::exists(sambaPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Samba is not installed"));
        return;
    }

    auto updateUI = [this](bool isRunning, bool enabled) {
        ui->pushStartStopSamba->setText(isRunning ? tr("Sto&p Samba") : tr("Star&t Samba"));
        ui->textSambaStatus->setText(isRunning ? tr("Samba is running") : tr("Samba is not running"));
        ui->textServiceStatus->setText(enabled ? tr("Samba autostart is enabled") : tr("Samba autostart is disabled"));
        ui->pushEnableDisableSamba->setText(enabled ? tr("&Disable Automatic Samba Startup")
                                                    : tr("E&nable Automatic Samba Startup"));
    };

    bool isRunning = (run("pgrep", {"smbd"}) == 0);

    bool enabled = false;
    if (run("grep", {"-q", "systemd", "/proc/1/comm"}) == 0) {
        QProcess isEnabled;
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("LANG", "C");
        isEnabled.setProcessEnvironment(env);
        isEnabled.start("systemctl", {"is-enabled", "smbd"});
        isEnabled.waitForFinished();
        if (QString::fromLocal8Bit(isEnabled.readAllStandardOutput()).trimmed() == "enabled") {
            enabled = true;
        }
    } else {
        // Disabled services keep K links, so look for an S link in the
        // multi-user runlevels
        for (const QString runlevel : {"2", "3", "4", "5"}) {
            if (!QDir("/etc/rc" + runlevel + ".d").entryList({"S??smbd"}, QDir::Files | QDir::System).isEmpty()) {
                enabled = true;
                break;
            }
        }
    }

    updateUI(isRunning, enabled);
}

void MainWindow::disablesamba()
{
    run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-lib", "disablesamba"});
}

void MainWindow::enablesamba()
{
    run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-lib", "enablesamba"});
}

void MainWindow::startsamba()
{
    run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-lib", "startsamba"});
}

void MainWindow::stopsamba()
{
    run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-lib", "stopsamba"});
}

void MainWindow::pushEnableDisableSamba_clicked()
{
    if (ui->pushEnableDisableSamba->text() == tr("E&nable Automatic Samba Startup")) {
        enablesamba();
    } else {
        disablesamba();
    }
    checksamba();
}

void MainWindow::pushStartStopSamba_clicked()
{
    if (ui->pushStartStopSamba->text() == tr("Star&t Samba")) {
        startsamba();
    } else {
        stopsamba();
    }

    checksamba();
    refreshShareList();
    refreshUserList();
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        this,
        tr("About %1").arg(tr("MX Samba Config")),
        R"(<p align="center"><b><h2>MX Samba Config</h2></b></p><p align="center">)" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Program for configuring Samba shares and users.")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        docPath(QStringLiteral("license.html")), tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    displayDoc(this, docPath(QStringLiteral("mx-samba-config.html")), tr("%1 Help").arg(windowTitle()), true);
}

void MainWindow::pushRemoveUser_clicked()
{
    const auto selectedItems = ui->listWidgetUsers->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    const QString user = selectedItems.first()->text();

    // Find shares whose ACL names this local user, plain or qualified with the
    // local NetBIOS name. Domain accounts and SIDs may be other accounts.
    struct ShareUpdate {
        QTreeWidgetItem *item;
        QStringList remaining;
    };
    QList<ShareUpdate> updates;
    QStringList updatedNames;
    QStringList removedNames;
    const QString localNetbiosName = netbiosName();
    for (int i = 0; i < ui->treeWidgetShares->topLevelItemCount(); ++i) {
        auto *item = ui->treeWidgetShares->topLevelItem(i);
        QStringList remaining;
        bool listed = false;
        for (const QString &entry : item->text(3).split(',', Qt::SkipEmptyParts)) {
            const QString principal = entry.section(':', 0, 0);
            const QString qualifier = principal.contains('\\') ? principal.section('\\', 0, -2) : QString();
            const bool isUser = principal.section('\\', -1).compare(user, Qt::CaseInsensitive) == 0
                                && (qualifier.isEmpty() || qualifier.compare(localNetbiosName, Qt::CaseInsensitive) == 0);
            if (isUser) {
                listed = true;
            } else {
                // net usershare add rejects some uppercase letters that info prints
                remaining << principal + ':' + entry.section(':', 1).toLower();
            }
        }
        if (listed) {
            // A share left with only Deny entries is as unreachable as one left
            // with none, and an empty ACL would make net usershare add default
            // to Everyone:R, so offer to remove both instead
            const bool grantsAccess = std::any_of(remaining.cbegin(), remaining.cend(),
                                                  [](const QString &entry) { return !entry.endsWith(":d"); });
            if (!grantsAccess) {
                remaining.clear();
            }
            updates.append({item, remaining});
            (remaining.isEmpty() ? removedNames : updatedNames) << item->text(0);
        }
    }

    bool updateShares = false;
    if (!updates.isEmpty()) {
        QString text = tr("%1 is listed on these shares:").arg(user);
        if (!updatedNames.isEmpty()) {
            text += "\n\n" + tr("Remove %1's access from: %2").arg(user, updatedNames.join(", "));
        }
        if (!removedNames.isEmpty()) {
            text += "\n\n" + tr("Remove these shares, which no one else can access: %1").arg(removedNames.join(", "));
        }
        QMessageBox box(QMessageBox::Question, tr("Remove User"), text, QMessageBox::NoButton, this);
        auto *removeButton = box.addButton(tr("Remove from Shares"), QMessageBox::AcceptRole);
        auto *keepButton = box.addButton(tr("Keep Entries"), QMessageBox::ActionRole);
        box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(removeButton);
        box.exec();
        if (box.clickedButton() != removeButton && box.clickedButton() != keepButton) {
            return;
        }
        updateShares = box.clickedButton() == removeButton;
    }

    if (run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-lib", "removesambauser", user}) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot delete user: ") + user);
        refreshUserList();
        return;
    }

    if (updateShares) {
        QStringList failed;
        for (const ShareUpdate &update : updates) {
            const QString name = update.item->text(0);
            const int result = update.remaining.isEmpty()
                ? run("net", {"usershare", "delete", name})
                : run("net", {"usershare", "add", name, update.item->text(1), update.item->text(2),
                              update.remaining.join(','), update.item->text(4) == "y" ? "guest_ok=y" : "guest_ok=n"});
            if (result != 0) {
                failed << name;
            }
        }
        refreshShareList();
        if (!failed.isEmpty()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("%1 was removed, but these shares could not be updated: %2").arg(user, failed.join(", ")));
        }
    }
    refreshUserList();
}

void MainWindow::pushAddUser_clicked()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(tr("Enter the username and password:")));

    auto *username = new QLineEdit(&dialog);
    auto *password = new QLineEdit(&dialog);
    auto *password2 = new QLineEdit(&dialog);
    password->setEchoMode(QLineEdit::Password);
    password2->setEchoMode(QLineEdit::Password);
    form.addRow(tr("Username:"), username);
    form.addRow(tr("Password:"), password);
    form.addRow(tr("Confirm password:"), password2);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        const QString userText = username->text();
        const QString passText = password->text();
        const QString passText2 = password2->text();

        if (userText.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Empty username, please enter a name."));
            return;
        }
        // getent also matches numeric UIDs; accept only a name match and use the
        // canonical spelling NSS reports
        const QString accountName = run("getent", {"passwd", "--", userText}) == 0
            ? QString::fromLocal8Bit(proc.readAllStandardOutput()).section(':', 0, 0)
            : QString();
        if (accountName.compare(userText, Qt::CaseInsensitive) != 0) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Matching linux user not found on system, "
                                     "make sure you enter a valid username."));
            return;
        }
        if (passText.isEmpty() || passText2.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Password fields cannot be empty."));
            return;
        }
        if (passText != passText2) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }
        QStringList args {"/usr/lib/mx-samba-config/mx-samba-config-lib", "addsambauser", accountName};
        QByteArray passwordInput = passText.toUtf8();
        passwordInput.append('\n');
        if (run("pkexec", args, passwordInput) != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not add user."));
            return;
        }
    }
    refreshUserList();
}

void MainWindow::pushUserPassword_clicked()
{
    const auto selectedItems = ui->listWidgetUsers->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No user selected."));
        return;
    }

    const QString currentUser = selectedItems.first()->text();
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(tr("Change the password for '%1'").arg(currentUser)));

    auto *password = new QLineEdit(&dialog);
    auto *passwordConfirm = new QLineEdit(&dialog);
    password->setEchoMode(QLineEdit::Password);
    passwordConfirm->setEchoMode(QLineEdit::Password);
    form.addRow(tr("Password:"), password);
    form.addRow(tr("Confirm password:"), passwordConfirm);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        const QString passwordText = password->text();
        const QString passwordConfirmText = passwordConfirm->text();

        if (passwordText.isEmpty() || passwordConfirmText.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Password fields cannot be empty."));
            return;
        }

        if (passwordText != passwordConfirmText) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }

        QStringList args {"/usr/lib/mx-samba-config/mx-samba-config-lib", "changesambapasswd", currentUser};
        QByteArray passwordInput = passwordText.toUtf8();
        passwordInput.append('\n');
        if (run("pkexec", args, passwordInput) != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not change password."));
            return;
        }
        QMessageBox::information(this, tr("Success"), tr("Password changed successfully for '%1'.").arg(currentUser));
    }
}

void MainWindow::pushRemoveShare_clicked()
{
    const auto selectedItems = ui->treeWidgetShares->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No share selected."));
        return;
    }

    const QString share = selectedItems.first()->text(0);
    if (share.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Selected share is empty."));
        return;
    }

    if (run("net", {"usershare", "delete", share}) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot delete share: ") + share);
        return;
    }

    refreshShareList();
    QMessageBox::information(this, tr("Success"), tr("Share deleted successfully: ") + share);
}

void MainWindow::pushEditShare_clicked()
{
    if (commandRunning) {
        return;
    }
    const auto selectedItems = ui->treeWidgetShares->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No share selected."));
        return;
    }

    if (run("pgrep", {"smbd"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Samba service is not running. Please start Samba before adding or editing shares"));
        return;
    }

    EditShare editshare(this);
    editshare.setWindowTitle(tr("Edit Share"));
    buildUserList(&editshare);

    auto *selectedItem = selectedItems.first();
    editshare.ui->textShareName->setText(selectedItem->text(0));
    editshare.ui->textShareName->setReadOnly(true);
    editshare.ui->textShareName->setToolTip(tr("Share names cannot be changed when editing an existing share."));
    editshare.ui->labelEditNote->show();
    editshare.ui->textSharePath->setText(selectedItem->text(1));
    editshare.ui->textComment->setText(selectedItem->text(2));
    editshare.ui->comboGuestOK->setCurrentIndex(selectedItem->text(4) == "y" ? 0 : 1);

    QStringList permissionList = selectedItem->text(3).split(',', Qt::SkipEmptyParts);

    const QString localNetbiosName = netbiosName();

    // Only reuse a bare-name control for an unknown qualifier when no other
    // principal in the ACL claims that name. Keep the original principal on save.
    QMap<QString, QStringList> principalsByName;
    for (const QString &item : permissionList) {
        const QString principal = item.section(':', 0, 0);
        const QString name = principal.section('\\', -1);
        if (!principalsByName[name].contains(principal)) {
            principalsByName[name] << principal;
        }
    }

    for (const QString &item : permissionList) {
        const QStringList parts = item.split(':');
        if (parts.size() != 2) {
            QMessageBox::critical(this, tr("Error"), tr("Error processing permissions: ") + item);
            return;
        }

        const QString principal = parts.at(0);
        const qsizetype separator = principal.lastIndexOf('\\');
        const QString qualifier = separator < 0 ? QString() : principal.left(separator);
        const QString user = separator < 0 ? principal : principal.mid(separator + 1);
        const QString permission = parts.at(1).toLower();
        if (permission != "d" && permission != "r" && permission != "f") {
            QMessageBox::critical(this, tr("Error"), tr("Error processing permissions: ") + item);
            return;
        }

        const bool knownLocal = qualifier.isEmpty() || qualifier.compare(localNetbiosName, Qt::CaseInsensitive) == 0;
        auto *groupBox = editshare.ui->frameUsers->findChild<QGroupBox *>(user, Qt::FindDirectChildrenOnly);
        if (!groupBox || (!knownLocal && principalsByName.value(user).size() != 1)) {
            groupBox = editshare.ui->frameUsers->findChild<QGroupBox *>(principal, Qt::FindDirectChildrenOnly);
            if (!groupBox) {
                groupBox = editshare.addUser(principal);
            }
        }

        const QString controlName = groupBox->objectName();
        // Each control holds a single permission, so several rules for one
        // account cannot be represented without dropping or reordering them.
        if (editshare.permissionOrder.contains(controlName)) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("This share has more than one permission rule for %1 and cannot be edited "
                                     "here without changing its access. Use 'net usershare' to edit it.")
                                      .arg(groupBox->property("principal").toString()));
            return;
        }
        editshare.permissionOrder << controlName;
        groupBox->setProperty("principal", principal);
        editshare.addRemoveButton(groupBox);
        if (!knownLocal) {
            groupBox->setTitle(principal);
        }
        const QString controlPrefix = permission == "d" ? "*Deny*" : permission == "r" ? "*ReadOnly*" : "*FullAccess*";
        groupBox->findChild<QRadioButton *>(controlPrefix + controlName)->setChecked(true);
    }

    // Do not offer an unused bare-name rule alongside several qualified entries
    // whose relationship to the local account cannot be established.
    for (auto it = principalsByName.cbegin(); it != principalsByName.cend(); ++it) {
        auto *groupBox = editshare.ui->frameUsers->findChild<QGroupBox *>(it.key(), Qt::FindDirectChildrenOnly);
        if (groupBox && !editshare.permissionOrder.contains(it.key()) && it.value().size() > 1) {
            groupBox->setEnabled(false);
            groupBox->setToolTip(tr("Several existing rules refer to this name; edit them individually."));
        }
    }

    addEditShares(&editshare);
}

void MainWindow::pushAddShare_clicked()
{
    if (run("pgrep", {"smbd"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Samba service is not running. Please start Samba before adding or editing shares"));
        return;
    }

    if (ui->listWidgetUsers->count() == 0) {
        QMessageBox::critical(this, tr("Error"), tr("Please add a Samba user before creating a share."));
        return;
    }

    EditShare editshare(this);
    editshare.setWindowTitle(tr("Add Share"));
    for (int i = 0; i < ui->treeWidgetShares->topLevelItemCount(); ++i) {
        editshare.existingShareNames << ui->treeWidgetShares->topLevelItem(i)->text(0);
    }
    buildUserList(&editshare);
    addEditShares(&editshare);
}

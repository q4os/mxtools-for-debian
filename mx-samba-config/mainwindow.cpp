/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2021 MX Authors
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

#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QRadioButton>
#include <QScreen>
#include <QScrollBar>
#include <QTextStream>

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
        connect(button, &QPushButton::clicked, this, slot);
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

    QStringList userList {":Everyone"};
    run("getent", {"group", "users"});
    userList << QString(proc.readAllStandardOutput()).trimmed().split(',');

    QString permissions;
    for (const QString &user : userList) {
        QString userName = user.section(':', -1);
        if (!permissions.isEmpty()) {
            permissions.append(',');
        }
        if (editshare->findChild<QRadioButton *>("*Deny*" + userName)->isChecked()) {
            permissions += userName + ":d";
        } else if (editshare->findChild<QRadioButton *>("*ReadOnly*" + userName)->isChecked()) {
            permissions += userName + ":r";
        } else if (editshare->findChild<QRadioButton *>("*FullAccess*" + userName)->isChecked()) {
            permissions += userName + ":f";
        }
    }

    const QStringList args {"usershare", "add", shareName, sharePath, comment.isEmpty() ? "" : comment, permissions, guestOK};

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

    const QStringList output = QString(proc.readAllStandardOutput().trimmed()).split('\n');
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
    QStringList userList {":Everyone"}; // Add Everyone with a column in front to follow general format of getent
    run("getent", {"group", "users"});
    userList << QString(proc.readAllStandardOutput()).trimmed().split(',');

    for (const QString &user : userList) {
        QString userName = user.section(':', -1);
        auto *groupBox = new QGroupBox(userName);
        groupBox->setObjectName(userName);
        auto *hbox = new QHBoxLayout;

        auto createRadioButton = [&](const QString &text, const QString &objectName) {
            auto *radio = new QRadioButton(text);
            radio->setObjectName(objectName);
            hbox->addWidget(radio);
            connect(radio, &QRadioButton::pressed, radio, [radio]() { radio->setAutoExclusive(!radio->isChecked()); });
            return radio;
        };

        createRadioButton(tr("&Deny"), "*Deny*" + userName);
        createRadioButton(tr("&Read Only"), "*ReadOnly*" + userName);
        createRadioButton(tr("&Full Access"), "*FullAccess*" + userName);

        hbox->addStretch(1);
        groupBox->setLayout(hbox);
        layout->addWidget(groupBox);
    }
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Ignored, QSizePolicy::Expanding));
}

void MainWindow::refreshShareList()
{
    ui->treeWidgetShares->clear();
    ui->labelSambaSharesFound->hide();

    if (run("net", {"usershare", "info"}) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Error listing shares"));
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
        if (shareDetails.isEmpty()) {
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
int MainWindow::run(const QString &cmd, const QStringList &args)
{
    setCursor(QCursor(Qt::BusyCursor));
    QEventLoop loop;
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    proc.start(cmd, args, QIODevice::ReadOnly);
    loop.exec();
    setCursor(QCursor(Qt::ArrowCursor));
    return proc.exitCode();
}

void MainWindow::checkSambashareGroup()
{
    if (run("/bin/bash", {"-c", "groups | grep -q sambashare"}) != 0) {
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
        if (run("/bin/bash", {"-c", "LANG=C systemctl is-enabled smbd | grep enabled"}) == 0) {
            enabled = true;
        }
    } else {
        if (run("grep", {"-q", "smbd", "/etc/init.d/.depend.start"}) == 0) {
            enabled = true;
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
        tr("About %1").arg(tr("MX Samba Config")),
        R"(<p align="center"><b><h2>MX Samba Config</h2></b></p><p align="center">)" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Program for configuring Samba shares and users.")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-samba-config/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    const QString &url = "https://mxlinux.org/wiki/help-files/help-mx-samba-config/";
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::pushRemoveUser_clicked()
{
    if (ui->listWidgetUsers->currentItem() == nullptr) {
        return;
    }
    const QString &user = ui->listWidgetUsers->currentItem()->text();

    if (run("pkexec", {"/usr/lib/mx-samba-config/mx-samba-config-lib", "removesambauser", user}) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot delete user: ") + user);
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
        if (run("grep", {"^" + userText + ":", "/etc/passwd"}) != 0) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Matching linux user not found on system, "
                                     "make sure you enter a valid username."));
            return;
        }
        if (passText != passText2) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }
        QStringList args {"/usr/lib/mx-samba-config/mx-samba-config-lib", "addsambauser", passText, userText};
        if (run("pkexec", args) != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not add user."));
            return;
        }
    }
    refreshUserList();
}

void MainWindow::pushUserPassword_clicked()
{
    auto *currentItem = ui->listWidgetUsers->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("Warning"), tr("No user selected."));
        return;
    }

    const QString currentUser = currentItem->text();
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

        const QStringList args {"/usr/lib/mx-samba-config/mx-samba-config-lib", "changesambapasswd", passwordText, currentUser};
        if (run("pkexec", args) != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not change password."));
            return;
        }
    }
}

void MainWindow::pushRemoveShare_clicked()
{
    auto *currentItem = ui->treeWidgetShares->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("Warning"), tr("No share selected."));
        return;
    }

    const QString share = currentItem->text(0);
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
    auto *currentItem = ui->treeWidgetShares->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, tr("Warning"), tr("No share selected."));
        return;
    }

    if (run("pgrep", {"smbd"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Samba service is not running. Please start Samba before adding or editing shares"));
        return;
    }

    auto *editshare = new EditShare;
    buildUserList(editshare);

    auto selectedItem = ui->treeWidgetShares->selectedItems().at(0);
    editshare->ui->textShareName->setText(selectedItem->text(0));
    editshare->ui->textSharePath->setText(selectedItem->text(1));
    editshare->ui->textComment->setText(selectedItem->text(2));
    editshare->ui->comboGuestOK->setCurrentIndex(selectedItem->text(4) == "y" ? 0 : 1);

    QStringList permissionList = selectedItem->text(3).split(',', Qt::SkipEmptyParts);

    for (const QString &item : permissionList) {
        const QStringList parts = item.split(':');
        if (parts.size() != 2) {
            QMessageBox::critical(this, tr("Error"), tr("Error processing permissions: ") + item);
            return;
        }

        QString user = parts.at(0).section('\\', -1);
        const QString permission = parts.at(1).toLower();
        QRadioButton *button = nullptr;

        if (permission == "d") {
            button = editshare->findChild<QRadioButton *>("*Deny*" + user);
        } else if (permission == "r") {
            button = editshare->findChild<QRadioButton *>("*ReadOnly*" + user);
        } else if (permission == "f") {
            button = editshare->findChild<QRadioButton *>("*FullAccess*" + user);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Error processing permissions: ") + item);
            return;
        }

        if (button) {
            button->setChecked(true);
        }
    }

    addEditShares(editshare);
}

void MainWindow::pushAddShare_clicked()
{
    if (run("pgrep", {"smbd"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Samba service is not running. Please start Samba before adding or editing shares"));
        return;
    }
    auto *editshare = new EditShare;
    buildUserList(editshare);
    addEditShares(editshare);
}

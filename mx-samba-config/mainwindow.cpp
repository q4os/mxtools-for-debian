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
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    setConnections();

    const QSize &size = this->size();
    if (settings.contains(QStringLiteral("geometry"))) {
        restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
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
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    delete ui;
}

void MainWindow::centerWindow()
{
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::setConnections()
{
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushAddShare, &QPushButton::clicked, this, &MainWindow::pushAddShare_clicked);
    connect(ui->pushAddUser, &QPushButton::clicked, this, &MainWindow::pushAddUser_clicked);
    connect(ui->pushEditShare, &QPushButton::clicked, this, &MainWindow::pushEditShare_clicked);
    connect(ui->pushEnableDisableSamba, &QPushButton::clicked, this, &MainWindow::pushEnableDisableSamba_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushRemoveShare, &QPushButton::clicked, this, &MainWindow::pushRemoveShare_clicked);
    connect(ui->pushRemoveUser, &QPushButton::clicked, this, &MainWindow::pushRemoveUser_clicked);
    connect(ui->pushStartStopSamba, &QPushButton::clicked, this, &MainWindow::pushStartStopSamba_clicked);
    connect(ui->pushUserPassword, &QPushButton::clicked, this, &MainWindow::pushUserPassword_clicked);
}

void MainWindow::addEditShares(EditShare *editshare)
{
    if (editshare->exec() == QDialog::Accepted) {
        if (editshare->ui->textShareName->text().isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Error, could not add share. Empty share name"));
            return;
        }
        if (!QFileInfo::exists(editshare->ui->textSharePath->text())) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Path: %1 doesn't exist.").arg(editshare->ui->textSharePath->text()));
            return;
        }
        QStringList list {":Everyone"}; // add Everyone with a column in front to follow general format of getent
        run(QStringLiteral("getent"), QStringList {"group", "users"});
        list << QString(proc.readAllStandardOutput()).trimmed().split(QStringLiteral(","));
        QString permissions;
        for (const QString &item : qAsConst(list)) {
            QString user = item.section(QStringLiteral(":"), -1);
            if (!permissions.isEmpty()) {
                permissions += QLatin1String(",");
            }
            if (editshare->findChild<QRadioButton *>("*Deny*" + user)->isChecked()) {
                permissions += user + ":d";
            } else if (editshare->findChild<QRadioButton *>("*ReadOnly*" + user)->isChecked()) {
                permissions += user + ":r";
            } else if (editshare->findChild<QRadioButton *>("*FullAccess*" + user)->isChecked()) {
                permissions += user + ":f";
            }
            permissions.remove(QRegularExpression(QStringLiteral(",$")));
        }
        const QStringList &args {"usershare",
                                 "add",
                                 editshare->ui->textShareName->text(),
                                 editshare->ui->textSharePath->text(),
                                 editshare->ui->textComment->text().isEmpty() ? "" : editshare->ui->textComment->text(),
                                 permissions,
                                 editshare->ui->comboGuestOK->currentText() == tr("Yes") ? "guest_ok=y" : "guest_ok=n"};
        if (run(QStringLiteral("net"), args) != 0) {
            QMessageBox::critical(
                this, tr("Error"),
                tr("Could not add share. Error message:\n\n%1").arg(QString(proc.readAllStandardError())));
            return;
        }
        refreshShareList();
    }
}

QStringList MainWindow::listUsers()
{
    run(QStringLiteral("pkexec"), QStringList {"/usr/lib/mx-samba-config/mx-samba-config-list-users"});
    if (proc.exitCode() != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Error listing users"));
        return {};
    }
    const QStringList output = QString(proc.readAllStandardOutput().trimmed()).split(QStringLiteral("\n"));
    QStringList list;
    list.reserve(output.size());
    if (output.isEmpty()) {
        return {};
    }
    for (const QString &item : output) {
        list << item.section(QStringLiteral(":"), 0, 0);
    }
    list.sort();
    return list;
}

void MainWindow::buildUserList(EditShare *editshare)
{
    auto *layout = editshare->ui->frameUsers->layout();
    QStringList list {":Everyone"}; // add Everyone with a column in front to follow general format of getent
    run(QStringLiteral("getent"), QStringList {"group", "users"});
    list << QString(proc.readAllStandardOutput()).trimmed().split(QStringLiteral(","));
    for (const QString &item : qAsConst(list)) {
        QString user = item.section(QStringLiteral(":"), -1);
        auto *groupBox = new QGroupBox(user);
        groupBox->setObjectName(user);
        auto *hbox = new QHBoxLayout;

        auto *radio = new QRadioButton(tr("&Deny"));
        radio->setObjectName("*Deny*" + user);
        hbox->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio]() {
            if (radio->isChecked()) {
                radio->setAutoExclusive(false);
            } else {
                radio->setAutoExclusive(true);
            }
        });

        radio = new QRadioButton(tr("&Read Only"));
        radio->setObjectName("*ReadOnly*" + user);
        hbox->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio]() {
            if (radio->isChecked()) {
                radio->setAutoExclusive(false);
            } else {
                radio->setAutoExclusive(true);
            }
        });

        radio = new QRadioButton(tr("&Full Access"));
        radio->setObjectName("*FullAccess*" + user);
        hbox->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio]() {
            if (radio->isChecked()) {
                radio->setAutoExclusive(false);
            } else {
                radio->setAutoExclusive(true);
            }
        });

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

    run(QStringLiteral("net"), QStringList {"usershare", "info"});
    if (proc.exitCode() != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Error listing shares"));
        return;
    }
    const QString &output = proc.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        ui->labelSambaSharesFound->show();
        return;
    }
    const QStringList &listShares {output.split("\n\n")};
    qDebug() << listShares;
    for (const QString &share : listShares) {
        QStringList list = share.split(QStringLiteral("\n"));
        if (list.isEmpty()) {
            continue;
        }
        list.first()
            .remove(QRegularExpression(QStringLiteral("^\\[")))
            .remove(QRegularExpression(QStringLiteral("]$")));
        if (!list.at(1).isEmpty()) {
            list[1].remove(QRegularExpression(QStringLiteral("^path=")));
        }
        if (!list.at(2).isEmpty()) {
            list[2].remove(QRegularExpression(QStringLiteral("^comment=")));
        }
        if (!list.at(3).isEmpty()) {
            list[3].remove(QRegularExpression(QStringLiteral("^usershare_acl=")));
            list[3].remove(QRegularExpression(QStringLiteral(",$")));
        }
        if (!list.at(4).isEmpty()) {
            list[4].remove(QRegularExpression(QStringLiteral("^guest_ok=")));
        }
        ui->treeWidgetShares->insertTopLevelItem(0, new QTreeWidgetItem(list));
    }
    for (auto i = 0; i < ui->treeWidgetShares->columnCount(); ++i) {
        ui->treeWidgetShares->resizeColumnToContents(i);
    }
    connect(ui->treeWidgetShares, &QTreeWidget::itemDoubleClicked, this, &MainWindow::pushEditShare_clicked,
            Qt::UniqueConnection);
}

void MainWindow::refreshUserList()
{
    ui->listWidgetUsers->clear();
    ui->labelUserNotFound->hide();
    const QStringList &users = listUsers();
    if (!users.isEmpty()) {
        ui->listWidgetUsers->addItems(users);
    }
    if (users.isEmpty()) {
        ui->labelUserNotFound->show();
    }
}

/* Convenience function for running an external command that take a considerable amount of time
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
    if (QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "groups | grep -q sambashare"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Your user doesn't belong to 'sambashare' group  "
                                 "if you just installed the app you might need to restart the system first."));
        exit(EXIT_FAILURE);
    }
}

void MainWindow::checksamba()
{
    if (QFileInfo::exists(QStringLiteral("/usr/sbin/smbd"))) {
        if (QProcess::execute(QStringLiteral("pgrep"), {"smbd"}) == 0) {
            ui->pushStartStopSamba->setText(tr("Sto&p Samba"));
            ui->textSambaStatus->setText(tr("Samba is running"));
        } else {
            ui->pushStartStopSamba->setText(tr("Star&t Samba"));
            ui->textSambaStatus->setText(tr("Samba is not running"));
        }
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Samba not installed"));
        return;
    }
    bool enabled = false;

    if (QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "pgrep --oldest systemd | grep -qw 1"}) == 0) {
        if (QProcess::execute(QStringLiteral("/bin/bash"), {"-c", "LANG=C systemctl is-enabled smbd | grep enabled"})
            == 0) {
            enabled = true;
        }
    } else {
        if (QProcess::execute(QStringLiteral("grep"), {"-q", "smbd", "/etc/init.d/.depend.start"}) == 0) {
            enabled = true;
        }
    }

    if (enabled) {
        ui->textServiceStatus->setText(QStringLiteral("Samba autostart is enabled"));
        ui->pushEnableDisableSamba->setText(tr("&Disable Automatic Samba Startup"));
    } else {
        ui->textServiceStatus->setText(QStringLiteral("Samba autostart is disabled"));
        ui->pushEnableDisableSamba->setText(tr("E&nable Automatic Samba Startup"));
    }
}

void MainWindow::disablesamba()
{
    run(QStringLiteral("pkexec"), QStringList {"/usr/lib/mx-samba-config/mx-samba-config-lib", "disablesamba"});
}

void MainWindow::enablesamba()
{
    run(QStringLiteral("pkexec"), QStringList {"/usr/lib/mx-samba-config/mx-samba-config-lib", "enablesamba"});
}

void MainWindow::startsamba()
{
    run(QStringLiteral("pkexec"), QStringList {"/usr/lib/mx-samba-config/mx-samba-config-lib", "startsamba"});
}

void MainWindow::stopsamba()
{
    run(QStringLiteral("pkexec"), QStringList {"/usr/lib/mx-samba-config/mx-samba-config-lib", "stopsamba"});
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
        QStringLiteral("/usr/share/doc/mx-samba-config/license.html"), tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    const QString &url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-mx-samba-config/");
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::pushRemoveUser_clicked()
{
    if (ui->listWidgetUsers->currentItem() == nullptr) {
        return;
    }
    const QString &user = ui->listWidgetUsers->currentItem()->text();

    if (run(QStringLiteral("pkexec"),
            QStringList {"/usr/lib/mx-samba-config/mx-samba-config-lib", "removesambauser", user})
        != 0) {
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
        if (username->text().isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Empty username, please enter a name."));
            return;
        }
        if (QProcess::execute(QStringLiteral("grep"), {"^" + username->text() + ":", "/etc/passwd"}) != 0) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Matching linux user not found on system, "
                                     "make sure you enter a valid username."));
            return;
        }
        if (password->text() != password2->text()) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }
        QStringList args {"/usr/lib/mx-samba-config/mx-samba-config-lib", "addsambauser", password->text(),
                          username->text()};
        if (run(QStringLiteral("pkexec"), args) != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not add user."));
            return;
        }
    }
    refreshUserList();
}

void MainWindow::pushUserPassword_clicked()
{
    if (ui->listWidgetUsers->currentItem() == nullptr) {
        return;
    }
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(tr("Change the password for \'%1\'").arg(ui->listWidgetUsers->currentItem()->text())));

    auto *password = new QLineEdit(&dialog);
    auto *password2 = new QLineEdit(&dialog);
    password->setEchoMode(QLineEdit::Password);
    password2->setEchoMode(QLineEdit::Password);
    form.addRow(tr("Password:"), password);
    form.addRow(tr("Confirm password:"), password2);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (password->text() != password2->text()) {
            QMessageBox::critical(this, tr("Error"), tr("Passwords don't match, please enter again."));
            return;
        }
        const QStringList &args {"/usr/lib/mx-samba-config/mx-samba-config-lib", "changesambapasswd", password->text(),
                                 ui->listWidgetUsers->currentItem()->text()};
        if (run(QStringLiteral("pkexec"), args) != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not change password."));
            return;
        }
    }
}

void MainWindow::pushRemoveShare_clicked()
{
    if (ui->treeWidgetShares->currentItem() == nullptr) {
        return;
    }
    const QString &share = ui->treeWidgetShares->selectedItems().at(0)->text(0);
    if (share.isEmpty()) {
        return;
    }
    if (QProcess::execute(QStringLiteral("net"), {"usershare", "delete", share}) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot delete share: ") + share);
    }
    refreshShareList();
}

void MainWindow::pushEditShare_clicked()
{
    if (ui->treeWidgetShares->currentItem() == nullptr) {
        return;
    }

    if (QProcess::execute(QStringLiteral("pgrep"), {"smbd"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Samba service is not running. Please start Samba before adding or editing shares"));
        return;
    }

    auto *editshare = new EditShare;
    buildUserList(editshare);
    editshare->ui->textShareName->setText(ui->treeWidgetShares->selectedItems().at(0)->text(0));
    editshare->ui->textSharePath->setText(ui->treeWidgetShares->selectedItems().at(0)->text(1));
    editshare->ui->textComment->setText(ui->treeWidgetShares->selectedItems().at(0)->text(2));
    editshare->ui->comboGuestOK->setCurrentIndex(
        ui->treeWidgetShares->selectedItems().at(0)->text(4) == QLatin1String("y") ? 0 : 1);
    QString permissions = ui->treeWidgetShares->selectedItems().at(0)->text(3);
    QStringList permission_list;
    if (!permissions.isEmpty()) {
        permission_list = permissions.split(QStringLiteral(","));
    }

    for (const QString &item : qAsConst(permission_list)) {
        if (item.isEmpty()) {
            continue;
        }

        const QStringList parts = item.split(':');
        if (parts.size() != 2) {
            QMessageBox::critical(this, tr("Error"), tr("Error: trying to process permissions: ") + item);
            return;
        }

        QString user = parts.at(0);
        if (user.contains('\\')) {
            user = user.section('\\', 1);
        }

        const QString permission = parts.at(1).toLower();
        QRadioButton *button = nullptr;
        if (permission == QLatin1String("d")) {
            button = editshare->findChild<QRadioButton *>("*Deny*" + user);
        } else if (permission == QLatin1String("r")) {
            button = editshare->findChild<QRadioButton *>("*ReadOnly*" + user);
        } else if (permission == QLatin1String("f")) {
            button = editshare->findChild<QRadioButton *>("*FullAccess*" + user);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Error: trying to process permissions: ") + item);
            return;
        }
        if (button != nullptr) {
            button->setChecked(true);
        }
    }
    addEditShares(editshare);
}

void MainWindow::pushAddShare_clicked()
{
    if (QProcess::execute(QStringLiteral("pgrep"), {"smbd"}) != 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Samba service is not running. Please start Samba before adding or editing shares"));
        return;
    }
    auto *editshare = new EditShare;
    buildUserList(editshare);
    addEditShares(editshare);
}

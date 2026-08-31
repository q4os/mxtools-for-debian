/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2019-2026 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *          using live-usb-maker by BitJam
 *          and mx-live-usb-maker gui by adrian
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
#include "about.h"
#include "ui_mainwindow.h"
#include "version.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextStream>

#include <QDebug>
#include <unistd.h>

QString sessionLogPath()
{
    const QString name = QStringLiteral("formatusb.log");
    if (::geteuid() == 0) {
        return QStringLiteral("/run/") + name;
    }
    const QString runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (!runtimeDir.isEmpty() && QFileInfo(runtimeDir).isDir()) {
        return runtimeDir + QLatin1Char('/') + name;
    }
    return QStringLiteral("/tmp/") + name;
}

MainWindow::MainWindow()
    : ui(new Ui::MainWindow)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup();
    ui->comboBoxUsbList->addItems(buildUsbList());
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeUsb(const QString &options)
{
    setConnections();
    qDebug() << cmd->getCmdOut(options);
    // label drive
    // labeldrive();
}

// setup various items first time program runs
void MainWindow::setup()
{
    cmd = new Cmd(this);
    setWindowTitle("Format USB");
    ui->buttonBack->setHidden(true);
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonCancel->setEnabled(true);
    ui->buttonNext->setEnabled(true);
    ui->outputBox->setCursorWidth(0);
    ui->lineEditFSlabel->setText("USB-DATA");
}

// Build the option list to be passed to live-usb-maker
QString MainWindow::buildOptionList()
{
    device = ui->comboBoxUsbList->currentText().split(" ").at(0);
    label = ui->lineEditFSlabel->text();
    QString partOption;
    QString options;

    QString format = ui->comboBoxDataFormat->currentText();
    if (format.contains("fat32")) {
        format = "vfat";
    }

    // 0 = defualts
    // 1 = msdos
    // 2 - gpt

    if (ui->comboBoxPartitionTableType->isEnabled()){
        if (ui->comboBoxPartitionTableType->currentIndex() == 0 ) {
            partOption = "defaults";
        }
        if (ui->comboBoxPartitionTableType->currentIndex() == 1 ) {
            partOption = "msdos";
        }
        if (ui->comboBoxPartitionTableType->currentIndex() == 2 ) {
            partOption = "gpt";
        }
    } else {
        partOption = "part";
    }

    QString authentication = "pkexec";

    if (!QFile::exists("/usr/bin/pkexec") && QFile::exists("/usr/bin/gksu")) {
        authentication = "gksu";
    }

    if (getuid() == 0) {
        authentication = "";
    }

    QString prefix = authentication.isEmpty() ? QString() : authentication + " ";
    options = prefix + "/usr/lib/formatusb/formatusb_lib \"" + device + "\" " + format + " \"" + label
              + "\" " + partOption;
    qDebug() << "partition is" << device << "label " << label;
    qDebug() << "Options: " << options;
    return options;
}

// build the USB list
QStringList MainWindow::buildUsbList()
{
    QString drives;
    if (ui->checkBoxshowpartitions->isChecked()) {
        drives = cmd->getCmdOut("lsblk -nlo NAME,SIZE,LABEL,TYPE -I 3,8,22,179,259 |grep -v disk");
    } else {
        drives = cmd->getCmdOut("lsblk --nodeps -nlo NAME,SIZE,MODEL,VENDOR -I 3,8,22,179,259 ");
    }

    return removeUnsuitable(drives.split("\n"));
}

// remove unsuitable drives from the list (live and unremovable)
QStringList MainWindow::removeUnsuitable(const QStringList &devices)
{
    QStringList list;
    QString name;
    bool showAll = ui->checkBoxShowAll->isChecked();
    for (const QString &line : devices) {
        name = line.split(" ").at(0);
        if (!showAll) {
            if (system(cli_utils.toUtf8() + "is_usb_or_removable " + name.toUtf8()) == 0) {
                if (cmd->getCmdOut(cli_utils + "get_drive $(get_live_dev) ") != name) {
                    list << line;
                }
            }
        } else {
            if (cmd->getCmdOut(cli_utils + "get_drive $(get_live_dev) ") != name) {
                list << line;
            }
        }
    }

    return list;
}

void MainWindow::cmdStart()
{
    // setCursor(QCursor(Qt::BusyCursor));
    // ui->lineEdit->setFocus();
}

void MainWindow::cmdDone()
{
    setCursor(QCursor(Qt::ArrowCursor));
    ui->buttonBack->setEnabled(true);
    if (cmd->exitCode() == 0 && cmd->exitStatus() == QProcess::NormalExit) {
        QMessageBox::information(this, tr("Success"), tr("Format successful!"));
    } else {
        QMessageBox::critical(this, tr("Failure"), tr("Error encountered in the Format process"));
    }
    cmd->disconnect();
}

void MainWindow::setConnections()
{
    // Display output as Cmd reads it. Cmd's constructor already drains stdout/stderr
    // into outBuffer via readAllStandardOutput()/readAllStandardError() on each
    // readyRead*, so connecting to QProcess::readyRead and calling readAll() here would
    // race that and find the channel empty. Consume the chunk Cmd hands us instead.
    connect(cmd, &Cmd::outputAvailable, this, &MainWindow::updateOutput);
    connect(cmd, &Cmd::errorAvailable, this, &MainWindow::updateOutput);
    connect(cmd, &QProcess::started, this, &MainWindow::cmdStart);
    connect(cmd, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::cmdDone);
}

void MainWindow::updateOutput(const QString &output)
{
    // remove escape sequences that are not handled by code
    QString out = output;
    out.remove("[0m")
        .remove("]0;")
        .remove("")
        .remove("")
        .remove("[1000D")
        .remove("[74C|")
        .remove("[?25l")
        .remove("[?25h")
        .remove("[0;36m")
        .remove("[1;37m");
    ui->outputBox->moveCursor(QTextCursor::End);
    ui->outputBox->insertPlainText(out);
    QScrollBar *sb = ui->outputBox->verticalScrollBar();
    sb->setValue(sb->maximum());
    qApp->processEvents();
}

// Next button clicked
void MainWindow::on_buttonNext_clicked()
{

    // on first page
    if (ui->stackedWidget->currentIndex() == 0) {
        if (ui->comboBoxUsbList->currentText() == "") {
            QMessageBox::critical(this, tr("Error"), tr("Please select a USB device to write to"));
            return;
        }

        // confirm action
        QString msg = tr("These actions will destroy all data on \n\n")
                      + ui->comboBoxUsbList->currentText().simplified() + "\n\n " + tr("Do you wish to continue?");
        if (QMessageBox::Yes != QMessageBox::warning(this, windowTitle(), msg, QMessageBox::Yes, QMessageBox::No)) {
            return;
        }
        if (cmd->state() != QProcess::NotRunning) {
            ui->stackedWidget->setCurrentWidget(ui->outputPage);
            return;
        }
        ui->buttonBack->setHidden(false);
        ui->buttonBack->setEnabled(false);
        ui->buttonNext->setEnabled(false);
        ui->stackedWidget->setCurrentWidget(ui->outputPage);

        makeUsb(buildOptionList());

        // do nothing on the output page; quit from any other page
    } else if (ui->stackedWidget->currentWidget() != ui->outputPage) {
        qApp->quit();
    }
}

void MainWindow::on_buttonBack_clicked()
{
    setWindowTitle("Format USB Device");
    ui->stackedWidget->setCurrentIndex(0);
    ui->buttonNext->setEnabled(true);
    ui->buttonBack->setDisabled(true);
    ui->outputBox->clear();
}

// About button clicked
void MainWindow::on_buttonAbout_clicked()
{
    hide();
    displayAboutMsgBox(tr("About %1").arg(windowTitle()),
                       "<p align=\"center\"><b><h2>" + windowTitle() + "</h2></b></p><p align=\"center\">"
                           + tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>"
                           + tr("Program for formatting USB devices")
                           + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br "
                             "/></p><p align=\"center\">"
                            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                       "/usr/share/doc/formatusb/license.html", tr("%1 License").arg(windowTitle()), true);
    show();
}

// Help button clicked
void MainWindow::on_buttonHelp_clicked()
{
    QString url = "/usr/share/doc/formatusb/help/formatusb.html";
    displayDoc(url, tr("%1 Help").arg(windowTitle()), true);
}

void MainWindow::on_buttonRefresh_clicked()
{
    ui->comboBoxUsbList->clear();
    ui->comboBoxUsbList->addItems(buildUsbList());
}

void MainWindow::on_checkBoxShowAll_clicked()
{
    on_buttonRefresh_clicked();
}

void MainWindow::on_checkBoxshowpartitions_clicked()
{
    on_buttonRefresh_clicked();
    if (ui->checkBoxshowpartitions->isChecked()) {
        ui->comboBoxPartitionTableType->setEnabled(false);
    }
    if (!ui->checkBoxshowpartitions->isChecked()) {
        ui->comboBoxPartitionTableType->setEnabled(true);
    }
}

void MainWindow::validate_name()
{
    // see if name is reasonable
    QString test;
    test = ui->lineEditFSlabel->text();

    if (test.isEmpty()) {
        return;
    }
    QString regexString = "^[A-Za-z0-9_.-]{1,11}$";
    if (ui->comboBoxDataFormat->currentText() == "ext4") {
        regexString = "^[A-Za-z0-9_.-]{1,16}$";
    }
    if (ui->comboBoxDataFormat->currentText() == "ntfs") {
        regexString = "^[A-Za-z0-9_.-]{1,32}$";
    }

    if (!test.contains(QRegularExpression(regexString))) {
        if (ui->buttonNext->isEnabled()) {
            QMessageBox::critical(this, tr("Failure"), tr("Invalid Name"));
            ui->buttonNext->setEnabled(false);
        }
    } else {
        ui->buttonNext->setEnabled(true);
    }
}

void MainWindow::on_lineEditFSlabel_textChanged(const QString &arg1)
{
    validate_name();
}

void MainWindow::on_comboBoxDataFormat_currentIndexChanged(int index)
{
    validate_name();
}

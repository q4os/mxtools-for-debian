/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-remastercc.
 *
 * mx-remastercc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-remastercc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-remastercc.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "about.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setConnections();
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    setup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Setup versious items first time program runs
void MainWindow::setup()
{
    setWindowTitle(tr("MX Remaster Control Center"));
    adjustSize();
    ui->pushSetupPersistence->setStyleSheet("text-align:left;");
    ui->pushConfigPersistence->setStyleSheet("text-align:left;");
    ui->pushSaveRootPersist->setStyleSheet("text-align:left;");
    ui->pushRemaster->setStyleSheet("text-align:left;");
    ui->pushSaveRootPersist->setIcon(QIcon::fromTheme("filesave", QIcon(":/icons/filesave.svg")));
}

void MainWindow::setConnections()
{
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushCancel, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->pushConfigPersistence, &QPushButton::clicked, this, &MainWindow::pushConfigPersistence_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushRemaster, &QPushButton::clicked, this, &MainWindow::pushRemaster_clicked);
    connect(ui->pushSaveRootPersist, &QPushButton::clicked, this, &MainWindow::pushSaveRootPersist_clicked);
    connect(ui->pushSetupPersistence, &QPushButton::clicked, this, &MainWindow::pushSetupPersistence_clicked);
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About MX Remaster Control Center"),
        "<p align=\"center\"><b><h2>" + tr("MX Remaster Control Center") + "</h2></b></p><p align=\"center\">"
            + tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>"
            + tr("This program provides access to different remaster and persistence tools in MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-remastercc/license.html", tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::pushHelp_clicked()
{
    displayHelpDoc("/usr/share/doc/mx-remastercc/mx-remastercc.html", tr("%1 Help").arg(windowTitle()));
}

void MainWindow::pushSetupPersistence_clicked()
{
    hide();
    QString cmd = QFile::exists("/usr/local/bin/persist-makefs") ?
                 "/usr/local/bin/persist-makefs" : "/usr/bin/persist-makefs";
    QProcess::execute("pkexec", {cmd});
    show();
}

void MainWindow::pushConfigPersistence_clicked()
{
    hide();
    QString cmd = QFile::exists("/usr/local/bin/persist-config") ?
                 "/usr/local/bin/persist-config" : "/usr/bin/persist-config";
    QProcess::execute("pkexec", {cmd});
    show();
}

void MainWindow::pushSaveRootPersist_clicked()
{
    hide();
    QString cmd = QFile::exists("/usr/local/bin/persist-save") ?
                 "/usr/local/bin/persist-save" : "/usr/bin/persist-save";
    QProcess::execute("pkexec", {cmd});
    show();
}

void MainWindow::pushRemaster_clicked()
{
    hide();
    QString cmd = QFile::exists("/usr/local/bin/live-remaster") ?
                 "/usr/local/bin/live-remaster" : "/usr/bin/live-remaster";
    QProcess::execute("pkexec", {cmd});
    show();
}

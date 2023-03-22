/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/

#include <QApplication>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSplitter>
#include <QToolBar>

#include "about.h"
#include "Clib.h"
#include "CronModel.h"
#include "CronView.h"
#include "Crontab.h"
#include "ExecuteList.h"
#include "ExecuteView.h"
#include "MainWindow.h"
#include "QCloseEvent"
#include "SaveDialog.h"
#include "TCommandEdit.h"
#include "VariableEdit.h"
#include "Version.h"

MainWindow::MainWindow()
{
    readSettings();
    createActions();

    //	statusBar();

    auto *cronModel = new CronModel(&crontabs);
    cronView = new CronView(cronModel);
    auto *tCommandEdit = new TCommandEdit();
    auto *variableEdit = new VariableEdit();
    executeList = new ExecuteList(exeMaxNum, exeMaxDate, &crontabs);

    auto *tab = new QTabWidget;
    {
        tab->addTab(tCommandEdit, QIcon::fromTheme(QStringLiteral("edit-symbolic"), QIcon(":/images/edit_small.png")),
                    tr("&Command"));
        tab->addTab(variableEdit, QIcon::fromTheme(QStringLiteral("edit-tag-symbolic"), QIcon(":/images/edit_small.png")),
                    tr("&Variables"));
        tab->addTab(executeList, QIcon::fromTheme(QStringLiteral("view-list-symbolic"), QIcon(":/images/view_text.png")),
                    tr("&Job List"));
    }

    auto *spl = new QSplitter;
    {
        spl->addWidget(cronView);
        spl->addWidget(tab);
    }

    connect(cronView, &CronView::viewSelected, this, &MainWindow::changeCurrent);
    connect(cronView, &CronView::viewSelected, tCommandEdit, &TCommandEdit::changeCurrent);
    connect(cronView, &CronView::viewSelected, variableEdit, &VariableEdit::changeCurrent);
    connect(cronView, &CronView::viewSelected, executeList, &ExecuteList::changeCurrent);
    connect(executeList->executeView, &ExecuteView::viewSelected, cronView, &CronView::changeCurrent);

    connect(cronView, &CronView::dataChanged, this, &MainWindow::dataChanged);
    connect(cronView, &CronView::dataChanged, executeList, &ExecuteList::dataChanged);
    connect(tCommandEdit, &TCommandEdit::dataChanged, this, &MainWindow::dataChanged);
    connect(tCommandEdit, &TCommandEdit::dataChanged, executeList, &ExecuteList::dataChanged);
    connect(tCommandEdit, &TCommandEdit::dataChanged, cronView, &CronView::tCommandChanged);
    connect(variableEdit, &VariableEdit::dataChanged, this, &MainWindow::dataChanged);

    connect(deleteAction, &QAction::triggered, cronView, &CronView::removeTCommand);
    connect(copyAction, &QAction::triggered, cronView, &CronView::copyTCommand);
    connect(cutAction, &QAction::triggered, cronView, &CronView::cutTCommand);
    connect(pasteAction, &QAction::triggered, cronView, &CronView::pasteTCommand);
    connect(newAction, &QAction::triggered, cronView, &CronView::newTCommand);

    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveCron);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::reloadCron);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::AboutJobScheduler);
    connect(helpAction, &QAction::triggered, this, &MainWindow::displayHelp);
    connect(cronView, &CronView::pasted, pasteAction, &QAction::setEnabled);
    connect(chuserAction, &QAction::triggered, this, &MainWindow::changeUser);

    initCron();

    if (this->isMaximized()) this->resize(QSize(670,480)); // reset size if started maximized
    cronView->resize(viewSize);

    setWindowTitle(Clib::uName() + " - " + tr("Job Scheduler"));
    setWindowIcon(QIcon(":/images/job-scheduler.svg"));

    setCentralWidget(spl);
}

void MainWindow::changeUser()
{
    saveCron();
    if (Clib::uId() != 0) {
        QProcess::startDetached(QStringLiteral("job-scheduler-launcher"), {});
    } else {
        QProcess proc;
        proc.start(QStringLiteral("logname"), {}, QIODevice::ReadOnly);
        proc.waitForFinished();
        QString user = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        QProcess::startDetached(QStringLiteral("runuser"), {QStringLiteral("-u"), user, QStringLiteral("job-scheduler")});
    }
    qApp->quit();
}

void MainWindow::createActions()
{

    auto *fileMenu = new QMenu(tr("&File"), this);
    QToolBar *fileToolBar = addToolBar(tr("File"));

    newAction = fileMenu->addAction(
                QIcon::fromTheme(QStringLiteral("filenew"), QIcon(":/images/filenew.png")), tr("&New Item"));
    newAction->setShortcut(QKeySequence(tr("Ctrl+N")));
    fileMenu->addSeparator();
    reloadAction = fileMenu->addAction(
                QIcon::fromTheme(QStringLiteral("reload"), QIcon(":/images/reload.png")), tr("&Reload"));
    reloadAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    saveAction = fileMenu->addAction(
                QIcon::fromTheme(QStringLiteral("filesave"), QIcon(":/images/filesave.png")), tr("&Save"));
    saveAction->setShortcut(QKeySequence(tr("Ctrl+S")));
    fileMenu->addSeparator();
    if (Clib::uId() != 0)
        chuserAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("go-up")), tr("Start as &Root"));
    else
        chuserAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("go-down")), tr("Start as &Regular user"));
    chuserAction->setShortcut(QKeySequence(tr("Ctrl+U")));
    fileMenu->addSeparator();

    quitAction = fileMenu->addAction(
                 QIcon::fromTheme(QStringLiteral("exit"), QIcon(":images/exit.png")), tr("E&xit"));
    quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    fileToolBar->addAction(newAction);
    fileToolBar->addAction(reloadAction);
    fileToolBar->addAction(saveAction);
    menuBar()->addMenu(fileMenu);

    auto *editMenu = new QMenu(tr("&Edit"), this);
    QToolBar *editToolBar = addToolBar(tr("Edit"));
    cutAction = editMenu->addAction(
                QIcon::fromTheme(QStringLiteral("edit-cut"), QIcon(":/images/editcut.png")), tr("Cu&t"));
    cutAction->setShortcut(QKeySequence(tr("Ctrl+X")));
    copyAction = editMenu->addAction(
                QIcon::fromTheme(QStringLiteral("edit-copy"), QIcon(":/images/editcopy.png")), tr("&Copy"));
    copyAction->setShortcut(QKeySequence(tr("Ctrl+C")));
    pasteAction = editMenu->addAction(
                QIcon::fromTheme(QStringLiteral("edit-paste"), QIcon(":/images/editpaste.png")), tr("&Paste"));
    pasteAction->setShortcut(QKeySequence(tr("Ctrl+V")));
    editMenu->addSeparator();
    deleteAction = editMenu->addAction(
                QIcon::fromTheme(QStringLiteral("edit-delete"), QIcon(":/images/editdelete.png")), tr("&Delete"));
    deleteAction->setShortcut(QKeySequence(tr("Del")));

    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    editToolBar->addSeparator();
    editToolBar->addAction(deleteAction);
    editToolBar->addSeparator();
    editToolBar->addAction(chuserAction);
    menuBar()->addMenu(editMenu);

    auto *helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(
                QIcon(":/images/job-scheduler.svg"), tr("&About"));
    helpAction = helpMenu->addAction(QIcon::fromTheme(QStringLiteral("help")), tr("&Help"));
    helpAction->setShortcut(QKeySequence(QStringLiteral("F1")));
    menuBar()->addMenu(helpMenu);

    saveAction->setEnabled(false);
    pasteAction->setEnabled(false);
}

void MainWindow::displayHelp()
{
    QString url = QStringLiteral("file:///usr/share/doc/job-scheduler/help.html");
    displayDoc(url, tr("Job Scheduler"));
}

void MainWindow::initCron()
{
    for (auto *d : qAsConst(crontabs)) delete d;
    crontabs.clear();

    QString user = Clib::uName();
    auto *cron = new Crontab(user);
    if (cron->tCommands.count() == 0 && cron->comment.isEmpty() &&
            cron->variables.count() == 0) {
        cron->comment = QLatin1String("");
        cron->variables << new Variable(QStringLiteral("HOME"), Clib::uHome(), QStringLiteral("Home"));
        cron->variables << new Variable(QStringLiteral("PATH"), Clib::getEnv("PATH"), QStringLiteral("Path"));
        cron->variables << new Variable(QStringLiteral("SHELL"), Clib::uShell(), QStringLiteral("Shell"));
    }
    crontabs << cron;
    if (Clib::uId() == 0) {
        cron = new Crontab( QStringLiteral("/etc/crontab") );
        crontabs << cron;
        for (const auto& s : Clib::allUsers()) {
            if (s == user)
                continue;

            cron = new Crontab(s);
            if (cron->tCommands.count() == 0)
                delete cron;
            else
                crontabs << cron;
        }
    } else {
        cronView->hideUser();
    }
    executeList->dataChanged();
    cronView->resetView();

    saveAction->setEnabled(false);
}

void MainWindow::reloadCron()
{

    if (saveAction->isEnabled()) {
        if (QMessageBox::Ok == QMessageBox::question(this,
                                                     tr("Job Scheduler"),
                                                     tr("Not saved since last change.\nAre you OK to reload?"),
                                                     QMessageBox::Ok, QMessageBox::Cancel)) {
            initCron();
        }
    }

}

void MainWindow::saveCron()
{

    bool saved = false;
    bool notSaved =false;

    for (auto *cron : qAsConst(crontabs)) {
        if (cron->changed) {
            SaveDialog dialog(cron->cronOwner, cron->cronText());
            if (dialog.exec()==QDialog::Accepted) {
                bool ret = cron->putCrontab(dialog.getText());
                if (!ret) {
                    QMessageBox::critical(this, tr("Job Scheduler"), cron->estr);
                    notSaved = true;
                } else {
                    auto *newCron = new Crontab(cron->cronOwner);
                    int p = crontabs.indexOf(cron);
                    crontabs.replace(p, newCron);
                    delete cron;
                    saved = true;
                }
            } else {
                notSaved = true;
            }
        }
    }

    if (saved) {
        executeList->dataChanged();
        cronView->resetView();
    }
    saveAction->setEnabled(notSaved);

}

void MainWindow::dataChanged()
{
    auto *cron = cronView->getCurrentCrontab();
    cron->changed = true;
    saveAction->setEnabled(true);
    cronView->resizeColumnToContents(0);
}

void MainWindow::changeCurrent(Crontab * /*unused*/, TCommand *cmnd)
{
    bool flg = (cmnd != nullptr);
    cutAction->setEnabled(flg);
    copyAction->setEnabled(flg);
    deleteAction->setEnabled(flg);
}

void MainWindow::readSettings()
{
    settings.beginGroup(QStringLiteral("Main"));
    exeMaxNum = settings.value(QStringLiteral("MaxListNum"), 100 ).toInt();
    exeMaxDate = settings.value(QStringLiteral("MaxListDate"), 1 ).toInt();
    viewSize = settings.value(QStringLiteral("ViewSize"), QSize(200,460)).toSize();
    restoreGeometry(settings.value(QStringLiteral("Geometry")).toByteArray());
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    settings.beginGroup(QStringLiteral("Main"));
    settings.setValue(QStringLiteral("MaxListNum"), executeList->maxNum);
    settings.setValue(QStringLiteral("MaxListDate"), executeList->maxDate);
    settings.setValue(QStringLiteral("Geometry"), saveGeometry());
    settings.setValue(QStringLiteral("ViewSize"), cronView->size());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    bool changed = false;
    for (const auto& cron : qAsConst(crontabs)) {
        if (cron->changed) {
            changed = true;
            break;
        }
    }
    if (changed && QMessageBox::question(this,
                                         tr("Job Scheduler"),
                                         tr("Not saved since last change.\nAre you OK to exit?"),
                                         QMessageBox::Ok, QMessageBox::Cancel)
            == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    writeSettings();
    event->accept();
}

void MainWindow::AboutJobScheduler()
{
    this->hide();
    displayAboutMsgBox(tr("About Job Scheduler"), tr("<b>Job Scheduler</b>") + " - " + tr("Version: %1").arg(VERSION) + "<p>" +
                       tr("Job Scheduler is based upon qroneko 0.5.4, released in 2005 by korewaisai (<a href=\"mailto:korewaisai@yahoo.co.jp\">korewaisai@yahoo.co.jp</a>)") + "<p>" +
                       tr("Original project page: %1").arg(QStringLiteral("<a href=\"https://qroneko.sourceforge.net\">https://qroneko.sourceforge.net</a>")) + "<p>" +
                       tr("MX project page: %1").arg(QStringLiteral("<a href=\"https://github.com/mx-linux/job-scheduler\">https://github.com/mx-linux/job-scheduler</a>")),
                       QStringLiteral("/usr/share/doc/job-scheduler/license.html"),
                       tr("%1 License").arg(this->windowTitle()));
    this->show();
}

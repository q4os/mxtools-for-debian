/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class TCommand;
class Crontab;
class CronView;
class ExecuteList;
class QModelIndex;
class QTabWidget;
class QItemSelectionModel;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void changeUser();
    void saveCron();
    void reloadCron();
    void dataChanged();
    void changeCurrent(Crontab *cron, TCommand *cmnd);

    void AboutJobScheduler();

signals:

private:
    void createActions();
    void createMenus();
    void createToolBar();
    static void displayHelp();
    void initCron();

    void readSettings();
    void writeSettings();

    void closeEvent(QCloseEvent *event);

    QAction *aboutAction {};
    QAction *aboutQtAction {};
    QAction *chuserAction {};
    QAction *copyAction {};
    QAction *cutAction {};
    QAction *deleteAction {};
    QAction *helpAction {};
    QAction *newAction {};
    QAction *pasteAction {};
    QAction *quitAction {};
    QAction *reloadAction {};
    QAction *saveAction {};

    CronView *cronView;
    ExecuteList *executeList;

    bool useEtcCron {};
    int exeMaxNum {};
    int exeMaxDate {};

    QSize winSize;
    QSize viewSize;
    QSettings settings;

    QList<Crontab *> crontabs;
};

#endif

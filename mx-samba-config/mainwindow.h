/**********************************************************************
 *  mainwindow.h
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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>
#include <QProcess>
#include <QSettings>

#include "editshare.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void centerWindow();
    void setConnections();

public slots:

private slots:
    void addEditShares(EditShare*);
    void pushAbout_clicked();
    void pushAddShare_clicked();
    void pushAddUser_clicked();
    void pushEditShare_clicked();
    void pushEnableDisableSamba_clicked();
    void pushHelp_clicked();
    void pushRemoveShare_clicked();
    void pushRemoveUser_clicked();
    void pushStartStopSamba_clicked();
    void pushUserPassword_clicked();

private:
    Ui::MainWindow *ui;
    QProcess proc;
    QSettings settings;
    QStringList listUsers();
    int run(const QString&, const QStringList&);
    void buildUserList(EditShare*);
    void checkSambashareGroup();
    void checksamba();
    void disablesamba();
    void enablesamba();
    void refreshShareList();
    void refreshUserList();
    void startsamba();
    void stopsamba();
};


#endif


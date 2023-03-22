/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2019 MX Authors
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



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QFile>

#include <cmd.h>

const QString cli_utils = ". /usr/local/lib/cli-shell-utils/cli-shell-utils.bash;";

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void makeUsb(const QString &options);
    void setup();
    QString buildOptionList();
    QStringList buildUsbList();
    QStringList removeUnsuitable(const QStringList &devices); // remove unsuitable disks from the list (live and unremovable)

private slots:
    void cleanup();
    void cmdDone();
    void cmdStart();
    void setConnections();
    void updateOutput();
    void on_buttonAbout_clicked();
    void on_buttonBack_clicked();
    void on_buttonHelp_clicked();
    void on_buttonNext_clicked();
    void on_buttonRefresh_clicked();
    void on_checkBoxShowAll_clicked();
    void on_checkBoxshowpartitions_clicked();

private:
    Ui::MainWindow *ui;
    Cmd *cmd;
    Cmd *cmdprog;
    QString device;
    QString label;
    int height;
};


#endif


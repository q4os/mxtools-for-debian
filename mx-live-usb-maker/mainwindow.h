/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2017 MX Authors
 *
 * Authors: Adrian
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
#include <QTimer>

#include <cmd.h>

class QFile;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QStringList &args, QDialog *parent = nullptr);
    ~MainWindow() override;

    QString buildOptionList();
    QStringList buildUsbList();
    QStringList removeUnsuitable(const QStringList &devices); // remove unsuitable disks (live and unremovable)
    bool checkDestSize();
    static bool isRunningLive();
    static bool isToRam();
    void makeUsb(const QString &options);
    void progress();
    void setGeneralConnections();
    void setup();

public slots:

private slots:
    static bool isantiX_mx_family(const QString &selected);
    void cleanup();
    void cmdDone();
    void setConnections();
    void setDefaultMode(const QString &iso_name);
    void updateBar();
    void updateOutput();

    void checkCloneLive_clicked(bool checked);
    void checkCloneMode_clicked(bool checked);
    void checkDataFirst_clicked(bool checked);
    void checkUpdate_clicked(bool checked);
    void pushAbout_clicked();
    void pushBack_clicked();
    void pushHelp_clicked();
    void pushLumLogFile_clicked();
    void pushNext_clicked();
    void pushOptions_clicked();
    void pushRefresh_clicked();
    void pushSelectSource_clicked();
    void radioDd_clicked();
    void radioNormal_clicked();
    void spinBoxSize_valueChanged(int arg1);
    void textLabel_textChanged(QString arg1);

private:
    Ui::MainWindow *ui;
    Cmd cmd;
    QFile *stat_file {};
    QString LUM;
    QString device;
    QString elevate;
    QTimer timer;
    bool advancedOptions {};
    const QString cli_utils {". /usr/local/lib/cli-shell-utils/cli-shell-utils.bash;"};
    int height {};
    uint size_check;
};

#endif

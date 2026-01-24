/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2016-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-system-sounds.
 *
 * mx-system-sounds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-system-sounds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-system-sounds.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#pragma once

#include <QMessageBox>
#include <QProcess>

// struct for outputing both the exit code and the strings when running a command
struct Output {
    int exit_code;
    QString str;
};

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    Output runCmd(const QString &cmd);
    void setup();

private slots:
    static void on_buttonHelp_clicked();
    void on_buttonAbout_clicked();
    void on_buttonApply_clicked();
    void on_button_login_sound_clicked();
    void on_button_logout_sound_clicked();
    void on_button_play_login_clicked();
    void on_button_play_logout_clicked();
    void on_button_reset_login_clicked();
    void on_button_reset_logout_clicked();
    void on_checkbox_eventsounds_clicked();
    void on_checkbox_inputsounds_clicked();
    void on_checkbox_login_clicked();
    void on_checkbox_logout_clicked();
    void on_comboBox_theme_activated(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QString currentlogin;
    QString currentlogout;
    QString customloginsound;
    QString customlogoutsound;
    QString defualtloginsound;
    QString defualtlogoutsound;
    bool loginreset {};
    bool logoutreset {};
    bool theme_login_flag {};
    bool theme_logout_flag {};
};

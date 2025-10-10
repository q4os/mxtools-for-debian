/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2015-2024 MX Authors
 *
 * Authors: Adrian
 *          Paul David Callahan
 *          Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-welcome.
 *
 * mx-welcome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-welcome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-welcome.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QCommandLineParser>
#include <QDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QWidget>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(const QCommandLineParser &arg_parser, QWidget *parent = nullptr);
    ~MainWindow() override;

    static QString runCmd(const QString &cmd);
    static QString getVersion(const QString &name);

    void setup();

private slots:
    void on_ButtonQSI_clicked() const;
    void on_buttonAbout_clicked();
    void on_buttonContribute_clicked() const;
    void on_buttonFAQ_clicked() const;
    void on_buttonForum_clicked() const;
    void on_buttonManual_clicked() const;
    void on_buttonPackageInstall_clicked() const;
    void on_buttonPanelOrient_clicked() const;
    void on_buttonSetup_clicked() const;
    void on_buttonTOS_clicked() const;
    void on_buttonTools_clicked() const;
    void on_buttonTour_clicked() const;
    void on_buttonVideo_clicked() const;
    void on_buttonWiki_clicked() const;
    void on_checkBox_clicked(bool checked);
    void on_tabWidget_currentChanged(int index);
    void resizeEvent(QResizeEvent * /*unused*/) override;
    void setTabStyle();
    void shortSystemInfo();
    void termsofuse() const;

private:
    Ui::MainWindow *ui;
    QProcess *proc {};
    QSettings user_settings;

    QString CONTRIBUTECMD;
    QString FAQCMD;
    QString FORUMCMD;
    QString MANUALCMD;
    QString PACKAGEINSTALLERCMD;
    QString SETUPCMD;
    QString TOOLSCMD;
    QString TOSCMD;
    QString TOURCMD;
    QString TWEAKCMD;
    QString VIDEOCMD;
    QString WIKICMD;
    QString debian_version;
    QString output;
    QString version;
    bool isfluxbox = false;
};

/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2016 MX Authors
 *
 * Authors: dolphin_oracle
 *          Adrian
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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QTextEdit>

#include "about.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "Program Version:" << VERSION;
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    connect(ui->buttonCancel, &QPushButton::pressed, this, &MainWindow::close);
    if (ui->buttonApply->icon().isNull())
        ui->buttonApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    setup();
}

MainWindow::~MainWindow() { delete ui; }

// util function for getting bash command output and error code
Output MainWindow::runCmd(const QString &cmd)
{
    auto *proc = new QProcess(this);
    QEventLoop loop;
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->start(QStringLiteral("/bin/bash"), QStringList() << QStringLiteral("-c") << cmd);
    proc->waitForFinished();
    Output out = {proc->exitCode(), proc->readAll().trimmed()};
    delete proc;
    return out;
}

// setup versious items first time program runs
void MainWindow::setup()
{
    this->setWindowTitle(tr("MX System Sounds"));
    this->adjustSize();

    const QString home_path = QDir::homePath();

    // setup mx-login-logout_sounds.conf if necessary
    QFileInfo file_conf(home_path + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    if (file_conf.exists()) {
        qDebug() << "Found Config File";
    } else {
        runCmd("mkdir " + home_path + "/.config/MX-Linux/mx-system-sounds/");
        runCmd("echo '#true is enabled, false is disabled' >>" + home_path
               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
        runCmd("echo startup=false>>" + home_path + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
        // runCmd("echo logout=false>>" + home_path + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    }

    // intialize variables
    theme_login_flag = true;
    theme_logout_flag = true;
    // get sound theme. note there is no path associated with this

    QString soundtheme = (runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/SoundThemeName")).str);

    // check home directory location first
    if (QDir(home_path + "/.local/share/sounds" + soundtheme).exists()) {
        defualtloginsound = (runCmd(home_path + "/.local/share/sounds" + soundtheme + "/ |grep desktop-login").str);
        defualtlogoutsound = (runCmd(home_path + "/.local/share/sounds" + soundtheme + "/ |grep desktop-logout").str);
    } else {
        // check system level location second
        defualtloginsound = (runCmd("find  /usr/share/sounds/" + soundtheme + "/ |grep desktop-login").str);
        defualtlogoutsound = (runCmd("find  /usr/share/sounds/" + soundtheme + "/ |grep desktop-logout").str);
    }

    // check for custom sounds
    QFileInfo file_info(home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf");
    // startupsound.conf, if exists get filename and path of custom startup sound
    if (file_info.exists()) {
        currentlogin = (runCmd("cat " + home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf").str);
        QFileInfo file_info(currentlogin);
        ui->button_login_sound->setText(file_info.baseName());
        ui->button_login_sound->setToolTip(file_info.filePath());
    } else {
        // set default sound if custom startupsound.conf doesn't exist
        // if default sound doesn't exist, then set text to "None"
        QFileInfo file_info(defualtloginsound);
        if (file_info.exists()) {
            ui->button_login_sound->setText(QStringLiteral("Default"));
            ui->button_login_sound->setToolTip(file_info.filePath());
            currentlogin = defualtloginsound;
        } else {
            ui->button_login_sound->setText(QStringLiteral("None"));
            ui->button_login_sound->setToolTip(QStringLiteral("None"));
            currentlogin = QStringLiteral("None");
        }
    }
    qDebug() << "current sound theme is";
    qDebug() << soundtheme;
    qDebug() << " default login is ";
    qDebug() << defualtloginsound;
    qDebug() << " current login is ";
    qDebug() << currentlogin;

    // logoutsound.conf sound, if exists get filename and path of custom logout sound
    //    QFileInfo file_info2(home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf");
    //    if (file_info2.exists()) {
    //        currentlogout = (runCmd("cat " + home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf").str);
    //        QFileInfo file_info2(currentlogout);
    //        ui->button_logout_sound->setText(file_info2.baseName());
    //        ui->button_logout_sound->setToolTip(file_info2.filePath());
    //    } else {
    //        // set default sound if custom startupsound.conf doesn't exist
    //        // if default sound doesn't exist, then set text to "None"
    //        QFileInfo file_info2(defualtlogoutsound);
    //        if (file_info2.exists()) {
    //            ui->button_logout_sound->setText(QStringLiteral("Default"));
    //            ui->button_logout_sound->setToolTip(file_info2.filePath());
    //            currentlogout = defualtlogoutsound;
    //        } else {
    //            ui->button_logout_sound->setText(QStringLiteral("None"));
    //            ui->button_logout_sound->setToolTip(QStringLiteral("None"));
    //            currentlogout = QStringLiteral("None");
    //        }
    //    }
    //    qDebug() << " default logout is ";
    //    qDebug() << defualtlogoutsound;
    //    qDebug() << " current logout is ";
    //    qDebug() << currentlogout;

    // disble logout for now
    ui->button_logout_sound->hide();
    ui->button_play_logout->hide();
    ui->checkbox_logout->hide();
    ui->label_5->hide();
    ui->button_reset_logout->hide();

    // initial event sound setting
    if (runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/EnableEventSounds")).str == QLatin1String("false"))
        ui->checkbox_eventsounds->setChecked(false);
    else
        ui->checkbox_eventsounds->setChecked(true);

    // initial input feedback sound setting
    if (runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/EnableInputFeedbackSounds")).str
        == QLatin1String("false"))
        ui->checkbox_inputsounds->setChecked(false);
    else
        ui->checkbox_inputsounds->setChecked(true);

    // initial login sound setting
    if (system("grep startup=false " + home_path.toUtf8()
               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf")
        == 0)
        ui->checkbox_login->setChecked(false);
    else
        ui->checkbox_login->setChecked(true);

    // initial logout setting
    //    if (system("grep logout=false " + home_path.toUtf8()
    //               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf")
    //        == 0)
    //        ui->checkbox_logout->setChecked(false);
    //    else
    //        ui->checkbox_logout->setChecked(true);

    // inital sound theme dialog setting
    ui->comboBox_theme->clear();
    QStringList theme_list;
    QStringList filter(QStringLiteral("index.theme"));
    QDirIterator it(QStringLiteral("/usr/share/sounds"), filter, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QDir dir = file_info.absoluteDir();
        theme_list << dir.dirName();
    }
    QDirIterator it2(home_path + "/.local/share/sounds", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it2.hasNext()) {
        QFileInfo file_info(it2.next());
        QDir dir = file_info.absoluteDir();
        theme_list << dir.dirName();
    }
    ui->comboBox_theme->addItems(theme_list);
    ui->comboBox_theme->setCurrentText(soundtheme);

    // disable apply button unless changes
    ui->buttonApply->setEnabled(false);
}

void MainWindow::on_buttonApply_clicked()
{
    QString home_path = QDir::homePath();

    // Event Sounds Enable or Disable
    if (ui->checkbox_eventsounds->isChecked()) {
        system("xfconf-query -c xsettings -p /Net/EnableEventSounds -s true");
    } else {
        system("xfconf-query -c xsettings -p /Net/EnableEventSounds -s false");
        ui->checkbox_inputsounds->setChecked(false);
    }

    // Input feedback Sounds Enable or Disable
    if (ui->checkbox_inputsounds->isChecked()) {
        if (ui->checkbox_eventsounds->isChecked())
            system("xfconf-query -c xsettings -p /Net/EnableInputFeedbackSounds -s true");
        else
            system("xfconf-query -c xsettings -p /Net/EnableInputFeedbackSounds -s false");
    } else {
        system("xfconf-query -c xsettings -p /Net/EnableInputFeedbackSounds -s false");
    }

    // Login Sound Enable/disable

    if (ui->checkbox_login->isChecked()) {
        runCmd("sed -i -r s/startup=.*/startup=true/ " + home_path
               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    } else {
        runCmd("sed -i -r s/startup=.*/startup=false/ " + home_path
               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    }

    //    // Logout Sound Enable/disable
    //    if (ui->checkbox_logout->isChecked()) {
    //        runCmd("sed -i -r s/logout=.*/logout=true/  " + home_path
    //               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    //    } else {
    //        runCmd("sed -i -r s/logout=.*/logout=false/  " + home_path
    //               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    //    }

    // Set Sound Theme
    QString soundtheme2 = QString(ui->comboBox_theme->currentText());
    runCmd("xfconf-query -c xsettings -p /Net/SoundThemeName -s " + soundtheme2);

    // Set custom sounds

    qDebug() << " current login is";
    qDebug() << currentlogin;

    // check home directory location first for defaults first
    if (QDir(home_path + "/.local/share/sounds" + soundtheme2).exists()) {
        defualtloginsound = (runCmd(home_path + "/.local/share/sounds" + soundtheme2 + "/ |grep desktop-login").str);
        // defualtlogoutsound = (runCmd(home_path + "/.local/share/sounds" + soundtheme2 + "/ |grep
        // desktop-logout").str);
    } else {
        // if not present, check system location second
        defualtloginsound = (runCmd("find  /usr/share/sounds/" + soundtheme2 + "/ |grep desktop-login").str);
        // defualtlogoutsound = (runCmd("find  /usr/share/sounds/" + soundtheme2 + "/ |grep desktop-logout").str);
    }
    // set login sound
    if (currentlogin != defualtloginsound) {
        if (currentlogin != QLatin1String("None")) {
            runCmd("echo " + currentlogin + ">" + home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf");
        }
    } else {
        runCmd("rm -f " + home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf");
    }
    // Set logout sound
    //    if (currentlogout != defualtlogoutsound) {
    //        if (currentlogout != QLatin1String("None")) {
    //            runCmd("echo " + currentlogout + ">" + home_path +
    //            "/.config/MX-Linux/mx-system-sounds/logoutsound.conf");
    //        }
    //    } else {
    //        runCmd("rm -f " + home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf");
    //    }
    // disable Apply button
    ui->buttonApply->setEnabled(false);
}

void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About MX System Sounds"),
        "<p align=\"center\"><b><h2>" + tr("MX System Sounds") + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + VERSION + "</p><p align=\"center\"><h3>" + tr("Configure Event & Session Sounds")
            + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
              "<p align=\"center\">"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-system-sounds/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void MainWindow::on_buttonHelp_clicked()
{
    QLocale locale;
    const QString lang = locale.bcp47Name();

    QString url = QStringLiteral("/usr/share/doc/mx-system-sounds/mx-system-sounds.html");

    if (lang.startsWith(QLatin1String("fr")))
        url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-mx-sons-syst%C3%A8me");

    displayDoc(url, tr("%1 Help").arg(tr("MX System Sounds")));
}

void MainWindow::on_button_login_sound_clicked()
{

    // select a user defined login sound
    //   this->hide(); //hides main ui
    // QString home_path = QDir::homePath();
    QString home_path;
    QFileInfo sound_info(currentlogin);
    if (sound_info.exists())
        home_path = sound_info.absolutePath();
    else
        home_path = QStringLiteral("/usr/share/sounds");
    customloginsound = QFileDialog::getOpenFileName(this, tr("Select Sound File"), home_path,
                                                    tr("Sound Files (*.mp3 *.m4a *.aac *.flac *.ogg *.oga *.wav)"));
    QFileInfo file_info(customloginsound);
    if (file_info.exists()) {
        ui->button_login_sound->setText(file_info.baseName());
        ui->button_login_sound->setToolTip(file_info.filePath());
        currentlogin = customloginsound;
        theme_login_flag = false;
    } else {
        currentlogin = defualtloginsound;
    }

    qDebug() << " current login is";
    qDebug() << currentlogin;

    // enable apply button
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_button_logout_sound_clicked()
{
    // choose a user defined logout sound.
    // QString home_path = QDir::homePath();
    QString home_path;
    QFileInfo sound_info(currentlogout);
    if (sound_info.exists())
        home_path = sound_info.absolutePath();
    else
        home_path = QStringLiteral("/usr/share/sounds");
    customlogoutsound = QFileDialog::getOpenFileName(this, tr("Select Sound File"), home_path,
                                                     tr("Sound Files (*.mp3 *.m4a *.aac *.flac *.ogg *.oga *.wav)"));
    QFileInfo file_info(customlogoutsound);
    if (file_info.exists()) {
        ui->button_logout_sound->setText(file_info.baseName());
        ui->button_logout_sound->setToolTip(file_info.filePath());
        currentlogout = customlogoutsound;
        theme_logout_flag = false;
    } else {
        currentlogout = defualtlogoutsound;
    }

    qDebug() << " current logout is ";
    qDebug() << currentlogout;
    qDebug() << theme_logout_flag;
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_button_play_login_clicked()
{

    // play the sound currently defined in the login selection box
    qDebug() << " current login is ";
    qDebug() << currentlogin;
    runCmd(QStringLiteral("pkill play"));
    QString cmd = QString("play \"" + currentlogin + "\" &");
    system(cmd.toUtf8());
}

void MainWindow::on_button_play_logout_clicked()
{
    // play the sound currently defined in the logout selection box
    qDebug() << " current logout is ";
    qDebug() << currentlogout;
    runCmd(QStringLiteral("pkill play"));
    QString cmd = QString("play \"" + currentlogout + "\" &");
    system(cmd.toUtf8());
}

void MainWindow::on_button_reset_login_clicked()
{
    // reset the login sound to the theme default.  if the theme doesn't have a desktop-login defined, set to "None"

    defualtloginsound
        = (runCmd("find  /usr/share/sounds/" + ui->comboBox_theme->currentText() + "/ |grep desktop-login").str);
    QFileInfo file_info(defualtloginsound);
    if (file_info.exists()) {
        ui->button_login_sound->setText(QStringLiteral("Default"));
        ui->button_login_sound->setToolTip(file_info.filePath());
        currentlogin = defualtloginsound;
    } else {
        ui->button_login_sound->setText(QStringLiteral("None"));
        ui->button_login_sound->setToolTip(QStringLiteral("None"));
        currentlogin = QStringLiteral("None");
    }
    theme_login_flag = true;
    loginreset = true;
    qDebug() << " current login reset box is ";
    qDebug() << currentlogin;
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_button_reset_logout_clicked()
{
    // reset the logout sound to the theme default.  if the theme doesn't have a desktop-logout defined, set to "None"

    defualtlogoutsound
        = (runCmd("find  /usr/share/sounds/" + ui->comboBox_theme->currentText() + "/ |grep desktop-logout").str);
    QFileInfo file_info(defualtlogoutsound);
    if (file_info.exists()) {
        ui->button_logout_sound->setText(QStringLiteral("Default"));
        ui->button_logout_sound->setToolTip(file_info.filePath());
        currentlogout = defualtlogoutsound;
    } else {
        ui->button_logout_sound->setText(QStringLiteral("None"));
        ui->button_logout_sound->setToolTip(QStringLiteral("None"));
        currentlogout = QStringLiteral("None");
    }
    theme_logout_flag = true;
    logoutreset = true;
    qDebug() << " current logout reset box is ";
    qDebug() << currentlogout;
    qDebug() << theme_logout_flag;
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_comboBox_theme_activated(const QString &arg1)
{
    // when changing sound theme, set new default selections, unless custom sounds indicated by "theme_login" flags are
    // set. also doesn't change selections if customsounds are previously defined in startupsound.conf or
    // logoutsound.conf

    QString home_path = QDir::homePath();
    if (theme_login_flag) {
        QFileInfo file_info(home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf");
        if (file_info.exists()) {
            currentlogin = (runCmd("cat " + home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf").str);
            QFileInfo file_info(currentlogin);
            ui->button_login_sound->setText(file_info.baseName());
            ui->button_login_sound->setToolTip(file_info.filePath());
        } else {
            defualtloginsound = (runCmd("find  /usr/share/sounds/" + arg1 + "/ |grep desktop-login").str);
            QFileInfo file_info(defualtloginsound);
            if (file_info.exists()) {
                ui->button_login_sound->setText(QStringLiteral("Default"));
                ui->button_login_sound->setToolTip(file_info.filePath());
                currentlogin = defualtloginsound;
            } else {
                ui->button_login_sound->setText(QStringLiteral("None"));
                ui->button_login_sound->setToolTip(QStringLiteral("None"));
                currentlogin = QStringLiteral("None");
            }
        }
    }
    qDebug() << " current login theme change box is ";
    qDebug() << currentlogin;

    if (theme_logout_flag) {
        QFileInfo file_info2(home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf");
        if (file_info2.exists()) {
            currentlogout = (runCmd("cat " + home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf").str);
            QFileInfo file_info(currentlogout);
            ui->button_logout_sound->setText(file_info.baseName());
            ui->button_logout_sound->setToolTip(file_info.filePath());
        } else {
            defualtlogoutsound = (runCmd("find  /usr/share/sounds/" + arg1 + "/ |grep desktop-logout").str);
            QFileInfo file_info(defualtlogoutsound);
            if (file_info.exists()) {
                ui->button_logout_sound->setText(QStringLiteral("Default"));
                ui->button_logout_sound->setToolTip(file_info.filePath());
                currentlogout = defualtlogoutsound;
            } else {
                ui->button_logout_sound->setText(QStringLiteral("None"));
                ui->button_logout_sound->setToolTip(QStringLiteral("None"));
                currentlogout = QStringLiteral("None");
            }
        }
        qDebug() << " current logout theme change box is ";
        qDebug() << currentlogout;
    }
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_checkbox_eventsounds_clicked() { ui->buttonApply->setEnabled(true); }

void MainWindow::on_checkbox_inputsounds_clicked() { ui->buttonApply->setEnabled(true); }

void MainWindow::on_checkbox_login_clicked() { ui->buttonApply->setEnabled(true); }

void MainWindow::on_checkbox_logout_clicked() { ui->buttonApply->setEnabled(true); }

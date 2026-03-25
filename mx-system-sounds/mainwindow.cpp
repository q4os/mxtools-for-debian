/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2016-2025 MX Authors
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
#include <QFileInfo>
#include <QSet>
#include <QSaveFile>
#include <QTextEdit>
#include <QTextStream>

#include "about.h"

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

namespace
{
QString findSoundEvent(const QString &baseDir, const QString &eventName)
{
    QDir dir(baseDir);
    if (!dir.exists()) {
        return {};
    }

    QDirIterator it(baseDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        if (QFileInfo(path).fileName().contains(eventName)) {
            return path;
        }
    }

    return {};
}

QString findThemeSoundEvent(const QString &homePath, const QString &themeName, const QString &eventName)
{
    const QString localThemeDir = homePath + "/.local/share/sounds/" + themeName;
    if (QDir(localThemeDir).exists()) {
        return findSoundEvent(localThemeDir, eventName);
    }

    return findSoundEvent(QStringLiteral("/usr/share/sounds/") + themeName, eventName);
}

void addThemeName(QStringList &themeList, QSet<QString> &seenThemes, const QString &themeName)
{
    if (!themeName.isEmpty() && !seenThemes.contains(themeName)) {
        seenThemes.insert(themeName);
        themeList << themeName;
    }
}

QString readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

bool writeTextFile(const QString &path, const QString &content)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream << content;
    return file.commit();
}

QString readConfigValue(const QString &path, const QString &key)
{
    const QStringList lines = readTextFile(path).split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.startsWith(key + '=')) {
            return line.section('=', 1);
        }
    }
    return {};
}

bool writeConfigValue(const QString &path, const QString &key, const QString &value)
{
    QStringList lines = readTextFile(path).split('\n', Qt::SkipEmptyParts);
    bool found = false;

    for (QString &line : lines) {
        if (line.startsWith(key + '=')) {
            line = key + '=' + value;
            found = true;
            break;
        }
    }

    if (!found) {
        lines << (key + '=' + value);
    }

    return writeTextFile(path, lines.join('\n') + '\n');
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    qDebug() << "Program Version:" << VERSION;
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    connect(ui->buttonCancel, &QPushButton::pressed, this, &MainWindow::close);
    if (ui->buttonApply->icon().isNull()) {
        ui->buttonApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    setup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Util function for getting process output and error code
Output MainWindow::runCmd(const QString &program, const QStringList &args)
{
    QProcess proc(this);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(program, args);
    proc.waitForFinished();
    return {proc.exitCode(), QString::fromUtf8(proc.readAll()).trimmed()};
}

// Setup versious items first time program runs
void MainWindow::setup()
{
    setWindowTitle(tr("MX System Sounds"));
    adjustSize();

    const QString home_path = QDir::homePath();
    const QString configDir = home_path + "/.config/MX-Linux/mx-system-sounds";
    const QString settingsPath = configDir + "/mx-login-logout_sounds.conf";
    const QString startupSoundPath = configDir + "/startupsound.conf";

    // Setup mx-login-logout_sounds.conf if necessary
    QFileInfo file_conf(settingsPath);
    if (file_conf.exists()) {
        qDebug() << "Found Config File";
    } else {
        QDir().mkpath(configDir);
        writeTextFile(settingsPath, QStringLiteral("#true is enabled, false is disabled\nstartup=false\n"));
        // runCmd("echo logout=false>>" + home_path + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf");
    }

    // Intialize variables
    theme_login_flag = true;
    theme_logout_flag = true;
    // Get sound theme. note there is no path associated with this

    QString soundtheme = runCmd(QStringLiteral("xfconf-query"),
                                {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                                 QStringLiteral("/Net/SoundThemeName")})
                             .str;

    // Check home directory location first
    defualtloginsound = findThemeSoundEvent(home_path, soundtheme, QStringLiteral("desktop-login"));
    defualtlogoutsound = findThemeSoundEvent(home_path, soundtheme, QStringLiteral("desktop-logout"));

    // Check for custom sounds
    QFileInfo file_info(startupSoundPath);
    // startupsound.conf, if exists get filename and path of custom startup sound
    if (file_info.exists()) {
        currentlogin = readTextFile(startupSoundPath);
        QFileInfo file_info(currentlogin);
        ui->button_login_sound->setText(file_info.baseName());
        ui->button_login_sound->setToolTip(file_info.filePath());
    } else {
        // Set default sound if custom startupsound.conf doesn't exist
        // If default sound doesn't exist, then set text to "None"
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
    //        currentlogout = runCmd("cat " + home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf").str;
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

    // Disble logout for now
    ui->button_logout_sound->hide();
    ui->button_play_logout->hide();
    ui->checkbox_logout->hide();
    ui->label_5->hide();
    ui->button_reset_logout->hide();

    // Initial event sound setting
    if (runCmd(QStringLiteral("xfconf-query"),
               {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                QStringLiteral("/Net/EnableEventSounds")})
            .str
        == QLatin1String("false")) {
        ui->checkbox_eventsounds->setChecked(false);
    } else {
        ui->checkbox_eventsounds->setChecked(true);
    }

    // Initial input feedback sound setting
    if (runCmd(QStringLiteral("xfconf-query"),
               {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                QStringLiteral("/Net/EnableInputFeedbackSounds")})
            .str
        == QLatin1String("false")) {
        ui->checkbox_inputsounds->setChecked(false);
    } else {
        ui->checkbox_inputsounds->setChecked(true);
    }

    // Initial login sound setting
    if (readConfigValue(settingsPath, QStringLiteral("startup")) == QLatin1String("false")) {
        ui->checkbox_login->setChecked(false);
    } else {
        ui->checkbox_login->setChecked(true);
    }

    // Initial logout setting
    //    if (system("grep logout=false " + home_path.toUtf8()
    //               + "/.config/MX-Linux/mx-system-sounds/mx-login-logout_sounds.conf")
    //        == 0)
    //        ui->checkbox_logout->setChecked(false);
    //    else
    //        ui->checkbox_logout->setChecked(true);

    // inital sound theme dialog setting
    ui->comboBox_theme->clear();
    QStringList theme_list;
    QSet<QString> seenThemes;
    QStringList filter(QStringLiteral("index.theme"));
    QDirIterator it(QStringLiteral("/usr/share/sounds"), filter, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QDir dir = file_info.absoluteDir();
        addThemeName(theme_list, seenThemes, dir.dirName());
    }
    QDirIterator it2(home_path + "/.local/share/sounds", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it2.hasNext()) {
        QFileInfo file_info(it2.next());
        QDir dir = file_info.absoluteDir();
        addThemeName(theme_list, seenThemes, dir.dirName());
    }
    ui->comboBox_theme->addItems(theme_list);
    ui->comboBox_theme->setCurrentText(soundtheme);

    // Disable apply button unless changes
    ui->buttonApply->setEnabled(false);
}

void MainWindow::on_buttonApply_clicked()
{
    QString home_path = QDir::homePath();
    const QString configDir = home_path + "/.config/MX-Linux/mx-system-sounds";
    const QString settingsPath = configDir + "/mx-login-logout_sounds.conf";
    const QString startupSoundPath = configDir + "/startupsound.conf";

    // Event Sounds Enable or Disable
    if (ui->checkbox_eventsounds->isChecked()) {
        runCmd(QStringLiteral("xfconf-query"),
               {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                QStringLiteral("/Net/EnableEventSounds"), QStringLiteral("-s"), QStringLiteral("true")});
    } else {
        runCmd(QStringLiteral("xfconf-query"),
               {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                QStringLiteral("/Net/EnableEventSounds"), QStringLiteral("-s"), QStringLiteral("false")});
        ui->checkbox_inputsounds->setChecked(false);
    }

    // Input feedback Sounds Enable or Disable
    if (ui->checkbox_inputsounds->isChecked()) {
        if (ui->checkbox_eventsounds->isChecked()) {
            runCmd(QStringLiteral("xfconf-query"),
                   {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                    QStringLiteral("/Net/EnableInputFeedbackSounds"), QStringLiteral("-s"), QStringLiteral("true")});
        } else {
            runCmd(QStringLiteral("xfconf-query"),
                   {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                    QStringLiteral("/Net/EnableInputFeedbackSounds"), QStringLiteral("-s"), QStringLiteral("false")});
        }
    } else {
        runCmd(QStringLiteral("xfconf-query"),
               {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
                QStringLiteral("/Net/EnableInputFeedbackSounds"), QStringLiteral("-s"), QStringLiteral("false")});
    }

    // Login Sound Enable/disable

    if (ui->checkbox_login->isChecked()) {
        writeConfigValue(settingsPath, QStringLiteral("startup"), QStringLiteral("true"));
    } else {
        writeConfigValue(settingsPath, QStringLiteral("startup"), QStringLiteral("false"));
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
    QString soundtheme2 = ui->comboBox_theme->currentText();
    runCmd(QStringLiteral("xfconf-query"),
           {QStringLiteral("-c"), QStringLiteral("xsettings"), QStringLiteral("-p"),
            QStringLiteral("/Net/SoundThemeName"), QStringLiteral("-s"), soundtheme2});

    // Set custom sounds

    qDebug() << " current login is";
    qDebug() << currentlogin;

    // Check home directory location first for defaults first
    defualtloginsound = findThemeSoundEvent(home_path, soundtheme2, QStringLiteral("desktop-login"));
    // Set login sound
    if (currentlogin != defualtloginsound) {
        if (currentlogin != QLatin1String("None")) {
            writeTextFile(startupSoundPath, currentlogin + '\n');
        } else {
            QFile::remove(startupSoundPath);
        }
    } else {
        QFile::remove(startupSoundPath);
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
    hide();
    displayAboutMsgBox(
        tr("About MX System Sounds"),
        "<p align=\"center\"><b><h2>" + tr("MX System Sounds") + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + VERSION + "</p><p align=\"center\"><h3>" + tr("Configure Event & Session Sounds")
            + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
              "<p align=\"center\">"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-system-sounds/license.html"), tr("%1 License").arg(windowTitle()));
    show();
}

void MainWindow::on_buttonHelp_clicked()
{
    QLocale locale;
    const QString lang = locale.bcp47Name();

    QString url = QStringLiteral("/usr/share/doc/mx-system-sounds/mx-system-sounds.html");
    if (lang.startsWith(QLatin1String("fr"))) {
        url = QStringLiteral("/usr/share/doc/mx-system-sounds/mx-system-sounds_fr.html");
    }

    displayHelpDoc(url, tr("%1 Help").arg(tr("MX System Sounds")));
}

void MainWindow::on_button_login_sound_clicked()
{

    // Select a user defined login sound
    QFileInfo sound_info(currentlogin);
    QString path = sound_info.exists() ? sound_info.absolutePath() : "/usr/share/sounds";
    customloginsound = QFileDialog::getOpenFileName(this, tr("Select Sound File"), path,
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

    // Enable apply button
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_button_logout_sound_clicked()
{
    // Choose a user defined logout sound.
    QFileInfo sound_info(currentlogout);
    QString path = sound_info.exists() ? sound_info.absolutePath() : "/usr/share/sounds";
    customlogoutsound = QFileDialog::getOpenFileName(this, tr("Select Sound File"), path,
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

    // Play the sound currently defined in the login selection box
    qDebug() << " current login is ";
    qDebug() << currentlogin;
    runCmd(QStringLiteral("pkill"), {QStringLiteral("play")});
    if (currentlogin != QLatin1String("None")) {
        QProcess::startDetached(QStringLiteral("play"), {currentlogin});
    }
}

void MainWindow::on_button_play_logout_clicked()
{
    // Play the sound currently defined in the logout selection box
    qDebug() << " current logout is ";
    qDebug() << currentlogout;
    runCmd(QStringLiteral("pkill"), {QStringLiteral("play")});
    if (currentlogout != QLatin1String("None")) {
        QProcess::startDetached(QStringLiteral("play"), {currentlogout});
    }
}

void MainWindow::on_button_reset_login_clicked()
{
    // Reset the login sound to the theme default.  if the theme doesn't have a desktop-login defined, set to "None"

    defualtloginsound
        = findThemeSoundEvent(QDir::homePath(), ui->comboBox_theme->currentText(), QStringLiteral("desktop-login"));
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
    // Reset the logout sound to the theme default.  if the theme doesn't have a desktop-logout defined, set to "None"

    defualtlogoutsound
        = findThemeSoundEvent(QDir::homePath(), ui->comboBox_theme->currentText(), QStringLiteral("desktop-logout"));
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
    //     When changing sound theme, set new default selections, unless custom sounds indicated by "theme_login" flags
    //     are set. also doesn't change selections if customsounds are previously defined in startupsound.conf or
    //     logoutsound.conf

    QString home_path = QDir::homePath();
    const QString startupSoundPath = home_path + "/.config/MX-Linux/mx-system-sounds/startupsound.conf";
    const QString logoutSoundPath = home_path + "/.config/MX-Linux/mx-system-sounds/logoutsound.conf";
    if (theme_login_flag) {
        QFileInfo file_info(startupSoundPath);
        if (file_info.exists()) {
            currentlogin = readTextFile(startupSoundPath);
            QFileInfo file_info(currentlogin);
            ui->button_login_sound->setText(file_info.baseName());
            ui->button_login_sound->setToolTip(file_info.filePath());
        } else {
            defualtloginsound = findThemeSoundEvent(home_path, arg1, QStringLiteral("desktop-login"));
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
        QFileInfo file_info2(logoutSoundPath);
        if (file_info2.exists()) {
            currentlogout = readTextFile(logoutSoundPath);
            QFileInfo file_info(currentlogout);
            ui->button_logout_sound->setText(file_info.baseName());
            ui->button_logout_sound->setToolTip(file_info.filePath());
        } else {
            defualtlogoutsound = findThemeSoundEvent(home_path, arg1, QStringLiteral("desktop-logout"));
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

void MainWindow::on_checkbox_eventsounds_clicked()
{
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_checkbox_inputsounds_clicked()
{
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_checkbox_login_clicked()
{
    ui->buttonApply->setEnabled(true);
}

void MainWindow::on_checkbox_logout_clicked()
{
    ui->buttonApply->setEnabled(true);
}

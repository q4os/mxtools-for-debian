/**********************************************************************
 *  mainwindow.cpp
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

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextEdit>

#include "about.h"
#include "flatbutton.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"

MainWindow::MainWindow(const QCommandLineParser& arg_parser, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    // qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // For the close, min and max buttons
    connect(ui->buttonCancel, &QPushButton::pressed, this, &MainWindow::close);
    setup();

    ui->tabWidget->setCurrentIndex(0);
    if (arg_parser.isSet("about")) {
        ui->tabWidget->setCurrentIndex(1);
    }

    if (arg_parser.isSet("test")) {
        ui->labelLoginInfo->show();
        ui->buttonSetup->show();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Setup versious items first time program runs
void MainWindow::setup()
{
    version = getVersion("mx-welcome");
    this->setWindowTitle(tr("MX Welcome"));

    QString old_conf_file = QDir::homePath() + "/.config/" + QApplication::applicationName() + ".conf";
    if (QFileInfo::exists(old_conf_file)) {
        QSettings old_settings(QApplication::applicationName());
        user_settings.setValue("AutoStartup", old_settings.value("AutoStartup", false).toBool());
        QFile::remove(old_conf_file);
    }
    bool autostart = user_settings.value("AutoStartup", false).toBool();
    ui->checkBox->setChecked(autostart);
    if (!autostart) {
        QFile::remove(QDir::homePath() + "/.config/autostart/mx-welcome.desktop");
    }

    // Setup title block & icons
    QSettings settings("/usr/share/mx-welcome/mx-welcome.conf", QSettings::NativeFormat);
    QSettings settingsusr("/etc/mx-welcome/mx-welcome.conf", QSettings::NativeFormat);
    QString DISTRO = settingsusr.value("DISTRO", settings.value("DISTRO").toString()).toString();
    QString CODENAME = settingsusr.value("CODENAME", settings.value("CODENAME").toString()).toString();
    QString HEADER = settingsusr.value("HEADER", settings.value("HEADER").toString()).toString();
    QString LOGO = settingsusr.value("LOGO", settings.value("LOGO").toString()).toString();
    QString SUPPORTED = settingsusr.value("SUPPORTED", settings.value("SUPPORTED").toString()).toString();
    TOSCMD = settingsusr.value("TOSCMD", settings.value("TOSCMD").toString()).toString();
    QString TOSTEXT = settingsusr.value("TOSTEXT", settings.value("TOSTEXT").toString()).toString();
    if (!TOSTEXT.isEmpty()) {
        ui->labelTOS->setText(TOSTEXT);
    }
    QString SETUP = settingsusr.value("1icon", settings.value("1icon").toString()).toString();
    QString SETUPTEXT = settingsusr.value("1text", settings.value("1text").toString()).toString();
    if (!SETUPTEXT.isEmpty()) {
        ui->buttonSetup->setText(SETUPTEXT);
    }
    SETUPCMD = settingsusr.value("1command", settings.value("1command").toString()).toString();
    QString FAQ = settingsusr.value("2icon", settings.value("2icon").toString()).toString();
    QString FAQTEXT = settingsusr.value("2text", settings.value("2text").toString()).toString();
    if (!FAQTEXT.isEmpty()) {
        ui->buttonFAQ->setText(FAQTEXT);
    }
    FAQCMD = settingsusr.value("2command", settings.value("2command").toString()).toString();
    QString FORUMS = settingsusr.value("3icon", settings.value("3icon").toString()).toString();
    QString FORUMTEXT = settingsusr.value("3text", settings.value("3text").toString()).toString();
    if (!FORUMTEXT.isEmpty()) {
        ui->buttonForum->setText(FORUMTEXT);
    }
    FORUMCMD = settingsusr.value("3command", settings.value("3command").toString()).toString();
    QString MANUAL = settingsusr.value("4icon", settings.value("4icon").toString()).toString();
    QString MANUALTEXT = settingsusr.value("4text", settings.value("4text").toString()).toString();
    if (!MANUALTEXT.isEmpty()) {
        ui->buttonManual->setText(MANUALTEXT);
    }
    MANUALCMD = settingsusr.value("4command", settings.value("4command").toString()).toString();
    QString VIDEOS = settingsusr.value("5icon", settings.value("5icon").toString()).toString();
    QString VIDEOTEXT = settingsusr.value("5text", settings.value("5text").toString()).toString();
    if (!VIDEOTEXT.isEmpty()) {
        ui->buttonVideo->setText(VIDEOTEXT);
    }
    VIDEOCMD = settingsusr.value("5command", settings.value("5command").toString()).toString();
    QString WIKI = settingsusr.value("6icon", settings.value("6icon").toString()).toString();
    QString WIKITEXT = settingsusr.value("6text", settings.value("6text").toString()).toString();
    if (!WIKITEXT.isEmpty()) {
        ui->buttonWiki->setText(WIKITEXT);
    }
    WIKICMD = settingsusr.value("6command", settings.value("6command").toString()).toString();
    QString CONTRIBUTE = settingsusr.value("7icon", settings.value("7icon").toString()).toString();
    QString CONTRIBUTETEXT = settingsusr.value("7text", settings.value("7text").toString()).toString();
    if (!CONTRIBUTETEXT.isEmpty()) {
        ui->buttonContribute->setText(CONTRIBUTETEXT);
    }
    CONTRIBUTECMD = settingsusr.value("7command", settings.value("7command").toString()).toString();
    QString TOOLS = settingsusr.value("8icon", settings.value("8icon").toString()).toString();
    QString TOOLSTEXT = settingsusr.value("8text", settings.value("8text").toString()).toString();
    if (!TOOLSTEXT.isEmpty()) {
        ui->buttonTools->setText(TOOLSTEXT);
    }
    TOOLSCMD = settingsusr.value("8command", settings.value("8command").toString()).toString();
    QString PACKAGEINSTALLER = settingsusr.value("9icon", settings.value("9icon").toString()).toString();
    QString PACKAGEINSTALLERTEXT = settingsusr.value("9text", settings.value("9text").toString()).toString();
    if (!PACKAGEINSTALLERTEXT.isEmpty()) {
        ui->buttonPackageInstall->setText(PACKAGEINSTALLERTEXT);
    }
    PACKAGEINSTALLERCMD = settingsusr.value("9command", settings.value("9command").toString()).toString();
    QString TWEAK = settingsusr.value("10icon", settings.value("10icon").toString()).toString();
    QString TWEAKTEXT = settingsusr.value("10text", settings.value("10text").toString()).toString();
    if (!TWEAKTEXT.isEmpty()) {
        ui->buttonPanelOrient->setText(TWEAKTEXT);
    }
    TWEAKCMD = settingsusr.value("10command", settings.value("10command").toString()).toString();
    QString TOUR = settingsusr.value("11icon", settings.value("11icon").toString()).toString();
    QString TOURTEXT = settingsusr.value("11text", settings.value("11text").toString()).toString();
    if (!TOURTEXT.isEmpty()) {
        ui->buttonTour->setText(TOURTEXT);
    }
    TOURCMD = settingsusr.value("11command", settings.value("11command").toString()).toString();

    // Hide tour if not present AND TOURTEXT.ISEMPTY
    if (TOURTEXT.isEmpty()) {
        if (!QFile::exists("/usr/bin/mx-tour")) {
            ui->buttonTour->hide();
        }
    }

    QString LIVEUSERINFOTEXT
        = settingsusr.value("LIVEUSERINFOTEXT", settings.value("LIVEUSERINFOTEXT").toString()).toString();
    if (!LIVEUSERINFOTEXT.isEmpty()) {
        ui->labelLoginInfo->setText(LIVEUSERINFOTEXT);
    }
    QString SHOWLIVEUSERINFO
        = settingsusr.value("SHOWLIVEUSERINFO", settings.value("SHOWLIVEUSERINFO", "true").toString()).toString();
    ui->labelLoginInfo->setText("<p align=\"center\">" + tr("User demo, password:") + "<b> demo</b>. "
                                + tr("Superuser root, password:") + "<b> root</b>." + "</p>");

    // If running live
    QString test = runCmd("df -T / |tail -n1 |awk '{print $2}'");
    if (test == "aufs" || test == "overlay") {
        ui->checkBox->setVisible(false);
        ui->labelLoginInfo->setVisible(SHOWLIVEUSERINFO != "false");
    } else {
        ui->labelLoginInfo->setVisible(false);
        ui->buttonSetup->setVisible(false);
    }

    // Check /etc/lsb-release file, overridable
    QString CHECKLSB_RELEASE
        = settingsusr.value("CHECKLSB_RELEASE", settings.value("CHECKLSB_RELEASE", "true").toString()).toString();

    if (CHECKLSB_RELEASE != "false") {
        if (QFileInfo::exists("/etc/lsb-release")) {
            QSettings lsb("/etc/lsb-release", QSettings::NativeFormat);
            QString MAINDISTRO = lsb.value("DISTRIB_ID").toString();
            CODENAME = lsb.value("DISTRIB_CODENAME").toString();
            QString DISTRIB_RELEASE = lsb.value("DISTRIB_RELEASE").toString();
            DISTRO = MAINDISTRO + "-" + DISTRIB_RELEASE;
        }
    }

    QFile file("/etc/debian_version");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Error"), file.errorString());
    }

    QTextStream in(&file);
    debian_version = in.readLine();
    file.close();

    ui->LabelDebianVersion->setText(debian_version);

    ui->labelSupportUntil->setText(SUPPORTED);

    QString DESKTOP = runCmd("LANG=C inxi -c 0 -S | grep Desktop | cut -d':' -f5-6").remove(" Distro");
    qDebug() << "desktop is " << DESKTOP;
    if (DESKTOP.contains("Fluxbox")) {
        isfluxbox = true;
        QFile file("/etc/mxfb_version");
        if (file.exists()) {
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Error"), file.errorString());
            }
            QTextStream in(&file);
            QString mxfluxbox_version = in.readLine();
            qDebug() << "mxfluxbox" << mxfluxbox_version;
            file.close();
            if (!mxfluxbox_version.isEmpty()) {
                DESKTOP.append(" " + mxfluxbox_version);
            }
        }
    }

    ui->labelDesktopVersion->setText(DESKTOP);

    ui->labelTitle->setText(
        tr(R"(<html><head/><body><p align="center"><span style=" font-size:14pt; font-weight:600;">%1 &quot;%2&quot;</span></p></body></html>)")
            .arg(DISTRO, CODENAME));
    if (QFile::exists(HEADER)) {
        ui->labelgraphic->setPixmap(HEADER);
    }

    // Setup icons
    // ui->buttonCodecs->setIcon(QIcon(CODECS));
    ui->buttonContribute->setIcon(QIcon(CONTRIBUTE));
    ui->buttonFAQ->setIcon(QIcon(FAQ));
    ui->buttonForum->setIcon(QIcon(FORUMS));
    ui->labelMX->setPixmap(QPixmap(LOGO));
    ui->buttonPackageInstall->setIcon(QIcon(PACKAGEINSTALLER));
    ui->buttonPanelOrient->setIcon(QIcon(TWEAK));
    ui->buttonSetup->setIcon(QIcon(SETUP));
    ui->buttonTools->setIcon(QIcon(TOOLS));
    ui->buttonManual->setIcon(QIcon(MANUAL));
    ui->buttonVideo->setIcon(QIcon(VIDEOS));
    ui->buttonWiki->setIcon(QIcon(WIKI));
    ui->buttonTour->setIcon(QIcon(TOUR));

    // Setup about labels
    ui->labelMXversion->setText(DISTRO);

    setTabStyle();
    this->adjustSize();
}

// Util function for getting bash command output and error code
QString MainWindow::runCmd(const QString& cmd)
{
    QEventLoop loop;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    proc.start("/bin/bash", {"-c", cmd});
    loop.exec();
    return proc.readAll().trimmed();
}

// Get version of the program
QString MainWindow::getVersion(const QString& name)
{
    return runCmd("dpkg-query -f '${Version}' -W " + name);
}

void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About MX Welcome"),
        "<p align=\"center\"><b><h2>" + tr("MX Welcome") + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + version + "</p><p align=\"center\"><h3>" + tr("Program for displaying a welcome screen in MX Linux")
            + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
              "<p align=\"center\">"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-welcome/license.html", tr("%1 License").arg(this->windowTitle()));
    this->show();
}

// Add/remove autostart at login
void MainWindow::on_checkBox_clicked(bool checked)
{
    user_settings.setValue("AutoStartup", checked);
    if (checked) {
        QFile::copy("/usr/share/mx-welcome/mx-welcome.desktop",
                    QDir::homePath() + "/.config/autostart/mx-welcome.desktop");
    } else {
        QFile::remove(QDir::homePath() + "/.config/autostart/mx-welcome.desktop");
    }
}

// Start MX-Tools
void MainWindow::on_buttonTools_clicked() const
{
    QString cmd = TOOLSCMD.isEmpty() ? "mx-tools&" : TOOLSCMD;
    system(cmd.toUtf8());
}

// Launch Manual in browser
void MainWindow::on_buttonManual_clicked() const
{
    QString cmd = isfluxbox ? "mxfb-help&" : "mx-manual&";
    if (!MANUALCMD.isEmpty()) {
        cmd = MANUALCMD;
    }
    system(cmd.toUtf8());
}

// Launch Forum in browser
void MainWindow::on_buttonForum_clicked() const
{
    QString cmd = FORUMCMD.isEmpty() ? "xdg-open http://forum.mxlinux.org/index.php" : FORUMCMD;
    system(cmd.toUtf8());
}

// Launch Wiki in browser
void MainWindow::on_buttonWiki_clicked() const
{
    QString cmd = WIKICMD.isEmpty() ? "xdg-open http://www.mxlinux.org/wiki" : WIKICMD;
    system(cmd.toUtf8());
}

// Launch Video links in browser
void MainWindow::on_buttonVideo_clicked() const
{
    QString cmd = VIDEOCMD.isEmpty() ? "xdg-open http://www.mxlinux.org/videos/" : VIDEOCMD;
    system(cmd.toUtf8());
}

// Launch Contribution page
void MainWindow::on_buttonContribute_clicked() const
{
    QString cmd = CONTRIBUTECMD.isEmpty() ? "xdg-open http://www.mxlinux.org/donate" : CONTRIBUTECMD;
    system(cmd.toUtf8());
}

void MainWindow::on_buttonPanelOrient_clicked() const
{
    QString cmd = TWEAKCMD.isEmpty() ? "mx-tweak&" : TWEAKCMD;
    system(cmd.toUtf8());
}

void MainWindow::on_buttonPackageInstall_clicked() const
{
    QString cmd = PACKAGEINSTALLERCMD.isEmpty() ? "mx-packageinstaller&" : PACKAGEINSTALLERCMD;
    system(cmd.toUtf8());
}

void MainWindow::on_buttonFAQ_clicked() const
{
    QString cmd = FAQCMD.isEmpty() ? "mx-faq&" : FAQCMD;
    system(cmd.toUtf8());
}

void MainWindow::on_buttonSetup_clicked() const
{
    QString cmd = SETUPCMD.isEmpty() ? "minstall-launcher&" : SETUPCMD;
    system(cmd.toUtf8());
}

void MainWindow::on_buttonTOS_clicked() const
{
    QString cmd = TOSCMD.isEmpty() ? "xdg-open https://mxlinux.org/terms-of-use/" : TOSCMD;
    system(cmd.toUtf8());
}

void MainWindow::on_ButtonQSI_clicked() const
{
    if (debian_version.section(".", 0, 0) == "10") {
        system("x-terminal-emulator -e bash -c \"/usr/bin/quick-system-info-mx\" &");
    } else {
        system("/usr/bin/quick-system-info-gui &");
    }
}

void MainWindow::shortSystemInfo()
{
    ui->textBrowser->setText(runCmd("LANG=C inxi -c 0"));
}
void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1) {
        shortSystemInfo();
    }
    setTabStyle();
}

void MainWindow::resizeEvent(QResizeEvent* /*unused*/)
{
    setTabStyle();
}

void MainWindow::setTabStyle()
{
    QString tw = QString::number(ui->tabWidget->width() / 2 - 1);
    // qDebug() << "width" << ui->tabWidget->width() << "tw" << tw;
    ui->tabWidget->setStyleSheet(""
                                 "QTabBar::tab:!selected{width: "
                                 + tw
                                 + "px; background:  rgba(140, 135, 135, 50)}"
                                   ""
                                   "QTabBar::tab:selected{width: "
                                 + tw
                                 + "px}"
                                   "");
}

void MainWindow::on_buttonTour_clicked() const
{
    QString cmd = TOURCMD.isEmpty() ? "mx-tour&" : TOURCMD;
    system(cmd.toUtf8());
}

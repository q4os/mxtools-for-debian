/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
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
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    connect(ui->buttonCancel, &QPushButton::pressed, this, &MainWindow::close);
    setup();

    ui->tabWidget->setCurrentIndex(0);
    if (arg_parser.isSet(QStringLiteral("about")))
        ui->tabWidget->setCurrentIndex(1);

    if (arg_parser.isSet(QStringLiteral("test"))) {
        ui->labelLoginInfo->show();
        ui->buttonSetup->show();
    }
}

MainWindow::~MainWindow() { delete ui; }

// setup versious items first time program runs
void MainWindow::setup()
{
    version = getVersion(QStringLiteral("mx-welcome"));
    this->setWindowTitle(tr("MX Welcome"));

    QString old_conf_file = QDir::homePath() + "/.config/" + QApplication::applicationName() + ".conf";
    if (QFileInfo::exists(old_conf_file)) {
        QSettings old_settings(QApplication::applicationName());
        user_settings.setValue(QStringLiteral("AutoStartup"),
                               old_settings.value(QStringLiteral("AutoStartup"), false).toBool());
        QFile::remove(old_conf_file);
    }
    bool autostart = user_settings.value(QStringLiteral("AutoStartup"), false).toBool();
    ui->checkBox->setChecked(autostart);
    if (!autostart)
        QFile::remove(QDir::homePath() + "/.config/autostart/mx-welcome.desktop");

    // setup title block & icons
    QSettings settings(QStringLiteral("/usr/share/mx-welcome/mx-welcome.conf"), QSettings::NativeFormat);
    QSettings settingsusr(QStringLiteral("/etc/mx-welcome/mx-welcome.conf"), QSettings::NativeFormat);
    QString DISTRO = settings.value(QStringLiteral("DISTRO")).toString();
    if (DISTRO.isEmpty()) {
        DISTRO = settingsusr.value(QStringLiteral("DISTRO")).toString();
    }
    QString CODENAME = settings.value(QStringLiteral("CODENAME")).toString();
    if (CODENAME.isEmpty()) {
        CODENAME = settingsusr.value(QStringLiteral("CODENAME")).toString();
    }

    QString HEADER = settings.value(QStringLiteral("HEADER")).toString();
    if (HEADER.isEmpty()) {
        HEADER = settingsusr.value(QStringLiteral("HEADER")).toString();
    }

    QString LOGO = settings.value(QStringLiteral("LOGO")).toString();
    if (LOGO.isEmpty()) {
        LOGO = settingsusr.value(QStringLiteral("LOGO")).toString();
    }

    QString SUPPORTED = settings.value(QStringLiteral("SUPPORTED")).toString();
    if (SUPPORTED.isEmpty()) {
        SUPPORTED = settingsusr.value(QStringLiteral("SUPPORTED")).toString();
    }

    TOSCMD = settings.value(QStringLiteral("TOSCMD")).toString();
    if (TOSCMD.isEmpty()) {
        TOSCMD = settingsusr.value(QStringLiteral("TOSCMD")).toString();
    }
    QString TOSTEXT = settings.value(QStringLiteral("TOSTEXT")).toString();
    if (TOSTEXT.isEmpty()) {
        TOSTEXT = settingsusr.value(QStringLiteral("TOSTEXT")).toString();
    }
    if (!TOSTEXT.isEmpty()) {
        ui->buttonFAQ->setText(TOSTEXT);
    }

    QString SETUP = settings.value(QStringLiteral("1icon")).toString();
    if (SETUP.isEmpty()) {
        SETUP = settingsusr.value(QStringLiteral("1icon")).toString();
    }
    QString SETUPTEXT = settings.value(QStringLiteral("1text")).toString();
    if (SETUPTEXT.isEmpty()) {
        SETUPTEXT = settingsusr.value(QStringLiteral("1text")).toString();
    }
    if (!SETUPTEXT.isEmpty()) {
        ui->buttonSetup->setText(SETUPTEXT);
    }
    SETUPCMD = settings.value(QStringLiteral("1command")).toString();
    if (SETUPCMD.isEmpty()) {
        SETUPCMD = settingsusr.value(QStringLiteral("1command")).toString();
    }

    QString FAQ = settings.value(QStringLiteral("2icon")).toString();
    if (FAQ.isEmpty()) {
        FAQ = settingsusr.value(QStringLiteral("2icon")).toString();
    }
    QString FAQTEXT = settings.value(QStringLiteral("2text")).toString();
    if (FAQTEXT.isEmpty()) {
        FAQTEXT = settingsusr.value(QStringLiteral("2text")).toString();
    }
    if (!FAQTEXT.isEmpty()) {
        ui->buttonFAQ->setText(FAQTEXT);
    }
    FAQCMD = settings.value(QStringLiteral("2command")).toString();
    if (FAQCMD.isEmpty()) {
        FAQCMD = settingsusr.value(QStringLiteral("2command")).toString();
    }

    QString FORUMS = settings.value(QStringLiteral("3icon")).toString();
    if (FORUMS.isEmpty()) {
        FORUMS = settingsusr.value(QStringLiteral("3icon")).toString();
    }
    QString FORUMTEXT = settings.value(QStringLiteral("3text")).toString();
    if (FORUMTEXT.isEmpty()) {
        FORUMTEXT = settingsusr.value(QStringLiteral("3text")).toString();
    }
    if (!FORUMTEXT.isEmpty()) {
        ui->buttonForum->setText(FORUMTEXT);
    }
    FORUMCMD = settings.value(QStringLiteral("3command")).toString();
    if (FORUMCMD.isEmpty()) {
        FORUMCMD = settingsusr.value(QStringLiteral("3command")).toString();
    }

    QString MANUAL = settings.value(QStringLiteral("4icon")).toString();
    if (MANUAL.isEmpty()) {
        MANUAL = settingsusr.value(QStringLiteral("4icon")).toString();
    }
    QString MANUALTEXT = settings.value(QStringLiteral("4text")).toString();
    if (MANUALTEXT.isEmpty()) {
        MANUALTEXT = settingsusr.value(QStringLiteral("4text")).toString();
    }
    if (!MANUALTEXT.isEmpty()) {
        ui->buttonManual->setText(MANUALTEXT);
    }
    MANUALCMD = settings.value(QStringLiteral("4command")).toString();
    if (MANUALCMD.isEmpty()) {
        MANUALCMD = settingsusr.value(QStringLiteral("4command")).toString();
    }

    QString VIDEOS = settings.value(QStringLiteral("5icon")).toString();
    if (VIDEOS.isEmpty()) {
        VIDEOS = settingsusr.value(QStringLiteral("5icon")).toString();
    }
    QString VIDEOTEXT = settings.value(QStringLiteral("5text")).toString();
    if (VIDEOTEXT.isEmpty()) {
        VIDEOTEXT = settingsusr.value(QStringLiteral("5text")).toString();
    }
    if (!VIDEOTEXT.isEmpty()) {
        ui->buttonVideo->setText(VIDEOTEXT);
    }
    VIDEOCMD = settings.value(QStringLiteral("5command")).toString();
    if (VIDEOCMD.isEmpty()) {
        VIDEOCMD = settingsusr.value(QStringLiteral("5command")).toString();
    }

    QString WIKI = settings.value(QStringLiteral("6icon")).toString();
    if (WIKI.isEmpty()) {
        WIKI = settingsusr.value(QStringLiteral("6icon")).toString();
    }
    QString WIKITEXT = settings.value(QStringLiteral("6text")).toString();
    if (WIKITEXT.isEmpty()) {
        WIKITEXT = settingsusr.value(QStringLiteral("6text")).toString();
    }
    if (!WIKITEXT.isEmpty()) {
        ui->buttonWiki->setText(WIKITEXT);
    }
    WIKICMD = settings.value(QStringLiteral("6command")).toString();
    if (WIKICMD.isEmpty()) {
        WIKICMD = settingsusr.value(QStringLiteral("6command")).toString();
    }

    QString CONTRIBUTE = settings.value(QStringLiteral("7icon")).toString();
    if (CONTRIBUTE.isEmpty()) {
        CONTRIBUTE = settingsusr.value(QStringLiteral("7icon")).toString();
    }
    QString CONTRIBUTETEXT = settings.value(QStringLiteral("7text")).toString();
    if (CONTRIBUTETEXT.isEmpty()) {
        CONTRIBUTETEXT = settingsusr.value(QStringLiteral("7text")).toString();
    }
    if (!CONTRIBUTETEXT.isEmpty()) {
        ui->buttonContribute->setText(CONTRIBUTETEXT);
    }
    CONTRIBUTECMD = settings.value(QStringLiteral("7command")).toString();
    if (CONTRIBUTECMD.isEmpty()) {
        CONTRIBUTECMD = settingsusr.value(QStringLiteral("7command")).toString();
    }

    QString TOOLS = settings.value(QStringLiteral("8icon")).toString();
    if (TOOLS.isEmpty()) {
        TOOLS = settingsusr.value(QStringLiteral("8icon")).toString();
    }
    QString TOOLSTEXT = settings.value(QStringLiteral("8text")).toString();
    if (TOOLSTEXT.isEmpty()) {
        TOOLSTEXT = settingsusr.value(QStringLiteral("8text")).toString();
    }
    if (!TOOLSTEXT.isEmpty()) {
        ui->buttonTools->setText(TOOLSTEXT);
    }
    TOOLSCMD = settings.value(QStringLiteral("8command")).toString();
    if (TOOLSCMD.isEmpty()) {
        TOOLSCMD = settingsusr.value(QStringLiteral("8command")).toString();
    }

    QString PACKAGEINSTALLER = settings.value(QStringLiteral("9icon")).toString();
    if (PACKAGEINSTALLER.isEmpty()) {
        PACKAGEINSTALLER = settingsusr.value(QStringLiteral("9icon")).toString();
    }
    QString PACKAGEINSTALLERTEXT = settings.value(QStringLiteral("9text")).toString();
    if (PACKAGEINSTALLERTEXT.isEmpty()) {
        PACKAGEINSTALLERTEXT = settingsusr.value(QStringLiteral("9text")).toString();
    }
    if (!PACKAGEINSTALLERTEXT.isEmpty()) {
        ui->buttonPackageInstall->setText(PACKAGEINSTALLERTEXT);
    }
    PACKAGEINSTALLERCMD = settings.value(QStringLiteral("9command")).toString();
    if (PACKAGEINSTALLERCMD.isEmpty()) {
        PACKAGEINSTALLERCMD = settingsusr.value(QStringLiteral("9command")).toString();
    }

    QString TWEAK = settings.value(QStringLiteral("10icon")).toString();
    if (TWEAK.isEmpty()) {
        TWEAK = settingsusr.value(QStringLiteral("10icon")).toString();
    }
    QString TWEAKTEXT = settings.value(QStringLiteral("10text")).toString();
    if (TWEAKTEXT.isEmpty()) {
        TWEAKTEXT = settingsusr.value(QStringLiteral("10text")).toString();
    }
    if (!TWEAKTEXT.isEmpty()) {
        ui->buttonPanelOrient->setText(TWEAKTEXT);
    }
    TWEAKCMD = settings.value(QStringLiteral("10command")).toString();
    if (TWEAKCMD.isEmpty()) {
        TWEAKCMD = settingsusr.value(QStringLiteral("10command")).toString();
    }

    QString TOUR = settings.value(QStringLiteral("11icon")).toString();
    if (TOUR.isEmpty()) {
        TOUR = settingsusr.value(QStringLiteral("11icon")).toString();
    }
    QString TOURTEXT = settings.value(QStringLiteral("11text")).toString();
    if (TOURTEXT.isEmpty()) {
        TOURTEXT = settingsusr.value(QStringLiteral("11text")).toString();
    }
    if (!TOURTEXT.isEmpty()) {
        ui->buttonTour->setText(TOURTEXT);
    }
    TOURCMD = settings.value(QStringLiteral("11command")).toString();
    if (TOURCMD.isEmpty()) {
        TOURCMD = settingsusr.value(QStringLiteral("11command")).toString();
    }

    // hide tour if not present AND TOURTEXT.ISEMPTY
    if (TOURTEXT.isEmpty()) {
        if (!QFile::exists(QStringLiteral("/usr/bin/mx-tour"))) {
            ui->buttonTour->hide();
        }
    }

    QString LIVEUSERINFOTEXT = settings.value(QStringLiteral("LIVEUSERINFOTEXT")).toString();
    if (LIVEUSERINFOTEXT.isEmpty()) {
        LIVEUSERINFOTEXT = settingsusr.value(QStringLiteral("LIVEUSERINFOTEXT")).toString();
    }
    if (!LIVEUSERINFOTEXT.isEmpty()) {
        ui->labelLoginInfo->setText(LIVEUSERINFOTEXT);
    }
    QString SHOWLIVEUSERINFO = settings.value(QStringLiteral("SHOWLIVEUSERINFO"), "true").toString();
    if (SHOWLIVEUSERINFO.isEmpty()) {
        SHOWLIVEUSERINFO = settingsusr.value(QStringLiteral("SHOWLIVEUSERINFO"), "true").toString();
    }
    // qDebug() << "hide value: " << SHOWLIVEUSERINFO;
    bool LIVEUSERINFOSHOW = true;
    if (SHOWLIVEUSERINFO == QLatin1String("false")) {
        LIVEUSERINFOSHOW = false;
    }

    ui->labelLoginInfo->setText("<p align=\"center\">" + tr("User demo, password:") + "<b> demo</b>. "
                                + tr("Superuser root, password:") + "<b> root</b>." + "</p>");

    // if running live
    QString test = runCmd(QStringLiteral("df -T / |tail -n1 |awk '{print $2}'")).output;
    if (test == QLatin1String("aufs") || test == QLatin1String("overlay")) {
        ui->checkBox->setVisible(false);
        ui->labelLoginInfo->setVisible(LIVEUSERINFOSHOW);
    } else {
        ui->labelLoginInfo->setVisible(false);
        ui->buttonSetup->setVisible(false);
    }

    // check /etc/lsb-release file, overridable
    QString CHECKLSB_RELEASE = settings.value(QStringLiteral("CHECKLSB_RELEASE"), "true").toString();
    if (CHECKLSB_RELEASE.isEmpty()) {
        CHECKLSB_RELEASE = settingsusr.value(QStringLiteral("CHECKLSB_RELEASE"), "true").toString();
    }

    bool CHECKLSB = true;
    if (CHECKLSB_RELEASE == QLatin1String("false")) {
        CHECKLSB = false;
    }

    if (CHECKLSB) {
        if (QFileInfo::exists(QStringLiteral("/etc/lsb-release"))) {
            QSettings lsb(QStringLiteral("/etc/lsb-release"), QSettings::NativeFormat);
            QString MAINDISTRO = lsb.value(QStringLiteral("DISTRIB_ID")).toString();
            CODENAME = lsb.value(QStringLiteral("DISTRIB_CODENAME")).toString();
            QString DISTRIB_RELEASE = lsb.value(QStringLiteral("DISTRIB_RELEASE")).toString();
            DISTRO = MAINDISTRO + "-" + DISTRIB_RELEASE;
        }
    }

    QString mxfluxbox_version;

    QFile file(QStringLiteral("/etc/debian_version"));
    if (!file.open(QIODevice::ReadOnly))
        QMessageBox::information(nullptr, tr("Error"), file.errorString());

    QTextStream in(&file);
    debian_version = in.readLine();
    file.close();

    ui->LabelDebianVersion->setText(debian_version);

    ui->labelSupportUntil->setText(SUPPORTED);

    QString DESKTOP
        = runCmd(QStringLiteral("LANG=C inxi -c 0 -S | grep Desktop | cut -d':' -f5-6")).output.remove(" Distro");
    qDebug() << "desktop is " << DESKTOP;
    if (DESKTOP.contains(QLatin1String("Fluxbox"))) {
        isfluxbox = true;
        QFile file(QStringLiteral("/etc/mxfb_version"));
        if (file.exists()) {
            if (!file.open(QIODevice::ReadOnly))
                QMessageBox::information(nullptr, tr("Error"), file.errorString());
            QTextStream in(&file);
            mxfluxbox_version = in.readLine();
            qDebug() << "mxfluxbox" << mxfluxbox_version;
            file.close();
            if (!mxfluxbox_version.isEmpty())
                DESKTOP.append(" " + mxfluxbox_version);
        }
    }

    ui->labelDesktopVersion->setText(DESKTOP);

    ui->labelTitle->setText(
        tr(R"(<html><head/><body><p align="center"><span style=" font-size:14pt; font-weight:600;">%1 &quot;%2&quot;</span></p></body></html>)")
            .arg(DISTRO, CODENAME));
    if (QFile::exists(HEADER))
        ui->labelgraphic->setPixmap(HEADER);

    // setup icons
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

    // setup about labels
    ui->labelMXversion->setText(DISTRO);

    settabstyle();
    this->adjustSize();
}

// Util function for getting bash command output and error code
Result MainWindow::runCmd(const QString& cmd)
{
    QEventLoop loop;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    proc.start(QStringLiteral("/bin/bash"), QStringList() << QStringLiteral("-c") << cmd);
    loop.exec();
    return {proc.exitCode(), proc.readAll().trimmed()};
}

// Get version of the program
QString MainWindow::getVersion(const QString& name) { return runCmd("dpkg-query -f '${Version}' -W " + name).output; }

// About button clicked
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
        QStringLiteral("/usr/share/doc/mx-welcome/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

// Add/remove autostart at login
void MainWindow::on_checkBox_clicked(bool checked)
{
    user_settings.setValue(QStringLiteral("AutoStartup"), checked);
    if (checked)
        QFile::copy(QStringLiteral("/usr/share/mx-welcome/mx-welcome.desktop"),
                    QDir::homePath() + "/.config/autostart/mx-welcome.desktop");
    else
        QFile::remove(QDir::homePath() + "/.config/autostart/mx-welcome.desktop");
}

// Start MX-Tools
void MainWindow::on_buttonTools_clicked() const
{
    QString cmd = QStringLiteral("mx-tools&");
    if (!TOOLSCMD.isEmpty()) {
        cmd = TOOLSCMD;
    }
    system(cmd.toUtf8());
}

// Launch Manual in browser
void MainWindow::on_buttonManual_clicked() const
{
    QString cmd;
    if (isfluxbox)
        cmd = QStringLiteral("mxfb-help&");
    else
        cmd = QStringLiteral("mx-manual&");
    if (!MANUALCMD.isEmpty())
        cmd = MANUALCMD;

    system(cmd.toUtf8());
}

// Launch Forum in browser
void MainWindow::on_buttonForum_clicked() const
{
    QString cmd = QStringLiteral("xdg-open http://forum.mxlinux.org/index.php");
    if (!FORUMCMD.isEmpty()) {
        cmd = FORUMCMD;
    }
    system(cmd.toUtf8());
}

// Launch Wiki in browser
void MainWindow::on_buttonWiki_clicked() const
{
    QString cmd = QStringLiteral("xdg-open http://www.mxlinux.org/wiki");
    if (!WIKICMD.isEmpty()) {
        cmd = WIKICMD;
    }
    system(cmd.toUtf8());
}

// Launch Video links in browser
void MainWindow::on_buttonVideo_clicked() const
{
    QString cmd = QStringLiteral("xdg-open http://www.mxlinux.org/videos/");
    if (!VIDEOCMD.isEmpty()) {
        cmd = VIDEOCMD;
    }
    system(cmd.toUtf8());
}

// Launch Contribution page
void MainWindow::on_buttonContribute_clicked() const
{
    QString cmd = QStringLiteral("xdg-open http://www.mxlinux.org/donate");
    if (!CONTRIBUTECMD.isEmpty()) {
        cmd = CONTRIBUTECMD;
    }
    system(cmd.toUtf8());
}

void MainWindow::on_buttonPanelOrient_clicked() const
{
    QString cmd = QStringLiteral("mx-tweak&");
    if (!TWEAKCMD.isEmpty()) {
        cmd = TWEAKCMD;
    }
    system(cmd.toUtf8());
}

void MainWindow::on_buttonPackageInstall_clicked() const
{
    QString cmd = QStringLiteral("mxpi-launcher&");
    if (!PACKAGEINSTALLERCMD.isEmpty()) {
        cmd = PACKAGEINSTALLERCMD;
    }
    system(cmd.toUtf8());
}

void MainWindow::on_buttonFAQ_clicked() const
{
    QString cmd = QStringLiteral("mx-faq&");
    if (!FAQCMD.isEmpty()) {
        cmd = FAQCMD;
    }
    system(cmd.toUtf8());
}

void MainWindow::on_buttonSetup_clicked() const
{
    QString cmd = QStringLiteral("minstall-launcher&");
    if (!SETUPCMD.isEmpty()) {
        cmd = SETUPCMD;
    }
    system(cmd.toUtf8());
}

void MainWindow::on_buttonTOS_clicked() const
{
    QString cmd = QStringLiteral("xdg-open https://mxlinux.org/terms-of-use/");
    if (!TOSCMD.isEmpty()) {
        cmd = TOSCMD;
    }
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

void MainWindow::shortsysteminfo() { ui->textBrowser->setText(runCmd(QStringLiteral("LANG=C inxi -c 0")).output); }
void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1)
        shortsysteminfo();
    settabstyle();
}

void MainWindow::resizeEvent(QResizeEvent* /*unused*/) { settabstyle(); }

void MainWindow::settabstyle()
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
    QString cmd = QStringLiteral("mx-tour&");
    if (!TOURCMD.isEmpty()) {
        cmd = TOURCMD;
    }
    system(cmd.toUtf8());
}

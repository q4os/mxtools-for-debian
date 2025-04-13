/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2024 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
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
#include "about.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QVBoxLayout>

#include "common.h"
#include <pwd.h>
#include <unistd.h>

// Display doc as normal user when run as root
void displayDoc(const QString &url, const QString &title)
{
    bool started_as_root = false;
    QString logname;
    if (getuid() == 0) {
        started_as_root = true;
        logname = QString::fromUtf8(getlogin());
        if (logname.isEmpty()) {
            QProcess proc;
            proc.start("logname", {}, QIODevice::ReadOnly);
            proc.waitForFinished();
            logname = QString::fromUtf8(proc.readAllStandardOutput().trimmed());
        }
        if (!logname.isEmpty()) {
            QString homeDir;
            struct passwd *pw = getpwnam(logname.toUtf8().constData());
            if (pw != nullptr) {
                homeDir = QString::fromUtf8(pw->pw_dir);
            }
            if (!homeDir.isEmpty()) {
                qputenv("HOME", homeDir.toUtf8()); // Use original home for theming purposes
            } else {
                qWarning("Failed to determine home directory for user: %s", qPrintable(logname));
            }
        } else {
            qWarning("Failed to determine the username to set HOME environment variable.");
        }
    }
    // Prefer mx-viewer otherwise use xdg-open (use runuser to run that as logname user)
    static const QString executablePath = QStandardPaths::findExecutable("mx-viewer");
    if (!executablePath.isEmpty()) {
        QProcess::startDetached("mx-viewer", {url, title});
    } else {
        if (!started_as_root) {
            QProcess::startDetached("xdg-open", {url});
        } else if (!logname.isEmpty()) {
            static const QString runuserPath = QStandardPaths::findExecutable("runuser");
            if (!runuserPath.isEmpty()) {
                QUrl parsedUrl(url);
                if (parsedUrl.isValid() && (parsedUrl.scheme() == "http" || parsedUrl.scheme() == "https")) {
                    QProcess::startDetached("runuser", {"-u", logname, "--", "xdg-open", url});
                } else {
                    qWarning("Invalid URL provided: %s", qPrintable(url));
                }
            } else {
                qWarning("runuser command is not available on the system. Cannot open URL as the specified user.");
            }
        } else {
            qWarning("Failed to determine the username to run xdg-open as.");
        }
    }
    if (started_as_root) {
        qputenv("HOME", starting_home.toUtf8());
    }
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title)
{
    QMessageBox msgBox(QMessageBox::NoIcon, title, message);

    QPushButton *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    QPushButton *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    QPushButton *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(licence_url, license_title);
    } else if (msgBox.clickedButton() == btnChangelog) {
        QDialog changelog;
        auto *text = new QTextEdit(&changelog);
        text->setReadOnly(true);

        QString changelogPath
            = "/usr/share/doc/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName() + "/changelog.gz";
        bool zlessExists = !QStandardPaths::findExecutable("zless").isEmpty();
        bool changelogExists = QFileInfo::exists(changelogPath);

        if (zlessExists && changelogExists) {
            QProcess proc;
            proc.start("zless", {changelogPath}, QIODevice::ReadOnly);
            proc.waitForFinished(5000);
            text->setText(proc.readAllStandardOutput());
        } else {
            if (!changelogExists) {
                text->setText(QObject::tr("Error: Changelog file is missing."));
            } else if (!zlessExists) {
                text->setText(QObject::tr("Error: Required utility 'zless' is missing."));
            }
        }

        auto *layout = new QVBoxLayout(&changelog);
        layout->addWidget(text);

        auto *btnClose = new QPushButton(QObject::tr("&Close"), &changelog);
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        QObject::connect(btnClose, &QPushButton::clicked, &changelog, &QDialog::close);
        layout->addWidget(btnClose);
        changelog.setLayout(layout);
        changelog.resize(600, 500);
        changelog.exec();
    }
}

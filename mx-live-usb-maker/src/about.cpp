/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2024 MX Authors
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

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QVBoxLayout>

#include "common.h"
#include <unistd.h>

// Display document as normal user when run as root
void displayDoc(const QString &url, const QString &title)
{
    bool isRunningAsRoot = (qEnvironmentVariable("HOME") == "root");
    QString originalHome = isRunningAsRoot ? starting_home : QString();

    if (isRunningAsRoot) {
        qputenv("HOME", originalHome.toUtf8()); // Use original home for theming purposes
    }

    // Prefer mx-viewer, otherwise use xdg-open (use runuser to run that as logname user)
    QString viewerExecutable = QStandardPaths::findExecutable("mx-viewer");
    if (!viewerExecutable.isEmpty()) {
        QProcess::startDetached(viewerExecutable, {url, title});
    } else {
        if (getuid() != 0) {
            QProcess::startDetached("xdg-open", {url});
        } else {
            QProcess proc;
            proc.start("logname", {}, QIODevice::ReadOnly);
            proc.waitForFinished();
            QString user = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            QProcess::startDetached("runuser", {"-u", user, "--", "xdg-open", url});
        }
    }

    if (isRunningAsRoot) {
        qputenv("HOME", "/root");
    }
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licenceUrl,
                        const QString &licenseTitle, QWidget *parent)
{
    const int dialogWidth = 600;
    const int dialogHeight = 500;
    QMessageBox msgBox(QMessageBox::NoIcon, title, message);

    auto *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    auto *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(licenceUrl, licenseTitle);
    } else if (msgBox.clickedButton() == btnChangelog) {
        QDialog changelogDialog(parent);
        changelogDialog.setWindowTitle(QObject::tr("Changelog"));
        changelogDialog.resize(dialogWidth, dialogHeight);

        auto *textEdit = new QTextEdit(&changelogDialog);
        textEdit->setReadOnly(true);

        QProcess changelogProc;
        changelogProc.start(
            "zless",
            {"/usr/share/doc/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName() + "/changelog.gz"},
            QIODevice::ReadOnly);
        changelogProc.waitForFinished();
        textEdit->setText(changelogProc.readAllStandardOutput());

        auto *btnClose = new QPushButton(QObject::tr("&Close"), &changelogDialog);
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        QObject::connect(btnClose, &QPushButton::clicked, &changelogDialog, &QDialog::close);

        auto *layout = new QVBoxLayout(&changelogDialog);
        layout->addWidget(textEdit);
        layout->addWidget(btnClose);
        changelogDialog.setLayout(layout);
        changelogDialog.exec();
    }
}

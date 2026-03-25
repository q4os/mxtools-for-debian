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
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
void setupDocDialog(QDialog &dialog, QTextBrowser *browser, const QString &title)
{
    dialog.setWindowTitle(title);
    dialog.resize(700, 600);

    browser->setOpenExternalLinks(true);

    auto *btnClose = new QPushButton(QObject::tr("&Close"), &dialog);
    btnClose->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
    QObject::connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::close);

    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(browser);
    layout->addWidget(btnClose);
}

void showHtmlDoc(const QString &url, const QString &title)
{
    QDialog dialog;
    auto *browser = new QTextBrowser(&dialog);
    setupDocDialog(dialog, browser, title);

    const QUrl sourceUrl = QUrl::fromUserInput(url);
    const QString localPath = sourceUrl.isLocalFile() ? sourceUrl.toLocalFile() : url;
    if (QFileInfo::exists(localPath)) {
        browser->setSource(sourceUrl.isLocalFile() ? sourceUrl : QUrl::fromLocalFile(url));
    } else {
        browser->setText(QObject::tr("Could not load %1").arg(url));
        qDebug() << "Could not load HTML document" << url;
    }
    dialog.exec();
}
} // namespace

void displayDoc(const QString &url, const QString &title)
{
    showHtmlDoc(url, title);
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title)
{
    const auto width = 600;
    const auto height = 500;
    QMessageBox msgBox(QMessageBox::NoIcon, title, message);
    auto *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    auto *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(licence_url, license_title);
    } else if (msgBox.clickedButton() == btnChangelog) {
        QDialog changelog;
        changelog.setWindowTitle(QObject::tr("Changelog"));
        changelog.resize(width, height);

        auto *text = new QTextEdit(&changelog);
        text->setReadOnly(true);
        QProcess proc;
        const QString appName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        const QString changelogPath = QStringLiteral("/usr/share/doc/") + appName + QStringLiteral("/changelog.gz");
        proc.start(QStringLiteral("zcat"), {changelogPath}, QIODevice::ReadOnly);
        if (proc.waitForStarted(3000) && proc.waitForFinished(3000)) {
            text->setText(proc.readAllStandardOutput());
        } else {
            text->setText(QObject::tr("Could not load changelog."));
        }

        auto *btnClose = new QPushButton(QObject::tr("&Close"), &changelog);
        btnClose->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        QObject::connect(btnClose, &QPushButton::clicked, &changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(&changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog.setLayout(layout);
        changelog.exec();
    }
}

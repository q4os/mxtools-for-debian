/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2025 MX Authors
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
#define QT_USE_QSTRINGBUILDER
#include "about.h"

#include <QCoreApplication>
#include <QDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

void displayDoc(QWidget *parent, const QString &path, const QString &title, bool largeWindow)
{
    auto *dialog = new QDialog(parent);
    auto *browser = new QTextBrowser(dialog);
    auto *btnClose = new QPushButton(QObject::tr("&Close"), dialog);
    auto *layout = new QVBoxLayout(dialog);
    const QFileInfo fileInfo(path);
    const QUrl sourceUrl = QUrl::fromLocalFile(path);
    const bool nonModal = largeWindow;

    dialog->setWindowTitle(title);
    if (largeWindow) {
        dialog->setWindowFlags(Qt::Window);
        dialog->resize(1000, 800);
    } else {
        dialog->resize(700, 600);
    }

    browser->setOpenExternalLinks(true);
    browser->setSearchPaths({fileInfo.absolutePath()});

    btnClose->setIcon(QIcon::fromTheme("window-close"));
    QObject::connect(btnClose, &QPushButton::clicked, dialog, &QDialog::close);

    layout->addWidget(browser);
    layout->addWidget(btnClose);

    auto loadDocument = [browser, path, sourceUrl]() {
        if (QFileInfo::exists(path)) {
            browser->setSource(sourceUrl);
        } else {
            browser->setText(QObject::tr("Could not load %1").arg(path));
        }
    };

    if (nonModal) {
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        QTimer::singleShot(0, dialog, loadDocument);
    } else {
        QTimer::singleShot(0, dialog, loadDocument);
        dialog->exec();
        dialog->deleteLater();
    }
}

void displayAboutMsgBox(QWidget *parent, const QString &title, const QString &message, const QString &licenceSource,
                        const QString &licenseTitle)
{
    const auto width = 600;
    const auto height = 500;
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::NoIcon);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    auto *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    auto *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(parent, licenceSource, licenseTitle);
    } else if (msgBox.clickedButton() == btnChangelog) {
        auto *changelog = new QDialog(parent);
        changelog->setWindowTitle(QObject::tr("Changelog"));
        changelog->resize(width, height);

        auto *text = new QTextEdit(changelog);
        text->setReadOnly(true);
        QProcess proc;
        proc.start(
            "zless",
            {"/usr/share/doc/" % QFileInfo(QCoreApplication::applicationFilePath()).fileName() % "/changelog.gz"},
            QIODevice::ReadOnly);
        proc.waitForFinished();
        text->setText(proc.readAllStandardOutput());

        auto *btnClose = new QPushButton(QObject::tr("&Close"), changelog);
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        QObject::connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
        changelog->deleteLater();
    }
}

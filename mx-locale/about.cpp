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

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QTextBrowser>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include "common.h"

namespace
{
void showHtmlDoc(const QString &url, const QString &title, bool largeWindow)
{
    QDialog dialog;
    dialog.setWindowTitle(title);
    if (largeWindow) {
        dialog.setWindowFlags(Qt::Window);
        dialog.resize(1000, 800);
    } else {
        dialog.resize(700, 600);
    }

    auto *browser = new QTextBrowser(&dialog);
    browser->setOpenExternalLinks(true);

    const QUrl sourceUrl = QUrl::fromUserInput(url);
    const QString localPath = sourceUrl.isLocalFile() ? sourceUrl.toLocalFile() : url;
    if (sourceUrl.isLocalFile() ? QFileInfo::exists(localPath) : QFileInfo::exists(url)) {
        browser->setSource(sourceUrl.isLocalFile() ? sourceUrl : QUrl::fromLocalFile(url));
    } else {
        browser->setText(QObject::tr("Could not load %1").arg(url));
        qDebug() << "Could not load HTML document" << url;
    }

    auto *btnClose = new QPushButton(QObject::tr("&Close"), &dialog);
    btnClose->setIcon(QIcon::fromTheme("window-close"));
    QObject::connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::close);

    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(browser);
    layout->addWidget(btnClose);
    dialog.exec();
}

void addHelpHtmlBlock(QVBoxLayout *layout, const QString &html)
{
    auto *label = new QLabel;
    QFont font = label->font();
    font.setPointSize(font.pointSize() + 3);
    label->setFont(font);
    label->setTextFormat(Qt::RichText);
    label->setText(html);
    label->setWordWrap(true);
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(label);
}

void addHelpImageBlock(QVBoxLayout *layout, const QString &imagePath)
{
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        auto *label = new QLabel(QObject::tr("Could not load %1").arg(imagePath));
        layout->addWidget(label);
        qDebug() << "Could not load help image" << imagePath;
        return;
    }

    auto *label = new QLabel;
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setPixmap(pixmap);
    layout->addWidget(label);
}
} // namespace

void displayDoc(const QString &url, const QString &title, bool largeWindow)
{
    showHtmlDoc(url, title, largeWindow);
}

void displayHelpDoc(const QString &path, const QString &title)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open help document" << path;
        showHtmlDoc(path, title, true);
        return;
    }

    const QString html = QString::fromUtf8(file.readAll());
    const QString baseDir = QFileInfo(path).absolutePath();

    QDialog dialog;
    dialog.setWindowTitle(title);
    dialog.setWindowFlags(Qt::Window);
    dialog.resize(1000, 800);

    auto *scrollArea = new QScrollArea(&dialog);
    scrollArea->setWidgetResizable(true);

    auto *content = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(10);

    const QRegularExpression blockRegex(R"(<(h1|p)\b[^>]*>.*?<\/\1>|<img\b[^>]*>)",
                                        QRegularExpression::CaseInsensitiveOption
                                            | QRegularExpression::DotMatchesEverythingOption);
    auto matchIterator = blockRegex.globalMatch(html);
    while (matchIterator.hasNext()) {
        const QString block = matchIterator.next().captured(0).trimmed();
        if (block.startsWith("<img", Qt::CaseInsensitive)) {
            const QRegularExpression srcRegex(R"re(src\s*=\s*"([^"]+)")re",
                                              QRegularExpression::CaseInsensitiveOption);
            const auto srcMatch = srcRegex.match(block);
            if (srcMatch.hasMatch()) {
                addHelpImageBlock(contentLayout, QDir(baseDir).filePath(srcMatch.captured(1)));
            }
            continue;
        }
        addHelpHtmlBlock(contentLayout, block);
    }
    contentLayout->addStretch();

    scrollArea->setWidget(content);

    auto *btnClose = new QPushButton(QObject::tr("&Close"), &dialog);
    btnClose->setIcon(QIcon::fromTheme("window-close"));
    QObject::connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::close);

    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(scrollArea);
    layout->addWidget(btnClose);
    dialog.exec();
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licenceUrl,
                        const QString &licenseTitle)
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
        displayDoc(licenceUrl, licenseTitle);
    } else if (msgBox.clickedButton() == btnChangelog) {
        QDialog changelog;
        changelog.setWindowTitle(QObject::tr("Changelog"));
        changelog.resize(width, height);

        auto *text = new QTextEdit(&changelog);
        text->setReadOnly(true);
        QProcess proc;
        const QString appName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        const QString changelogPath = QDir(Paths::usrShareDoc).filePath(appName + "/changelog.gz");
        proc.start("zcat", {changelogPath}, QIODevice::ReadOnly);
        if (proc.waitForStarted(3000) && proc.waitForFinished(3000)) {
            text->setText(proc.readAllStandardOutput());
        } else {
            text->setText(QObject::tr("Could not load changelog."));
        }

        auto *btnClose = new QPushButton(QObject::tr("&Close"), &changelog);
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        QObject::connect(btnClose, &QPushButton::clicked, &changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(&changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog.exec();
    }
}

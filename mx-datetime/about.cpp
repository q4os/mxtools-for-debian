#include "about.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace
{
void setupDocDialog(QDialog &dialog, QTextBrowser *browser, const QString &title, bool largeWindow)
{
    dialog.setWindowTitle(title);
    if (largeWindow) {
        dialog.setWindowFlags(Qt::Window);
        dialog.resize(1000, 800);
    } else {
        dialog.resize(700, 600);
    }

    browser->setOpenExternalLinks(true);

    auto *btnClose = new QPushButton(QObject::tr("&Close"), &dialog);
    btnClose->setIcon(QIcon::fromTheme(u"window-close"_s));
    QObject::connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::close);

    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(browser);
    layout->addWidget(btnClose);
}

void showHtmlDoc(const QString &url, const QString &title, bool largeWindow)
{
    QDialog dialog;
    auto *browser = new QTextBrowser(&dialog);
    setupDocDialog(dialog, browser, title, largeWindow);

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

void displayDoc(const QString &url, const QString &title, bool largeWindow)
{
    showHtmlDoc(url, title, largeWindow);
}

void displayHelpDoc(const QString &path, const QString &title)
{
    showHtmlDoc(path, title, true);
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licenceUrl,
                        const QString &licenseTitle)
{
    const auto width = 600;
    const auto height = 500;
    QMessageBox msgBox(QMessageBox::NoIcon, title, message);
    auto *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    auto *btnClose = msgBox.addButton(QObject::tr("Close"), QMessageBox::NoRole);
    btnClose->setIcon(QIcon::fromTheme(u"window-close"_s));

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
        const QString changelogPath = u"/usr/share/doc/"_s + appName + u"/changelog.gz"_s;
        proc.start(u"zcat"_s, {changelogPath}, QIODevice::ReadOnly);
        if (proc.waitForStarted(3000) && proc.waitForFinished(3000)) {
            text->setText(QString::fromUtf8(proc.readAllStandardOutput()));
        } else {
            text->setText(QObject::tr("Could not load changelog."));
        }

        auto *btnCloseDialog = new QPushButton(QObject::tr("&Close"), &changelog);
        btnCloseDialog->setIcon(QIcon::fromTheme(u"window-close"_s));
        QObject::connect(btnCloseDialog, &QPushButton::clicked, &changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(&changelog);
        layout->addWidget(text);
        layout->addWidget(btnCloseDialog);
        changelog.exec();
    }
}

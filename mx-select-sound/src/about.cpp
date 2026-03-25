#include "about.h"

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
QString docPath(const QString &fileName)
{
    if (QFileInfo::exists(fileName)) {
        return QFileInfo(fileName).canonicalFilePath();
    }

    const QString installedPath = QStringLiteral("/usr/share/doc/mx-select-sound/") + fileName;
    const QString appDirPath = QCoreApplication::applicationDirPath();
    const QStringList candidates {
        QDir(appDirPath).filePath(QStringLiteral("../help/") + fileName),
        QDir(appDirPath).filePath(QStringLiteral("../../help/") + fileName),
        QDir::current().filePath(QStringLiteral("help/") + fileName),
        installedPath,
    };

    for (const QString &candidate : candidates) {
        const QString normalized = QFileInfo(candidate).canonicalFilePath();
        if (!normalized.isEmpty() && QFileInfo::exists(normalized)) {
            return normalized;
        }
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return installedPath;
}
}

void displayDoc(const QString &fileName, const QString &title, bool largeWindow)
{
    const QString path = docPath(fileName);
    auto *dialog = new QDialog(qApp->activeWindow());
    auto *browser = new QTextBrowser(dialog);
    auto *btnClose = new QPushButton(QObject::tr("&Close"), dialog);
    auto *layout = new QVBoxLayout(dialog);
    const QFileInfo fileInfo(path);
    const QUrl sourceUrl = QUrl::fromLocalFile(path);

    dialog->setWindowTitle(title);
    if (largeWindow) {
        dialog->setWindowFlags(Qt::Window);
        dialog->resize(1000, 800);
    } else {
        dialog->resize(700, 600);
    }

    browser->setOpenExternalLinks(true);
    browser->setSearchPaths({fileInfo.absolutePath()});

    btnClose->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
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

    if (largeWindow) {
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        QTimer::singleShot(0, dialog, loadDocument);
    } else {
        QTimer::singleShot(0, dialog, loadDocument);
        dialog->exec();
        dialog->deleteLater();
    }
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
    btnCancel->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(licence_url, license_title);
    } else if (msgBox.clickedButton() == btnChangelog) {
        auto *changelog = new QDialog;
        changelog->setWindowTitle(QObject::tr("Changelog"));
        changelog->resize(width, height);

        auto *text = new QTextEdit(changelog);
        text->setReadOnly(true);
        QProcess proc;
        proc.start(QStringLiteral("zcat"),
                   {QStringLiteral("/usr/share/doc/") + QFileInfo(QCoreApplication::applicationFilePath()).fileName()
                    + QStringLiteral("/changelog.gz")},
                   QIODevice::ReadOnly);
        proc.waitForFinished();
        text->setText(proc.readAllStandardOutput());

        auto *btnClose = new QPushButton(QObject::tr("&Close"), changelog);
        btnClose->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        QObject::connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
        delete changelog;
    }
}

#include "about.h"

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

QString docPath(const QString &fileName)
{
    const QString installedPath =
        QStringLiteral("/usr/share/doc/") + QFileInfo(QCoreApplication::applicationFilePath()).fileName() + "/"
        + fileName;
    const QString appDirPath = QCoreApplication::applicationDirPath();
    const QStringList candidates {
        QDir(appDirPath).filePath(QStringLiteral("../docs/") + fileName),
        QDir(appDirPath).filePath(QStringLiteral("../../docs/") + fileName),
        QDir::current().filePath(QStringLiteral("docs/") + fileName),
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

void displayAboutMsgBox(QWidget *parent, const QString &title, const QString &message, const QString &licensePath,
                        const QString &licenseTitle)
{
    const auto width = 600;
    const auto height = 500;
    QMessageBox msgBox(QMessageBox::NoIcon, title, message, QMessageBox::NoButton, parent);
    auto *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    auto *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    auto *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));

    msgBox.exec();
    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(parent, licensePath, licenseTitle);
    } else if (msgBox.clickedButton() == btnChangelog) {
        auto *changelog = new QDialog(parent);
        changelog->setWindowTitle(QObject::tr("Changelog"));
        changelog->resize(width, height);

        auto *text = new QTextEdit(changelog);
        text->setReadOnly(true);
        QProcess proc;
        proc.start(
            QStringLiteral("zless"),
            {"/usr/share/doc/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName() + "/changelog.gz"});
        proc.waitForFinished();
        text->setText(QString::fromLatin1(proc.readAllStandardOutput()));

        auto *btnClose = new QPushButton(QObject::tr("&Close"), changelog);
        btnClose->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        QObject::connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
    }
}

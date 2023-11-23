#include "about.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QVBoxLayout>

#include "version.h"
#include <unistd.h>

extern const QString starting_home;

// display doc as nomal user when run as root
void displayDoc(const QString &url, const QString &title)
{
    qputenv("HOME", starting_home.toUtf8());
    // prefer mx-viewer otherwise use xdg-open (use runuser to run that as logname user) "gio open" would also work here
    QString executablePath = QStandardPaths::findExecutable("mx-viewer");
    if (!executablePath.isEmpty()) {
        QProcess::startDetached("mx-viewer", {url, title});
    } else {
        if (getuid() != 0) {
            QProcess::startDetached("xdg-open", {url});
            return;
        } else {
            QProcess proc;
            proc.start("logname", {}, QIODevice::ReadOnly);
            proc.waitForFinished();
            QByteArray user = proc.readAllStandardOutput().trimmed();
            proc.start("id", {"-u", user});
            proc.waitForFinished();
            QByteArray id = proc.readAllStandardOutput().trimmed();

            qunsetenv("KDE_FULL_SESSION");
            qunsetenv("XDG_CURRENT_DESKTOP");
            qputenv("XDG_RUNTIME_DIR", "/run/user/" + id);
            QProcess::startDetached("runuser", {"-u", user, "--", "xdg-open", url});
            qputenv("XDG_RUNTIME_DIR", "/run/user/0");
        }
    }
    if (getuid() == 0) {
        qputenv("HOME", "/root");
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
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

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
        proc.start("zless", {"/usr/share/doc/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName()
                             + "/changelog.gz"});
        proc.waitForFinished();
        text->setText(QString::fromLatin1(proc.readAllStandardOutput()));

        auto *btnClose = new QPushButton(QObject::tr("&Close"), changelog);
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        QObject::connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        auto *layout = new QVBoxLayout(changelog);
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
        delete changelog;
    }
}

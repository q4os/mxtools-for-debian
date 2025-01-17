#include "about.h"
#include "version.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <unistd.h>

// Display doc as nomal user when run as root
void displayDoc(const QString &url, const QString &title, bool runned_as_root)
{
    if (system("command -v mx-viewer >/dev/null") == 0) {
        system("mx-viewer " + url.toUtf8() + " \"" + title.toUtf8() + "\"&");
        return;
    }

    if (system("command -v antix-viewer >/dev/null") == 0) {
        system("antix-viewer " + url.toUtf8() + " \"" + title.toUtf8() + "\"&");
        return;
    }

    if (getuid() != 0) {
        QString cmd = "xdg-open " + url;
        system(cmd.toUtf8());
    } else {
        system("su $(logname) -c \"env XDG_RUNTIME_DIR=/run/user/$(id -u $(logname)) xdg-open " + url.toUtf8() + "\"&");
    }
}

void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title, bool runned_as_root)
{
    QMessageBox msgBox(QMessageBox::NoIcon, title, message);
    QPushButton *btnLicense = msgBox.addButton(QObject::tr("License"), QMessageBox::HelpRole);
    QPushButton *btnChangelog = msgBox.addButton(QObject::tr("Changelog"), QMessageBox::HelpRole);
    QPushButton *btnCancel = msgBox.addButton(QObject::tr("Cancel"), QMessageBox::NoRole);
    btnCancel->setIcon(QIcon::fromTheme("window-close"));

    msgBox.exec();

    if (msgBox.clickedButton() == btnLicense) {
        displayDoc(licence_url, license_title, runned_as_root);
    } else if (msgBox.clickedButton() == btnChangelog) {
        QDialog *changelog = new QDialog();
        changelog->setWindowTitle(QObject::tr("Changelog"));
        changelog->resize(600, 500);

        QTextEdit *text = new QTextEdit;
        text->setReadOnly(true);
        Cmd cmd;
        text->setText(cmd.getOut("zless /usr/share/doc/" + QFileInfo(QCoreApplication::applicationFilePath()).fileName()
                                 + "/changelog.gz"));

        QPushButton *btnClose = new QPushButton(QObject::tr("&Close"));
        btnClose->setIcon(QIcon::fromTheme("window-close"));
        QObject::connect(btnClose, &QPushButton::clicked, changelog, &QDialog::close);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(text);
        layout->addWidget(btnClose);
        changelog->setLayout(layout);
        changelog->exec();
    }
}

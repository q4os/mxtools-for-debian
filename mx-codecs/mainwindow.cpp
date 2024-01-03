/*****************************************************************************
 * mx-codecs.cpp
 *****************************************************************************
 * Copyright (C) 2014 MX Authors
 *
 * Authors: Jerry 3904
 *          Anticaptilista
 *          Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Codecs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Codecs.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QTimer>

#include "about.h"
#include "cmd.h"
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
    , lock_file(QStringLiteral("/var/lib/dpkg/lock"))
    , reply(nullptr)
{
    qDebug().noquote() << QApplication::applicationName() << "version:" << QApplication::applicationVersion();
    ui->setupUi(this);
    connect(ui->buttonAbout, &QPushButton::clicked, this, &MainWindow::buttonAbout_clicked);
    connect(ui->buttonHelp, &QPushButton::clicked, this, &MainWindow::buttonHelp_clicked);
    connect(ui->buttonOk, &QPushButton::clicked, this, &MainWindow::buttonOk_clicked);

    ui->stackedWidget->setCurrentIndex(0);
    // get arch info
    arch = cmd.getCmdOut(QStringLiteral("dpkg --print-architecture"), true);

    setWindowFlags(Qt::Window); // for the close, min and max buttons
    if (ui->buttonOk->icon().isNull())
        ui->buttonOk->setIcon(QIcon(":/icons/dialog-ok.svg"));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::updateStatus(const QString &msg, int val)
{
    ui->labelDownload->setText(msg);
    ui->progressBar->setValue(val);
    QApplication::processEvents();
}

bool MainWindow::isOnline()
{
    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", QApplication::applicationName().toUtf8() + "/"
                                           + QApplication::applicationVersion().toUtf8() + " (linux-gnu)");

    auto error = QNetworkReply::NoError;
    for (const QString &address : {"http://mxrepo.com", "http://google.com"}) {
        error = QNetworkReply::NoError; // reset for each tried address
        QNetworkProxyQuery query {QUrl(address)};
        QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);
        if (!proxies.isEmpty())
            manager.setProxy(proxies.first());
        request.setUrl(QUrl(address));
        reply = manager.head(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                [&error](QNetworkReply::NetworkError err) { error = err; }); // errorOccured only in Qt >= 5.15
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), &loop, &QEventLoop::quit);
        QTimer::singleShot(5s, &loop, [&loop, &error]() {
            error = QNetworkReply::TimeoutError;
            loop.quit();
        }); // manager.setTransferTimeout(time) // only in Qt >= 5.15
        loop.exec();
        reply->disconnect();
        if (error == QNetworkReply::NoError)
            return true;
    }
    qDebug() << "No network detected:" << reply->url() << error;
    return false;
}

void MainWindow::buttonOk_clicked()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        setCursor(QCursor(Qt::WaitCursor));
        if (!isOnline()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Internet is not available, won't be able to download the list of packages"));
            setCursor(QCursor(Qt::ArrowCursor));
            return;
        }
        installDebs(downloadDebs());
    } else {
        QApplication::quit();
    }
}

bool MainWindow::downloadDeb(const QString &url, const QString &filepath)
{
    QFileInfo fi(filepath);
    QFile tofile(fi.fileName());
    if (!(downloadFile(url + "/" + filepath, &tofile))) {
        QMessageBox::critical(this, windowTitle(), QString(tr("Error downloading %1")).arg(fi.fileName()));
        return false;
    }
    return true;
}

bool MainWindow::downloadFile(const QString &url, QFile *file)
{
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open file:" << file->fileName();
        return false;
    }

    reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;

    bool success = true;
    connect(reply, &QNetworkReply::readyRead,
            [this, &file, &success]() { success = file->write(reply->readAll()) > 0; });
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->disconnect();

    if (!success) {
        QMessageBox::warning(
            this, tr("Error"),
            tr("There was an error writing file: %1. Please check if you have enough free space on your drive")
                .arg(file->fileName()));
        exit(EXIT_FAILURE);
    }

    file->close();
    return (reply->error() == QNetworkReply::NoError);
}

// download .deb codecs returns download path
QString MainWindow::downloadDebs()
{
    // set progressBar and refresh
    ui->progressBar->setValue(0);
    ui->stackedWidget->setCurrentIndex(1);
    QApplication::processEvents();

    // create temp folder and set it current
    if (!tempdir.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("Could not create temp directory. "));
        exit(EXIT_FAILURE);
    }
    QString path = tempdir.path();
    QDir::setCurrent(path);

    // get release info
    QString release = cmd.getCmdOut(QStringLiteral("grep VERSION_CODENAME /etc/os-release |grep -Eo [a-z]+ "));
    if ( release == "libretto" ){
        release = "bookworm";
    }
    qDebug() << "release is " << release;
    int idx = 0;
    const int inc = 10;
    QTemporaryFile file;
    QString url = QStringLiteral("http://deb-multimedia.org");
    updateStatus(tr("<b>Running command...</b><p>") + tr("downloading Packages.gz from 'main'"), idx += inc);
    downloadInfoAndPackage(url, release, QStringLiteral("main"), arch, &file,
                           QStringList {"libdvdcss2", "libtxc-dxtn0"}, idx += inc);

    // download and install w32 or w64 codecs on x86 platforms
    if (arch == QLatin1String("amd64") || arch == QLatin1String("i386")) {
        QTemporaryFile file_nonfree;
        updateStatus(tr("<b>Running command...</b><p>") + tr("downloading Packages.gz from 'non-free'"), idx += inc);
        if (!downloadInfoAndPackage(url, "bullseye", QStringLiteral("non-free"), arch, &file_nonfree,
                                    QStringList {"w.*codecs.*deb"}, idx += inc))
            arch_flag = false;
    }

    // Debian fix: do not install 32bit packages on 64bit systems

    updateStatus(tr("<b>Download Finished.</b>"), idx += inc);
    return path;
}

bool MainWindow::downloadInfoAndPackage(const QString &url, const QString &release, const QString &repo,
                                        const QString &arch, QFile *file, const QStringList &search_terms, int progress)
{
    if (!downloadFile(url + "/dists/" + release + "/" + repo + "/binary-" + arch + "/Packages.gz", file)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot connect to the download site"));
        return false;
    }

    const int step = 10;
    for (const QString &search_deb : search_terms) {
        QString out = cmd.getCmdOut("zgrep ^Filename " + file->fileName() + " |grep " + search_deb
                                    + " |cut -d' ' -f2 |head -n1");
        if (out.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("Cannot connect find %1 package").arg(search_deb));
            return false;
        } else {
            updateStatus(tr("<b>Running command...</b><p>") + "downloading: " + out, progress);
            downloadDeb(url, out);
        }
        progress += step;
    }
    return true;
}

void MainWindow::installDebs(const QString &path)
{
    QDir::setCurrent(path);
    QStringList filter;
    filter << QStringLiteral("*.deb");
    QDir dir;
    dir.setNameFilters(filter);

    QStringList fileList = dir.entryList();
    ui->groupBox->setTitle(tr("Installing downloaded files"));

    if (fileList.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("No downloaded *.debs files found."));
        QApplication::exit(EXIT_FAILURE);
    }

    qDebug() << "filelist " << fileList;

    lock_file.unlock();
    if (arch_flag)
        cmd.run("dpkg --remove libtxc-dxtn-s2tc:" + arch);

    if (i386_flag)
        cmd.run(QStringLiteral("dpkg --remove libtxc-dxtn-s2tc:i386"));

    int idx = ui->progressBar->value();
    int inc = (ui->progressBar->maximum() - idx) / fileList.size();

    bool error = false;
    for (const QString &file : fileList) {
        updateStatus(tr("<b>Installing...</b><p>") + file, idx += inc);
        if (!cmd.run("dpkg -i " + file)) {
            QMessageBox::critical(this, windowTitle(), QString(tr("Error installing %1")).arg(file));
            error = true;
        }
    }

    updateStatus("<b>" + tr("Fix missing dependencies...") + "</b><p>", 99);
    if (!cmd.run(QStringLiteral("apt-get -f install"))) {
        QMessageBox::critical(this, windowTitle(),
                              (tr("Error running %1 command").arg(QStringLiteral("'apt-get -f install'"))));
        error = true;
    }

    ui->groupBox->setTitle(QString());
    updateStatus(tr("<b>Installation process has finished</b>"), ui->progressBar->maximum());

    setCursor(QCursor(Qt::ArrowCursor));
    if (!error) {
        QMessageBox::information(this, tr("Finished"),
                                 tr("Codecs files have been downloaded and installed successfully."));
        QApplication::exit(EXIT_SUCCESS);
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Process finished. Errors have occurred during the installation."));
        QApplication::exit(EXIT_FAILURE);
    }
}

void MainWindow::buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About MX Codecs"),
        "<p align=\"center\"><b><h2>" + this->windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>"
            + tr("Simple codecs downloader for MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("file:///usr/share/doc/mx-codecs/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void MainWindow::buttonHelp_clicked()
{
    QLocale locale;
    QString lang = locale.bcp47Name();
    QString url = QStringLiteral("file:///usr/share/doc/mx-codecs/mx-codecs.html");
    if (lang.startsWith(QLatin1String("fr")))
        url = QStringLiteral("https://mxlinux.org/french-wiki/help-files/help-mx-codecs-installer");
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

/*****************************************************************************
 * downloadwidget.cpp
 *****************************************************************************
 * Copyright (C) 2022-2026 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MX Viewer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MX Viewer.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "downloadwidget.h"
#include "ui_downloadwidget.h"

#include <algorithm>

DownloadWidget::DownloadWidget(QWidget* browserWindow)
    : browserWindow(browserWindow),
      ui(new Ui::DownloadWidget)
{
    ui->setupUi(this);
}

DownloadWidget::~DownloadWidget()
{
    // The profile is shared with other windows and outlives this one, so stop this window's
    // downloads here rather than leave them running with nothing showing them.
    for (const auto& download : std::as_const(downloads)) {
        if (download && download->state() == QWebEngineDownloadRequest::DownloadInProgress) {
            download->cancel();
        }
    }
    delete ui;
}

void DownloadWidget::downloadRequested(QWebEngineDownloadRequest* download, QWebEngineProfile* profile)
{
    QWidget* dialogParent = browserWindow ? browserWindow.data() : this;
    QString path = QFileDialog::getSaveFileName(
        dialogParent, tr("Save as"), QDir(download->downloadDirectory()).filePath(download->downloadFileName()));
    if (path.isEmpty()) {
        return;
    }
    download->setDownloadDirectory(QFileInfo(path).path());
    profile->setDownloadPath(download->downloadDirectory());
    download->setDownloadFileName(QFileInfo(path).fileName());
    auto* downloadLabel = new QLabel;
    auto* pushButton = new QPushButton(QIcon::fromTheme("cancel"), tr("cancel"));
    auto* progressBar = new QProgressBar(this);
    // Offered once the file is complete.
    auto* finishedActions = new QWidget;
    auto* actionsLayout = new QHBoxLayout(finishedActions);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    auto* openButton = new QPushButton(QIcon::fromTheme("document-open"), tr("Open"), finishedActions);
    openButton->setToolTip(tr("Open the file"));
    auto* folderButton = new QPushButton(QIcon::fromTheme("folder-open"), tr("Show in folder"), finishedActions);
    folderButton->setToolTip(tr("Open the folder that contains the file"));
    actionsLayout->addWidget(openButton);
    actionsLayout->addWidget(folderButton);
    finishedActions->hide();
    downloadLabel->setText(download->downloadFileName());
    int row = ui->gridLayout->rowCount();
    ui->gridLayout->removeItem(ui->verticalSpacer);
    ui->gridLayout->addWidget(downloadLabel, row, 0);
    ui->gridLayout->addWidget(progressBar, row, 1);
    ui->gridLayout->addWidget(finishedActions, row, 2);
    ui->gridLayout->addWidget(pushButton, row, 3);
    ui->gridLayout->addItem(ui->verticalSpacer, row + 1, 1);

    progressBar->setProperty("startTime", QDateTime::currentDateTime());
    if (!isVisible()) {
        restoreGeometry(settings.value("DownloadGeometry").toByteArray());
        show();
    }
    raise();
    download->accept();
    downloads.append(download);

    const QString filePath = QDir(download->downloadDirectory()).filePath(download->downloadFileName());
    connect(openButton, &QPushButton::clicked, this,
            [filePath] { QDesktopServices::openUrl(QUrl::fromLocalFile(filePath)); });
    connect(folderButton, &QPushButton::clicked, this,
            [filePath] { QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).path())); });

    connect(pushButton, &QPushButton::pressed, this,
            [this, download, pushButton, downloadLabel, progressBar, finishedActions] {
                if (download->state() == QWebEngineDownloadRequest::DownloadInProgress) {
                    download->cancel();
                } else {
                    downloads.removeAll(download);
                    ui->gridLayout->removeWidget(downloadLabel);
                    ui->gridLayout->removeWidget(pushButton);
                    ui->gridLayout->removeWidget(progressBar);
                    ui->gridLayout->removeWidget(finishedActions);
                    downloadLabel->deleteLater();
                    pushButton->deleteLater();
                    progressBar->deleteLater();
                    finishedActions->deleteLater();
                }
            });

    connect(download, &QWebEngineDownloadRequest::receivedBytesChanged, this,
            [download, pushButton, progressBar, finishedActions] {
                updateDownload(download, pushButton, progressBar, finishedActions);
            });
    connect(download, &QWebEngineDownloadRequest::stateChanged, this,
            [download, pushButton, progressBar, finishedActions] {
                updateDownload(download, pushButton, progressBar, finishedActions);
            });
}

int DownloadWidget::activeDownloadCount() const
{
    return static_cast<int>(std::count_if(downloads.cbegin(), downloads.cend(), [](const auto& download) {
        return download && download->state() == QWebEngineDownloadRequest::DownloadInProgress;
    }));
}

QString DownloadWidget::withUnit(qreal bytes)
{
    if (bytes < (1 << 10)) {
        return tr("%L1 B").arg(bytes);
    } else if (bytes < (1 << 20)) {
        return tr("%L1 KiB").arg(bytes / (1 << 10), 0, 'f', 2);
    } else if (bytes < (1 << 30)) {
        return tr("%L1 MiB").arg(bytes / (1 << 20), 0, 'f', 2);
    } else {
        return tr("%L1 GiB").arg(bytes / (1 << 30), 0, 'f', 2);
    }
}

QString DownloadWidget::timeUnit(int seconds)
{
    if (seconds < 60) {
        return tr("%1sec.").arg(seconds);
    } else if (seconds < 3600) {
        return tr("%1min. %2sec.").arg(seconds / 60).arg(seconds % 60);
    } else {
        return tr("%1h. %2m. %3s.").arg(seconds / 3600).arg((seconds % 3600) / 60).arg(seconds % 60);
    }
}

void DownloadWidget::updateDownload(QWebEngineDownloadRequest* download, QPushButton* pushButton,
                                     QProgressBar* progressBar, QWidget* finishedActions)
{
    auto totalBytes = static_cast<qreal>(download->totalBytes());
    auto receivedBytes = static_cast<qreal>(download->receivedBytes());
    auto startTime = progressBar->property("startTime").toDateTime();
    auto elapsed = startTime.msecsTo(QDateTime::currentDateTime());
    auto bytesPerSecond = elapsed > 0 ? receivedBytes / static_cast<qreal>(elapsed) * 1000 : 0;

    auto state = download->state();
    switch (state) {
    case QWebEngineDownloadRequest::DownloadRequested:
        Q_UNREACHABLE();
        break;
    case QWebEngineDownloadRequest::DownloadInProgress:
        if (totalBytes > 0) {
            progressBar->setValue(static_cast<int>(100 * receivedBytes / totalBytes));
            progressBar->setDisabled(false);
            progressBar->setFormat(tr("%p% - %1 of %2 at %3/s - %4 left")
                                       .arg(withUnit(receivedBytes), withUnit(totalBytes), withUnit(bytesPerSecond),
                                            bytesPerSecond > 0 ? timeUnit(static_cast<int>((totalBytes - receivedBytes) / bytesPerSecond)) : tr("unknown")));
        } else {
            progressBar->setValue(0);
            progressBar->setDisabled(false);
            progressBar->setFormat(
                tr("unknown size - %1 at %2/s").arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
        }
        break;
    case QWebEngineDownloadRequest::DownloadCompleted:
        progressBar->setValue(progressBar->maximum());
        progressBar->setDisabled(true);
        progressBar->setFormat(tr("completed - %1 at %2/s").arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
        break;
    case QWebEngineDownloadRequest::DownloadCancelled:
        progressBar->setValue(0);
        progressBar->setDisabled(true);
        progressBar->setFormat(tr("cancelled"));
        break;
    case QWebEngineDownloadRequest::DownloadInterrupted:
        progressBar->setValue(0);
        progressBar->setDisabled(true);
        progressBar->setFormat(tr("interrupted: %1").arg(download->interruptReasonString()));
        break;
    }

    finishedActions->setVisible(state == QWebEngineDownloadRequest::DownloadCompleted);
    if (state == QWebEngineDownloadRequest::DownloadInProgress) {
        pushButton->setIcon(QIcon::fromTheme("process-stop"));
        pushButton->setText(tr("Cancel"));
        pushButton->setToolTip(tr("Cancel downloading"));
    } else {
        pushButton->setIcon(QIcon::fromTheme("edit-clear"));
        pushButton->setText(tr("Clear"));
        pushButton->setToolTip(tr("Remove from list"));
    }
}

void DownloadWidget::closeEvent(QCloseEvent* event)
{
    event->accept();
    settings.setValue("DownloadGeometry", saveGeometry());
}

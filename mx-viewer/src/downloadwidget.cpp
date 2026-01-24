/*****************************************************************************
 * downloadwidget.cpp
 *****************************************************************************
 * Copyright (C) 2022 MX Authors
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

DownloadWidget::DownloadWidget(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::DownloadWidget)
{
    ui->setupUi(this);
}

DownloadWidget::~DownloadWidget()
{
    delete ui;
}

void DownloadWidget::downloadRequested(QWebEngineDownloadRequest* download)
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save as"), QDir(download->downloadDirectory()).filePath(download->downloadFileName()));
    if (path.isEmpty()) {
        return;
    }
    download->setDownloadDirectory(QFileInfo(path).path());
    QWebEngineProfile::defaultProfile()->setDownloadPath(download->downloadDirectory());
    download->setDownloadFileName(QFileInfo(path).fileName());
    auto* downloadLabel = new QLabel;
    auto* pushButton = new QPushButton(QIcon::fromTheme("cancel"), tr("cancel"));
    auto* progressBar = new QProgressBar(this);
    downloadLabel->setText(download->downloadFileName());
    int row = ui->gridLayout->rowCount();
    ui->gridLayout->removeItem(ui->verticalSpacer);
    ui->gridLayout->addWidget(downloadLabel, row, 0);
    ui->gridLayout->addWidget(progressBar, row, 1);
    ui->gridLayout->addWidget(pushButton, row, 3);
    ui->gridLayout->addItem(ui->verticalSpacer, row + 1, 1);

    progressBar->setProperty("startTime", QDateTime::currentDateTime());
    if (!isVisible()) {
        restoreGeometry(settings.value("DownloadGeometry").toByteArray());
        show();
    }
    raise();
    download->accept();

    connect(pushButton, &QPushButton::pressed, this, [this, download, pushButton, downloadLabel, progressBar] {
        if (download->state() == QWebEngineDownloadRequest::DownloadInProgress) {
            download->cancel();
        } else {
            ui->gridLayout->removeWidget(downloadLabel);
            ui->gridLayout->removeWidget(pushButton);
            ui->gridLayout->removeWidget(progressBar);
            downloadLabel->deleteLater();
            pushButton->deleteLater();
            progressBar->deleteLater();
        }
    });

    connect(download, &QWebEngineDownloadRequest::receivedBytesChanged, this,
            [download, pushButton, progressBar] { updateDownload(download, pushButton, progressBar); });
    connect(download, &QWebEngineDownloadRequest::stateChanged, this,
            [download, pushButton, progressBar] { updateDownload(download, pushButton, progressBar); });
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
                                     QProgressBar* progressBar)
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

    if (state == QWebEngineDownloadRequest::DownloadInProgress) {
        pushButton->setIcon(QIcon::fromTheme("process-stop"));
        pushButton->setText(tr("Cancel"));
        pushButton->setToolTip(tr("Cancel downloading"));
    } else {
        pushButton->setIcon(QIcon::fromTheme("edit-clear"));
        pushButton->setText(tr("Remove"));
        pushButton->setToolTip(tr("Remove from list"));
    }
}

void DownloadWidget::closeEvent(QCloseEvent* event)
{
    event->accept();
    settings.setValue("DownloadGeometry", saveGeometry());
}

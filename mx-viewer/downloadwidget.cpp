/*****************************************************************************
 * downloadwidget.cpp
 *****************************************************************************
 * Copyright (C) 2022 MX Authors
 *
 * Authors: Adrian
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

DownloadWidget::DownloadWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DownloadWidget)
{
    ui->setupUi(this);
}

DownloadWidget::~DownloadWidget()
{
    delete ui;
}

void DownloadWidget::downloadRequested(QWebEngineDownloadItem* download)
{
    timerDownload.start();
    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), QDir(download->downloadDirectory()).filePath(download->downloadFileName()));
    if (path.isEmpty())
        return;
    download->setDownloadDirectory(QFileInfo(path).path());
    QWebEngineProfile::defaultProfile()->setDownloadPath(download->downloadDirectory());
    download->setDownloadFileName(QFileInfo(path).fileName());
    auto* downloadLabel = new QLabel;
    auto* pushButton = new QPushButton(QIcon::fromTheme(QStringLiteral("cancel")), tr("cancel"));
    auto* prog = new QProgressBar(this);
    downloadLabel->setText(download->downloadFileName());
    int row = ui->gridLayout->rowCount();
    ui->gridLayout->removeItem(ui->verticalSpacer);
    ui->gridLayout->addWidget(downloadLabel, row, 0);
    ui->gridLayout->addWidget(prog, row, 1);
    ui->gridLayout->addWidget(pushButton, row, 3);
    ui->gridLayout->addItem(ui->verticalSpacer, row + 1, 1);

    if (!this->isVisible()) {
        this->restoreGeometry(settings.value(QStringLiteral("DownloadGeometry")).toByteArray());
        this->show();
    }
    this->raise();
    download->accept();

    connect(pushButton, &QPushButton::pressed, [this, download, pushButton, downloadLabel, prog]() {
        if (download->state() == QWebEngineDownloadItem::DownloadInProgress) {
            download->cancel();
        } else {
            ui->gridLayout->removeWidget(downloadLabel);
            ui->gridLayout->removeWidget(pushButton);
            ui->gridLayout->removeWidget(prog);
            downloadLabel->deleteLater();
            pushButton->deleteLater();
            prog->deleteLater();
        }
    });

    connect(download, &QWebEngineDownloadItem::downloadProgress, [download, pushButton, prog]() { updateDownload(download, pushButton, prog); });
    connect(download, &QWebEngineDownloadItem::stateChanged, [download, pushButton, prog]() { updateDownload(download, pushButton, prog); });
}

inline QString DownloadWidget::withUnit(qreal bytes)
{
    if (bytes < (1 << 10))
        return tr("%L1 B").arg(bytes);
    else if (bytes < (1 << 20))
        return tr("%L1 KiB").arg(bytes / (1 << 10), 0, 'f', 2);
    else if (bytes < (1 << 30))
        return tr("%L1 MiB").arg(bytes / (1 << 20), 0, 'f', 2);
    else
        return tr("%L1 GiB").arg(bytes / (1 << 30), 0, 'f', 2);
}

inline QString DownloadWidget::timeUnit(int seconds)
{
    if (seconds < 60)
        return tr("%1sec.").arg(seconds);
    else if (seconds < 3600)
        return tr("%1min. %2sec.").arg(seconds / 60).arg(seconds % 60);
    else
        return tr("%1h. %2m. %3s.").arg(seconds / 3600, seconds % 3600, seconds % 3600 % 60);
}

void DownloadWidget::updateDownload(QWebEngineDownloadItem* download, QPushButton* pushButton, QProgressBar* prog)
{
    qreal totalBytes = download->totalBytes();
    qreal receivedBytes = download->receivedBytes();
    qreal bytesPerSecond = receivedBytes / DownloadWidget::timerDownload.elapsed() * 1000;

    auto state = download->state();
    switch (state) {
    case QWebEngineDownloadItem::DownloadRequested:
        Q_UNREACHABLE();
        break;
    case QWebEngineDownloadItem::DownloadInProgress:
        if (totalBytes >= 0) {
            prog->setValue(qRound(100 * receivedBytes / totalBytes));
            prog->setDisabled(false);
            prog->setFormat(tr("%p% - %1 of %2 at %3/s - %4 left")
                                .arg(withUnit(receivedBytes), withUnit(totalBytes), withUnit(bytesPerSecond),
                                    timeUnit(qRound((totalBytes - receivedBytes) / bytesPerSecond))));
        } else {
            prog->setValue(0);
            prog->setDisabled(false);
            prog->setFormat(tr("unknown size - %1 at %2/s")
                                .arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
        }
        break;
    case QWebEngineDownloadItem::DownloadCompleted:
        prog->setValue(prog->maximum());
        prog->setDisabled(true);
        prog->setFormat(tr("completed - %1 at %2/s")
                            .arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
        break;
    case QWebEngineDownloadItem::DownloadCancelled:
        prog->setValue(0);
        prog->setDisabled(true);
        prog->setFormat(tr("cancelled"));
        break;
    case QWebEngineDownloadItem::DownloadInterrupted:
        prog->setValue(0);
        prog->setDisabled(true);
        prog->setFormat(tr("interrupted: %1").arg(download->interruptReasonString()));
        break;
    }

    if (state == QWebEngineDownloadItem::DownloadInProgress) {
        pushButton->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
        pushButton->setText(tr("Cancel"));
        pushButton->setToolTip(tr("Cancel downloading"));
    } else {
        pushButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
        pushButton->setText(tr("Remove"));
        pushButton->setToolTip(tr("Remove from list"));
    }
}

void DownloadWidget::closeEvent(QCloseEvent* event)
{
    event->accept();
    settings.setValue(QStringLiteral("DownloadGeometry"), saveGeometry());
}

/*****************************************************************************
 * downloadwidget.h
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
#pragma once

#include <QtWebEngineWidgets>

namespace Ui
{
class DownloadWidget;
}

class DownloadWidget : public QWidget
{
    Q_OBJECT

public:
    // Stays a top-level window; browserWindow parents the save dialog so it opens over the browser.
    explicit DownloadWidget(QWidget* browserWindow);
    ~DownloadWidget() override;

    // Downloads from this window that are still transferring; they stop when the window closes.
    [[nodiscard]] int activeDownloadCount() const;
    static QString withUnit(qreal bytes);
    static QString timeUnit(int seconds);
    // The profile remembers the chosen folder for the next download.
    void downloadRequested(QWebEngineDownloadRequest* download, QWebEngineProfile* profile);
    static void updateDownload(QWebEngineDownloadRequest* download, QPushButton* pushButton, QProgressBar* progressBar,
                               QWidget* finishedActions);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QSettings settings;
    QPointer<QWidget> browserWindow;
    Ui::DownloadWidget* ui;
    QList<QPointer<QWebEngineDownloadRequest>> downloads;
};

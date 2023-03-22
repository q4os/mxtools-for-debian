/*****************************************************************************
 * downloadwidget.h
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

#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QtWebEngineWidgets>

namespace Ui {
class DownloadWidget;
}

class DownloadWidget : public QWidget {
    Q_OBJECT
public:
    explicit DownloadWidget(QWidget* parent = nullptr);
    ~DownloadWidget();
    Ui::DownloadWidget *ui;

    inline static QString withUnit(qreal bytes);
    inline static QString timeUnit(int sec);
    void downloadRequested(QWebEngineDownloadItem* download);
    static void updateDownload(QWebEngineDownloadItem* download, QPushButton* pushButton, QProgressBar* prog);

protected:
    inline static QElapsedTimer timerDownload;
    void closeEvent(QCloseEvent* event) override;

private:
    QSettings settings;
};

#endif // DOWNLOADWIDGET_H

/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2023 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#pragma once

#include <QListWidgetItem>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

#include "cmd.h"
#include "service.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void centerWindow();

private slots:
    void cmdDone();
    void cmdStart();
    void itemUpdated();
    void onSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void pushAbout_clicked();
    void pushEnableDisable_clicked();
    void pushHelp_clicked();
    void pushStartStop_clicked();
    void setGeneralConnections();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    Cmd cmd;
    QStringList dependTargets {};
    QColor defaultForeground;
    QColor runningColor {Qt::darkGreen};
    QColor enabledColor {Qt::darkYellow};
    QList<QSharedPointer<Service>> services;
    int savedRow = 0;

    static QString getHtmlColor(const QColor &color);
    void displayServices();
    void listServices();
};

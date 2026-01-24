/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2023-2025 MX Authors
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

#include <QFutureWatcher>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPersistentModelIndex>
#include <QProcess>
#include <QSettings>
#include <QSet>

#include <optional>

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
    void pushRefresh_clicked();
    void pushStartStop_clicked();
    void setGeneralConnections() noexcept;

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
    QTimer *searchTimer = nullptr;

    // Tooltip management
    QTimer *tooltipTimer = nullptr;
    QFutureWatcher<QString> *tooltipWatcher = nullptr;
    QPersistentModelIndex pendingTooltipIndex;
    QPersistentModelIndex activeTooltipIndex;
    Service *activeTooltipService = nullptr;
    bool tooltipInProgress = false;

    void cancelPendingTooltip();
    void fetchTooltipDescription();
    [[nodiscard]] std::optional<QString> sanitizeServiceName(const QString &rawName);
    QSet<QString> loadSystemdEnabledServices(bool isUserService);
    QString decodeEscapeSequences(const QString &input);
    QString getHtmlColor(const QColor &color) noexcept;
    void displayServices() noexcept;
    void listServices();
    void processNonSystemdServices();
    void processSystemdActiveInactiveServices(QStringList &names,
                                              const QSet<QString> &enabledServices,
                                              bool isUserService = false);
    void processSystemdMaskedServices(QStringList &names, bool isUserService = false);
    void processSystemdServices();
};

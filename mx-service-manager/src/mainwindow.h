/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
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

#include <functional>
#include <optional>

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

    // Background service-list loading
    QFutureWatcher<QList<QSharedPointer<Service>>> *servicesWatcher = nullptr;
    std::function<void()> onServicesLoaded;

    // Selected-service detail fetching (async, mirrors the tooltip mechanism)
    QTimer *selectionInfoTimer = nullptr;
    QFutureWatcher<QString> *selectionInfoWatcher = nullptr;
    QPersistentModelIndex pendingSelectionIndex;
    QPersistentModelIndex activeSelectionIndex;
    Service *activeSelectionService = nullptr;
    bool selectionInfoInProgress = false;

    void cancelPendingTooltip();
    void cancelPendingSelectionInfo();
    [[nodiscard]] QString docPath(const QString &fileName) const;
    [[nodiscard]] QSharedPointer<Service> findServiceShared(Service *raw) const;
    void fetchTooltipDescription();
    void fetchSelectionInfo();
    void loadServicesAsync(std::function<void()> onLoaded);
    [[nodiscard]] static std::optional<QString> sanitizeServiceName(const QString &rawName);
    [[nodiscard]] static QString systemctlCmd(const QString &baseCmd, bool isUserService);
    static void loadSystemdUnitFileStates(bool isUserService, QSet<QString> &enabledNames, QStringList &maskedNames);
    static QString decodeEscapeSequences(const QString &input);
    QString getHtmlColor(const QColor &color) noexcept;
    void displayServices() noexcept;
    [[nodiscard]] static QList<QSharedPointer<Service>> buildServiceList(const QStringList &dependTargets);
    static void processNonSystemdServices(QList<QSharedPointer<Service>> &services, const QStringList &dependTargets);
    static void processSystemdActiveInactiveServices(QList<QSharedPointer<Service>> &services,
                                              QStringList &names,
                                              const QSet<QString> &enabledServices,
                                              const QStringList &dependTargets,
                                              bool isUserService = false);
    static void appendMaskedServices(QList<QSharedPointer<Service>> &services, QStringList &names,
                                    const QStringList &maskedNames, bool isUserService = false);
    static void processSystemdServices(QList<QSharedPointer<Service>> &services, const QStringList &dependTargets);
};

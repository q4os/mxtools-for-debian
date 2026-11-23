/**********************************************************************
 *  mainwindow.cpp
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
#define QT_USE_QSTRINGBUILDER
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QScreen>
#include <QScrollBar>
#include <QShortcut>
#include <QTimer>
#include <QtConcurrent>

#include "about.h"
#include "cmd.h"
#include "common.h"
#include "service.h"
#include "serviceunit.h"

#include <chrono>

using namespace std::chrono_literals;

namespace {
constexpr int kTooltipFetchedRole = Qt::UserRole + 1;
}

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // Enable close, minimize, and maximize buttons
    setGeneralConnections();

    const auto size = this->size();
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
        if (isMaximized()) { // Add option to resize if maximized
            resize(size);
            centerWindow();
        }
    }
    if (initSystem != QLatin1String("systemd")
        && !initSystem.startsWith(QLatin1String("init"))) { // Can be "init(mxlinux)" when running in WSL for example
        QMessageBox::warning(
            this, tr("Error"),
            tr("Could not determine the init system. This program is supposed to run either with systemd or sysvinit")
                % "\nINIT:" % initSystem);
    }
    QPalette palette = ui->listServices->palette();
    defaultForeground = palette.color(QPalette::Text);
    ui->listServices->addItem(tr("Loading..."));

    ui->labelCount->setText(tr("Loading..."));
    auto loadingTimer = new QTimer(this);
    connect(loadingTimer, &QTimer::timeout, this, [this]() {
        static bool toggle = false;
        ui->labelCount->setText(toggle ? tr("Loading...") : QString());
        toggle = !toggle;
    });
    loadingTimer->start(300);

    // Load services on a background thread to keep the UI responsive
    loadServicesAsync([this, loadingTimer] {
        loadingTimer->stop();
        loadingTimer->deleteLater();
        displayServices();
        ui->listServices->setFocus();
    });
    connect(ui->listServices, &QListWidget::itemEntered, this, [this](QListWidgetItem *item) {
        // Cancel any pending tooltip
        cancelPendingTooltip();

        if (item->data(Qt::UserRole).value<Service *>()) {
            if (item->toolTip().isEmpty() && !item->data(kTooltipFetchedRole).toBool()) {
                pendingTooltipIndex = ui->listServices->indexFromItem(item);
                if (!pendingTooltipIndex.isValid()) {
                    return;
                }

                // Start tooltip timer with delay
                if (!tooltipTimer) {
                    tooltipTimer = new QTimer(this);
                    tooltipTimer->setSingleShot(true);
                    connect(tooltipTimer, &QTimer::timeout, this, &MainWindow::fetchTooltipDescription);
                }
                tooltipTimer->start(300); // 300ms delay
            }
        }
    });

    auto *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, [this]() {
        ui->lineSearch->setFocus(Qt::ShortcutFocusReason);
        ui->lineSearch->selectAll();
    });
}

MainWindow::~MainWindow()
{
    settings.setValue("geometry", saveGeometry());
    if (tooltipWatcher) {
        tooltipWatcher->cancel();
        tooltipWatcher->waitForFinished();
    }
    if (selectionInfoWatcher) {
        selectionInfoWatcher->cancel();
        selectionInfoWatcher->waitForFinished();
    }
    if (servicesWatcher) {
        servicesWatcher->waitForFinished();
    }
    delete ui;
}

void MainWindow::centerWindow()
{
    const auto screenGeometry = QApplication::primaryScreen()->geometry();
    const int x = (screenGeometry.width() - width()) / 2;
    const int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

QString MainWindow::docPath(const QString &fileName) const
{
    const QString appDirPath = QCoreApplication::applicationDirPath();
    const QString installedPath = QDir(appDirPath).filePath(QStringLiteral(APP_DOC_DIR "/") + fileName);
    const QStringList candidates {
        QDir(appDirPath).filePath(QStringLiteral("../docs/") + fileName),
        QDir(appDirPath).filePath(QStringLiteral("../../docs/") + fileName),
        QDir::current().filePath(QStringLiteral("docs/") + fileName),
        installedPath,
        QStringLiteral("/usr/share/doc/mx-service-manager/") + fileName,
    };

    for (const QString &candidate : candidates) {
        const QString normalized = QFileInfo(candidate).canonicalFilePath();
        if (!normalized.isEmpty() && QFileInfo::exists(normalized)) {
            return normalized;
        }
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return installedPath;
}

void MainWindow::cmdStart()
{
    setCursor(QCursor(Qt::BusyCursor));
}

void MainWindow::itemUpdated()
{
    blockSignals(true);
    displayServices();
    blockSignals(false);
}

void MainWindow::onSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    cancelPendingSelectionInfo();

    updateServiceControls();
    const auto *service = current ? current->data(Qt::UserRole).value<Service *>() : nullptr;
    if (!service) {
        ui->textBrowser->clear();
        ui->pushEnableDisable->setToolTip({});
        return;
    }

    ui->textBrowser->setText(tr("Loading..."));
    pendingSelectionIndex = ui->listServices->indexFromItem(current);
    if (pendingSelectionIndex.isValid()) {
        if (!selectionInfoTimer) {
            selectionInfoTimer = new QTimer(this);
            selectionInfoTimer->setSingleShot(true);
            connect(selectionInfoTimer, &QTimer::timeout, this, &MainWindow::fetchSelectionInfo);
        }
        // Short debounce so holding an arrow key or dragging the scrollbar doesn't spawn
        // a (possibly root-elevated) subprocess for every item passed through.
        selectionInfoTimer->start(150);
    }

    bool running = service->isRunning();
    bool enabled = service->isEnabled();
    if (!service->hasKnownRunState()) {
        ui->pushStartStop->setText(tr("Status unknown"));
        ui->pushStartStop->setIcon({});
    } else if (running) {
        ui->pushStartStop->setText(tr("&Stop"));
        ui->pushStartStop->setIcon(QIcon::fromTheme("stop"));
        current->setForeground(runningColor);
    } else {
        ui->pushStartStop->setIcon(QIcon::fromTheme("start"));
        ui->pushStartStop->setText(tr("S&tart"));
        if (!enabled) {
            current->setForeground(defaultForeground);
        }
    }
    ui->pushEnableDisable->setToolTip(enabled && initSystem == "systemd"
        ? tr("Disabling also masks this service. Manual and dependency starts are blocked until it is unmasked.")
        : QString());
    if (enabled) {
        ui->pushEnableDisable->setText(tr("&Disable at boot"));
        ui->pushEnableDisable->setIcon(QIcon::fromTheme("stop"));
        if (!running && service->hasKnownRunState()) {
            current->setForeground(enabledColor);
        }
    } else {
        ui->pushEnableDisable->setIcon(QIcon::fromTheme("start"));
        ui->pushEnableDisable->setText(tr("&Enable at boot"));
    }
}

void MainWindow::cmdDone()
{
    setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::setGeneralConnections() noexcept
{
    connect(ui->comboFilter, &QComboBox::currentTextChanged, this, &MainWindow::displayServices);
    connect(ui->lineSearch, &QLineEdit::textChanged, this, [this]() {
        // Debounce search to prevent excessive updates
        if (!searchTimer) {
            searchTimer = new QTimer(this);
            searchTimer->setSingleShot(true);
            connect(searchTimer, &QTimer::timeout, this, &MainWindow::displayServices);
        }
        searchTimer->start(150); // 150ms delay
    });
    connect(ui->listServices, &QListWidget::currentItemChanged, this, &MainWindow::onSelectionChanged);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushCancel, &QPushButton::pressed, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushEnableDisable, &QPushButton::clicked, this, &MainWindow::pushEnableDisable_clicked);
    connect(ui->pushRefresh, &QPushButton::clicked, this, &MainWindow::pushRefresh_clicked);
    connect(ui->pushStartStop, &QPushButton::clicked, this, &MainWindow::pushStartStop_clicked);
}

void MainWindow::loadServicesAsync(std::function<void()> onLoaded)
{
    if (servicesWatcher && servicesWatcher->isRunning()) {
        // A load is already in flight (e.g. a stray Refresh click); ignore this request
        // rather than orphaning the caller whose completion callback would otherwise be lost.
        return;
    }

    serviceListLoading = true;
    updateServiceControls();
    onServicesLoaded = std::move(onLoaded);

    if (!servicesWatcher) {
        servicesWatcher = new QFutureWatcher<QList<QSharedPointer<Service>>>(this);
        connect(servicesWatcher, &QFutureWatcher<QList<QSharedPointer<Service>>>::finished, this, [this]() {
            services = servicesWatcher->result();
            serviceListLoading = false;
            if (onServicesLoaded) {
                onServicesLoaded();
            }
            updateServiceControls();
        });
    }

    servicesWatcher->setFuture(QtConcurrent::run(&MainWindow::buildServiceList));
}

QSharedPointer<Service> MainWindow::findServiceShared(Service *raw) const
{
    for (const auto &svc : services) {
        if (svc.get() == raw) {
            return svc;
        }
    }
    return {};
}

void MainWindow::cancelPendingTooltip()
{
    if (tooltipTimer) {
        tooltipTimer->stop();
    }
    pendingTooltipIndex = QPersistentModelIndex();
}

void MainWindow::fetchTooltipDescription()
{
    if (!pendingTooltipIndex.isValid() || tooltipInProgress) {
        return;
    }

    QListWidgetItem *item = ui->listServices->itemFromIndex(pendingTooltipIndex);
    if (!item) {
        pendingTooltipIndex = QPersistentModelIndex();
        return;
    }

    auto *service = item->data(Qt::UserRole).value<Service *>();
    if (!service) {
        pendingTooltipIndex = QPersistentModelIndex();
        return;
    }

    // Keep the Service alive for the duration of the background fetch: a refresh
    // (listServices()) can clear the `services` list and drop the last QSharedPointer
    // reference to this Service while the background thread is still using it.
    const QSharedPointer<Service> serviceShared = findServiceShared(service);
    if (!serviceShared) {
        pendingTooltipIndex = QPersistentModelIndex();
        return;
    }

    tooltipInProgress = true;
    activeTooltipIndex = pendingTooltipIndex;
    pendingTooltipIndex = QPersistentModelIndex();
    activeTooltipService = service;

    if (!tooltipWatcher) {
        tooltipWatcher = new QFutureWatcher<QString>(this);
        connect(tooltipWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
            const QString description = tooltipWatcher->result();
            QListWidgetItem *activeItem = ui->listServices->itemFromIndex(activeTooltipIndex);
            const bool serviceValid = activeTooltipService && findServiceShared(activeTooltipService);

            if (activeItem && activeItem->listWidget() && activeTooltipIndex.isValid() && serviceValid) {
                if (!description.isEmpty()) {
                    activeItem->setToolTip(description);
                }
                activeItem->setData(kTooltipFetchedRole, !description.isNull());
            }

            tooltipInProgress = false;
            activeTooltipIndex = QPersistentModelIndex();
            activeTooltipService = nullptr;
            if (pendingTooltipIndex.isValid()) {
                fetchTooltipDescription();
            }
        });
    }

    tooltipWatcher->setFuture(QtConcurrent::run([serviceShared]() { return serviceShared->getDescription(); }));
}

void MainWindow::cancelPendingSelectionInfo()
{
    if (selectionInfoTimer) {
        selectionInfoTimer->stop();
    }
    pendingSelectionIndex = QPersistentModelIndex();
}

void MainWindow::fetchSelectionInfo()
{
    if (!pendingSelectionIndex.isValid() || selectionInfoInProgress) {
        return;
    }

    QListWidgetItem *item = ui->listServices->itemFromIndex(pendingSelectionIndex);
    if (!item) {
        pendingSelectionIndex = QPersistentModelIndex();
        return;
    }

    auto *service = item->data(Qt::UserRole).value<Service *>();
    if (!service) {
        pendingSelectionIndex = QPersistentModelIndex();
        return;
    }

    const QSharedPointer<Service> serviceShared = findServiceShared(service);
    if (!serviceShared) {
        pendingSelectionIndex = QPersistentModelIndex();
        return;
    }

    selectionInfoInProgress = true;
    activeSelectionIndex = pendingSelectionIndex;
    pendingSelectionIndex = QPersistentModelIndex();
    activeSelectionService = service;

    if (!selectionInfoWatcher) {
        selectionInfoWatcher = new QFutureWatcher<QString>(this);
        connect(selectionInfoWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
            const QString info = selectionInfoWatcher->result();
            QListWidgetItem *activeItem = ui->listServices->itemFromIndex(activeSelectionIndex);
            const bool serviceValid = activeSelectionService && findServiceShared(activeSelectionService);
            const bool stillSelected = activeItem && activeItem->listWidget()
                && activeItem == ui->listServices->currentItem();

            if (stillSelected && activeSelectionIndex.isValid() && serviceValid) {
                ui->textBrowser->setText(info);
            }

            selectionInfoInProgress = false;
            activeSelectionIndex = QPersistentModelIndex();
            activeSelectionService = nullptr;
            if (pendingSelectionIndex.isValid()) {
                fetchSelectionInfo();
            }
        });
    }

    selectionInfoWatcher->setFuture(QtConcurrent::run([serviceShared]() { return serviceShared->getInfo(); }));
}

// static
QStringList MainWindow::systemctlCmd(QStringList arguments, bool isUserService)
{
    if (isUserService) {
        arguments.prepend("--user");
    }
    arguments.prepend("systemctl");
    return arguments;
}

// static
void MainWindow::loadSystemdUnitFileStates(bool isUserService, QSet<QString> &enabledNames, QStringList &installedNames)
{
    const auto out = Cmd().getOut(
        systemctlCmd({"list-unit-files", "--type=service",
                      "--state=enabled,enabled-runtime,disabled,masked,masked-runtime,generated", "-o", "json"}, isUserService)).trimmed();
    auto doc = QJsonDocument::fromJson(out.toUtf8());
    if (!doc.isArray()) {
        qDebug() << "Could not read the systemd unit-file list.";
        return;
    }

    const auto jsonArray = doc.array();
    enabledNames.reserve(enabledNames.size() + jsonArray.size());

    const QLatin1String unitFileKey("unit_file");
    const QLatin1String stateKey("state");

    for (const auto &value : jsonArray) {
        if (!value.isObject()) [[unlikely]] {
            continue;
        }
        const auto obj = value.toObject();
        const QString name = obj.value(unitFileKey).toString();
        if (!isServiceUnit(name)) [[unlikely]] {
            continue;
        }

        const QString state = obj.value(stateKey).toString();
        if (!name.endsWith("@.service")) {
            installedNames.append(name);
        }
        if (Service::unitEnabled(name, state, isUserService)) {
            enabledNames.insert(name);
        }
    }
}

QString MainWindow::getHtmlColor(const QColor &color) noexcept
{
    return QString("#%1%2%3")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'));
}

// static
QList<QSharedPointer<Service>> MainWindow::buildServiceList()
{
    QList<QSharedPointer<Service>> services;
    if (initSystem != QLatin1String("systemd")) {
        processNonSystemdServices(services);
    } else {
        processSystemdServices(services);
    }
    return services;
}

// static
void MainWindow::processNonSystemdServices(QList<QSharedPointer<Service>> &services)
{
    static const QRegularExpression record(QStringLiteral(R"(^\s*\[ ([+?-]) \]\s+([^\s]+)\s*$)"));
    Cmd cmd;
    const QString output = cmd.getOutAsRoot({"service", "--status-all"}, true);
    const QString errors = QString::fromUtf8(cmd.readAllStandardError());
    const QString records = output + QLatin1Char('\n') + errors;
    const auto lines = records.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const auto match = record.match(line);
        if (!match.hasMatch()) {
            qDebug().noquote() << line;
            continue;
        }
        const QString name = match.captured(2);
        if (name == "debian" || name.contains("dpkg-")) {
            continue;
        }
        auto state = Service::RunState::Unknown;
        if (match.captured(1) == "+") {
            state = Service::RunState::Running;
        } else if (match.captured(1) == "-") {
            state = Service::RunState::Stopped;
        }
        services.append(QSharedPointer<Service>::create(name, state, Service::isEnabled(name)));
    }
}

// static
void MainWindow::processSystemdServices(QList<QSharedPointer<Service>> &services)
{
    for (const bool userScope : {false, true}) {
        QSet<QString> names;
        QSet<QString> enabledServices;
        QStringList installedServices;
        loadSystemdUnitFileStates(userScope, enabledServices, installedServices);
        processSystemdActiveInactiveServices(services, names, enabledServices, userScope);
        appendUnloadedServices(services, names, installedServices, enabledServices, userScope);
    }
}

// static
void MainWindow::processSystemdActiveInactiveServices(QList<QSharedPointer<Service>> &services,
                                                      QSet<QString> &names,
                                                      const QSet<QString> &enabledServices,
                                                      bool isUserService)
{
    const auto list = Cmd().getOut(
        systemctlCmd({"list-units", "--type=service", "--all", "-o", "json"}, isUserService)).trimmed();
    auto doc = QJsonDocument::fromJson(list.toUtf8());
    if (!doc.isArray()) {
        qDebug() << "JSON data is not an array for service units.";
        return;
    }

    auto jsonArray = doc.array();

    services.reserve(services.size() + jsonArray.size());
    names.reserve(names.size() + jsonArray.size());

    const QLatin1String unitKey("unit");
    const QLatin1String loadKey("load");
    const QLatin1String subKey("sub");
    const QLatin1String notFoundValue("not-found");

    for (const auto &value : jsonArray) {
        if (!value.isObject()) [[unlikely]] {
            continue;
        }

        const auto obj = value.toObject();
        const QString name = obj.value(unitKey).toString();

        if (!isServiceUnit(name) || names.contains(name) || obj.value(loadKey).toString() == notFoundValue) [[unlikely]] {
            continue;
        }

        names.insert(name);

        const bool isRunning = Service::isRunningSubState(obj.value(subKey).toString());
        const bool isEnabled = enabledServices.contains(name);

        const auto state = isRunning ? Service::RunState::Running : Service::RunState::Stopped;
        services.append(QSharedPointer<Service>::create(name, state, isEnabled, isUserService));
    }
}

// static
void MainWindow::appendUnloadedServices(QList<QSharedPointer<Service>> &services, QSet<QString> &names,
                                     const QStringList &installedNames, const QSet<QString> &enabledNames, bool isUserService)
{
    services.reserve(services.size() + installedNames.size());
    names.reserve(names.size() + installedNames.size());

    for (const auto &name : installedNames) {
        if (names.contains(name)) [[unlikely]] {
            continue;
        }
        names.insert(name);
        services.append(QSharedPointer<Service>::create(name, Service::RunState::Stopped, enabledNames.contains(name), isUserService));
    }
}

void MainWindow::displayServices() noexcept
{
    // Cancel any pending tooltips since items are being recreated
    cancelPendingTooltip();

    ui->listServices->blockSignals(true);
    ui->listServices->clear();

    uint countActive = 0;
    uint countEnabled = 0;
    const QString searchText = ui->lineSearch->text().toLower();
    const QStringList incrementalSearchPatterns = {"s", "sa", "sam", "samb", "samba"};
    const QString currentFilter = ui->comboFilter->currentText();

    ui->listServices->setUpdatesEnabled(false);

    const bool isFilterAll = currentFilter.isEmpty() || currentFilter == tr("System services");
    const bool isFilterRunning = currentFilter == tr("Running services");
    const bool isFilterEnabled = currentFilter == tr("Services enabled at boot");
    const bool isFilterDisabled = currentFilter == tr("Services disabled at boot");
    const bool isFilterUser = currentFilter == tr("User services");
    const bool isFilterSystem = currentFilter == tr("System services");

    for (const auto &service : services) {
        if (!service || !service.get()) [[unlikely]] {
            continue;
        }
        QString serviceName = service->getName();
        // Ensure service name is valid to prevent crashes
        if (serviceName.isNull() || serviceName.isEmpty()) [[unlikely]] {
            continue;
        }
        const bool isRunning = service->isRunning();
        const bool isEnabled = service->isEnabled();
        const bool isUserService = service->isUserService();

        // Check search criteria
        if (!searchText.isEmpty() && !serviceName.contains(searchText, Qt::CaseInsensitive)
            && !(serviceName.compare(QLatin1String("smbd"), Qt::CaseInsensitive) == 0 && incrementalSearchPatterns.contains(searchText))) {
            continue;
        }

        // Check filter criteria
        if ((isFilterRunning && !isRunning) || (isFilterEnabled && !isEnabled) || (isFilterDisabled && isEnabled)
            || (isFilterUser && !isUserService) || (isFilterSystem && isUserService)
            || (!isFilterAll && !isFilterRunning && !isFilterEnabled && !isFilterDisabled && !isFilterUser && !isFilterSystem)) {
            continue;
        }

        // Update counters
        if (isRunning) {
            ++countActive;
        } else if (isEnabled && service->hasKnownRunState()) {
            ++countEnabled;
        }

        // Create item and add it directly to the list widget
        QString displayName = serviceName;
        if (isUserService) [[unlikely]] {
            displayName = tr("[User] ") % serviceName;
        }
        auto *item = new QListWidgetItem(displayName, ui->listServices);
        item->setData(Qt::UserRole, QVariant::fromValue(service.get()));
        item->setForeground(isRunning ? runningColor : (isEnabled && service->hasKnownRunState() ? enabledColor : defaultForeground));
    }

    // Update status labels
    ui->labelCount->setText(tr("%1 total services, %2 currently <font color='%3'>running</font>")
                                .arg(services.count())
                                .arg(countActive)
                                .arg(getHtmlColor(runningColor)));

    ui->labelEnabledAtBoot->setText(tr("%1 <font color='%2'>enabled</font> at boot, but not running")
                                        .arg(countEnabled)
                                        .arg(getHtmlColor(enabledColor)));

    ui->listServices->sortItems();
    ui->listServices->setUpdatesEnabled(true);
    ui->listServices->blockSignals(false);
    if (ui->listServices->count() == 0) {
        savedRow = 0;
        onSelectionChanged(nullptr, nullptr);
        return;
    }
    savedRow = qBound(0, savedRow, ui->listServices->count() - 1);
    ui->listServices->setCurrentRow(savedRow);
}

void MainWindow::pushAbout_clicked()
{
    displayAboutMsgBox(
        this,
        tr("About %1") % tr("MX Service Manager"),
        R"(<p align="center"><b><h2>MX Service Manager</h2></b></p><p align="center">)" % tr("Version: ")
            % QApplication::applicationVersion() % "</p><p align=\"center\"><h3>" % tr("Service and daemon manager")
            % R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            % tr("Copyright (c) MX Linux") % "<br /><br /></p>",
        docPath(QStringLiteral("license.html")), tr("%1 License").arg(windowTitle()));
}

void MainWindow::updateServiceControls()
{
    const bool ready = !serviceListLoading && !serviceActionInProgress;
    const auto *item = ui->listServices->currentItem();
    const auto *service = item ? item->data(Qt::UserRole).value<Service *>() : nullptr;
    const bool actionable = ready && service;
    ui->pushRefresh->setEnabled(ready);
    ui->pushStartStop->setEnabled(actionable && service->hasKnownRunState());
    ui->pushEnableDisable->setEnabled(actionable);
}

void MainWindow::pushEnableDisable_clicked()
{
    if (serviceActionInProgress || serviceListLoading) {
        return;
    }
    savedRow = ui->listServices->currentRow();
    const auto *currentItem = ui->listServices->currentItem();
    auto ptrService = currentItem ? findServiceShared(currentItem->data(Qt::UserRole).value<Service *>())
                                  : QSharedPointer<Service>();
    if (!ptrService) {
        return;
    }
    serviceActionInProgress = true;
    updateServiceControls();
    const auto restoreControls = qScopeGuard([this] {
        serviceActionInProgress = false;
        updateServiceControls();
    });
    auto service = currentItem->text();
    if (ui->pushEnableDisable->text() == tr("&Enable at boot")) {
        if (!ptrService->enable()) {
            // Re-query the item's live status even on failure: a preceding step (e.g.
            // unmasking) can still have changed real state that a failed enable() didn't
            // restore, and this refreshes what's shown for it.
            itemUpdated();
            emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
            if (ptrService->hadUnrestoredMaskChange()) {
                QMessageBox::warning(
                    this, tr("Error"),
                    tr("Could not enable %1, and its previous state could not be fully restored. Please check its status.")
                        .arg(service));
            } else {
                QMessageBox::warning(this, tr("Error"),
                                      tr("Could not enable %1. Settings were not applied.").arg(service));
            }
        } else {
            itemUpdated();
            emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
            QMessageBox::information(this, tr("Success"), tr("%1 was enabled at boot time.").arg(service));
        }
    } else {
        if (!ptrService->disable()) {
            itemUpdated();
            QMessageBox::warning(this, tr("Error"),
                                 ptrService->isEnabled()
                                     ? tr("Could not verify that %1 was disabled. Check its status.").arg(service)
                                     : tr("%1 was disabled at boot, but its mask could not be verified.").arg(service));
        } else {
            itemUpdated();
            emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
            QMessageBox::information(this, tr("Success"), tr("%1 was disabled.").arg(service));
        }
    }
}

void MainWindow::pushHelp_clicked()
{
    displayDoc(this, docPath(QStringLiteral("mx-service-manager.html")), tr("%1 Help").arg(windowTitle()), true);
}

void MainWindow::pushRefresh_clicked()
{
    if (serviceActionInProgress || serviceListLoading) {
        return;
    }
    const auto *currentItem = ui->listServices->currentItem();
    const QString selectedService = currentItem ? currentItem->text() : QString();
    savedRow = ui->listServices->currentRow();

    ui->labelCount->setText(tr("Refreshing..."));
    ui->labelEnabledAtBoot->clear();
    ui->listServices->clear();
    ui->listServices->addItem(tr("Refreshing..."));

    loadServicesAsync([this, selectedService]() {
        displayServices();
        if (!selectedService.isEmpty()) {
            const auto matches = ui->listServices->findItems(selectedService, Qt::MatchExactly);
            if (!matches.isEmpty()) {
                ui->listServices->setCurrentItem(matches.first());
            }
        }
    });
}

void MainWindow::pushStartStop_clicked()
{
    if (serviceActionInProgress || serviceListLoading) {
        return;
    }
    savedRow = ui->listServices->currentRow();
    const auto *currentItem = ui->listServices->currentItem();
    auto ptrService = currentItem ? findServiceShared(currentItem->data(Qt::UserRole).value<Service *>())
                                  : QSharedPointer<Service>();
    if (!ptrService || !ptrService->hasKnownRunState()) {
        return;
    }
    serviceActionInProgress = true;
    updateServiceControls();
    const auto restoreControls = qScopeGuard([this] {
        serviceActionInProgress = false;
        updateServiceControls();
    });
    auto service = currentItem->text();
    if (ui->pushStartStop->text() == tr("S&tart")) {
        if (!ptrService->start()) {
            QMessageBox::warning(this, tr("Error"),
                                 ptrService->isMasked()
                                     ? tr("%1 is masked. Unmask it before starting it manually.").arg(service)
                                     : tr("Could not start %1").arg(service));
        } else {
            itemUpdated();
            emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
            QMessageBox::information(this, tr("Success"), tr("%1 was started.").arg(service));
        }
    } else {
        if (!ptrService->stop()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not stop %1").arg(service));
        } else {
            ptrService->setRunning(false);
            itemUpdated();
            emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
            QMessageBox::information(this, tr("Success"), tr("%1 was stopped.").arg(service));
        }
    }
}

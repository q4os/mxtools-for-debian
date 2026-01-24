/**********************************************************************
 *  mainwindow.cpp
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
#define QT_USE_QSTRINGBUILDER
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QTimer>
#include <QtConcurrent>

#include "about.h"
#include "common.h"
#include "service.h"

#include <algorithm>
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

    // Cache authentication for elevated commands
    Cmd().runAsRoot("true", true);

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

    dependTargets
        = cmd.getOut("grep --no-filename \"TARGETS = \" /etc/init.d/.depend.start /etc/init.d/.depend.boot |  "
                     "sed  -e ':a;N;$!ba;s/\\n/ /' -e 's/TARGETS = //g'",
                     true)
              .split(' ', Qt::SkipEmptyParts);

    ui->labelCount->setText(tr("Loading..."));
    auto loadingTimer = new QTimer(this);
    connect(loadingTimer, &QTimer::timeout, this, [this]() {
        static bool toggle = false;
        ui->labelCount->setText(toggle ? tr("Loading...") : QString());
        toggle = !toggle;
    });
    loadingTimer->start(300);

    // Load services asynchronously to keep UI responsive
    QTimer::singleShot(0, this, [this, loadingTimer] {
        listServices();
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
    delete ui;
}

void MainWindow::centerWindow()
{
    const auto screenGeometry = QApplication::primaryScreen()->geometry();
    const int x = (screenGeometry.width() - width()) / 2;
    const int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
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
    if (!current) {
        return;
    }
    ui->textBrowser->setText(current->data(Qt::UserRole).value<Service *>()->getInfo());
    bool running = current->data(Qt::UserRole).value<Service *>()->isRunning();
    bool enabled = current->data(Qt::UserRole).value<Service *>()->isEnabled();
    if (running) {
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
    if (enabled) {
        ui->pushEnableDisable->setText(tr("&Disable at boot"));
        ui->pushEnableDisable->setIcon(QIcon::fromTheme("stop"));
        if (!running) {
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

    tooltipInProgress = true;
    activeTooltipIndex = pendingTooltipIndex;
    pendingTooltipIndex = QPersistentModelIndex();
    activeTooltipService = service;

    if (!tooltipWatcher) {
        tooltipWatcher = new QFutureWatcher<QString>(this);
        connect(tooltipWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
            const QString description = tooltipWatcher->result();
            QListWidgetItem *activeItem = ui->listServices->itemFromIndex(activeTooltipIndex);
            const bool serviceValid = activeTooltipService
                && std::any_of(services.begin(), services.end(),
                               [this](const auto &svc) { return svc.get() == activeTooltipService; });

            if (activeItem && activeItem->listWidget() && activeTooltipIndex.isValid() && serviceValid) {
                if (!description.isEmpty()) {
                    activeItem->setToolTip(description);
                }
                activeItem->setData(kTooltipFetchedRole, true);
            }

            tooltipInProgress = false;
            activeTooltipIndex = QPersistentModelIndex();
            activeTooltipService = nullptr;
            if (pendingTooltipIndex.isValid()) {
                fetchTooltipDescription();
            }
        });
    }

    tooltipWatcher->setFuture(QtConcurrent::run([service]() { return service->getDescription(); }));
}

std::optional<QString> MainWindow::sanitizeServiceName(const QString &rawName)
{
    const QLatin1String dotSeparator(".");
    QString name = rawName.section(dotSeparator, 0, 0);
    name = name.simplified();
    name = decodeEscapeSequences(name);

    static const QRegularExpression invalidRegex("[^a-zA-Z0-9._@:+-]");
    if (name.isEmpty() || name.length() > 100 || name.contains(invalidRegex)) {
        return std::nullopt;
    }

    return name;
}

QSet<QString> MainWindow::loadSystemdEnabledServices(bool isUserService)
{
    QString cmdStr = "systemctl list-unit-files --type=service --state=enabled -o json";
    if (isUserService) [[unlikely]] {
        cmdStr = "systemctl --user list-unit-files --type=service --state=enabled -o json";
    }
    const auto enabled = cmd.getOut(cmdStr).trimmed();
    auto doc = QJsonDocument::fromJson(enabled.toUtf8());
    if (!doc.isArray()) {
        qDebug() << "JSON data is not an array for enabled services.";
        return {};
    }

    QSet<QString> enabledNames;
    const auto jsonArray = doc.array();
    enabledNames.reserve(jsonArray.size());

    const QLatin1String unitFileKey("unit_file");
    for (const auto &value : jsonArray) {
        if (!value.isObject()) [[unlikely]] {
            continue;
        }
        const auto obj = value.toObject();
        if (const auto name = sanitizeServiceName(obj.value(unitFileKey).toString())) {
            enabledNames.insert(*name);
        }
    }

    return enabledNames;
}

QString MainWindow::decodeEscapeSequences(const QString &input)
{
    static const QRegularExpression hexRegex("\\\\x([0-9a-fA-F]{2})");
    static const QRegularExpression unicodeRegex("\\\\u([0-9a-fA-F]{4})");
    static const QRegularExpression octalRegex("\\\\([0-7]{1,3})");
    static const QRegularExpression backslashCleanup("\\\\(?![0-7]{1,3}|[xu][0-9a-fA-F]+)");

    QString result = input;

    // Decode \xXX sequences (hex)
    QRegularExpressionMatchIterator hexIt = hexRegex.globalMatch(result);
    while (hexIt.hasNext()) {
        QRegularExpressionMatch match = hexIt.next();
        bool ok;
        int hexValue = match.captured(1).toInt(&ok, 16);
        if (ok) {
            result.replace(match.captured(0), QChar(hexValue));
        }
    }

    // Decode \uXXXX sequences (Unicode)
    QRegularExpressionMatchIterator unicodeIt = unicodeRegex.globalMatch(result);
    while (unicodeIt.hasNext()) {
        QRegularExpressionMatch match = unicodeIt.next();
        bool ok;
        int unicodeValue = match.captured(1).toInt(&ok, 16);
        if (ok) {
            result.replace(match.captured(0), QChar(unicodeValue));
        }
    }

    // Decode \OOO sequences (octal)
    QRegularExpressionMatchIterator octalIt = octalRegex.globalMatch(result);
    while (octalIt.hasNext()) {
        QRegularExpressionMatch match = octalIt.next();
        bool ok;
        int octalValue = match.captured(1).toInt(&ok, 8);
        if (ok) {
            result.replace(match.captured(0), QChar(octalValue));
        }
    }

    // Remove any remaining backslashes that aren't part of valid escape sequences
    result = result.remove(backslashCleanup);

    return result;
}

QString MainWindow::getHtmlColor(const QColor &color) noexcept
{
    return QString("#%1%2%3")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'));
}

void MainWindow::listServices()
{
    services.clear();
    if (initSystem != QLatin1String("systemd")) {
        processNonSystemdServices();
    } else {
        processSystemdServices();
    }
}

void MainWindow::processNonSystemdServices()
{
    static const QRegularExpression dpkgRegex("dpkg-.*$");
    const auto list = cmd.getOutAsRoot("/sbin/service --status-all", true).trimmed().split("\n");
    services.reserve(list.size());

    const QLatin1String sectionDelimiter("]  ");
    const QLatin1String debian("debian");
    const QLatin1String runningPrefix("[ + ]");

    for (const auto &item : list) {
        const QString trimmedItem = item.trimmed();
        if (item.section(sectionDelimiter, 1) == debian || trimmedItem.contains(dpkgRegex)) {
            continue;
        }

        const QString name = item.section(sectionDelimiter, 1);
        if (name.isEmpty()) {
            continue;
        }

        bool enabled = dependTargets.contains(name) || Service::isEnabled(name);
        services.append(QSharedPointer<Service>::create(name, trimmedItem.startsWith(runningPrefix), enabled, false));
    }
}

void MainWindow::processSystemdServices()
{
    QStringList names;
    const QSet<QString> enabledSystemServices = loadSystemdEnabledServices(false);
    const QSet<QString> enabledUserServices = loadSystemdEnabledServices(true);

    processSystemdActiveInactiveServices(names, enabledSystemServices, false); // System services
    processSystemdMaskedServices(names, false); // System services
    processSystemdActiveInactiveServices(names, enabledUserServices, true); // User services
    processSystemdMaskedServices(names, true); // User services
}

void MainWindow::processSystemdActiveInactiveServices(QStringList &names,
                                                      const QSet<QString> &enabledServices,
                                                      bool isUserService)
{
    QString cmdStr = "systemctl list-units --type=service --all -o json";
    if (isUserService) [[unlikely]] {
        cmdStr = "systemctl --user list-units --type=service --all -o json";
    }
    const auto list = cmd.getOut(cmdStr).trimmed();
    auto doc = QJsonDocument::fromJson(list.toUtf8());
    if (!doc.isArray()) {
        qDebug() << "JSON data is not an array for service units.";
        return;
    }

    auto jsonArray = doc.array();

    QSet<QString> nameSet(names.begin(), names.end());
    services.reserve(services.size() + jsonArray.size());
    nameSet.reserve(nameSet.size() + jsonArray.size());

    const QLatin1String unitKey("unit");
    const QLatin1String loadKey("load");
    const QLatin1String subKey("sub");
    const QLatin1String notFoundValue("not-found");
    const QLatin1String runningValue("running");

    for (const auto &value : jsonArray) {
        if (!value.isObject()) [[unlikely]] {
            continue;
        }

        const auto obj = value.toObject();
        const auto nameOpt = sanitizeServiceName(obj.value(unitKey).toString());

        if (!nameOpt || nameSet.contains(*nameOpt) || obj.value(loadKey).toString() == notFoundValue) [[unlikely]] {
            continue;
        }

        const QString &name = *nameOpt;
        nameSet.insert(name);

        const bool isRunning = (obj.value(subKey).toString() == runningValue);
        const bool isEnabled = (!isUserService && dependTargets.contains(name))
            || enabledServices.contains(name);

        services.append(QSharedPointer<Service>::create(name, isRunning, isEnabled, isUserService));
    }
    names = QStringList(nameSet.begin(), nameSet.end());
}

void MainWindow::processSystemdMaskedServices(QStringList &names, bool isUserService)
{
    QString cmdStr = "systemctl list-unit-files --type=service --state=masked -o json";
    if (isUserService) [[unlikely]] {
        cmdStr = "systemctl --user list-unit-files --type=service --state=masked -o json";
    }
    const auto masked = cmd.getOut(cmdStr).trimmed();
    auto doc = QJsonDocument::fromJson(masked.toUtf8());
    if (!doc.isArray()) {
        qDebug() << "JSON data is not an array for masked services.";
        return;
    }

    auto jsonArray = doc.array();
    QSet<QString> nameSet(names.begin(), names.end());
    services.reserve(services.size() + jsonArray.size());
    nameSet.reserve(nameSet.size() + jsonArray.size());

    const QLatin1String unitFileKey("unit_file");

    for (const auto &value : jsonArray) {
        if (!value.isObject()) [[unlikely]] {
            continue;
        }
        const auto obj = value.toObject();
        const auto nameOpt = sanitizeServiceName(obj.value(unitFileKey).toString());

        if (!nameOpt || nameSet.contains(*nameOpt)) [[unlikely]] {
            continue;
        }

        const QString &name = *nameOpt;
        nameSet.insert(name);
        services.append(QSharedPointer<Service>::create(name, false, false, isUserService));
    }
    names = QStringList(nameSet.begin(), nameSet.end());
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
        serviceName = serviceName.toLower();
        const bool isRunning = service->isRunning();
        const bool isEnabled = service->isEnabled();
        const bool isUserService = service->isUserService();

        // Check search criteria
        if (!searchText.isEmpty() && !serviceName.contains(searchText, Qt::CaseInsensitive)
            && !(serviceName == QLatin1String("smbd") && incrementalSearchPatterns.contains(searchText))) {
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
        } else if (isEnabled) {
            ++countEnabled;
        }

        // Create item and add it directly to the list widget
        QString displayName = serviceName;
        if (isUserService) [[unlikely]] {
            displayName = tr("[User] ") % serviceName;
        }
        auto *item = new QListWidgetItem(displayName, ui->listServices);
        item->setData(Qt::UserRole, QVariant::fromValue(service.get()));
        item->setForeground(isRunning ? runningColor : (isEnabled ? enabledColor : Qt::black));
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
    savedRow = qBound(0, savedRow, ui->listServices->count() - 1);
    ui->listServices->setCurrentRow(savedRow);
}

void MainWindow::pushAbout_clicked()
{
    hide();
    displayAboutMsgBox(
        tr("About %1") % tr("MX Service Manager"),
        R"(<p align="center"><b><h2>MX Service Manager</h2></b></p><p align="center">)" % tr("Version: ")
            % QApplication::applicationVersion() % "</p><p align=\"center\"><h3>" % tr("Service and daemon manager")
            % R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            % tr("Copyright (c) MX Linux") % "<br /><br /></p>",
        "/usr/share/doc/mx-service-manager/license.html", tr("%1 License").arg(windowTitle()));

    show();
}

void MainWindow::pushEnableDisable_clicked()
{
    ui->pushEnableDisable->setEnabled(false);
    savedRow = ui->listServices->currentRow();
    auto service = ui->listServices->currentItem()->text();
    auto *ptrService = ui->listServices->currentItem()->data(Qt::UserRole).value<Service *>();
    if (ui->pushEnableDisable->text() == tr("&Enable at boot")) {
        if (!ptrService->enable()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not enable %1").arg(service));
        }
        itemUpdated();
        emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
        QMessageBox::information(this, tr("Success"), tr("%1 was enabled at boot time.").arg(service));
    } else {
        if (!ptrService->disable()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not disable %1").arg(service));
        }
        itemUpdated();
        emit ui->listServices->currentItemChanged(ui->listServices->currentItem(), ui->listServices->currentItem());
        QMessageBox::information(this, tr("Success"), tr("%1 was disabled.").arg(service));
    }
    ui->pushEnableDisable->setEnabled(true);
}

void MainWindow::pushHelp_clicked()
{
    const QString url = "https://mxlinux.org/wiki/help-service-manager/";
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::pushRefresh_clicked()
{
    ui->pushRefresh->setEnabled(false);
    const auto *currentItem = ui->listServices->currentItem();
    const QString selectedService = currentItem ? currentItem->text() : QString();
    savedRow = ui->listServices->currentRow();

    ui->labelCount->setText(tr("Refreshing..."));
    ui->labelEnabledAtBoot->clear();
    ui->listServices->clear();
    ui->listServices->addItem(tr("Refreshing..."));

    QTimer::singleShot(0, this, [this, selectedService]() {
        listServices();
        displayServices();
        if (!selectedService.isEmpty()) {
            const auto matches = ui->listServices->findItems(selectedService, Qt::MatchExactly);
            if (!matches.isEmpty()) {
                ui->listServices->setCurrentItem(matches.first());
            }
        }
        ui->pushRefresh->setEnabled(true);
    });
}

void MainWindow::pushStartStop_clicked()
{
    ui->pushStartStop->setEnabled(false);
    savedRow = ui->listServices->currentRow();
    auto service = ui->listServices->currentItem()->text();
    auto *ptrService = ui->listServices->currentItem()->data(Qt::UserRole).value<Service *>();
    if (ui->pushStartStop->text() == tr("S&tart")) {
        if (!ptrService->start()) {
            QMessageBox::warning(this, tr("Error"), tr("Could not start %1").arg(service));
        } else {
            ptrService->setRunning(true);
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
    ui->pushStartStop->setEnabled(true);
}

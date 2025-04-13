/**********************************************************************
 *  mainwindow.cpp
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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>

#include "about.h"
#include "common.h"
#include "service.h"

#include <chrono>

using namespace std::chrono_literals;

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
    if (initSystem != "systemd"
        && !initSystem.startsWith("init")) { // Can be "init(mxlinux)" when running in WSL for example
        QMessageBox::warning(
            this, tr("Error"),
            tr("Could not determine the init system. This program is supposed to run either with systemd or sysvinit")
                + "\nINIT:" + initSystem);
    }
    QPalette palette = ui->listServices->palette();
    defaultForeground = palette.color(QPalette::Text);
    ui->listServices->addItem(tr("Loading..."));

    dependTargets
        = cmd.getOut("grep --no-filename \"TARGETS = \" /etc/init.d/.depend.start /etc/init.d/.depend.boot |  "
                     "sed  -e ':a;N;$!ba;s/\\n/ /' -e 's/TARGETS = //g'",
                     true)
              .split(' ', Qt::SkipEmptyParts);

    QTimer::singleShot(0, this, [this] {
        QTimer timer;
        timer.start(300ms);
        connect(&timer, &QTimer::timeout, this, [this] {
            static bool toggle = false;
            ui->labelCount->setText(toggle ? tr("Loading...") : QString());
            toggle = !toggle;
        });
        listServices();
        timer.disconnect();
        displayServices();
        ui->listServices->setFocus();
    });
    connect(ui->listServices, &QListWidget::itemEntered, this, [this](QListWidgetItem *item) {
        if (auto service = item->data(Qt::UserRole).value<Service *>()) {
            if (item->toolTip().isEmpty()) {
                const QString description = service->getDescription();
                item->setToolTip(description);
            }
        }
    });
}

MainWindow::~MainWindow()
{
    settings.setValue("geometry", saveGeometry());
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

void MainWindow::setGeneralConnections()
{
    connect(ui->comboFilter, &QComboBox::currentTextChanged, this, &MainWindow::displayServices);
    connect(ui->lineSearch, &QLineEdit::textChanged, this, &MainWindow::displayServices);
    connect(ui->listServices, &QListWidget::currentItemChanged, this, &MainWindow::onSelectionChanged);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushCancel, &QPushButton::pressed, this, &MainWindow::close);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushEnableDisable, &QPushButton::clicked, this, &MainWindow::pushEnableDisable_clicked);
    connect(ui->pushStartStop, &QPushButton::clicked, this, &MainWindow::pushStartStop_clicked);
}

QString MainWindow::getHtmlColor(const QColor &color)
{
    return QString("#%1%2%3")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'));
}

void MainWindow::listServices()
{
    services.clear();
    if (initSystem != "systemd") {
        processNonSystemdServices();
    } else {
        processSystemdServices();
    }
}

void MainWindow::processNonSystemdServices()
{
    const auto list = cmd.getOut("/sbin/service --status-all", true).trimmed().split("\n");
    QRegularExpression re("dpkg-.*$");
    services.reserve(list.size());

    const QLatin1String sectionDelimiter("]  ");
    const QLatin1String debian("debian");
    const QLatin1String runningPrefix("[ + ]");

    for (const auto &item : list) {
        const QString trimmedItem = item.trimmed();
        if (item.section(sectionDelimiter, 1) == debian || trimmedItem.contains(re)) {
            continue;
        }

        const QString name = item.section(sectionDelimiter, 1);
        if (name.isEmpty()) {
            continue;
        }

        bool enabled = dependTargets.contains(name) || Service::isEnabled(name);
        services.append(QSharedPointer<Service>::create(name, trimmedItem.startsWith(runningPrefix), enabled));
    }
}

void MainWindow::processSystemdServices()
{
    QStringList names;
    processSystemdActiveInactiveServices(names);
    processSystemdMaskedServices(names);
}

void MainWindow::processSystemdActiveInactiveServices(QStringList &names)
{
    const auto list = cmd.getOut("systemctl list-units --type=service --all -o json").trimmed();
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
    const QLatin1String dotSeparator(".");
    const QLatin1String notFoundValue("not-found");
    const QLatin1String runningValue("running");

    for (const auto &value : jsonArray) {
        if (!value.isObject()) {
            continue;
        }

        const auto obj = value.toObject();
        const QString name = obj.value(unitKey).toString().section(dotSeparator, 0, 0);

        if (name.isEmpty() || nameSet.contains(name) || obj.value(loadKey).toString() == notFoundValue) {
            continue;
        }

        nameSet.insert(name);

        const bool isRunning = (obj.value(subKey).toString() == runningValue);
        const bool isEnabled = dependTargets.contains(name) || Service::isEnabled(name);

        services.append(QSharedPointer<Service>::create(name, isRunning, isEnabled));
    }
    names = QStringList(nameSet.begin(), nameSet.end());
}

void MainWindow::processSystemdMaskedServices(QStringList &names)
{
    const auto masked = cmd.getOut("systemctl list-unit-files --type=service --state=masked -o json").trimmed();
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
    const QLatin1String dotSeparator(".");

    for (const auto &value : jsonArray) {
        if (!value.isObject()) {
            continue;
        }
        const auto obj = value.toObject();
        const QString name = obj.value(unitFileKey).toString().section(dotSeparator, 0, 0);

        if (name.isEmpty() || nameSet.contains(name)) {
            continue;
        }
        nameSet.insert(name);
        services.append(QSharedPointer<Service>::create(name));
    }
    names = QStringList(nameSet.begin(), nameSet.end());
}

void MainWindow::displayServices()
{
    ui->listServices->blockSignals(true);
    ui->listServices->clear();

    uint countActive = 0;
    uint countEnabled = 0;
    const QString searchText = ui->lineSearch->text().toLower();
    const QStringList incrementalSearchPatterns = {"s", "sa", "sam", "samb", "samba"};
    const QString currentFilter = ui->comboFilter->currentText();

    ui->listServices->setUpdatesEnabled(false);

    const bool isFilterAll = currentFilter.isEmpty() || currentFilter == tr("All services");
    const bool isFilterRunning = currentFilter == tr("Running services");
    const bool isFilterEnabled = currentFilter == tr("Services enabled at boot");
    const bool isFilterDisabled = currentFilter == tr("Services disabled at boot");

    for (const auto &service : services) {
        const QString serviceName = service->getName().toLower();
        const bool isRunning = service->isRunning();
        const bool isEnabled = service->isEnabled();

        // Check search criteria
        if (!searchText.isEmpty() && !serviceName.startsWith(searchText)
            && !(serviceName == QLatin1String("smbd") && incrementalSearchPatterns.contains(searchText))) {
            continue;
        }

        // Check filter criteria
        if ((isFilterRunning && !isRunning) || (isFilterEnabled && !isEnabled) || (isFilterDisabled && isEnabled)
            || (!isFilterAll && !isFilterRunning && !isFilterEnabled && !isFilterDisabled)) {
            continue;
        }

        // Update counters
        if (isRunning) {
            ++countActive;
        } else if (isEnabled) {
            ++countEnabled;
        }

        // Create item and add it directly to the list widget
        auto *item = new QListWidgetItem(serviceName, ui->listServices);
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
        tr("About %1") + tr("MX Service Manager"),
        R"(<p align="center"><b><h2>MX Service Manager</h2></b></p><p align="center">)" + tr("Version: ")
            + QApplication::applicationVersion() + "</p><p align=\"center\"><h3>" + tr("Service and daemon manager")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        "/usr/share/doc/mx-service-manager/license.html", tr("%1 License").arg(windowTitle()));

    show();
}

void MainWindow::pushEnableDisable_clicked()
{
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
}

void MainWindow::pushHelp_clicked()
{
    const QString url = "https://mxlinux.org/wiki/help-service-manager/";
    displayDoc(url, tr("%1 Help").arg(windowTitle()));
}

void MainWindow::pushStartStop_clicked()
{
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
}

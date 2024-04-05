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
    setWindowFlags(Qt::Window); // For the close, min and max buttons
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
              .split(" ");

    QTimer::singleShot(0, this, [this] {
        QTimer timer;
        timer.start(300ms);
        connect(&timer, &QTimer::timeout, this, [this] {
            static auto i = 0;
            (i % 2 == 0) ? ui->labelCount->setText(tr("Loading...")) : ui->labelCount->clear();
            ++i;
        });
        listServices();
        displayServices();
        ui->listServices->setFocus();
    });
    connect(ui->listServices, &QListWidget::itemEntered, this, [this](QListWidgetItem *item) {
        if (item->data(Qt::UserRole).value<Service *>()) {
            ui->listServices->blockSignals(true);
            if (!item->toolTip().isEmpty()) {
                ui->listServices->blockSignals(false);
                return;
            }
            ui->lineSearch->blockSignals(true);
            item->setToolTip(item->data(Qt::UserRole).value<Service *>()->getDescription());
            ui->lineSearch->blockSignals(false);
            ui->listServices->blockSignals(false);
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
        const auto list = cmd.getOut("/sbin/service --status-all", true).trimmed().split("\n");
        QRegularExpression re("dpkg-.*$");
        for (const auto &item : list) {
            if (item.trimmed().contains(re) || item.section("]  ", 1) == "debian") {
                continue;
            }
            QString name = item.section("]  ", 1);
            if (name.isEmpty()) {
                continue;
            }
            QSharedPointer<Service> service = QSharedPointer<Service>::create(name, item.trimmed().startsWith("[ + ]"));
            service->setEnabled(Service::isEnabled(name) || dependTargets.contains(name));
            services << service;
        }
    } else {
        const auto list = cmd.getOutAsRoot("systemctl list-units --type=service --all -o json").trimmed();
        auto doc = QJsonDocument::fromJson(list.toUtf8());
        if (!doc.isArray()) {
            qDebug() << "JSON data is not an array.";
            return;
        }
        auto jsonArray = doc.array();
        QStringList names;
        names.reserve(jsonArray.size() * 2);
        for (const auto &value : jsonArray) {
            if (!value.isObject()) {
                continue;
            }
            auto obj = value.toObject();
            QString name = obj.value("unit").toString().section(".", 0, 0);
            if (name.isEmpty() || names.contains(name) || obj.value("load").toString() == "not-found") {
                continue;
            }
            QSharedPointer<Service> service
                = QSharedPointer<Service>::create(name, obj.value("sub").toString() == "running");
            names << name;
            service->setEnabled(Service::isEnabled(name) || dependTargets.contains(name));
            services << service;
        }
        const auto masked
            = cmd.getOutAsRoot("systemctl list-unit-files --type=service --state=masked -o json").trimmed();
        doc = QJsonDocument::fromJson(masked.toUtf8());
        if (!doc.isArray()) {
            qDebug() << "JSON data is not an array.";
            return;
        }
        jsonArray = doc.array();
        for (const auto &value : jsonArray) {
            if (!value.isObject()) {
                continue;
            }
            auto obj = value.toObject();
            QString name = obj.value("unit_file").toString().section(".", 0, 0);
            if (name.isEmpty() || names.contains(name)) {
                continue;
            }
            QSharedPointer<Service> service = QSharedPointer<Service>::create(name, false);
            names << name;
            service->setEnabled(false);
            services << service;
        }
    }
}

void MainWindow::displayServices()
{
    ui->listServices->blockSignals(true);
    ui->listServices->clear();
    uint countActive = 0;
    uint countEnabled = 0;
    const QString searchText = ui->lineSearch->text().toLower();
    const QStringList incrementalSearchPatterns = {"s", "sa", "sam", "samb", "samba"};
    for (const auto &service : services) {
        const QString serviceName = service->getName().toLower();
        if (!searchText.isEmpty() && !serviceName.startsWith(searchText)
            && !(serviceName == "smbd" && incrementalSearchPatterns.contains(searchText))) {
            continue;
        }
        auto *item = new QListWidgetItem(serviceName, ui->listServices);
        item->setData(Qt::UserRole, QVariant::fromValue(service.get()));
        if (service->isRunning()) {
            ++countActive;
            item->setForeground(runningColor);
        } else if (service->isEnabled()) {
            ++countEnabled;
            item->setForeground(enabledColor);
        }
        if ((!service->isRunning() && (ui->comboFilter->currentText() == tr("Running services")))
            || (!service->isEnabled() && (ui->comboFilter->currentText() == tr("Services enabled at boot")))
            || (service->isEnabled() && (ui->comboFilter->currentText() == tr("Services disabled at boot")))) {
            delete item;
        } else {
            ui->listServices->addItem(item);
        }
    }
    const QString totalServicesText
        = tr("%1 total services, %2 currently <font color='%3'>running</font>")
              .arg(QString::number(services.count()), QString::number(countActive), getHtmlColor(runningColor));
    const QString enabledAtBootText = tr("%1 <font color='%2'>enabled</font> at boot, but not running")
                                          .arg(QString::number(countEnabled), getHtmlColor(enabledColor));
    ui->labelCount->setText(totalServicesText);
    ui->labelEnabledAtBoot->setText(enabledAtBootText);
    ui->listServices->blockSignals(false);
    ui->listServices->sortItems();
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

/*
   Copyright (C) 2003-2010 by Warren Woodford
   Copyright (C) 2014 by Adrian adrian@mxlinux.org

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


   With big modifications made by Adrian adrian@mxlinux.org
*/

#include "mainwindow.h"

#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QScreen>

#include "version.h"
#include <about.h>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
{
    qDebug() << "Program Version:" << VERSION;
    setupUi(this);
    this->setMinimumSize(400, 600);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setWindowIcon(QApplication::windowIcon());

    hwUnblock->hide();

    currentTab = Tab::Status;
    tabWidget->setCurrentIndex(Tab::Status);
    tabWidget->removeTab(2); // remove WindowsDrivers tab.

    pingProc = new QProcess(this);
    traceProc = new QProcess(this);
    installProc = new QProcess(this);

    installOutputEdit = new QTextEdit();

    pushDisable->setDisabled(true);
    pushEnable->setDisabled(true);

    connect(hwList, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showContextMenuForHw);
    connect(linuxDrvList, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenuForLinuxDrv);
    connect(windowsDrvList, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenuForWindowsDrv);
    connect(hwList, &QTreeWidget::itemSelectionChanged, this, [this]() {
        if (!hwList->currentItem()->isSelected()) {
            pushEnable->setEnabled(false);
            pushDisable->setEnabled(false);
            return;
        }
        pushEnable->setEnabled(!hwList->currentItem()->data(Col::Enabled, Qt::UserRole).toBool());
        pushDisable->setEnabled(hwList->currentItem()->data(Col::Enabled, Qt::UserRole).toBool());
    });

    tabWidget->setTabIcon(Tab::Status,
                          QIcon::fromTheme(QStringLiteral("emblem-documents"), QIcon(":/icons/emblem-documents.svg")));
}

bool MainWindow::replaceStringInFile(const QString &oldtext, const QString &newtext, const QString &filepath)
{
    const QString expr = QStringLiteral("s/%1/%2/g'").arg(oldtext, newtext);
    return (QProcess::execute(QStringLiteral("sed"), {QStringLiteral("-i"), expr, filepath}) == 0);
}

void MainWindow::refresh()
{
    hwUnblock->hide();
    groupWifi->hide();
    hwDiagnosePushButton->setEnabled(true);
    const QString out = cmd.getCmdOut(QStringLiteral("rfkill list 2>&1"));
    QApplication::processEvents();

    switch (tabWidget->currentIndex()) {
    case Tab::Status:
        on_hwDiagnosePushButton_clicked();
        if (out == QLatin1String("Can't open RFKILL control device: No such file or directory"))
            hwUnblock->hide();
        else
            hwUnblock->show();
        labelRouterIP->setText(tr("IP address from router:") + " " + getIPfromRouter());
        labelIP->setText(tr("External IP address:") + " " + getIP());
        labelInterface->setText(cmd.getCmdOut(QStringLiteral("ip route | grep ^default | grep -Po '(?<=dev )(\\S+)'")));
        checkWifiAvailable();
        checkWifiEnabled();
        break;
    case Tab::LinuxDrivers:
        on_linuxDrvDiagnosePushButton_clicked();
        break;
        //    case Tab::WindowsDrivers:
        //        on_windowsDrvDiagnosePushButton_clicked();
        //        break;
    }
}

void MainWindow::on_cancelPing_clicked()
{
    if (pingProc->state() != QProcess::NotRunning)
        pingProc->kill();
    cancelPing->setEnabled(false);
}

void MainWindow::on_cancelTrace_clicked()
{
    if (traceProc->state() != QProcess::NotRunning)
        traceProc->kill();
    cancelTrace->setEnabled(false);
}

void MainWindow::on_clearPingOutput_clicked() { pingOutputEdit->clear(); }

void MainWindow::hwListToClipboard()
{
    if (hwList->currentItem() != nullptr) {
        auto *clipboard = QApplication::clipboard();
        auto *item = hwList->currentItem();
        clipboard->setText(
            tr("Interface: %1").arg(item->text(Col::Interface)) + "\t" + tr("Driver: %1").arg(item->text(Col::Driver))
            + "\t" + tr("Description: %1").arg(item->text(Col::Description)) + "\t"
            + tr("Product: %1").arg(item->text(Col::Product)) + "\t" + tr("Vendor: %1").arg(item->text(Col::Vendor))
            + "\t" + tr("Enabled: %1").arg(item->data(Col::Enabled, Qt::UserRole).toString()));
    }
}

void MainWindow::hwListFullToClipboard()
{
    if (hwList->topLevelItemCount() > 0) {
        auto *clipboard = QApplication::clipboard();
        QString list;
        for (QTreeWidgetItemIterator it(hwList); (*it) != nullptr; ++it) {
            list += tr("Interface: %1").arg((*it)->text(Col::Interface)) + "\t"
                    + tr("Driver: %1").arg((*it)->text(Col::Driver)) + "\t"
                    + tr("Description: %1").arg((*it)->text(Col::Description)) + "\t"
                    + tr("Product: %1").arg((*it)->text(Col::Product)) + "\t"
                    + tr("Vendor: %1").arg((*it)->text(Col::Vendor)) + "\t"
                    + tr("Enabled: %1").arg((*it)->data(Col::Enabled, Qt::UserRole).toString()) + "\n";
        }
        clipboard->setText(list);
    }
}

void MainWindow::linuxDrvListToClipboard()
{
    if (linuxDrvList->currentRow() != -1) {
        auto *clipboard = QApplication::clipboard();
        auto *currentElement = linuxDrvList->currentItem();
        clipboard->setText(currentElement->text());
    }
}

void MainWindow::linuxDrvListFullToClipboard()
{
    if (hwList->topLevelItemCount() > 0) {
        auto *clipboard = QApplication::clipboard();
        QString elementList = QLatin1String("");
        for (int i = 0; i < linuxDrvList->count(); i++) {
            auto *currentElement = linuxDrvList->item(i);
            elementList += currentElement->text() + "\n";
        }
        clipboard->setText(elementList);
    }
}

void MainWindow::windowsDrvListToClipboard()
{
    if (linuxDrvList->currentRow() != -1) {
        auto *clipboard = QApplication::clipboard();
        auto *currentElement = windowsDrvList->currentItem();
        clipboard->setText(currentElement->text());
    }
}

void MainWindow::windowsDrvListFullToClipboard()
{
    if (hwList->topLevelItemCount() > 0) {
        auto *clipboard = QApplication::clipboard();
        QString elementList = QLatin1String("");
        for (int i = 0; i < windowsDrvList->count(); i++) {
            QListWidgetItem *currentElement = windowsDrvList->item(i);
            elementList += currentElement->text() + "\n";
        }
        clipboard->setText(elementList);
    }
}

void MainWindow::showContextMenuForHw(QPoint pos)
{
    QMenu contextMenu(this);
    auto *copyAction = new QAction(tr("&Copy"), this);
    connect(copyAction, &QAction::triggered, this, &MainWindow::hwListToClipboard);
    copyAction->setShortcut(tr("Ctrl+C"));
    auto *copyAllAction = new QAction(tr("Copy &All"), this);
    connect(copyAllAction, &QAction::triggered, this, &MainWindow::hwListFullToClipboard);
    copyAllAction->setShortcut(tr("Ctrl+A"));
    contextMenu.addAction(copyAction);
    contextMenu.addAction(copyAllAction);
    contextMenu.exec(hwList->mapToGlobal(pos));
}

void MainWindow::showContextMenuForLinuxDrv(QPoint pos)
{
    QMenu contextMenu(this);
    auto *copyAction = new QAction(tr("&Copy"), this);
    connect(copyAction, &QAction::triggered, this, &MainWindow::linuxDrvListToClipboard);
    copyAction->setShortcut(tr("Ctrl+C"));
    auto *copyAllAction = new QAction(tr("Copy &All"), this);
    connect(copyAllAction, &QAction::triggered, this, &MainWindow::linuxDrvListFullToClipboard);
    copyAllAction->setShortcut(tr("Ctrl+A"));
    contextMenu.addAction(copyAction);
    contextMenu.addAction(copyAllAction);
    contextMenu.exec(linuxDrvList->mapToGlobal(pos));
}

void MainWindow::showContextMenuForWindowsDrv(QPoint pos)
{
    QMenu contextMenu(this);
    auto *copyAction = new QAction(tr("&Copy"), this);
    connect(copyAction, &QAction::triggered, this, &MainWindow::windowsDrvListToClipboard);
    copyAction->setShortcut(tr("Ctrl+C"));
    auto *copyAllAction = new QAction(tr("Copy &All"), this);
    connect(copyAllAction, &QAction::triggered, this, &MainWindow::windowsDrvListFullToClipboard);
    copyAllAction->setShortcut(tr("Ctrl+A"));
    contextMenu.addAction(copyAction);
    contextMenu.addAction(copyAllAction);
    contextMenu.exec(windowsDrvList->mapToGlobal(pos));
}

void MainWindow::on_clearTraceOutput_clicked() { tracerouteOutputEdit->clear(); }

void MainWindow::writeTraceOutput()
{
    const auto bytes = traceProc->readAllStandardOutput();
    const QStringList lines = QString(bytes).split(QStringLiteral("\n"));
    for (const QString &line : lines)
        if (!line.isEmpty())
            tracerouteOutputEdit->append(line);
}

void MainWindow::tracerouteFinished() { cancelTrace->setEnabled(false); }

void MainWindow::on_tracerouteButton_clicked()
{
    QString statusl = cmd.getCmdOut(QStringLiteral("dpkg -s traceroute | grep ^Status"));
    if (statusl != QLatin1String("Status: install ok installed")) {
        if (internetConnection) {
            setCursor(QCursor(Qt::WaitCursor));
            int ret = QMessageBox::information(this, tr("Traceroute not installed"),
                                               tr("Traceroute is not installed, do you want to install it now?"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (ret == QMessageBox::Yes) {
                QProcess::execute(QStringLiteral("apt-get"), {"install", "-qq", "traceroute"});
                setCursor(QCursor(Qt::ArrowCursor));
                statusl = cmd.getCmdOut(QStringLiteral("dpkg -s traceroute | grep ^Status"));
                if (statusl != QLatin1String("Status: install ok installed")) {
                    QMessageBox::critical(this, tr("Traceroute hasn't been installed"),
                                          tr("Traceroute cannot be installed. This may mean you are using the LiveCD "
                                             "or you are unable to reach the software repository,"),
                                          QMessageBox::Ok);
                } else {
                    setCursor(QCursor(Qt::ArrowCursor));
                    return;
                }
            }
        } else {
            QMessageBox::critical(this, tr("Traceroute not installed"),
                                  tr("Traceroute is not installed and no Internet connection could be detected so it "
                                     "cannot be installed"),
                                  QMessageBox::Ok);
            return;
        }
    }
    if (tracerouteHostEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("No destination host"), tr("Please fill in the destination host field"),
                                 QMessageBox::Ok);
    } else {
        setCursor(QCursor(Qt::WaitCursor));

        QString program = QStringLiteral("traceroute");
        QStringList arguments;
        arguments << tracerouteHostEdit->text() << QStringLiteral("-m %1").arg(traceHopsNumber->value());

        if (traceProc->state() != QProcess::NotRunning)
            traceProc->kill();

        traceProc->start(program, arguments);
        disconnect(traceProc, &QProcess::readyReadStandardOutput, nullptr, nullptr);
        connect(traceProc, &QProcess::readyReadStandardOutput, this, &MainWindow::writeTraceOutput);
        disconnect(traceProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
        connect(traceProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                &MainWindow::tracerouteFinished);

        clearTraceOutput->setEnabled(true);
        cancelTrace->setEnabled(true);
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void MainWindow::writePingOutput()
{
    const auto bytes = pingProc->readAllStandardOutput();
    const QStringList lines = QString(bytes).split(QStringLiteral("\n"));
    for (const QString &line : lines)
        if (!line.isEmpty())
            pingOutputEdit->append(line);
}

void MainWindow::pingFinished() { cancelPing->setEnabled(false); }

void MainWindow::on_pingButton_clicked()
{
    if (pingHostEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("No destination host"), tr("Please fill in the destination host field"),
                                 QMessageBox::Ok);
    } else {
        setCursor(QCursor(Qt::WaitCursor));
        QString program = QStringLiteral("ping");
        QStringList arguments;
        arguments << QStringLiteral("-c %1").arg(pingPacketNumber->value()) << QStringLiteral("-W 5")
                  << pingHostEdit->text();

        if (pingProc->state() != QProcess::NotRunning)
            pingProc->kill();
        pingProc->start(program, arguments);
        disconnect(pingProc, &QProcess::readyReadStandardOutput, nullptr, nullptr);
        connect(pingProc, &QProcess::readyReadStandardOutput, this, &MainWindow::writePingOutput);
        disconnect(pingProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
        connect(pingProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                &MainWindow::pingFinished);

        // QStringList vals = getCmdOuts(QString("ping -c 4 -W 5 %1").arg(pingHostEdit->text()));
        // pingOutputEdit->append(vals.join("\n"));
        clearPingOutput->setEnabled(true);
        cancelPing->setEnabled(true);
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void MainWindow::show()
{
    QDialog::show();
    refresh();
}

void MainWindow::on_hwDiagnosePushButton_clicked()
{
    hwList->clear();
    const auto &jsonDoc = QJsonDocument::fromJson(
        cmd.getCmdOut(QStringLiteral("lshw -disable IDE -disable SCSI -class network -json")).toUtf8());
    const auto &jsonArray = jsonDoc.array();
    QString desc;
    QString vendor;
    QString name;
    QString disabled;
    QString version;
    QString product;
    QString driver;
    for (const auto &item : jsonArray) {
        desc = item[QStringLiteral("description")].toString();
        vendor = item[QStringLiteral("vendor")].toString();
        name = item[QStringLiteral("logicalname")].toString();
        bool isDisabled = item[QStringLiteral("disabled")].toBool();
        disabled = isDisabled ? QStringLiteral("\tdisabled") : QStringLiteral("\tenabled");
        version = item[QStringLiteral("version")].toString();
        version = !version.isEmpty() ? " (rev " + version + ")" : QLatin1String("");
        product = item[QStringLiteral("product")].toString();
        const auto obj = item[QStringLiteral("configuration")].toObject();
        driver = obj.value(QStringLiteral("driver")).toString();
        QIcon icon;
        if (desc == QLatin1String("Ethernet interface"))
            icon = QIcon::fromTheme(QStringLiteral("network-card"));
        else if (desc == QLatin1String("Network interface"))
            icon = QIcon::fromTheme(QStringLiteral("network-workgroup"));
        else
            icon = QIcon::fromTheme(QStringLiteral("network-server-database"));
        auto *tree_item = new QTreeWidgetItem();
        tree_item->setIcon(
            Col::Enabled,
            isDisabled ? QIcon::fromTheme(QStringLiteral("emblem-unreadable"), QIcon(":/icons/emblem-unreadable.svg"))
                       : QIcon::fromTheme(QStringLiteral("emblem-checked"), QIcon(":/icons/emblem-checked.svg")));
        tree_item->setData(Col::Enabled, Qt::UserRole, !isDisabled);

        tree_item->setIcon(Col::Interface, icon);
        tree_item->setText(Col::Interface, name);
        tree_item->setText(Col::Description, desc);
        tree_item->setText(Col::Product, product + version);
        tree_item->setText(Col::Vendor, vendor);
        tree_item->setText(Col::Driver, driver);
        hwList->addTopLevelItem(tree_item);
    }
    for (int i = 0; i < hwList->columnCount(); ++i)
        hwList->resizeColumnToContents(i);
    checkWifiEnabled();
}

void MainWindow::on_linuxDrvList_currentRowChanged(int currentRow)
{
    if (currentRow != -1 && !linuxDrvList->currentItem()->text().contains(QLatin1String("---------"))) {
        linuxDrvBlockPushButton->setEnabled(true);
        linuxDrvUnload->setEnabled(loadedModules.contains(linuxDrvList->currentItem()->text()));
        linuxDrvLoad->setEnabled(unloadedModules.contains(linuxDrvList->currentItem()->text()));
        if (blockedModules.contains(linuxDrvList->currentItem()->text())
            && !loadedModules.contains(linuxDrvList->currentItem()->text()))
            linuxDrvLoad->setEnabled(true);
    } else {
        linuxDrvBlockPushButton->setEnabled(false);
        linuxDrvLoad->setEnabled(false);
        linuxDrvUnload->setEnabled(false);
    }
    updateDriverStatus();
}

void MainWindow::on_linuxDrvDiagnosePushButton_clicked()
{
    linuxDrvList->clear();
    loadedModules.clear();
    // QStringList queryResult = getCmdOuts("lsmod | grep -i net");
    const QStringList loadedKernelModules = cmd.getCmdOut(QStringLiteral("lsmod")).split(QStringLiteral("\n"));
    QStringList completeKernelNetModules
        = cmd.getCmdOut(QStringLiteral("find /lib/modules/$(uname -r)/kernel/drivers/net -name *.ko"))
              .split(QStringLiteral("\n"));
    completeKernelNetModules = completeKernelNetModules.replaceInStrings(QStringLiteral(".ko"), QLatin1String(""));
    completeKernelNetModules
        = completeKernelNetModules.replaceInStrings(QRegularExpression("[\\w|\\.|-]*/"), QLatin1String(""));
    // Those three kernel modules are in the "misc" section we add them manually
    // To the filter list for convenience
    completeKernelNetModules << QStringLiteral("ndiswrapper");
    completeKernelNetModules << QStringLiteral("atl2");
    completeKernelNetModules << QStringLiteral("wl");
    for (const QString &mod : loadedKernelModules) {
        if (completeKernelNetModules.contains(mod.left(mod.indexOf(' '))))
            loadedModules.append(mod.left(mod.indexOf(' '))); // add to the QStringList of loadedModule
    }

    for (int i = 0; i < loadedModules.size(); ++i) {
        QString mod = loadedModules.at(i);
        if (i == 0)
            new QListWidgetItem("---------" + tr("Loaded Drivers") + "-------------", linuxDrvList);
        new QListWidgetItem(mod, linuxDrvList);
    }

    // list unloaded modules
    for (int i = 0; i < unloadedModules.size(); ++i) {
        QString mod = unloadedModules.at(i);
        if (i == 0)
            new QListWidgetItem("---------" + tr("Unloaded Drivers") + "-----------", linuxDrvList);
        auto *unloaded = new QListWidgetItem(mod, linuxDrvList);
        unloaded->setForeground(Qt::blue);
    }

    QFile inputBlockList(QStringLiteral("/etc/modprobe.d/blacklist.conf"));
    inputBlockList.open(QFile::ReadOnly | QFile::Text);

    QString driver;
    QString s;
    // add blocklisted modules to the list
    while (!inputBlockList.atEnd()) {
        if (inputBlockList.pos() == 0)
            new QListWidgetItem("---------" + tr("Blocked Drivers") + " --------", linuxDrvList);
        s = inputBlockList.readLine();
        QRegExp expr("^\\s*blacklist\\s*.*");
        if (expr.exactMatch(s)) {
            QString captured = expr.cap(0);
            captured.remove(QStringLiteral("blacklist"));
            driver = captured.trimmed();
            auto *blocklisted = new QListWidgetItem(driver, linuxDrvList);
            blocklisted->setForeground(Qt::red);
            blockedModules.append(driver);
        }
    }
    inputBlockList.close();

    // add blocklisted modules from /etc/modprobe.d/broadcom-sta-dkms.conf
    QFile inputBroadcomBlocklist(QStringLiteral("/etc/modprobe.d/broadcom-sta-dkms.conf"));
    inputBroadcomBlocklist.open(QFile::ReadOnly | QFile::Text);
    while (!inputBroadcomBlocklist.atEnd()) {
        if (inputBroadcomBlocklist.pos() == 0)
            new QListWidgetItem("---------" + tr("Blocked Broadcom Drivers") + "--------", linuxDrvList);
        s = inputBroadcomBlocklist.readLine();
        QRegExp expr("^\\s*blacklist\\s*.*");
        if (expr.exactMatch(s)) {
            QString captured = expr.cap(0);
            captured.remove(QStringLiteral("blacklist"));
            driver = captured.trimmed();
            auto *bloclisted = new QListWidgetItem(driver, linuxDrvList);
            bloclisted->setForeground(Qt::red);
            blockedModules.append(driver);
            broadcomModules.append(driver);
        }
    }
    inputBroadcomBlocklist.close();
}

void MainWindow::on_windowsDrvDiagnosePushButton_clicked()
{
    windowsDrvList->clear();
    if (!QFile::exists(QStringLiteral("/usr/sbin/ndiswrapper"))) {
        uninstallNdiswrapper->setVisible(false);
        QMessageBox::warning(this, windowTitle(), tr("Ndiswrapper is not installed"));
        return;
    }
    QStringList queryResult = cmd.getCmdOut(QStringLiteral("ndiswrapper -l 2>/dev/null")).split(QStringLiteral("\n"));

    if (queryResult.size() == 1 && queryResult.at(0).isEmpty())
        return;

    int i = 0;
    while (i < queryResult.size()) {
        const QString &currentElement = queryResult.at(i);
        QString label = currentElement.left(currentElement.indexOf(QLatin1String(":")));
        label.append(tr("driver installed"));
        // label = currentElement.remove(": ");
        if ((i + 1) < queryResult.size()) {
            if (!queryResult.at(i + 1).contains(QLatin1String(": driver installed"))) {
                const QString &installInfo = queryResult.at(i + 1);
                int infoPos = installInfo.indexOf(QRegExp(
                    R"([\d|A|B|C|D|E|F][\d|A|B|C|D|E|F][\d|A|B|C|D|E|F][\d|A|B|C|D|E|F]:[\d|A|B|C|D|E|F][\d|A|B|C|D|E|F][\d|A|B|C|D|E|F][\d|A|B|C|D|E|F])"));
                // device (14E4:4320) present (alternate driver: bcm43xx)
                if (infoPos != -1) {
                    label.append(tr(" and in use by "));
                    label.append(installInfo.midRef(infoPos, 9));
                }
                if (installInfo.contains(QLatin1String("alternate driver"))) {
                    infoPos = installInfo.lastIndexOf(QLatin1String(": "));
                    if (infoPos != -1) {
                        int strLen = installInfo.length();
                        label.append(tr(". Alternate driver: "));
                        QString s = installInfo.right(strLen - infoPos);
                        s.remove(QStringLiteral(")"));
                        s.remove(QStringLiteral(" "));
                        s.remove(QStringLiteral(":"));
                        label.append(s);
                    }
                }
                i++;
            }
        }
        new QListWidgetItem(label, windowsDrvList);
        i++;
    }
}

bool MainWindow::blockModule(const QString &module)
{
    QFile outputBlockList;
    if (!broadcomModules.contains(module))
        outputBlockList.setFileName(QStringLiteral("/etc/modprobe.d/blacklist.conf"));
    else
        outputBlockList.setFileName(QStringLiteral("/etc/modprobe.d/broadcom-sta-dkms.conf"));

    if (!outputBlockList.open(QFile::Append | QFile::Text))
        return false;

    outputBlockList.write(QStringLiteral("blacklist %1\n").arg(module).toUtf8());
    outputBlockList.close();

    if (removable(module))
        if (!removeModule(module))
            return false;
    loadedModules.removeAll(module);
    return true;
}

void MainWindow::on_linuxDrvBlockPushButton_clicked()
{
    if (linuxDrvList->currentRow() != -1) {
        QListWidgetItem *currentDriver = linuxDrvList->currentItem();
        QString driver = currentDriver->text();
        driver = driver.left(driver.indexOf(QLatin1String(" ")));
        if (driverBlocklisted) {
            QFile InputBlockList;
            QFile outputBlockList;
            if (broadcomModules.contains(driver)) {
                InputBlockList.setFileName(QStringLiteral("/etc/modprobe.d/broadcom-sta-dkms.conf"));
                outputBlockList.setFileName(QStringLiteral("/etc/modprobe.d/broadcom-sta-dkms.conf"));

            } else {
                InputBlockList.setFileName(QStringLiteral("/etc/modprobe.d/blacklist.conf"));
                outputBlockList.setFileName(QStringLiteral("/etc/modprobe.d/blacklist.conf"));
            }
            if (!InputBlockList.open(QFile::ReadOnly | QFile::Text))
                return;

            QString s;
            QString outputString(QLatin1String(""));
            while (!InputBlockList.atEnd()) {
                s = InputBlockList.readLine();
                QString expr = QStringLiteral("^\\s*(blacklist)\\s*(%1)\\s*").arg(driver);
                if (!s.contains(QRegExp(expr)))
                    outputString += s;
                outputBlockList.write(s.toUtf8());
            }
            InputBlockList.close();
            if (!outputBlockList.open(QFile::WriteOnly | QFile::Text))
                return;
            outputBlockList.write(outputString.toUtf8());
            outputBlockList.close();
            QMessageBox::information(this, tr("Driver removed from blocklist"), tr("Driver removed from blocklist."));
            loadModule(driver);
            driverBlocklisted = false;
            unloadedModules.removeAll(driver);
            blockedModules.removeAll(driver);
        } else if (blockModule(driver)) {
            QMessageBox::information(this, tr("Module blocked"), tr("Module blocked"));
            driverBlocklisted = true;
        }
    }
    on_linuxDrvDiagnosePushButton_clicked();
}

// load module
bool MainWindow::loadModule(const QString &module)
{
    QProcess::execute(QStringLiteral("service"), {"network-manager", "stop"});
    QProcess::execute(QStringLiteral("modprobe"),
                      {"cfg80211"}); // this has to get loaded and some drivers don't put it back correctly
    if (QProcess::execute(QStringLiteral("modprobe"), {module}) != 0) {
        // run depmod and try to load again
        QProcess::execute(QStringLiteral("depmod"), {});
        if (QProcess::execute(QStringLiteral("modprobe"), {module}) != 0) {
            QString msg = QObject::tr("Could not load ");
            msg += module;
            QMessageBox::information(this, windowTitle(), msg);
            QProcess::execute(QStringLiteral("pkill"), {"wpa_supplicant"});
            QProcess::execute(QStringLiteral("service"), {"network-manager", "start"});
            return false;
        }
    }
    if (!loadedModules.contains(module))
        loadedModules.append(module);
    QProcess::execute(QStringLiteral("pkill"), {"wpa_supplicant"});
    QProcess::execute(QStringLiteral("service"), {"network-manager", "start"});
    return true;
}

bool MainWindow::removable(const QString &module)
{
    return (QProcess::execute(QStringLiteral("modprobe"), {"-rn", module}) == 0);
}

bool MainWindow::removeModule(const QString &module)
{
    QProcess::execute(QStringLiteral("service"), {"network-manager", "stop"});
    if (QProcess::execute(QStringLiteral("modprobe"), {"-r", module}) != 0) {
        QString msg = QObject::tr("Could not unload ");
        msg += module;
        QMessageBox::information(this, windowTitle(), msg);
        QProcess::execute(QStringLiteral("service"), {"network-manager", "start"});
        return false;
    }
    QProcess::execute(QStringLiteral("service"), {"network-manager", "start"});
    return true;
}

bool MainWindow::removeStart(const QString &module)
{
    QFile modulesFile("/etc/modules");
    if (!modulesFile.open(QIODevice::ReadWrite | QIODevice::Text))
        return false;

    QString inputString;
    QString outputString;
    QTextStream in(&modulesFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        const QString expr = QStringLiteral("^\\s*(%1)\\s*").arg(module);
        if (!line.contains(QRegularExpression(expr)))
            outputString += line + "\n";
    }

    QTextStream out(&modulesFile);
    out << outputString;
    modulesFile.close();
    return true;
}

bool MainWindow::installModule(const QString &module)
{
    if (!loadModule(module))
        return false;
    QFile outputModules(QStringLiteral("/etc/modules"));
    if (!outputModules.open(QFile::Append | QFile::Text))
        return false;
    outputModules.write(QStringLiteral("%1\n").arg(module).toUtf8());
    outputModules.close();
    return true;
}

// run apt-get update and at the end start installNDIS
void MainWindow::on_installNdiswrapper_clicked()
{
    setCursor(QCursor(Qt::BusyCursor));
    if (installProc->state() != QProcess::NotRunning)
        installProc->kill();
    installProc->start(QStringLiteral("apt-get"), {"update"});
    installOutputEdit->clear();
    installOutputEdit->show();
    const int height = 600;
    const int width = 800;
    installOutputEdit->resize(width, height);
    // center output window
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    const int x = (screenGeometry.width() - installOutputEdit->width()) / 2;
    const int y = (screenGeometry.height() - installOutputEdit->height()) / 2;
    installOutputEdit->move(x, y);
    // hide main window
    this->hide();
    installOutputEdit->raise();
    disconnect(installProc, &QProcess::readyReadStandardOutput, nullptr, nullptr);
    connect(installProc, &QProcess::readyReadStandardOutput, this, &MainWindow::writeInstallOutput);
    disconnect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
    connect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &MainWindow::aptUpdateFinished);
}

void MainWindow::on_uninstallNdiswrapper_clicked()
{
    setCursor(QCursor(Qt::BusyCursor));
    if (installProc->state() != QProcess::NotRunning)
        installProc->kill();
    removeModule(QStringLiteral("ndiswrapper"));
    installProc->start(QStringLiteral("apt-get"),
                       {"purge", "-y", "ndiswrapper-utils-1.9", "ndiswrapper-dkms", "ndiswrapper-common"});
    installOutputEdit->clear();
    installOutputEdit->show();
    const int height = 600;
    const int width = 800;
    installOutputEdit->resize(height, width);
    // center output window
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    const int x = (screenGeometry.width() - installOutputEdit->width()) / 2;
    const int y = (screenGeometry.height() - installOutputEdit->height()) / 2;
    installOutputEdit->move(x, y);
    // hide main window
    this->hide();
    installOutputEdit->raise();
    disconnect(installProc, &QProcess::readyReadStandardOutput, nullptr, nullptr);
    connect(installProc, &QProcess::readyReadStandardOutput, this, &MainWindow::writeInstallOutput);
    disconnect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
    connect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &MainWindow::uninstallNdisFinished);
}

void MainWindow::aptUpdateFinished()
{
    if (installProc->state() != QProcess::NotRunning)
        installProc->kill();
    installProc->start(QStringLiteral("apt-get"), {"install", "-y", "ndiswrapper-utils-1.9", "ndiswrapper-dkms"});
    disconnect(installProc, &QProcess::readyReadStandardOutput, nullptr, nullptr);
    connect(installProc, &QProcess::readyReadStandardOutput, this, &MainWindow::writeInstallOutput);
    disconnect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), nullptr, nullptr);
    connect(installProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &MainWindow::installFinished);
}

void MainWindow::installFinished(int errorCode)
{
    installOutputEdit->close();
    this->show();
    setCursor(QCursor(Qt::ArrowCursor));
    if (errorCode == 0) {
        if (installModule(QStringLiteral("ndiswrapper"))) {
            uninstallNdiswrapper->setVisible(true);
            QMessageBox::information(this, windowTitle(), tr("Installation successful"));
        } else {
            QMessageBox::information(this, windowTitle(), tr("Error detected, could not compile ndiswrapper driver."));
        }
    } else {
        QMessageBox::warning(this, windowTitle(), tr("Error detected, could not install ndiswrapper."));
    }
}

void MainWindow::uninstallNdisFinished(int errorCode)
{
    installOutputEdit->close();
    this->show();
    setCursor(QCursor(Qt::ArrowCursor));
    if (errorCode == 0)
        removeStart(QStringLiteral("ndiswrapper"));
    else
        QMessageBox::warning(this, windowTitle(), tr("Error encountered while removing Ndiswrapper"));
}

void MainWindow::writeInstallOutput()
{
    const auto bytes = installProc->readAllStandardOutput();
    const QStringList lines = QString(bytes).split(QStringLiteral("\n"));

    for (const QString &line : lines)
        if (!line.isEmpty())
            installOutputEdit->append(line);
}

void MainWindow::updateDriverStatus()
{
    driverBlocklisted = false;
    QFile InputBlockList(QStringLiteral("/etc/modprobe.d/blacklist.conf"));
    QFile inputBroadcomBlocklist(QStringLiteral("/etc/modprobe.d/broadcom-sta-dkms.conf"));
    InputBlockList.open(QFile::ReadOnly | QFile::Text);
    inputBroadcomBlocklist.open(QFile::ReadOnly | QFile::Text);

    QString driver;

    if (linuxDrvList->currentRow() != -1) {
        QListWidgetItem *currentDriver = linuxDrvList->currentItem();
        driver = currentDriver->text();
        driver = driver.left(driver.indexOf(QLatin1String(" ")));
    }

    QString s;
    while (!InputBlockList.atEnd()) {
        s = InputBlockList.readLine();
        QString expr = QStringLiteral("^\\s*(blacklist)\\s*(%1)\\s*").arg(driver);
        if (s.contains(QRegularExpression(expr))) {
            driverBlocklisted = true;
            break;
        }
    }
    while (!inputBroadcomBlocklist.atEnd()) {
        s = inputBroadcomBlocklist.readLine();
        QString expr = QStringLiteral("^\\s*(blacklist)\\s*(%1)\\s*").arg(driver);
        if (s.contains(QRegularExpression(expr))) {
            driverBlocklisted = true;
            break;
        }
    }
    if (driverBlocklisted) {
        linuxDrvBlockPushButton->setText(tr("Unblock Driver"));
        linuxDrvBlockPushButton->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
    } else {
        linuxDrvBlockPushButton->setText(tr("Block Driver"));
        linuxDrvBlockPushButton->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));
    }
    InputBlockList.close();
}

bool MainWindow::checkSysFileExists(const QDir &searchPath, const QString &fileName, Qt::CaseSensitivity cs)
{
    const QStringList fileList = searchPath.entryList(QStringList() << QStringLiteral("*.SYS"));
    const auto it = std::find_if(fileList.cbegin(), fileList.cend(),
                                 [&](const QString &file) { return file.contains(fileName, cs); });
    return it != fileList.cend();
}

bool MainWindow::checkWifiAvailable()
{
    if (cmd.run(QStringLiteral("lspci | grep -Ei 'wireless|wifi' || lspci | grep -Ei 'wireless|wifi'"))) {
        groupWifi->show();
        return true;
    } else {
        groupWifi->hide();
        return false;
    }
}

bool MainWindow::checkWifiEnabled()
{
    hwUnblock->hide();
    if (cmd.getCmdOut(QStringLiteral("nmcli -t --fields WIFI r")) == QLatin1String("enabled")) {
        labelWifi->setText(tr("enabled"));
        return true;
    } else if (cmd.getCmdOut(QStringLiteral("nmcli -t --fields WIFI-HW r")) == QLatin1String("enabled")) {
        labelWifi->setText(tr("disabled"));
        hwUnblock->show();
    } else {
        labelWifi->setText(tr("WiFi hardware switch is off"));
    }
    return false;
}

void MainWindow::on_windowsDrvAddPushButton_clicked()
{
    QString infFileName
        = QFileDialog::getOpenFileName(this, tr("Locate the Windows driver you want to add"), QStringLiteral("/home"),
                                       tr("Windows installation information file (*.inf)"));
    if (!infFileName.isEmpty()) {
        // Search in the inf file the name of the .sys file
        QFile infFile(infFileName);
        infFile.open(QFile::ReadOnly | QFile::Text);

        QString s;
        bool found = false;
        bool exist = false;
        QStringList foundSysFiles;
        QDir sysDir(infFileName);
        sysDir.cdUp();
        while ((!infFile.atEnd())) {
            s = infFile.readLine();
            if (s.contains(QLatin1String("ServiceBinary"), Qt::CaseInsensitive)) {
                s = s.right(s.length() - s.lastIndexOf('\\'));
                s = s.remove('\\');
                s = s.remove('\n');
                found = true;
                if (this->checkSysFileExists(sysDir, s, Qt::CaseInsensitive))
                    exist = true;
                else
                    foundSysFiles << s;
            }
        }
        infFile.close();

        if (found) {
            if (!exist) {
                QMessageBox::warning(
                    this, QString(tr("*.sys file not found")),
                    tr("The *.sys files must be in the same location as the *.inf file. %1 cannot be found")
                        .arg(foundSysFiles.join(QStringLiteral(", "))));
            } else {
                QString cmd_str = QStringLiteral("ndiswrapper -i %1").arg(infFileName);
                cmd.run(cmd_str);
                cmd_str = QStringLiteral("ndiswrapper -ma");
                cmd.run(cmd_str);
                on_windowsDrvDiagnosePushButton_clicked();
            }
        } else {
            QMessageBox::critical(
                this, QString(tr("sys file reference not found")).arg(infFile.fileName()),
                tr("The sys file for the given driver cannot be determined after parsing the inf file"));
        }
    }
}

void MainWindow::on_windowsDrvList_currentRowChanged(int row) { windowsDrvRemovePushButton->setEnabled(row != -1); }

void MainWindow::on_windowsDrvRemovePushButton_clicked()
{
    if (windowsDrvList->currentRow() != -1) {
        QListWidgetItem *currentDriver = windowsDrvList->currentItem();
        QString driver = currentDriver->text();
        QString cmd_str = QStringLiteral("ndiswrapper -r %1").arg(driver.left(driver.indexOf(QLatin1String(" "))));
        cmd.run(cmd_str);
        QMessageBox::information(this, windowTitle(), tr("Ndiswrapper driver removed."));
        on_windowsDrvDiagnosePushButton_clicked();
    }
}

void MainWindow::on_generalHelpPushButton_clicked()
{
    const QString url = QStringLiteral("/usr/share/doc/mx-network-assistant/mx-network-assistant.html");
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::on_tabWidget_currentChanged() { refresh(); }

void MainWindow::on_hwUnblock_clicked()
{
    if (QProcess::execute(QStringLiteral("rfkill"), {"unblock", "wlan", "wifi"}) != 0)
        QMessageBox::warning(this, windowTitle(),
                             tr("Could not unlock devices.\nWiFi device(s) might already be unlocked."));
    else
        QMessageBox::information(this, windowTitle(), tr("WiFi devices unlocked."));
    checkWifiEnabled();
}

// close but do not apply
void MainWindow::on_buttonCancel_clicked() { close(); }

void MainWindow::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(this->windowTitle()),
        "<p align=\"center\"><b><h2>" + this->windowTitle() + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + VERSION + "</p><p align=\"center\"><h3>"
            + tr("Program for troubleshooting and configuring network for MX Linux")
            + R"(</h3></p><p align="center"><a href="http://mxlinux.org">http://mxlinux.org</a><br /></p><p align="center">)"
            + tr("Copyright (c) MEPIS LLC and MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-network-assistant/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

QString MainWindow::getIP() { return cmd.getCmdOut(QStringLiteral("wget -qO - icanhazip.com")); }

QString MainWindow::getIPfromRouter()
{
    return cmd.getCmdOut(QStringLiteral("ip a | awk '/127.0.0.1/ {next} /inet / {print $2}' | cut -d/ -f1"));
}

void MainWindow::on_linuxDrvLoad_clicked()
{
    if (linuxDrvList->currentRow() != -1) {
        QListWidgetItem *currentDriver = linuxDrvList->currentItem();
        QString driver = currentDriver->text();
        driver = driver.left(driver.indexOf(QLatin1String(" ")));
        if (loadModule(driver)) {
            unloadedModules.removeAll(driver);
            QMessageBox::information(this, tr("Driver loaded successfully"), tr("Driver loaded successfully"));
            on_linuxDrvDiagnosePushButton_clicked();
        }
    }
}

void MainWindow::on_linuxDrvUnload_clicked()
{
    if (linuxDrvList->currentRow() != -1) {
        QListWidgetItem *currentDriver = linuxDrvList->currentItem();
        QString driver = currentDriver->text();
        driver = driver.left(driver.indexOf(QLatin1String(" ")));
        if ((removable(driver)) && !unloadedModules.contains(driver)) {
            if (removeModule(driver)) {
                unloadedModules.append(driver);
                QMessageBox::information(this, tr("Driver unloaded successfully"), tr("Driver unloaded successfully"));
                on_linuxDrvDiagnosePushButton_clicked();
            }
        }
    }
}

void MainWindow::on_pushEnable_clicked()
{
    pushEnable->setEnabled(false);
    pushDisable->setEnabled(false);
    hwDiagnosePushButton->setEnabled(false);
    cmd.run("ip link set " + hwList->currentItem()->text(Col::Interface) + " up");
    refresh();
}

void MainWindow::on_pushDisable_clicked()
{
    pushEnable->setEnabled(false);
    pushDisable->setEnabled(false);
    hwDiagnosePushButton->setEnabled(false);
    cmd.run("ip link set " + hwList->currentItem()->text(Col::Interface) + " down");
    refresh();
}

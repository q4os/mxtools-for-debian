/**********************************************************************
 *  mainwindow.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-select-sound.
 *
 * mx-select-sound is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-select-sound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-select-sound.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>

#include "about.h"
#include "cardutils.h"

#ifndef VERSION
    #define VERSION "?.?.?.?"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::MainWindow)
{
    qDebug().noquote() << QCoreApplication::applicationName() << "version:" << VERSION;
    ui->setupUi(this);
    setConnections();
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    if (ui->pushApply->icon().isNull()) {
        ui->pushApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    setWindowTitle(tr("MX Select Sound"));

    m_pipewire = CardUtils::isPipewireRunning();
    updateAudioSystemLabels();
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateAudioSystemLabels()
{
    if (m_pipewire) {
        qDebug() << "Audio system: PipeWire";
        ui->label->setText(tr("<html><head/><body><p>If your computer has more than one audio output, "
                              "you can use this application to select the default. "
                              "Audio system: <b>PipeWire</b><br/></p></body></html>"));
        ui->label1->setText(tr("<html><head/><body><p>Your current default audio device is:</p></body></html>"));
        ui->label_2->setText(tr("<html><head/><body><p>Select another audio device if necessary:</p></body></html>"));
    } else {
        qDebug() << "Audio system: ALSA";
        ui->label->setText(tr("<html><head/><body><p>If your computer has more than one sound card, "
                              "you can use this application to select the one to be default. "
                              "<br/></p></body></html>"));
        ui->label1->setText(tr("<html><head/><body><p>Your current default sound card is:</p></body></html>"));
        ui->label_2->setText(tr("<html><head/><body><p>Select another sound card if necessary:</p></body></html>"));
    }
}

// Get the list of sound cards
QStringList MainWindow::listCards()
{
    if (m_pipewire) {
        QProcess proc;
        proc.start(QStringLiteral("pactl"), {QStringLiteral("list"), QStringLiteral("sinks")});
        if (!proc.waitForFinished(5000) || proc.exitCode() != 0) {
            qDebug() << "Could not query PipeWire audio devices; falling back to ALSA";
            m_pipewire = false;
            m_currentPipewireSink.clear();
            updateAudioSystemLabels();
        } else {
            const auto sinks = CardUtils::parsePipewireSinks(QString::fromUtf8(proc.readAllStandardOutput()));
            if (sinks.isEmpty()) {
                QMessageBox::critical(this, tr("MX Select Sound"), tr("No audio devices were found."));
                return {};
            }
            QStringList baseLabels;
            for (const auto &sink : sinks) {
                baseLabels << (sink.description.isEmpty() ? sink.name : sink.description);
            }

            QStringList descriptions;
            for (int i = 0; i < sinks.size(); i++) {
                QString label = baseLabels.at(i);
                if (baseLabels.count(label) > 1)
                    label = QStringLiteral("%1 (%2)").arg(label, sinks.at(i).name);
                ui->comboBox->addItem(label, sinks.at(i).name);
                descriptions << label;
            }
            return descriptions;
        }
    }

    QStringList card_list;
    QFile file(QStringLiteral("/proc/asound/cards"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        card_list = CardUtils::parseCards(stream.readAll());
    }
    if (card_list.isEmpty()) {
        QMessageBox::critical(this, tr("MX Select Sound"), tr("No sound cards/devices were found."));
    } else {
        ui->comboBox->addItems(card_list);
    }
    return card_list;
}

// Get default card
QString MainWindow::getDefault()
{
    if (m_pipewire) {
        QProcess proc;
        proc.start(QStringLiteral("pactl"), {QStringLiteral("get-default-sink")});
        if (proc.waitForFinished(2000) && proc.exitCode() == 0) {
            const QString defaultName = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            for (int i = 0; i < ui->comboBox->count(); i++) {
                if (ui->comboBox->itemData(i).toString() == defaultName) {
                    ui->comboBox->setCurrentIndex(i);
                    const QString desc = ui->comboBox->itemText(i);
                    qDebug() << "Default audio device:" << desc;
                    m_currentPipewireSink = defaultName;
                    ui->pushTest->setEnabled(true);
                    ui->labelCurrent->setText(desc);
                    return desc;
                }
            }
        }
        qDebug() << "Default audio device: none";
        m_currentPipewireSink.clear();
        ui->pushTest->setEnabled(false);
        ui->labelCurrent->setText(tr("none"));
        return tr("none");
    }

    QString default_card = tr("none");

    QFile asoundrc(QDir::homePath() + QStringLiteral("/.asoundrc"));
    if (asoundrc.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&asoundrc);
        const QString prev_card = CardUtils::parseDefaultCard(stream.readAll());
        for (int i = 0; i < ui->comboBox->count(); i++) {
            if (prev_card == ui->comboBox->itemText(i).section(QStringLiteral(":"), 0, 0)) {
                default_card = prev_card;
                ui->comboBox->setCurrentIndex(i);
                break;
            }
        }
    }

    qDebug() << "Default sound card:" << default_card;
    ui->pushTest->setDisabled(default_card == tr("none"));
    ui->labelCurrent->setText(default_card);
    return default_card;
}

void MainWindow::setConnections()
{
    connect(ui->pushApply, &QPushButton::clicked, this, &MainWindow::pushApply_clicked);
    connect(ui->pushAbout, &QPushButton::clicked, this, &MainWindow::pushAbout_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &MainWindow::pushHelp_clicked);
    connect(ui->pushTest, &QPushButton::clicked, this, &MainWindow::pushTest_clicked);
}

void MainWindow::pushApply_clicked()
{
    if (m_pipewire) {
        const QString name = ui->comboBox->currentData().toString();
        if (name == m_currentPipewireSink)
            return;
        QProcess proc;
        proc.start(QStringLiteral("pactl"), {QStringLiteral("set-default-sink"), name});
        const bool finished = proc.waitForFinished(2000);
        if (!finished) {
            proc.kill();
            proc.waitForFinished(1000);
        }
        if (!finished || proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
            const QString error = QString::fromUtf8(proc.readAllStandardError()).trimmed();
            const QString message = !finished ? tr("Could not set default audio device: command timed out.")
                : error.isEmpty() ? tr("Could not set default audio device.")
                                  : tr("Could not set default audio device:\n%1").arg(error);
            QMessageBox::critical(this, tr("MX Select Sound"), message);
            return;
        }
        getDefault();
        return;
    }

    QString selected = ui->comboBox->currentText().section(QStringLiteral(":"), 0, 0);
    if (selected == ui->labelCurrent->text()) {
        QFile check(QDir::homePath() + QStringLiteral("/.asoundrc"));
        if (check.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString expected = QStringLiteral("defaults.pcm.!card %1\ndefaults.ctl.!card %1").arg(selected);
            if (QTextStream(&check).readAll().trimmed() == expected)
                return;
        }
    }
    QFile asoundrc;

    asoundrc.setFileName(QDir::homePath() + "/.asoundrc");
    if (asoundrc.open(QFile::WriteOnly | QFile::Text)) {
        asoundrc.write(QStringLiteral("defaults.pcm.!card %1\ndefaults.ctl.!card %1").arg(selected).toUtf8());
        asoundrc.close();
        getDefault();
    }
}

void MainWindow::pushAbout_clicked()
{
    displayAboutMsgBox(
        tr("About MX Select Sound"),
        "<p align=\"center\"><b><h2>" + tr("MX Select Sound") + "</h2></b></p><p align=\"center\">" + tr("Version: ")
            + VERSION + "</p><p align=\"center\"><h3>" + tr("Program for selecting the default sound card in MX Linux")
            + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
              "<p align=\"center\">"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-select-sound/license.html"), tr("%1 License").arg(windowTitle()));
}

void MainWindow::pushHelp_clicked()
{
    displayDoc(QStringLiteral("mx-select-sound.html"), tr("%1 Help").arg(tr("MX Select Sound")), true);
}

// Test default sound card
void MainWindow::pushTest_clicked()
{
    if (QStandardPaths::findExecutable(QStringLiteral("speaker-test")).isEmpty()) {
        QMessageBox::critical(this, tr("MX Select Sound"),
                              tr("speaker-test not found. Please install the alsa-utils package."));
        return;
    }
    ui->pushTest->setEnabled(false);
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::errorOccurred, this, [this, proc](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;

        QMessageBox::critical(this, tr("MX Select Sound"), tr("Could not play test sound."));
        ui->pushTest->setEnabled(true);
        proc->deleteLater();
    });
    connect(proc, &QProcess::finished, this, [this, proc](int exitCode) {
        if (exitCode != 0) {
            const QString err = proc->readAllStandardError().trimmed();
            const QString msg = err.isEmpty() ? tr("Could not play test sound.")
                                              : tr("Could not play test sound:\n%1").arg(err);
            QMessageBox::critical(this, tr("MX Select Sound"), msg);
        }
        ui->pushTest->setEnabled(true);
        proc->deleteLater();
    });
    proc->start(QStringLiteral("speaker-test"),
                {QStringLiteral("-c"), QStringLiteral("2"),
                 QStringLiteral("-t"), QStringLiteral("wav"),
                 QStringLiteral("-l"), QStringLiteral("4")});
}

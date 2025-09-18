/**********************************************************************
 *  PreviewDialog.cpp
 **********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "previewdialog.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QRegularExpression>
#include <QScreen>
#include <QStandardPaths>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

PreviewDialog::PreviewDialog(ConkyManager *manager, ConkyItem *selectedItem, QWidget *parent)
    : QDialog(parent),
      m_manager(manager),
      m_selectedItem(selectedItem),
      m_currentIndex(0),
      m_isGenerating(false),
      m_generationTimer(new QTimer(this))
{
    setWindowTitle(tr("Generate Preview"));
    setModal(true);
    resize(400, 300);

    setupUI();

    // Set default selection based on whether we have a selected item
    if (m_selectedItem) {
        m_generateCurrentRadio->setChecked(true);
    } else {
        m_generateMissingRadio->setChecked(true);
    }

    // Connect generation timer
    m_generationTimer->setSingleShot(true);
    connect(m_generationTimer, &QTimer::timeout, this, &PreviewDialog::onPreviewGenerated);
}

PreviewDialog::~PreviewDialog() = default;

void PreviewDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // Generate options group
    auto *generateGroupBox = new QGroupBox(tr("Generate preview images for"));
    auto *generateLayout = new QVBoxLayout(generateGroupBox);

    m_generateCurrentRadio = new QRadioButton(tr("Selected Widget"));
    m_generateMissingRadio = new QRadioButton(tr("All Widgets with Missing Previews"));
    m_generateAllRadio = new QRadioButton(tr("All Widgets (Overwrite Existing Images)"));

    // Disable current option if no item is selected
    if (!m_selectedItem) {
        m_generateCurrentRadio->setEnabled(false);
        m_generateCurrentRadio->setToolTip(tr("No conky is currently selected"));
    }

    generateLayout->addWidget(m_generateCurrentRadio);
    generateLayout->addWidget(m_generateMissingRadio);
    generateLayout->addWidget(m_generateAllRadio);

    // Options group
    auto *optionsGroupBox = new QGroupBox(tr("Options"));
    auto *optionsLayout = new QVBoxLayout(optionsGroupBox);

    m_highQualityCheck = new QCheckBox(tr("High quality images (PNG)"));
    m_highQualityCheck->setToolTip(tr("Generate preview images in PNG format instead of JPEG"));
    m_highQualityCheck->setChecked(true);

    optionsLayout->addWidget(m_highQualityCheck);

    // Progress section (initially hidden)
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);

    m_statusLabel = new QLabel;
    m_statusLabel->setVisible(false);
    m_statusLabel->setWordWrap(true);

    // Button layout
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    m_okButton = new QPushButton(tr("OK"));
    m_okButton->setIcon(QIcon::fromTheme("dialog-ok"));
    m_okButton->setDefault(true);

    m_cancelButton = new QPushButton(tr("Cancel"));
    m_cancelButton->setIcon(QIcon::fromTheme("dialog-cancel"));

    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    // Main layout
    mainLayout->addWidget(generateGroupBox);
    mainLayout->addWidget(optionsGroupBox);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_okButton, &QPushButton::clicked, this, &PreviewDialog::onAccepted);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void PreviewDialog::onAccepted()
{
    if (m_isGenerating) {
        return;
    }

    m_itemsToProcess = getItemsToProcess();

    if (m_itemsToProcess.isEmpty()) {
        QMessageBox::information(this, tr("No Items"), tr("No conky widgets need preview generation."));
        return;
    }

    // Show progress UI
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(m_itemsToProcess.size());
    m_progressBar->setValue(0);

    m_statusLabel->setVisible(true);
    m_statusLabel->setText(tr("Starting preview generation..."));

    // Disable OK button and change Cancel to Stop
    m_okButton->setEnabled(false);
    m_cancelButton->setText(tr("Stop"));

    m_currentIndex = 0;
    m_isGenerating = true;

    generatePreviews();
}

void PreviewDialog::generatePreviews()
{
    if (m_currentIndex >= m_itemsToProcess.size()) {
        onAllPreviewsComplete();
        return;
    }

    // Clean up before starting next preview (except for the first one)
    if (m_currentIndex > 0) {
        cleanupBeforeNextPreview();
    }

    QString itemPath = m_itemsToProcess[m_currentIndex];

    // Find the ConkyItem for this path
    ConkyItem *item = nullptr;
    for (ConkyItem *conkyItem : m_manager->conkyItems()) {
        if (conkyItem->filePath() == itemPath) {
            item = conkyItem;
            break;
        }
    }

    if (!item) {
        // Skip this item and continue
        m_currentIndex++;
        generatePreviews();
        return;
    }

    QString folderName = QFileInfo(item->directory()).fileName();
    m_statusLabel->setText(tr("Generating preview for: %1").arg(folderName));
    m_progressBar->setValue(m_currentIndex);

    generatePreviewForItem(item);

    // Use timer to allow UI updates and prevent blocking
    m_generationTimer->start(1000ms); // Increased delay to allow for cleanup
}

void PreviewDialog::generatePreviewForItem(ConkyItem *item)
{
    // First, ensure conky is copied to ~/.conky if needed
    ConkyItem *workingItem = ensureConkyInUserDir(item);
    if (!workingItem) {
        qDebug() << "Failed to prepare conky for preview generation:" << item->filePath();
        return;
    }

    QString previewPath = generatePreviewImage(workingItem);

    if (!previewPath.isEmpty()) {
        qDebug() << "Preview generated:" << previewPath;
    } else {
        qDebug() << "Failed to generate preview for:" << workingItem->filePath();
    }
}

QString PreviewDialog::generatePreviewImage(ConkyItem *item)
{
    if (!item) {
        return QString();
    }

    QString itemDir = item->directory();
    QString configPath = item->filePath();
    QString folderName = QFileInfo(itemDir).fileName();

    // Determine output format and filename based on conky basename
    QString extension = m_highQualityCheck->isChecked() ? "png" : "jpg";
    QString conkyBasename = QFileInfo(configPath).baseName();
    QString outputPath = itemDir + "/" + conkyBasename + "." + extension;

    // Remove existing preview if it exists
    if (QFile::exists(outputPath)) {
        QFile::remove(outputPath);
    }

    // Step 1: Start conky in background
    QProcess conkyProcess;
    conkyProcess.setWorkingDirectory(itemDir);

    QStringList conkyArgs;
    conkyArgs << "-c" << configPath;

    conkyProcess.start("conky", conkyArgs);

    if (!conkyProcess.waitForStarted(3000)) {
        qDebug() << "Failed to start conky for preview generation:" << configPath;
        return QString();
    }

    // Wait a bit for conky to initialize and draw
    QThread::msleep(2000);

    // Step 2: Find conky window
    QProcess wmctrlProcess;
    wmctrlProcess.start("wmctrl", QStringList() << "-l");

    if (!wmctrlProcess.waitForFinished(3000)) {
        conkyProcess.kill();
        conkyProcess.waitForFinished(1000);
        qDebug() << "Failed to run wmctrl to find conky window";
        return QString();
    }

    QString wmctrlOutput = wmctrlProcess.readAllStandardOutput();
    QStringList lines = wmctrlOutput.split('\n', Qt::SkipEmptyParts);

    QString windowId;
    QString conkyPattern = QString("conky (%1)").arg(configPath);

    for (const QString &line : lines) {
        if (line.contains("conky") && line.contains(folderName, Qt::CaseInsensitive)) {
            QStringList parts = line.split(QRegularExpression("\\s+"));
            if (!parts.isEmpty()) {
                windowId = parts[0];
                break;
            }
        }
    }

    if (windowId.isEmpty()) {
        // Try alternative approach - find any conky window
        for (const QString &line : lines) {
            if (line.contains("conky")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (!parts.isEmpty()) {
                    windowId = parts[0];
                    break;
                }
            }
        }
    }

    if (windowId.isEmpty()) {
        conkyProcess.kill();
        conkyProcess.waitForFinished(1000);
        qDebug() << "Could not find conky window for screenshot";
        return QString();
    }

    // Step 3: Get window geometry
    QProcess xwinfoProcess;
    xwinfoProcess.start("xwininfo", QStringList() << "-id" << windowId);

    if (!xwinfoProcess.waitForFinished(3000)) {
        conkyProcess.kill();
        conkyProcess.waitForFinished(1000);
        qDebug() << "Failed to get window info";
        return QString();
    }

    QString xwinfoOutput = xwinfoProcess.readAllStandardOutput();

    // Parse window geometry
    int width = 0, height = 0, x = 0, y = 0;
    QStringList infoLines = xwinfoOutput.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : infoLines) {
        if (line.contains("Width:")) {
            width = line.split(':')[1].trimmed().toInt();
        } else if (line.contains("Height:")) {
            height = line.split(':')[1].trimmed().toInt();
        } else if (line.contains("Absolute upper-left X:")) {
            x = line.split(':')[1].trimmed().toInt();
        } else if (line.contains("Absolute upper-left Y:")) {
            y = line.split(':')[1].trimmed().toInt();
        }
    }

    if (width <= 0 || height <= 0) {
        conkyProcess.kill();
        conkyProcess.waitForFinished(1000);
        qDebug() << "Invalid window geometry";
        return QString();
    }

    // Step 4: Take screenshot with desktop background
    QProcess screenshotProcess;
    QStringList screenshotArgs;
    screenshotArgs << "-window"
                   << "root"
                   << "-crop" << QString("%1x%2+%3+%4").arg(width).arg(height).arg(x).arg(y) << outputPath;
    screenshotProcess.start("import", screenshotArgs);
    bool screenshotSuccess = screenshotProcess.waitForFinished(5000) && screenshotProcess.exitCode() == 0;
    
    if (!screenshotSuccess) {
        qDebug() << "Screenshot process failed:" << screenshotProcess.readAllStandardError();
    }

    // Step 5: Clean up conky process
    conkyProcess.kill();
    conkyProcess.waitForFinished(1000);

    // Additional cleanup - make sure conky is really dead
    QProcess killallProcess;
    killallProcess.start("pkill", QStringList() << "-f" << configPath);
    killallProcess.waitForFinished(1000);

    if (screenshotSuccess && QFile::exists(outputPath)) {
        qDebug() << "Successfully generated preview:" << outputPath;
        return outputPath;
    } else {
        qDebug() << "Screenshot failed or file not created:" << outputPath;
        return QString();
    }
}

QStringList PreviewDialog::getItemsToProcess() const
{
    QStringList items;

    if (m_generateCurrentRadio->isChecked() && m_selectedItem) {
        items << m_selectedItem->filePath();
    } else if (m_generateMissingRadio->isChecked()) {
        for (ConkyItem *item : m_manager->conkyItems()) {
            QString conkyBasename = QFileInfo(item->filePath()).baseName();
            QString previewPath = item->directory() + "/" + conkyBasename + ".png";
            QString previewPathJpg = item->directory() + "/" + conkyBasename + ".jpg";

            if (!QFile::exists(previewPath) && !QFile::exists(previewPathJpg)) {
                items << item->filePath();
            }
        }
    } else if (m_generateAllRadio->isChecked()) {
        for (ConkyItem *item : m_manager->conkyItems()) {
            items << item->filePath();
        }
    }

    return items;
}

void PreviewDialog::onPreviewGenerated()
{
    if (!m_isGenerating) {
        return;
    }

    m_currentIndex++;
    generatePreviews();
}

void PreviewDialog::onAllPreviewsComplete()
{
    m_isGenerating = false;

    // Final cleanup after all previews are done
    cleanupBeforeNextPreview();

    m_progressBar->setValue(m_progressBar->maximum());
    m_statusLabel->setText(tr("Preview generation complete! Generated %1 previews.").arg(m_itemsToProcess.size()));

    // Re-enable OK button and restore Cancel text
    m_okButton->setEnabled(true);
    m_okButton->setText(tr("Close"));
    m_cancelButton->setVisible(false);

    QMessageBox::information(this, tr("Preview Generation Complete"),
                             tr("Successfully generated %1 preview images.").arg(m_itemsToProcess.size()));

    accept();
}

ConkyItem *PreviewDialog::ensureConkyInUserDir(ConkyItem *item)
{
    if (!item) {
        return nullptr;
    }

    QString userConkyPath = QDir::homePath() + "/.conky";
    QString itemDir = item->directory();

    // Check if already in user directory
    if (itemDir.startsWith(userConkyPath)) {
        return item; // Already in user directory
    }

    // Check if the item directory is writable
    QFileInfo dirInfo(itemDir);
    if (dirInfo.isWritable()) {
        return item; // Directory is writable, no need to copy
    }

    // Need to copy to ~/.conky
    QString copiedPath = m_manager->copyFolderToUserConky(itemDir);
    if (copiedPath.isEmpty()) {
        qDebug() << "Failed to copy conky folder to ~/.conky:" << itemDir;
        return nullptr;
    }

    // Add the new copy to the conky list (much faster than full rescan)
    m_manager->addConkyItemsFromDirectory(copiedPath);

    // Find the ConkyItem in the new location
    QString fileName = QFileInfo(item->filePath()).fileName();
    QString newFilePath = copiedPath + "/" + fileName;

    // Emit signals to refresh UI and select the new item
    emit conkyListNeedsRefresh();
    emit conkyItemNeedsSelection(newFilePath);

    // Look for the ConkyItem with this path
    for (ConkyItem *conkyItem : m_manager->conkyItems()) {
        if (conkyItem->filePath() == newFilePath) {
            return conkyItem;
        }
    }

    // If not found, create a temporary ConkyItem for preview generation
    return new ConkyItem(newFilePath, this);
}

void PreviewDialog::cleanupBeforeNextPreview()
{
    // Kill all running conky processes
    QProcess killProcess;
    killProcess.start("killall", QStringList() << "conky");
    killProcess.waitForFinished(3000);

    // Wait a bit for cleanup
    QThread::msleep(500);

    // Refresh desktop by triggering a small screen update
    // This helps clear any remaining conky artifacts
    QProcess refreshProcess;
    refreshProcess.start("xrefresh", QStringList());
    refreshProcess.waitForFinished(1000);

    // Alternative fallback if xrefresh is not available
    if (refreshProcess.exitCode() != 0) {
        // Use xdotool to simulate a small window movement to trigger refresh
        QProcess dotoolProcess;
        dotoolProcess.start("xdotool", QStringList() << "search"
                                                     << "--name"
                                                     << "Desktop"
                                                     << "windowmove"
                                                     << "0"
                                                     << "0");
        dotoolProcess.waitForFinished(1000);
    }
}

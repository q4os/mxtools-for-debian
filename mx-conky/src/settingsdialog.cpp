/**********************************************************************
 *  SettingsDialog.cpp
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

#include "settingsdialog.h"
#include <QDir>

SettingsDialog::SettingsDialog(ConkyManager *manager, QWidget *parent)
    : QDialog(parent),
      m_manager(manager)
{
    setupUI();
    loadPaths();
    loadSettings();
    onPathSelectionChanged();
}

void SettingsDialog::onAddPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Conky Directory"), QDir::homePath());
    if (!path.isEmpty()) {
        m_pathListWidget->addItem(path);
    }
}

void SettingsDialog::onRemovePath()
{
    int currentRow = m_pathListWidget->currentRow();
    if (currentRow >= 0) {
        delete m_pathListWidget->takeItem(currentRow);
    }
}

void SettingsDialog::onEditPath()
{
    QListWidgetItem *currentItem = m_pathListWidget->currentItem();
    if (!currentItem) {
        return;
    }

    QString currentPath = currentItem->text();
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Conky Directory"), currentPath);
    if (!path.isEmpty()) {
        currentItem->setText(path);
    }
}

void SettingsDialog::onPathSelectionChanged()
{
    bool hasSelection = m_pathListWidget->currentItem() != nullptr;
    m_removeButton->setEnabled(hasSelection);
    m_editButton->setEnabled(hasSelection);
}

void SettingsDialog::onAccepted()
{
    savePaths();
    saveSettings();
}

void SettingsDialog::setupUI()
{
    setWindowTitle(tr("MX Conky Settings"));
    setModal(true);
    resize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);

    // Create tab widget
    m_tabWidget = new QTabWidget;
    m_tabWidget->addTab(createPathsTab(), tr("Search Paths"));
    m_tabWidget->addTab(createAutostartTab(), tr("Autostart"));

    auto *dialogButtonLayout = new QHBoxLayout;
    auto *okButton = new QPushButton(tr("OK"));
    auto *cancelButton = new QPushButton(tr("Cancel"));

    okButton->setIcon(QIcon::fromTheme("dialog-ok"));
    cancelButton->setIcon(QIcon::fromTheme("dialog-cancel"));

    dialogButtonLayout->addStretch();
    dialogButtonLayout->addWidget(okButton);
    dialogButtonLayout->addWidget(cancelButton);

    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(dialogButtonLayout);

    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onAccepted);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::loadPaths()
{
    m_pathListWidget->clear();
    QStringList paths = m_manager->searchPaths();
    for (const QString &path : paths) {
        m_pathListWidget->addItem(path);
    }
}

void SettingsDialog::savePaths()
{
    QStringList newPaths;
    newPaths.reserve(m_pathListWidget->count());
    for (int i = 0; i < m_pathListWidget->count(); ++i) {
        QString path = m_pathListWidget->item(i)->text();
        if (!newPaths.contains(path)) {
            newPaths.append(path);
        }
    }

    QStringList oldPaths = m_manager->searchPaths();

    for (const QString &path : oldPaths) {
        if (!newPaths.contains(path)) {
            m_manager->removeSearchPath(path);
        }
    }

    for (const QString &path : newPaths) {
        if (!oldPaths.contains(path)) {
            m_manager->addSearchPath(path);
        }
    }

    m_manager->saveSettings();
}

void SettingsDialog::loadSettings()
{
    m_startupDelaySpinBox->setValue(m_manager->startupDelay());
    m_systemStartupCheckBox->setChecked(m_manager->isAutostartEnabled());
}

void SettingsDialog::saveSettings()
{
    m_manager->setStartupDelay(m_startupDelaySpinBox->value());
    m_manager->setAutostart(m_systemStartupCheckBox->isChecked());
}

QWidget *SettingsDialog::createPathsTab()
{
    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);

    auto *pathGroupBox = new QGroupBox(tr("Conky Search Paths"));
    auto *pathLayout = new QVBoxLayout(pathGroupBox);

    m_pathListWidget = new QListWidget;

    auto *buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton(tr("Add Path"));
    m_addButton->setIcon(QIcon::fromTheme("list-add"));

    m_removeButton = new QPushButton(tr("Remove Path"));
    m_removeButton->setIcon(QIcon::fromTheme("list-remove"));

    m_editButton = new QPushButton(tr("Edit Path"));
    m_editButton->setIcon(QIcon::fromTheme("edit"));

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addStretch();

    pathLayout->addWidget(m_pathListWidget);
    pathLayout->addLayout(buttonLayout);

    layout->addWidget(pathGroupBox);
    layout->addStretch();

    connect(m_addButton, &QPushButton::clicked, this, &SettingsDialog::onAddPath);
    connect(m_removeButton, &QPushButton::clicked, this, &SettingsDialog::onRemovePath);
    connect(m_editButton, &QPushButton::clicked, this, &SettingsDialog::onEditPath);
    connect(m_pathListWidget, &QListWidget::itemSelectionChanged, this, &SettingsDialog::onPathSelectionChanged);

    return widget;
}

QWidget *SettingsDialog::createAutostartTab()
{
    auto *widget = new QWidget;
    auto *layout = new QVBoxLayout(widget);

    auto *startupGroupBox = new QGroupBox(tr("Autostart Settings"));
    auto *startupLayout = new QVBoxLayout(startupGroupBox);

    m_systemStartupCheckBox = new QCheckBox(tr("Start conky at system startup"));

    auto *delayLayout = new QHBoxLayout;
    auto *delayLabel = new QLabel(tr("Startup delay (seconds):"));
    m_startupDelaySpinBox = new QSpinBox;
    m_startupDelaySpinBox->setRange(0, 300);
    m_startupDelaySpinBox->setSuffix(" s");

    delayLayout->addWidget(delayLabel);
    delayLayout->addWidget(m_startupDelaySpinBox);
    delayLayout->addStretch();

    startupLayout->addWidget(m_systemStartupCheckBox);
    startupLayout->addLayout(delayLayout);

    layout->addWidget(startupGroupBox);
    layout->addStretch();

    return widget;
}

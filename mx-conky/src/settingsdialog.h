/**********************************************************************
 *  SettingsDialog.h
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

#pragma once

#include "conkymanager.h"
#include <QCheckBox>
#include <QDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(ConkyManager *manager, QWidget *parent = nullptr);

private slots:
    void onAccepted();
    void onAddPath();
    void onEditPath();
    void onPathSelectionChanged();
    void onRemovePath();

private:
    ConkyManager *m_manager;
    QTabWidget *m_tabWidget;
    QListWidget *m_pathListWidget;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_removeButton;
    QSpinBox *m_startupDelaySpinBox;
    QCheckBox *m_systemStartupCheckBox;

    void loadPaths();
    void loadSettings();
    void savePaths();
    void saveSettings();
    void setupUI();
    QWidget *createPathsTab();
    QWidget *createAutostartTab();
};

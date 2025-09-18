/**********************************************************************
 *  ConkyWidget.h
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

#include "conkyitem.h"
#include "conkymanager.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

class ConkyItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConkyItemWidget(ConkyItem *item, QWidget *parent = nullptr);

    ConkyItem *conkyItem() const;
    void updateFromItem();

signals:
    void enabledChanged(ConkyItem *item, bool enabled);
    void editRequested(ConkyItem *item);
    void customizeRequested(ConkyItem *item);
    void runToggleRequested(ConkyItem *item);
    void deleteRequested(ConkyItem *item);

private slots:
    void onEnabledChanged(bool enabled);
    void onEditClicked();
    void onCustomizeClicked();
    void onRunToggleClicked();
    void onDeleteClicked();

private:
    ConkyItem *m_item;
    QCheckBox *m_enabledCheckBox;
    QPushButton *m_editButton;
    QPushButton *m_customizeButton;
    QPushButton *m_runToggleButton;
    QPushButton *m_deleteButton;
    QLabel *m_nameLabel;
    QLabel *m_pathLabel;
    QLabel *m_statusLabel;

    void setupUI();
    void updateRunToggleButton();
};

class ConkyListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConkyListWidget(ConkyManager *manager, QWidget *parent = nullptr);

    void refreshList();
    ConkyItem *selectedConkyItem() const;
    void selectConkyItem(const QString &filePath);

    // Filter and search methods
    void setStatusFilter(const QString &filter); // "All", "Running", "Stopped"
    void setSearchText(const QString &searchText);

signals:
    void itemSelectionChanged(ConkyItem *item);
    void editRequested(ConkyItem *item);
    void customizeRequested(ConkyItem *item);
    void deleteRequested(ConkyItem *item);

private slots:
    void onConkyItemsChanged();
    void onItemSelectionChanged();
    void onEnabledChanged(ConkyItem *item, bool enabled);
    void onEditRequested(ConkyItem *item);
    void onCustomizeRequested(ConkyItem *item);
    void onRunToggleRequested(ConkyItem *item);
    void onDeleteRequested(ConkyItem *item);

private:
    ConkyManager *m_manager;
    QTreeWidget *m_treeWidget;
    QLabel *m_countLabel;
    QHash<ConkyItem *, ConkyItemWidget *> m_itemWidgets;
    QHash<ConkyItem *, QTreeWidgetItem *> m_treeItems;
    bool m_hasAutoSelected = false;

    // Filter and search state
    QString m_statusFilter = "All";
    QString m_searchText;

    void setupUI();
    void addConkyItem(ConkyItem *item);
    void removeConkyItem(ConkyItem *item);
    void applyFilters();
    bool itemMatchesFilters(ConkyItem *item) const;
    void updateCountLabel();
};

class ConkyPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConkyPreviewWidget(QWidget *parent = nullptr);

    void setConkyItem(ConkyItem *item);
    void clearPreview();

signals:
    void previewImageLoaded(const QSize &imageSize);

private:
    ConkyItem *m_currentItem = nullptr;
    QLabel *m_descriptionLabel;
    QLabel *m_nameLabel;
    QLabel *m_pathLabel;
    QLabel *m_previewLabel;

    void setupUI();
    void updatePreview();
};

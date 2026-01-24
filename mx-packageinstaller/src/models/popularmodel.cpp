/**********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-packageinstaller.
 *
 * mx-packageinstaller is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-packageinstaller is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-packageinstaller.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "popularmodel.h"

#include <QFont>
#include <QtAlgorithms>

PopularModel::PopularModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

PopularModel::~PopularModel()
{
    clearNodes();
}

void PopularModel::clearNodes()
{
    qDeleteAll(m_nodes);
    m_nodes.clear();
    m_categoryNodes.clear();
    m_appNodes.clear();
}

void PopularModel::buildNodes()
{
    clearNodes();

    m_categoryNodes.resize(m_categories.size());
    m_appNodes.resize(m_categories.size());

    for (int catIdx = 0; catIdx < m_categories.size(); ++catIdx) {
        auto *categoryNode = new Node{catIdx, -1, true};
        m_nodes.append(categoryNode);
        m_categoryNodes[catIdx] = categoryNode;

        const auto &appIndices = m_categories[catIdx].appIndices;
        m_appNodes[catIdx].resize(appIndices.size());
        for (int row = 0; row < appIndices.size(); ++row) {
            auto *appNode = new Node{catIdx, row, false};
            m_nodes.append(appNode);
            m_appNodes[catIdx][row] = appNode;
        }
    }
}

int PopularModel::getAppIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return -1;
    }

    auto *node = static_cast<Node *>(index.internalPointer());
    if (!node || node->isCategory) {
        return -1;
    }

    if (node->categoryIdx < 0 || node->categoryIdx >= m_categories.size()) {
        return -1;
    }

    const auto &appIndices = m_categories[node->categoryIdx].appIndices;
    if (node->rowInCategory < 0 || node->rowInCategory >= appIndices.size()) {
        return -1;
    }

    int appIdx = appIndices[node->rowInCategory];
    if (appIdx < 0 || appIdx >= m_apps.size()) {
        return -1;
    }

    return appIdx;
}

bool PopularModel::isCategory(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    auto *node = static_cast<Node *>(index.internalPointer());
    return node && node->isCategory;
}

QModelIndex PopularModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    if (!parent.isValid()) {
        // Top-level item (category)
        if (row < 0 || row >= m_categories.size()) {
            return {};
        }
        return createIndex(row, column, m_categoryNodes.value(row));
    }

    // Child item (app)
    auto *parentNode = static_cast<Node *>(parent.internalPointer());
    if (!parentNode || !parentNode->isCategory) {
        return {}; // Parent must be a category
    }

    int categoryIdx = parentNode->categoryIdx;
    if (categoryIdx < 0 || categoryIdx >= m_categories.size()) {
        return {};
    }

    if (row < 0 || row >= m_appNodes[categoryIdx].size()) {
        return {};
    }

    return createIndex(row, column, m_appNodes[categoryIdx][row]);
}

QModelIndex PopularModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return {};
    }

    auto *node = static_cast<Node *>(child.internalPointer());
    if (!node || node->isCategory) {
        return {}; // Categories have no parent
    }

    // It's an app, return its category as parent
    int categoryIdx = node->categoryIdx;
    if (categoryIdx < 0 || categoryIdx >= m_categories.size()) {
        return {};
    }

    return createIndex(categoryIdx, 0, m_categoryNodes.value(categoryIdx));
}

int PopularModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        // Root has categories as children
        return m_categories.size();
    }

    if (parent.column() != 0) {
        return 0;
    }

    // Only categories have children
    if (!isCategory(parent)) {
        return 0;
    }

    auto *node = static_cast<Node *>(parent.internalPointer());
    int categoryIdx = node ? node->categoryIdx : -1;
    if (categoryIdx >= 0 && categoryIdx < m_categories.size()) {
        return m_categories[categoryIdx].appIndices.size();
    }

    return 0;
}

int PopularModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return PopCol::MAX;
}

QVariant PopularModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (isCategory(index)) {
        // Category row
        auto *node = static_cast<Node *>(index.internalPointer());
        int categoryIdx = node ? node->categoryIdx : -1;
        if (categoryIdx < 0 || categoryIdx >= m_categories.size()) {
            return {};
        }

        const CategoryData &category = m_categories[categoryIdx];

        if (role == Qt::DisplayRole && index.column() == PopCol::Category) {
            return category.name;
        }
        if (role == Qt::DecorationRole && index.column() == PopCol::Category) {
            return m_iconFolder;
        }
        if (role == Qt::FontRole) {
            QFont font;
            font.setBold(true);
            return font;
        }
        return {};
    }

    int appIdx = getAppIndex(index);
    if (appIdx < 0) {
        return {};
    }

    const PopularAppData &app = m_apps[appIdx];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case PopCol::Name:
            return app.name;
        case PopCol::Description:
            return app.description;
        default:
            return {};
        }
    }

    if (role == Qt::DecorationRole) {
        if (index.column() == PopCol::Check && app.isInstalled) {
            return m_iconInstalled;
        }
        if (index.column() == PopCol::Info) {
            return m_iconInfo;
        }
    }

    if (role == Qt::CheckStateRole && index.column() == PopCol::Check) {
        return static_cast<int>(app.checkState);
    }

    // Custom roles for metadata
    if (role == Qt::UserRole) {
        switch (index.column()) {
        case PopCol::Name:
            return app.installNames;
        case PopCol::Description:
            return app.screenshot;
        default:
            return {};
        }
    }

    if (role == Qt::UserRole + 1) {
        return app.uninstallNames;
    }

    if (role == Qt::UserRole + 2) {
        return app.postUninstall;
    }

    if (role == Qt::UserRole + 3) {
        return app.preUninstall;
    }

    return {};
}

bool PopularModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (isCategory(index)) {
        return false;
    }

    int appIdx = getAppIndex(index);
    if (appIdx < 0) {
        return false;
    }

    if (role == Qt::CheckStateRole && index.column() == PopCol::Check) {
        m_apps[appIdx].checkState = static_cast<Qt::CheckState>(value.toInt());
        emit dataChanged(index, index, {Qt::CheckStateRole});
        emit checkStateChanged(index);
        return true;
    }

    return false;
}

Qt::ItemFlags PopularModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (isCategory(index)) {
        // Categories are not selectable
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == PopCol::Check) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

QVariant PopularModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case PopCol::Category:
        return tr("Category");
    case PopCol::Check:
        return QString();
    case PopCol::Name:
        return tr("Package");
    case PopCol::Info:
        return tr("Info");
    case PopCol::Description:
        return tr("Description");
    default:
        return {};
    }
}

void PopularModel::setPopularApps(const QList<PopularAppData> &apps)
{
    beginResetModel();

    m_apps.clear();
    m_categories.clear();
    m_categoryIndexMap.clear();

    m_apps.reserve(apps.size());

    // Group apps by category
    for (const PopularAppData &app : apps) {
        int appIdx = m_apps.size();
        m_apps.append(app);

        int categoryIdx = getCategoryIndex(app.category);
        if (categoryIdx == -1) {
            // New category
            CategoryData category;
            category.name = app.category;
            category.appIndices.append(appIdx);
            categoryIdx = m_categories.size();
            m_categories.append(category);
            m_categoryIndexMap.insert(app.category, categoryIdx);
        } else {
            // Existing category
            m_categories[categoryIdx].appIndices.append(appIdx);
        }
    }

    buildNodes();
    endResetModel();
}

void PopularModel::clear()
{
    beginResetModel();
    clearNodes();
    m_apps.clear();
    m_categories.clear();
    m_categoryIndexMap.clear();
    endResetModel();
}

void PopularModel::setInstalledPackages(const std::function<bool(const QString &)> &checkInstalled)
{
    for (int i = 0; i < m_apps.size(); ++i) {
        bool wasInstalled = m_apps[i].isInstalled;
        bool isInstalled = checkInstalled(m_apps[i].uninstallNames);
        m_apps[i].isInstalled = isInstalled;

        if (wasInstalled != isInstalled) {
            // Find the index for this app and emit dataChanged
            for (int catIdx = 0; catIdx < m_categories.size(); ++catIdx) {
                const auto &appIndices = m_categories[catIdx].appIndices;
                for (int row = 0; row < appIndices.size(); ++row) {
                    if (appIndices[row] == i) {
                        QModelIndex idx = index(row, PopCol::Check, index(catIdx, 0));
                        emit dataChanged(idx, idx, {Qt::DecorationRole});
                        break;
                    }
                }
            }
        }
    }
}

QStringList PopularModel::checkedPackageNames() const
{
    QStringList result;
    for (const PopularAppData &app : m_apps) {
        if (app.checkState == Qt::Checked) {
            result.append(app.name);
        }
    }
    return result;
}

QModelIndexList PopularModel::checkedItems() const
{
    QModelIndexList result;
    for (int catIdx = 0; catIdx < m_categories.size(); ++catIdx) {
        const auto &appIndices = m_categories[catIdx].appIndices;
        for (int row = 0; row < appIndices.size(); ++row) {
            int appIdx = appIndices[row];
            if (appIdx >= 0 && appIdx < m_apps.size() && m_apps[appIdx].checkState == Qt::Checked) {
                result.append(index(row, PopCol::Check, index(catIdx, 0)));
            }
        }
    }
    return result;
}

void PopularModel::uncheckAll()
{
    // Uncheck all apps and emit dataChanged per category to respect parent boundaries
    for (int catIdx = 0; catIdx < m_categories.size(); ++catIdx) {
        const auto &appIndices = m_categories[catIdx].appIndices;
        bool hadChecked = false;

        for (int appIdx : appIndices) {
            if (appIdx >= 0 && appIdx < m_apps.size() && m_apps[appIdx].checkState == Qt::Checked) {
                m_apps[appIdx].checkState = Qt::Unchecked;
                hadChecked = true;
            }
        }

        // Emit dataChanged for this category's apps
        if (hadChecked && !appIndices.isEmpty()) {
            QModelIndex categoryIndex = index(catIdx, 0);
            QModelIndex topLeft = index(0, PopCol::Check, categoryIndex);
            QModelIndex bottomRight = index(appIndices.size() - 1, PopCol::Check, categoryIndex);
            emit dataChanged(topLeft, bottomRight, {Qt::CheckStateRole});
        }
    }

    // Emit checkStateChanged to update button states
    // Emit with an invalid index since multiple items changed
    emit checkStateChanged(QModelIndex());
}

QModelIndex PopularModel::findItemByName(const QString &name) const
{
    for (int catIdx = 0; catIdx < m_categories.size(); ++catIdx) {
        const auto &appIndices = m_categories[catIdx].appIndices;
        for (int row = 0; row < appIndices.size(); ++row) {
            int appIdx = appIndices[row];
            if (appIdx >= 0 && appIdx < m_apps.size() && m_apps[appIdx].name == name) {
                return index(row, PopCol::Check, index(catIdx, 0));
            }
        }
    }
    return {};
}

const PopularAppData *PopularModel::getAppData(const QModelIndex &index) const
{
    int appIdx = getAppIndex(index);
    if (appIdx < 0) {
        return nullptr;
    }

    return &m_apps[appIdx];
}

void PopularModel::setIcons(const QIcon &installed, const QIcon &folder, const QIcon &info)
{
    m_iconInstalled = installed;
    m_iconFolder = folder;
    m_iconInfo = info;
}

int PopularModel::getCategoryIndex(const QString &categoryName) const
{
    return m_categoryIndexMap.value(categoryName, -1);
}

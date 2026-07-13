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
#include "snapmodel.h"

#include "../packagestatus.h"

SnapModel::SnapModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int SnapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_snaps.size());
}

int SnapModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return SnapCol::Classic + 1;
}

QVariant SnapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_snaps.size()) {
        return {};
    }

    const SnapData &snap = m_snaps.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case SnapCol::Name:
            return snap.name;
        case SnapCol::Version:
            return snap.version;
        case SnapCol::Publisher:
            return snap.publisher;
        case SnapCol::Notes:
            return snap.notes;
        case SnapCol::Description:
            return snap.description;
        default:
            return {};
        }

    case Qt::CheckStateRole:
        if (index.column() == SnapCol::Check) {
            return snap.checkState;
        }
        return {};

    case Qt::DecorationRole:
        if (index.column() == SnapCol::Check && snap.status == Status::Installed) {
            return m_iconInstalled;
        }
        return {};

    case Qt::UserRole:
        switch (index.column()) {
        case SnapCol::Status:
            return snap.status;
        case SnapCol::Classic:
            return snap.isClassic;
        case SnapCol::Name:
            return snap.name;
        default:
            return {};
        }

    default:
        return {};
    }
}

bool SnapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_snaps.size()) {
        return false;
    }

    if (role == Qt::CheckStateRole && index.column() == SnapCol::Check) {
        SnapData &snap = m_snaps[index.row()];
        auto newState = static_cast<Qt::CheckState>(value.toInt());
        if (snap.checkState != newState) {
            snap.checkState = newState;
            emit dataChanged(index, index, {Qt::CheckStateRole});
            emit checkStateChanged(snap.name, newState, snap.status);
            return true;
        }
    }

    return false;
}

Qt::ItemFlags SnapModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == SnapCol::Check) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

QVariant SnapModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case SnapCol::Check:
        return QString();
    case SnapCol::Name:
        return tr("Package");
    case SnapCol::Version:
        return tr("Version");
    case SnapCol::Publisher:
        return tr("Publisher");
    case SnapCol::Notes:
        return tr("Notes");
    case SnapCol::Description:
        return tr("Description");
    default:
        return QString();
    }
}

void SnapModel::setSnapData(const QVector<SnapData> &snaps)
{
    beginResetModel();
    m_snaps = snaps;
    m_nameToRow.clear();
    m_nameToRow.reserve(m_snaps.size());
    for (int i = 0; i < m_snaps.size(); ++i) {
        m_nameToRow.insert(m_snaps.at(i).name, i);
    }
    endResetModel();
}

void SnapModel::clear()
{
    beginResetModel();
    m_snaps.clear();
    m_nameToRow.clear();
    endResetModel();
}

QStringList SnapModel::checkedPackages() const
{
    QStringList result;
    for (const SnapData &snap : m_snaps) {
        if (snap.checkState == Qt::Checked) {
            result.append(snap.name);
        }
    }
    return result;
}

void SnapModel::setAllChecked(bool checked)
{
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < m_snaps.size(); ++i) {
        m_snaps[i].checkState = state;
    }
    if (!m_snaps.isEmpty()) {
        emit dataChanged(index(0, SnapCol::Check), index(m_snaps.size() - 1, SnapCol::Check), {Qt::CheckStateRole});
    }
}

const SnapData *SnapModel::snapAt(int row) const
{
    if (row >= 0 && row < m_snaps.size()) {
        return &m_snaps.at(row);
    }
    return nullptr;
}

int SnapModel::findSnapRow(const QString &name) const
{
    return m_nameToRow.value(name, -1);
}

bool SnapModel::isClassic(const QString &name) const
{
    int row = findSnapRow(name);
    if (row < 0) {
        return false;
    }
    return m_snaps.at(row).isClassic;
}

void SnapModel::updateInstalledStatus(const QStringList &installedNames)
{
    QSet<QString> installedSet(installedNames.begin(), installedNames.end());
    for (int i = 0; i < m_snaps.size(); ++i) {
        int oldStatus = m_snaps[i].status;
        m_snaps[i].status = installedSet.contains(m_snaps[i].name) ? Status::Installed : Status::NotInstalled;
        if (oldStatus != m_snaps[i].status) {
            emit dataChanged(index(i, SnapCol::Check), index(i, SnapCol::Status));
        }
    }
}

void SnapModel::setIcons(const QIcon &installed)
{
    m_iconInstalled = installed;
}

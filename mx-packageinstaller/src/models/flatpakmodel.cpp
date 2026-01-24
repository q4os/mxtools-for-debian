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
#include "flatpakmodel.h"

namespace Status
{
enum { Installed = 1, Upgradable, NotInstalled, Autoremovable };
}

FlatpakModel::FlatpakModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int FlatpakModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_flatpaks.size());
}

int FlatpakModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return FlatCol::FullName + 1;
}

QVariant FlatpakModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_flatpaks.size()) {
        return {};
    }

    const FlatpakData &fp = m_flatpaks.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case FlatCol::Name:
            return fp.shortName;
        case FlatCol::LongName:
            return fp.longName;
        case FlatCol::Version:
            return fp.version;
        case FlatCol::Branch:
            return fp.branch;
        case FlatCol::Size:
            return fp.size;
        default:
            return {};
        }

    case Qt::CheckStateRole:
        if (index.column() == FlatCol::Check) {
            return fp.checkState;
        }
        return {};

    case Qt::DecorationRole:
        if (index.column() == FlatCol::Check && fp.status == Status::Installed) {
            return m_iconInstalled;
        }
        return {};

    case Qt::UserRole:
        switch (index.column()) {
        case FlatCol::Status:
            return fp.status;
        case FlatCol::Duplicate:
            return fp.isDuplicate;
        case FlatCol::FullName:
            return fp.fullName;
        default:
            return {};
        }

    case Qt::UserRole + 1:
        if (index.column() == FlatCol::FullName) {
            return fp.canonicalRef;
        }
        return {};

    default:
        return {};
    }
}

bool FlatpakModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_flatpaks.size()) {
        return false;
    }

    if (role == Qt::CheckStateRole && index.column() == FlatCol::Check) {
        FlatpakData &fp = m_flatpaks[index.row()];
        auto newState = static_cast<Qt::CheckState>(value.toInt());
        if (fp.checkState != newState) {
            fp.checkState = newState;
            emit dataChanged(index, index, {Qt::CheckStateRole});
            emit checkStateChanged(fp.fullName, newState, fp.status);
            return true;
        }
    }

    return false;
}

Qt::ItemFlags FlatpakModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == FlatCol::Check) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

QVariant FlatpakModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case FlatCol::Check:
        return QString();
    case FlatCol::Name:
        return tr("Package");
    case FlatCol::LongName:
        return tr("Full Name");
    case FlatCol::Version:
        return tr("Version");
    case FlatCol::Branch:
        return tr("Branch");
    case FlatCol::Size:
        return tr("Size");
    case FlatCol::Status:
        return QString();
    case FlatCol::Duplicate:
        return QString();
    case FlatCol::FullName:
        return QString();
    default:
        return {};
    }
}

void FlatpakModel::setFlatpakData(const QVector<FlatpakData> &flatpaks)
{
    beginResetModel();
    m_flatpaks = flatpaks;
    m_refToRow.clear();
    m_refToRow.reserve(m_flatpaks.size());
    for (int i = 0; i < m_flatpaks.size(); ++i) {
        m_refToRow.insert(m_flatpaks.at(i).canonicalRef, i);
    }
    endResetModel();
}

void FlatpakModel::addFlatpak(const FlatpakData &flatpak)
{
    int row = static_cast<int>(m_flatpaks.size());
    beginInsertRows(QModelIndex(), row, row);
    m_flatpaks.append(flatpak);
    m_refToRow.insert(flatpak.canonicalRef, row);
    endInsertRows();
}

void FlatpakModel::clear()
{
    beginResetModel();
    m_flatpaks.clear();
    m_refToRow.clear();
    endResetModel();
}

QStringList FlatpakModel::checkedPackages() const
{
    QStringList result;
    for (const FlatpakData &fp : m_flatpaks) {
        if (fp.checkState == Qt::Checked) {
            result.append(fp.fullName);
        }
    }
    return result;
}

void FlatpakModel::setAllChecked(bool checked)
{
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < m_flatpaks.size(); ++i) {
        if (m_flatpaks[i].checkState != state) {
            m_flatpaks[i].checkState = state;
        }
    }
    if (!m_flatpaks.isEmpty()) {
        emit dataChanged(index(0, FlatCol::Check), index(m_flatpaks.size() - 1, FlatCol::Check),
                         {Qt::CheckStateRole});
    }
}

void FlatpakModel::setCheckedForVisible(const QVector<int> &visibleRows, bool checked)
{
    Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int row : visibleRows) {
        if (row >= 0 && row < m_flatpaks.size()) {
            if (m_flatpaks[row].checkState != state) {
                m_flatpaks[row].checkState = state;
                emit dataChanged(index(row, FlatCol::Check), index(row, FlatCol::Check),
                                 {Qt::CheckStateRole});
            }
        }
    }
}

const FlatpakData *FlatpakModel::flatpakAt(int row) const
{
    if (row >= 0 && row < m_flatpaks.size()) {
        return &m_flatpaks.at(row);
    }
    return nullptr;
}

int FlatpakModel::findFlatpakRow(const QString &canonicalRef) const
{
    return m_refToRow.value(canonicalRef, -1);
}

void FlatpakModel::markDuplicates()
{
    QHash<QString, int> refCount;
    for (const FlatpakData &fp : m_flatpaks) {
        refCount[fp.canonicalRef]++;
    }

    for (int i = 0; i < m_flatpaks.size(); ++i) {
        bool wasDuplicate = m_flatpaks[i].isDuplicate;
        m_flatpaks[i].isDuplicate = refCount.value(m_flatpaks[i].canonicalRef, 0) > 1;
        if (wasDuplicate != m_flatpaks[i].isDuplicate) {
            emit dataChanged(index(i, FlatCol::Duplicate), index(i, FlatCol::Duplicate));
        }
    }
}

void FlatpakModel::updateInstalledStatus(const QStringList &installedRefs)
{
    QSet<QString> installedSet(installedRefs.begin(), installedRefs.end());
    for (int i = 0; i < m_flatpaks.size(); ++i) {
        int oldStatus = m_flatpaks[i].status;
        if (installedSet.contains(m_flatpaks[i].canonicalRef)) {
            m_flatpaks[i].status = Status::Installed;
        } else {
            m_flatpaks[i].status = Status::NotInstalled;
        }
        if (oldStatus != m_flatpaks[i].status) {
            emit dataChanged(index(i, FlatCol::Check), index(i, FlatCol::Status));
        }
    }
}

void FlatpakModel::setInstalledSizes(const QHash<QString, QString> &sizeMap)
{
    for (int i = 0; i < m_flatpaks.size(); ++i) {
        auto it = sizeMap.find(m_flatpaks[i].canonicalRef);
        if (it != sizeMap.end() && m_flatpaks[i].size != it.value()) {
            m_flatpaks[i].size = it.value();
            emit dataChanged(index(i, FlatCol::Size), index(i, FlatCol::Size));
        }
    }
}

void FlatpakModel::setIcons(const QIcon &installed)
{
    m_iconInstalled = installed;
}

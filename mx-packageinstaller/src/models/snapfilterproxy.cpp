/**********************************************************************
 * Copyright (C) 2017-2026 MX Authors
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
#include "snapfilterproxy.h"

#include "../versionnumber.h"

SnapFilterProxy::SnapFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void SnapFilterProxy::invalidateRowFilter()
{
    // beginFilterChange()/endFilterChange() replaced invalidateFilter() in Qt 6.10;
    // invalidateFilter() is deprecated from 6.13 on. This proxy only filters rows.
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
}

void SnapFilterProxy::setSearchText(const QString &text)
{
    if (m_searchText != text) {
        m_searchText = text;
        invalidateRowFilter();
    }
}

void SnapFilterProxy::setStatusFilter(int status)
{
    if (m_statusFilter != status) {
        m_statusFilter = status;
        invalidateRowFilter();
    }
}

bool SnapFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    auto *model = qobject_cast<SnapModel *>(sourceModel());
    if (!model) {
        return false;
    }

    const SnapData *snap = model->snapAt(sourceRow);
    if (!snap) {
        return false;
    }

    if (!matchesStatus(snap->status)) {
        return false;
    }

    if (!m_searchText.isEmpty() && !matchesSearch(*snap)) {
        return false;
    }

    return true;
}

bool SnapFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() == SnapCol::Version) {
        const VersionNumber leftVersion(left.data(Qt::DisplayRole).toString());
        const VersionNumber rightVersion(right.data(Qt::DisplayRole).toString());
        return leftVersion < rightVersion;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

bool SnapFilterProxy::matchesSearch(const SnapData &snap) const
{
    return snap.name.contains(m_searchText, Qt::CaseInsensitive)
           || snap.publisher.contains(m_searchText, Qt::CaseInsensitive)
           || snap.description.contains(m_searchText, Qt::CaseInsensitive);
}

bool SnapFilterProxy::matchesStatus(int status) const
{
    if (m_statusFilter == 0) {
        return true;
    }
    return status == m_statusFilter;
}

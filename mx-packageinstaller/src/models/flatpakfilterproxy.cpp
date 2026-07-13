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
#include "flatpakfilterproxy.h"

#include "../versionnumber.h"

FlatpakFilterProxy::FlatpakFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void FlatpakFilterProxy::setSearchText(const QString &text)
{
    if (m_searchText != text) {
        m_searchText = text;
        invalidateFilter();
    }
}

void FlatpakFilterProxy::setStatusFilter(int status)
{
    if (m_statusFilter != status) {
        m_statusFilter = status;
        invalidateFilter();
    }
}

void FlatpakFilterProxy::setHideDuplicates(bool hide)
{
    if (m_hideDuplicates != hide) {
        m_hideDuplicates = hide;
        invalidateFilter();
    }
}

void FlatpakFilterProxy::setAllowedRefs(const QSet<QString> &refs)
{
    m_allowedRefs = refs;
    m_refFilterActive = true;
    invalidateFilter();
}

void FlatpakFilterProxy::clearAllowedRefs()
{
    m_allowedRefs.clear();
    m_refFilterActive = false;
    invalidateFilter();
}

QVector<int> FlatpakFilterProxy::visibleSourceRows() const
{
    QVector<int> rows;
    rows.reserve(rowCount());
    for (int i = 0; i < rowCount(); ++i) {
        rows.append(mapToSource(index(i, 0)).row());
    }
    return rows;
}

bool FlatpakFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    auto *model = qobject_cast<FlatpakModel *>(sourceModel());
    if (!model) {
        return false;
    }

    const FlatpakData *fp = model->flatpakAt(sourceRow);
    if (!fp) {
        return false;
    }

    // When a ref filter is active, only show items in the set — an empty set
    // legitimately means "show nothing" (e.g. no installed apps/runtimes).
    if (m_refFilterActive && !m_allowedRefs.contains(fp->canonicalRef)) {
        return false;
    }

    if (m_hideDuplicates && fp->isDuplicate) {
        return false;
    }

    if (!matchesStatus(fp->status)) {
        return false;
    }

    if (!m_searchText.isEmpty() && !matchesSearch(*fp)) {
        return false;
    }

    return true;
}

bool FlatpakFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() == FlatCol::Size && right.column() == FlatCol::Size) {
        const quint64 leftSize = left.data(Qt::UserRole).toULongLong();
        const quint64 rightSize = right.data(Qt::UserRole).toULongLong();
        if (leftSize != rightSize) {
            return leftSize < rightSize;
        }

        const QString leftName = left.siblingAtColumn(FlatCol::Name).data(Qt::DisplayRole).toString();
        const QString rightName = right.siblingAtColumn(FlatCol::Name).data(Qt::DisplayRole).toString();
        return QString::localeAwareCompare(leftName, rightName) < 0;
    }

    if (left.column() == FlatCol::Version && right.column() == FlatCol::Version) {
        const VersionNumber leftVersion(left.data(Qt::DisplayRole).toString());
        const VersionNumber rightVersion(right.data(Qt::DisplayRole).toString());
        return leftVersion < rightVersion;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool FlatpakFilterProxy::matchesSearch(const FlatpakData &fp) const
{
    return fp.shortName.contains(m_searchText, Qt::CaseInsensitive)
           || fp.longName.contains(m_searchText, Qt::CaseInsensitive)
           || fp.canonicalRef.contains(m_searchText, Qt::CaseInsensitive);
}

bool FlatpakFilterProxy::matchesStatus(int status) const
{
    if (m_statusFilter == 0) {
        return true;
    }
    return status == m_statusFilter;
}

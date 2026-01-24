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
#include "packagefilterproxy.h"

PackageFilterProxy::PackageFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void PackageFilterProxy::setSearchText(const QString &text)
{
    if (m_searchText != text) {
        m_searchText = text;
        invalidateFilter();
    }
}

void PackageFilterProxy::setStatusFilter(int status)
{
    if (m_statusFilter != status) {
        m_statusFilter = status;
        invalidateFilter();
    }
}

void PackageFilterProxy::setHideLibraries(bool hide)
{
    if (m_hideLibraries != hide) {
        m_hideLibraries = hide;
        invalidateFilter();
    }
}

QVector<int> PackageFilterProxy::visibleSourceRows() const
{
    QVector<int> rows;
    rows.reserve(rowCount());
    for (int i = 0; i < rowCount(); ++i) {
        rows.append(mapToSource(index(i, 0)).row());
    }
    return rows;
}

bool PackageFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    auto *model = qobject_cast<PackageModel *>(sourceModel());
    if (!model) {
        return false;
    }

    const PackageData *pkg = model->packageAt(sourceRow);
    if (!pkg) {
        return false;
    }

    if (m_hideLibraries && isLibraryPackage(pkg->name)) {
        return false;
    }

    if (!matchesStatus(pkg->status)) {
        return false;
    }

    if (!m_searchText.isEmpty() && !matchesSearch(pkg->name, pkg->description)) {
        return false;
    }

    return true;
}

bool PackageFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}

bool PackageFilterProxy::matchesSearch(const QString &name, const QString &description) const
{
    return name.contains(m_searchText, Qt::CaseInsensitive)
           || description.contains(m_searchText, Qt::CaseInsensitive);
}

bool PackageFilterProxy::matchesStatus(int status) const
{
    if (m_statusFilter == 0) {
        return true;
    }
    return status == m_statusFilter;
}

bool PackageFilterProxy::isLibraryPackage(const QString &name)
{
    return name.startsWith(QLatin1String("lib")) || name.endsWith(QLatin1String("-dev"))
           || name.endsWith(QLatin1String("-dbg")) || name.endsWith(QLatin1String("-dbgsym"));
}

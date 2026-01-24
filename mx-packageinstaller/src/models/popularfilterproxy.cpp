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
#include "popularfilterproxy.h"

PopularFilterProxy::PopularFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setRecursiveFilteringEnabled(true);
}

void PopularFilterProxy::sort(int column, Qt::SortOrder order)
{
    // Keep categories sorted A-Z/ Z-A; child ordering is driven by source sortChildren.
    if (column == PopCol::Category) {
        m_categorySortOrder = order;
    } else if (column == PopCol::Name || column == PopCol::Description) {
        m_childSortColumn = column;
        m_childSortOrder = order;
    }
    QSortFilterProxyModel::sort(PopCol::Category, Qt::AscendingOrder);
    invalidate();
}

void PopularFilterProxy::setSearchText(const QString &text)
{
    if (m_searchText != text) {
        m_searchText = text;
        invalidateFilter();
    }
}

bool PopularFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_searchText.isEmpty()) {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!sourceParent.isValid()) {
        return hasMatchingChild(index);
    }

    return matchesSearch(index);
}

bool PopularFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Categories are always sorted A-Z by name, regardless of which column the user clicks
    const bool leftIsCategory = !left.parent().isValid();
    const bool rightIsCategory = !right.parent().isValid();

    if (leftIsCategory && rightIsCategory) {
        // For categories, always sort by the Icon column (category name) in ascending order
        const QString leftCategory = left.sibling(left.row(), PopCol::Category).data(Qt::DisplayRole).toString();
        const QString rightCategory = right.sibling(right.row(), PopCol::Category).data(Qt::DisplayRole).toString();
        const int cmp = QString::localeAwareCompare(leftCategory, rightCategory);
        return (m_categorySortOrder == Qt::DescendingOrder) ? (cmp > 0) : (cmp < 0);
    }

    // Child items (apps) are sorted by the clicked column (Name or Description).
    if (!leftIsCategory && !rightIsCategory) {
        const int column = (m_childSortColumn == PopCol::Description) ? PopCol::Description : PopCol::Name;
        const QString leftText = left.sibling(left.row(), column).data(Qt::DisplayRole).toString();
        const QString rightText = right.sibling(right.row(), column).data(Qt::DisplayRole).toString();
        const int cmp = QString::localeAwareCompare(leftText, rightText);
        return (m_childSortOrder == Qt::DescendingOrder) ? (cmp > 0) : (cmp < 0);
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

bool PopularFilterProxy::matchesSearch(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }

    QModelIndex nameIndex = index.sibling(index.row(), PopCol::Name);
    QModelIndex descIndex = index.sibling(index.row(), PopCol::Description);

    QString name = nameIndex.data(Qt::DisplayRole).toString();
    QString description = descIndex.data(Qt::DisplayRole).toString();

    return name.contains(m_searchText, Qt::CaseInsensitive)
           || description.contains(m_searchText, Qt::CaseInsensitive);
}

bool PopularFilterProxy::hasMatchingChild(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return false;
    }

    int childCount = sourceModel()->rowCount(parent);
    for (int i = 0; i < childCount; ++i) {
        QModelIndex childIndex = sourceModel()->index(i, 0, parent);
        if (matchesSearch(childIndex)) {
            return true;
        }
    }

    return false;
}

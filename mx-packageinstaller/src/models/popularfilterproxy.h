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
#pragma once

#include <QSortFilterProxyModel>
#include "popularmodel.h"

class PopularFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit PopularFilterProxy(QObject *parent = nullptr);

    void setSearchText(const QString &text);
    [[nodiscard]] QString searchText() const { return m_searchText; }

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

protected:
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    [[nodiscard]] bool matchesSearch(const QModelIndex &index) const;
    [[nodiscard]] bool hasMatchingChild(const QModelIndex &parent) const;

    QString m_searchText;
    int m_childSortColumn {PopCol::Name};
    Qt::SortOrder m_childSortOrder {Qt::AscendingOrder};
    Qt::SortOrder m_categorySortOrder {Qt::AscendingOrder};
};

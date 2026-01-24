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

#include <QSet>
#include <QSortFilterProxyModel>
#include "flatpakmodel.h"

class FlatpakFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FlatpakFilterProxy(QObject *parent = nullptr);

    void setSearchText(const QString &text);
    void setStatusFilter(int status);
    void setHideDuplicates(bool hide);
    void setAllowedRefs(const QSet<QString> &refs);
    void clearAllowedRefs();

    [[nodiscard]] int statusFilter() const { return m_statusFilter; }
    [[nodiscard]] bool hideDuplicates() const { return m_hideDuplicates; }
    [[nodiscard]] QString searchText() const { return m_searchText; }
    [[nodiscard]] bool hasAllowedRefs() const { return !m_allowedRefs.isEmpty(); }

    [[nodiscard]] QVector<int> visibleSourceRows() const;

protected:
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    [[nodiscard]] bool matchesSearch(const FlatpakData &fp) const;
    [[nodiscard]] bool matchesStatus(int status) const;

    QString m_searchText;
    int m_statusFilter = 0;
    bool m_hideDuplicates = true;
    QSet<QString> m_allowedRefs;
};

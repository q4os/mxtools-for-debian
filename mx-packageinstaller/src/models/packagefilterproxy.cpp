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
#include "packagefilterproxy.h"

#include <QRegularExpression>

#include "../versionnumber.h"
#ifdef PACKAGE_BACKEND_PACMAN
#include "../packagebackend.h"
#endif

PackageFilterProxy::PackageFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void PackageFilterProxy::invalidateRowFilter()
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

void PackageFilterProxy::setSearchText(const QString &text)
{
    if (m_searchText != text) {
        m_searchText = text;
        invalidateRowFilter();
    }
}

void PackageFilterProxy::setStatusFilter(int status)
{
    if (m_statusFilter != status) {
        m_statusFilter = status;
        invalidateRowFilter();
    }
}

void PackageFilterProxy::setHideLibraries(bool hide)
{
    if (m_hideLibraries != hide) {
        m_hideLibraries = hide;
        invalidateRowFilter();
    }
}

void PackageFilterProxy::setRepoOnly(bool repoOnly)
{
    if (m_repoOnly != repoOnly) {
        m_repoOnly = repoOnly;
        invalidateRowFilter();
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

    if (m_repoOnly && !pkg->fromRepo) {
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
    if (left.column() == TreeCol::Check) {
        // The Check column only exposes a checkbox state (Qt::CheckStateRole) and a
        // status icon (Qt::DecorationRole), neither of which QSortFilterProxyModel's
        // default Qt::DisplayRole-based lessThan() can compare -- so clicking this
        // header would otherwise leave every row "equal" and never reorder. Sort by
        // the underlying package status instead, matching what the icon represents.
        auto *model = qobject_cast<PackageModel *>(sourceModel());
        const PackageData *leftPkg = model ? model->packageAt(left.row()) : nullptr;
        const PackageData *rightPkg = model ? model->packageAt(right.row()) : nullptr;
        if (leftPkg && rightPkg) {
            return leftPkg->status < rightPkg->status;
        }
        return false;
    }
    if (left.column() == TreeCol::RepoVersion || left.column() == TreeCol::InstalledVersion) {
        const QString leftStr = left.data(Qt::DisplayRole).toString();
        const QString rightStr = right.data(Qt::DisplayRole).toString();
#ifdef PACKAGE_BACKEND_PACMAN
        // See PackageModel::updateInstalledVersions() -- same reasoning: pacman
        // version strings must not be ordered with dpkg's VersionNumber rules.
        return PackageBackend::compareVersions(leftStr, rightStr) < 0;
#else
        const VersionNumber leftVersion(leftStr);
        const VersionNumber rightVersion(rightStr);
        return leftVersion < rightVersion;
#endif
    }
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
    // "Installed" means installed-or-upgradable, matching the Installed count
    // (installCount + upgradeCount) and PackageModel::checkByStatus().
    if (m_statusFilter == Status::Installed) {
        return status == Status::Installed || status == Status::Upgradable;
    }
    return status == m_statusFilter;
}

bool PackageFilterProxy::isLibraryPackage(const QString &name)
{
#ifdef PACKAGE_BACKEND_PACMAN
    // Arch doesn't split packages into -dev/-dbg/-dbgsym/-libs the way Debian does,
    // so there's no equivalent heuristic to apply here yet; "hide libraries" is a
    // no-op until an Arch-appropriate pattern is authored.
    Q_UNUSED(name);
    return false;
#else
    // Hide lib*, -dev, -dbg, -dbgsym and -libs packages, but keep known user-facing
    // applications whose names merely start with "lib".
    static const QRegularExpression libraryPattern(
        QStringLiteral(R"((^lib(?!re(?:cad|office|pcb|wolf)))|(-dev$)|(-dbg$)|(-dbgsym$)|(-libs$))"),
        QRegularExpression::CaseInsensitiveOption);
    return libraryPattern.match(name).hasMatch();
#endif
}

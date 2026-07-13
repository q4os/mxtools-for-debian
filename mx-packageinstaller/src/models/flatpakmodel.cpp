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

#include "../packagestatus.h"

#include <QRegularExpression>
#include <QSet>

namespace {
QString normalizeNumber(QString number)
{
    const qsizetype lastComma = number.lastIndexOf(',');
    const qsizetype lastDot = number.lastIndexOf('.');
    const QChar decimalSeparator = lastComma > lastDot ? QChar(',') : QChar('.');

    if (lastComma >= 0 && lastDot >= 0) {
        const QChar thousandsSeparator = decimalSeparator == QChar(',') ? QChar('.') : QChar(',');
        number.remove(thousandsSeparator);
    }

    if (decimalSeparator == QChar(',')) {
        number.replace(',', '.');
    }

    return number;
}
} // namespace

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
        case FlatCol::Size:
            return fp.sizeBytes;
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
        m_flatpaks[i].sizeBytes = sizeStringToBytes(m_flatpaks.at(i).size);
        m_refToRow.insert(m_flatpaks.at(i).canonicalRef, i);
    }
    endResetModel();
}

void FlatpakModel::addFlatpak(const FlatpakData &flatpak)
{
    int row = static_cast<int>(m_flatpaks.size());
    FlatpakData data = flatpak;
    data.sizeBytes = sizeStringToBytes(data.size);
    beginInsertRows(QModelIndex(), row, row);
    m_flatpaks.append(data);
    m_refToRow.insert(data.canonicalRef, row);
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
    // Keep the first occurrence of each canonical ref visible and mark only the
    // later copies as duplicates, so the hide-duplicates filter collapses each
    // ref to one installable row rather than hiding every copy.
    QSet<QString> seen;
    for (int i = 0; i < m_flatpaks.size(); ++i) {
        const bool wasDuplicate = m_flatpaks[i].isDuplicate;
        m_flatpaks[i].isDuplicate = seen.contains(m_flatpaks[i].canonicalRef);
        seen.insert(m_flatpaks[i].canonicalRef);
        if (wasDuplicate != m_flatpaks[i].isDuplicate) {
            emit dataChanged(index(i, FlatCol::Duplicate), index(i, FlatCol::Duplicate));
        }
    }
}

int FlatpakModel::findRowByFullName(const QString &fullName) const
{
    for (int i = 0; i < m_flatpaks.size(); ++i) {
        if (m_flatpaks.at(i).fullName == fullName) {
            return i;
        }
    }
    return -1;
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
            m_flatpaks[i].sizeBytes = sizeStringToBytes(it.value());
            emit dataChanged(index(i, FlatCol::Size), index(i, FlatCol::Size));
        }
    }
}

void FlatpakModel::setIcons(const QIcon &installed)
{
    m_iconInstalled = installed;
}

quint64 FlatpakModel::sizeStringToBytes(const QString &size)
{
    QString normalized = size.trimmed();
    normalized.replace(QChar(0x00a0), QLatin1Char(' '));
    normalized.replace(QChar(0x202f), QLatin1Char(' '));

    static const QRegularExpression sizeRegex(
        QStringLiteral(R"(^\s*([0-9][0-9.,]*)\s*([A-Za-z]+)?\s*$)"));
    const QRegularExpressionMatch match = sizeRegex.match(normalized);
    if (!match.hasMatch()) {
        return 0;
    }

    bool ok = false;
    const double value = normalizeNumber(match.captured(1)).toDouble(&ok);
    if (!ok || value < 0.0) {
        return 0;
    }

    // Flatpak formats sizes with SI decimal units (1 kB = 1000 bytes)
    const QString unit = match.captured(2).toUpper();
    quint64 multiplier = 1;
    if (unit == QLatin1String("KB")) {
        multiplier = 1000ULL;
    } else if (unit == QLatin1String("KIB")) {
        multiplier = 1024ULL;
    } else if (unit == QLatin1String("MB")) {
        multiplier = 1000ULL * 1000ULL;
    } else if (unit == QLatin1String("MIB")) {
        multiplier = 1024ULL * 1024ULL;
    } else if (unit == QLatin1String("GB")) {
        multiplier = 1000ULL * 1000ULL * 1000ULL;
    } else if (unit == QLatin1String("GIB")) {
        multiplier = 1024ULL * 1024ULL * 1024ULL;
    } else if (unit == QLatin1String("TB")) {
        multiplier = 1000ULL * 1000ULL * 1000ULL * 1000ULL;
    } else if (unit == QLatin1String("TIB")) {
        multiplier = 1024ULL * 1024ULL * 1024ULL * 1024ULL;
    }

    return static_cast<quint64>(value * static_cast<double>(multiplier));
}

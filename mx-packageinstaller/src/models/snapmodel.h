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

#include <QAbstractTableModel>
#include <QHash>
#include <QIcon>
#include <QVector>

namespace SnapCol
{
enum { Check, Name, Version, Publisher, Notes, Description, Status, Classic };
}

struct SnapData {
    QString name;
    QString version;
    QString publisher;
    QString notes;
    QString description;
    Qt::CheckState checkState = Qt::Unchecked;
    int status = 0;
    bool isClassic = false;
};

class SnapModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SnapModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    void setSnapData(const QVector<SnapData> &snaps);
    void clear();

    [[nodiscard]] QStringList checkedPackages() const;
    void setAllChecked(bool checked);

    [[nodiscard]] const SnapData *snapAt(int row) const;
    [[nodiscard]] int findSnapRow(const QString &name) const;
    [[nodiscard]] bool isClassic(const QString &name) const;

    void updateInstalledStatus(const QStringList &installedNames);

    void setIcons(const QIcon &installed);

signals:
    void checkStateChanged(const QString &name, Qt::CheckState state, int status);

private:
    QVector<SnapData> m_snaps;
    QHash<QString, int> m_nameToRow;
    QIcon m_iconInstalled;
};

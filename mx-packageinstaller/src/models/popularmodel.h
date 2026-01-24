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

#include <QAbstractItemModel>
#include <QHash>
#include <QIcon>
#include <QVector>

namespace PopCol
{
enum {
    Category,
    Check,
    Name,
    Info,
    Description,
    MAX
};
}

struct PopularAppData {
    QString category;
    QString name;
    QString description;
    QString installNames;
    QString uninstallNames;
    QString screenshot;
    QString postUninstall;
    QString preUninstall;
    QString qDistro;
    Qt::CheckState checkState = Qt::Unchecked;
    bool isInstalled = false;
};

struct CategoryData {
    QString name;
    QVector<int> appIndices; // Indices into m_apps vector
};

class PopularModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit PopularModel(QObject *parent = nullptr);
    ~PopularModel() override;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Custom methods
    void setPopularApps(const QList<PopularAppData> &apps);
    void clear();

    void setInstalledPackages(const std::function<bool(const QString &)> &checkInstalled);

    [[nodiscard]] QStringList checkedPackageNames() const;
    [[nodiscard]] QModelIndexList checkedItems() const;
    void uncheckAll();

    [[nodiscard]] QModelIndex findItemByName(const QString &name) const;
    [[nodiscard]] const PopularAppData *getAppData(const QModelIndex &index) const;

    void setIcons(const QIcon &installed, const QIcon &folder, const QIcon &info);

signals:
    void checkStateChanged(const QModelIndex &index);

private:
    struct Node {
        int categoryIdx;
        int rowInCategory; // -1 for categories
        bool isCategory;
    };

    [[nodiscard]] int getCategoryIndex(const QString &categoryName) const;
    [[nodiscard]] int getAppIndex(const QModelIndex &index) const;
    [[nodiscard]] bool isCategory(const QModelIndex &index) const;
    void clearNodes();
    void buildNodes();

    QVector<PopularAppData> m_apps;
    QVector<CategoryData> m_categories;
    QHash<QString, int> m_categoryIndexMap; // category name -> index in m_categories
    QVector<Node *> m_nodes;
    QVector<Node *> m_categoryNodes;
    QVector<QVector<Node *>> m_appNodes;

    QIcon m_iconInstalled;
    QIcon m_iconFolder;
    QIcon m_iconInfo;
};

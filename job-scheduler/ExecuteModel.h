/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <QAbstractItemModel>

class Execute;

class ExecuteModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ExecuteModel(QList<Execute *> *exe, QObject *parent = nullptr)
        : QAbstractItemModel(parent),
          executes(exe)
    {
    }

    ~ExecuteModel() override = default;

    enum Col { ExeTime, CronTime, User, Command };
    static Execute *getExecute(const QModelIndex &idx);
    void doSort();

private:
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex & /*index*/) const override
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    [[nodiscard]] QModelIndex parent(const QModelIndex & /*child*/) const override
    {
        return {};
    }
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/) const override
    {
        return 5;
    }
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override
    {
        return (parent.isValid() ? 0 : executes->count());
    }
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex & /*parent*/) const override
    {
        return createIndex(row, column, (*executes).at(row));
    }

    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QList<Execute *> *executes;
    int sortColumn {0};
    Qt::SortOrder sortOrder {Qt::AscendingOrder};
};

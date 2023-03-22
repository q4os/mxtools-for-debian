/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef EXECUTEMODEL_H
#define EXECUTEMODEL_H

#include <QAbstractItemModel>

class Execute;

class ExecuteModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ExecuteModel(QList<Execute *> *exe, QObject *parent = nullptr)
        : QAbstractItemModel(parent)
        , executes(exe)
        , sortColumn(0)
        , sortOrder(Qt::AscendingOrder)
    {
    }

    ~ExecuteModel() { }

    enum Col { ExeTime, CronTime, User, Command };
    static Execute *getExecute(const QModelIndex &idx);
    void doSort();

private:
    Qt::ItemFlags flags(const QModelIndex &) const { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
    QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
    int columnCount(const QModelIndex &) const { return 5; }
    int rowCount(const QModelIndex &parent) const { return (parent.isValid() ? 0 : executes->count()); }
    QModelIndex index(int row, int column, const QModelIndex &) const
    {
        return createIndex(row, column, (*executes).at(row));
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QList<Execute *> *executes;
    int sortColumn;
    Qt::SortOrder sortOrder;
};

#endif

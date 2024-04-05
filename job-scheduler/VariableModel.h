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

#include "Crontab.h"

// class Variable;

class VariableModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit VariableModel(QObject *parent = nullptr)
        : QAbstractItemModel(parent)
    {
        variables = &dummy;
    }

    ~VariableModel() override = default;

    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex & /*index*/) const override
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    [[nodiscard]] QModelIndex parent(const QModelIndex & /*child*/) const override
    {
        return {};
    }
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex & /*parent*/) const override
    {
        return createIndex(row, column, (*variables)[row]);
    }
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/) const override
    {
        return 3;
    }
    void resetData(QList<Variable *> *var)
    {
        variables = var;
    }
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override
    {
        return (parent.isValid() ? 0 : variables->count());
    }

    void varDataChanged(const QModelIndex &idx);
    static Variable *getVariable(const QModelIndex &idx);
    bool removeVariable(int row);
    bool insertVariable(int row, Variable *var);

    QList<Variable *> *variables;
    QList<Variable *> dummy;

private:
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role) const override;
};

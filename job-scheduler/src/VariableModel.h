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
        return createIndex(row, column, (*variables)[row].get());
    }
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/) const override
    {
        return 3;
    }
    void resetData(std::vector<std::unique_ptr<Variable>> *var)
    {
        variables = var;
    }
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override
    {
        return (parent.isValid() ? 0 : static_cast<int>(variables->size()));
    }

    void varDataChanged(const QModelIndex &idx);
    static Variable *getVariable(const QModelIndex &idx);
    bool removeVariable(int row);
    bool insertVariable(int row, std::unique_ptr<Variable> var);

    std::vector<std::unique_ptr<Variable>> *variables;
    std::vector<std::unique_ptr<Variable>> dummy;

private:
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role) const override;
};

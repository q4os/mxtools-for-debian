/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "VariableModel.h"

#include <QtGui>

#include "Crontab.h"

QVariant VariableModel::data(const QModelIndex &idx, int role) const
{
    if (idx.isValid() && role == Qt::DisplayRole) {
        auto *v = static_cast<Variable *>(idx.internalPointer());
        switch (idx.column()) {
        case 0:
            return v->name;
        case 1:
            return v->value;
        }
    }

    return {};
}

QVariant VariableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Value");
        }
    }

    return {};
}

bool VariableModel::removeVariable(int row)
{
    beginRemoveRows(QModelIndex(), row, row);

    variables->erase(variables->begin() + row);

    endRemoveRows();
    return true;
}

bool VariableModel::insertVariable(int row, std::unique_ptr<Variable> var)
{
    beginInsertRows(QModelIndex(), row, row);

    if (!variables->empty()) {
        variables->insert(variables->begin() + row, std::move(var));
    } else {
        variables->push_back(std::move(var));
    }

    endInsertRows();
    return true;
}

void VariableModel::varDataChanged(const QModelIndex &idx)
{
    QModelIndex first = index(idx.row(), 0, QModelIndex());
    QModelIndex end = index(idx.row(), 1, QModelIndex());
    emit dataChanged(first, end);
}

Variable *VariableModel::getVariable(const QModelIndex &idx)
{
    return static_cast<Variable *>(idx.internalPointer());
}

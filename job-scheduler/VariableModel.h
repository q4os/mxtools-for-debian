/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef VARIABLEMODEL_H
#define VARIABLEMODEL_H

#include <QAbstractItemModel>

#include "Crontab.h"

// class Variable;

class VariableModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    VariableModel(QObject *parent = nullptr)
        : QAbstractItemModel(parent)
    {
        variables = &dummy;
    }

    ~VariableModel() { }

    Qt::ItemFlags flags(const QModelIndex &) const { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
    QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
    QModelIndex index(int row, int column, const QModelIndex &) const
    {
        return createIndex(row, column, (*variables)[row]);
    }
    int columnCount(const QModelIndex &) const { return 3; }
    void resetData(QList<Variable *> *var) { variables = var; }
    int rowCount(const QModelIndex &parent) const { return (parent.isValid() ? 0 : variables->count()); }

    void varDataChanged(const QModelIndex &idx);
    static Variable *getVariable(const QModelIndex &idx);
    bool removeVariable(int row);
    bool insertVariable(int row, Variable *var);

    QList<Variable *> *variables;
    QList<Variable *> dummy;

private:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &idx, int role) const;
};

#endif

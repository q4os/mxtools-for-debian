/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "ExecuteModel.h"

#include <QtGui>

#include "Crontab.h"
#include "Execute.h"

ExecuteModel::~ExecuteModel() = default;

QVariant ExecuteModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            auto *e = static_cast<Execute *>(index.internalPointer());
            switch (index.column()) {
            case 0:
                return e->exeTime;
            case 1:
                return e->tCommands->time;
            case 2:
                return e->tCommands->user;
            case 3:
                return e->tCommands->command;
            }
        } else if (role == Qt::BackgroundRole) {
            auto *e = static_cast<Execute *>(index.internalPointer());
            switch (e->sel) {
            case 1:
                return QColor(229, 241, 255);
            case 2:
                return QColor(208, 255, 241);
            }
        } else if (role == Qt::ForegroundRole) {
            auto *e = static_cast<Execute *>(index.internalPointer());
            if (index.column() == 0 && e->flag != 0) {
                return QColor(189, 55, 44);
            } else {
                switch (e->sel) {
                case 1:
                    return QColor(0, 25, 0);
                case 2:
                    return QColor(35, 0, 0);
                }
            }
        }
    }

    return {};
}

QVariant ExecuteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Col::ExeTime:
            return tr("Execute Time");
        case Col::CronTime:
            return tr("Time");
        case Col::User:
            return tr("User");
        case Col::Command:
            return tr("Command");
        }
    }

    return {};
}
void ExecuteModel::sort(int column, Qt::SortOrder order)
{
    sortColumn = column;
    sortOrder = order;
    doSort();
}

void ExecuteModel::doSort()
{
    if (sortColumn < Col::ExeTime || sortColumn > Col::Command) {
        return;
    }

    std::sort(executes->begin(), executes->end(),
              [this](const std::unique_ptr<Execute> &lhs, const std::unique_ptr<Execute> &rhs) {
                  const QString *lhsVal;
                  const QString *rhsVal;
                  switch (sortColumn) {
                  case Col::ExeTime:
                      lhsVal = &lhs->exeTime;
                      rhsVal = &rhs->exeTime;
                      break;
                  case Col::CronTime:
                      lhsVal = &lhs->tCommands->time;
                      rhsVal = &rhs->tCommands->time;
                      break;
                  case Col::User:
                      lhsVal = &lhs->tCommands->user;
                      rhsVal = &rhs->tCommands->user;
                      break;
                  case Col::Command:
                      lhsVal = &lhs->tCommands->command;
                      rhsVal = &rhs->tCommands->command;
                      break;
                  default:
                      return false; // unreachable: sortColumn is range-checked above
                  }
                  return (sortOrder == Qt::AscendingOrder) ? (*lhsVal < *rhsVal) : (*lhsVal > *rhsVal);
              });

    emit layoutChanged();
}

Execute *ExecuteModel::getExecute(const QModelIndex &idx)
{

    if (idx.isValid()) {
        return static_cast<Execute *>(idx.internalPointer());
    } else {
        return nullptr;
    }
}

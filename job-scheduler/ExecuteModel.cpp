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
        } else if (role == Qt::BackgroundColorRole) {
            auto *e = static_cast<Execute *>(index.internalPointer());
            switch (e->sel) {
            case 1:
                return QColor(229, 241, 255);
            case 2:
                return QColor(208, 255, 241);
            }
        } else if (role == Qt::TextColorRole) {
            auto *e = static_cast<Execute *>(index.internalPointer());
            if (index.column() == 0 && e->flag != 0)
                return QColor(189, 55, 44);
            else
                switch (e->sel) {
                case 1:
                    return QColor(0, 25, 0);
                case 2:
                    return QColor(35, 0, 0);
                }
        }
    }

    return QVariant();
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

    return QVariant();
}
void ExecuteModel::sort(int column, Qt::SortOrder order)
{
    sortColumn = column;
    sortOrder = order;
    doSort();
}

bool ltExeTime(Execute *e1, Execute *e2) { return e1->exeTime < e2->exeTime; }
bool ltCronTime(Execute *e1, Execute *e2) { return e1->tCommands->time < e2->tCommands->time; }
bool ltUser(Execute *e1, Execute *e2) { return e1->tCommands->user < e2->tCommands->user; }
bool ltCommand(Execute *e1, Execute *e2) { return e1->tCommands->command < e2->tCommands->command; }
bool gtExeTime(Execute *e1, Execute *e2) { return e1->exeTime > e2->exeTime; }
bool gtCronTime(Execute *e1, Execute *e2) { return e1->tCommands->time > e2->tCommands->time; }
bool gtUser(Execute *e1, Execute *e2) { return e1->tCommands->user > e2->tCommands->user; }
bool gtCommand(Execute *e1, Execute *e2) { return e1->tCommands->command > e2->tCommands->command; }

void ExecuteModel::doSort()
{
    bool (*cmp)(Execute * e1, Execute * e2) {nullptr};
    if (sortOrder == Qt::AscendingOrder) {
        switch (sortColumn) {
        case Col::ExeTime:
            cmp = ltExeTime;
            break;
        case Col::CronTime:
            cmp = ltCronTime;
            break;
        case Col::User:
            cmp = ltUser;
            break;
        case Col::Command:
            cmp = ltCommand;
            break;
        default:
            return;
        }
    } else {
        switch (sortColumn) {
        case Col::ExeTime:
            cmp = gtExeTime;
            break;
        case Col::CronTime:
            cmp = gtCronTime;
            break;
        case Col::User:
            cmp = gtUser;
            break;
        case Col::Command:
            cmp = gtCommand;
            break;
        default:
            return;
        }
    }
    std::sort(executes->begin(), executes->end(), cmp);

    emit layoutChanged();
}

Execute *ExecuteModel::getExecute(const QModelIndex &idx)
{

    if (idx.isValid())
        return static_cast<Execute *>(idx.internalPointer());
    else
        return nullptr;
}

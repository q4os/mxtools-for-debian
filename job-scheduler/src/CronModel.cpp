/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include <memory>
#include <ranges>

#include "CronModel.h"
#include "Crontab.h"

void dumpIndex(const QModelIndex &idx, QString h);

QVariant CronModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || role != Qt::DisplayRole) {
        return {};
    }

    if (idx.parent().isValid() || isOneUser()) {
        auto *cmnd = getTCommand(idx);
        switch (idx.column()) {
        case Data::Time:
            return cmnd->time;
        case Data::User:
            return cmnd->user;
        case Data::Command:
            return cmnd->command;
        }
    } else {
        if (idx.column() == 0) {
            return getCrontab(idx)->cronOwner;
        }
    }

    return {};
}

QModelIndex CronModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }

    if (crontabs->size() > 1) {
        auto *t = static_cast<CronType *>(index.internalPointer());
        if (t->type == CronType::COMMAND) {
            auto *cmnd = static_cast<TCommand *>(t);
            Crontab *cron = cmnd->parent;
            for (size_t i = 0; i < crontabs->size(); ++i) {
                if ((*crontabs)[i].get() == cron) {
                    return createIndex(static_cast<int>(i), 0, cron);
                }
            }
            return {};
        }
    }

    return {};
}

QModelIndex CronModel::index(int row, int column, const QModelIndex &parent) const
{

    if (!parent.isValid()) {
        if (isOneUser()) {
            if (row < static_cast<int>((*crontabs).at(0)->tCommands.size())) {
                return createIndex(row, column, (*crontabs).at(0)->tCommands.at(row).get());
            }
        } else {
            if (row >= 0 && row < static_cast<int>(crontabs->size())) {
                return createIndex(row, column, (*crontabs).at(row).get());
            }
        }
    } else {
        if (!isOneUser()) {
            auto *cron = getCrontab(parent);
            if (row < static_cast<int>(cron->tCommands.size())) {
                return createIndex(row, column, cron->tCommands.at(row).get());
            }
        }
    }

    return {};
}

Qt::ItemFlags CronModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid()) {
        return {};
    }

    if (isOneUser() || idx.parent().isValid()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    } else {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }
}

int CronModel::rowCount(const QModelIndex &parent) const
{

    if (parent.isValid()) {
        if (!parent.parent().isValid()) {
            if (!isOneUser()) {
                return static_cast<int>(getCrontab(parent)->tCommands.size());
            }
        }
    } else {
        if (isOneUser()) {
            return static_cast<int>((*crontabs).at(0)->tCommands.size());
        } else {
            return static_cast<int>(crontabs->size());
        }
    }

    return 0;
}

QVariant CronModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Data::Time:
            return tr("Time");
        case Data::User:
            return tr("User");
        case Data::Command:
            return tr("Command");
        }
    }

    return {};
}

QModelIndex CronModel::removeCommand(const QModelIndex &idx)
{

    if (!idx.isValid()) {
        return {};
    }

    int cronPos = 0;
    int cmndPos = 0;
    QModelIndex del;
    if (idx.parent().isValid()) {
        cronPos = idx.parent().row();
        cmndPos = idx.row();
        del = idx.parent();
    } else {
        cronPos = 0;
        cmndPos = idx.row();
        del = QModelIndex();
    }

    beginRemoveRows(del, cmndPos, cmndPos);

    auto &commands = (*crontabs)[cronPos]->tCommands;
    commands.erase(commands.begin() + cmndPos);

    endRemoveRows();

    if (rowCount(del) > 0) {
        return index(0, 0, del);
    } else if (!isOneUser()) {
        return del;
    }

    return {};
}

QModelIndex CronModel::insertTCommand(const QModelIndex &idx, TCommand *cmnd)
{

    // insert cron command at insert position(idx)
    // if insert position is crontab, cron command be added at the end.
    //	dumpIndex(idx, "CronModel::insertTCommand");
    //	qDebug() << "CronModel::insertTCommand time=" << cmnd->time;

    int cronPos = 0;
    int cmndPos = 0;
    QModelIndex ins;
    if (idx.isValid()) {
        if (idx.parent().isValid()) {
            cronPos = idx.parent().row();
            cmndPos = idx.row() + 1;
            ins = idx.parent();
        } else {
            if (isOneUser()) {
                cronPos = 0;
                cmndPos = idx.row() + 1;
                ins = QModelIndex();
            } else {
                cronPos = idx.row();
                cmndPos = 0;
                ins = idx;
            }
        }
    } else {
        cronPos = 0;
        cmndPos = 0;
        ins = QModelIndex();
    }

    beginInsertRows(ins, cmndPos, cmndPos);

    auto &commands = (*crontabs)[cronPos]->tCommands;
    commands.insert(commands.begin() + cmndPos, std::unique_ptr<TCommand>(cmnd));

    endInsertRows();

    return index(cmndPos, 0, ins);
}

void CronModel::tCommandChanged(const QModelIndex &idx)
{

    QModelIndex from = index(idx.row(), 0, idx.parent());
    QModelIndex to = index(idx.row(), 3, idx.parent());
    emit dataChanged(from, to);
}

TCommand *CronModel::getTCommand(const QModelIndex &idx) const
{
    if (!idx.isValid()) {
        return nullptr;
    }

    if (isOneUser()) {
        return static_cast<TCommand *>(idx.internalPointer());
    }

    if (idx.parent().isValid()) {
        return static_cast<TCommand *>(idx.internalPointer());
    }

    return nullptr;
}

Crontab *CronModel::getCrontab(const QModelIndex &idx) const
{
    if (!idx.isValid() && crontabs->empty()) {
        return nullptr;
    }

    if (isOneUser()) {
        return (*crontabs)[0].get();
    }

    if (idx.parent().isValid()) {
        return static_cast<Crontab *>(idx.parent().internalPointer());
    }

    return static_cast<Crontab *>(idx.internalPointer());
}

QModelIndex CronModel::searchTCommand(TCommand *cmnd) const
{

    if (isOneUser()) {
        for (int i : std::views::iota(0, rowCount(QModelIndex()))) {
            QModelIndex idx = index(i, 0, QModelIndex());
            if (getTCommand(idx) == cmnd) {
                return idx;
            }
        }
    } else {
        for (int i : std::views::iota(0, rowCount(QModelIndex()))) {
            QModelIndex pidx = index(i, 0, QModelIndex());
            for (int j : std::views::iota(0, rowCount(pidx))) {
                QModelIndex idx = index(j, 0, pidx);
                if (getTCommand(idx) == cmnd) {
                    return idx;
                }
            }
        }
    }
    return {};
}

bool CronModel::dropMimeData(const QMimeData * /*data*/, Qt::DropAction /*action*/, int row, int /*column*/,
                             const QModelIndex &parent)
{
    //				   (0, --)
    //	   ----------
    //	   |        |  (-1, 0)
    //	   ----------
    //	               (1, --)
    //
    // ========================================
    //
    //				   (0, --)
    //	   ----------
    //	   |        |  (-1, 0)
    //	   ----------
    //	               (1, --)
    //	       ----------
    //	       |        |  (-1, 0, 0)
    //	       ----------
    //	               (1, 0)
    //	       ----------
    //	       |        |  (-1, 1, 0)
    //	       ----------
    //	               (1, --)
    //

    //	qDebug() << "CronModel::dropMimeData:row=" << row;
    //	dumpIndex(parent, "CronModel::dropMimeData");
    if (isOneUser()) {
        if (row < 0 && !parent.isValid()) {
            return false;
        }
    } else {
        if (!parent.isValid()) {
            return false;
        }
    }

    QModelIndex ins;
    QModelIndex next;
    if (row <= 0) {
        ins = parent;
        if (ins.isValid() && (isOneUser() || ins.parent().isValid())) {
            next = index(ins.row() + 1, 0, ins.parent());
        } else {
            next = index(0, 0, ins);
        }
    } else {
        ins = index(row - 1, 0, parent);
        next = index(row, 0, parent);
    }
    //	dumpIndex(ins, "CronModel::dropMimeData insert : ");

    if (getTCommand(ins) == drag || getTCommand(next) == drag) {
        return false;
    }

    auto t = std::make_unique<TCommand>();
    *t = *drag;
    Crontab *c = getCrontab(ins);
    if (c->cronOwner != QLatin1String("/etc/crontab")) {
        t->user = c->cronOwner;
    }
    t->parent = c;

    TCommand *raw_ptr = t.release();
    insertTCommand(ins, raw_ptr);

    QModelIndex del = searchTCommand(drag);
    removeCommand(del);

    drag = nullptr;

    emit moveTCommand(raw_ptr);

    return false;
}

void CronModel::dragTCommand(const QModelIndex &idx)
{
    drag = getTCommand(idx);
}

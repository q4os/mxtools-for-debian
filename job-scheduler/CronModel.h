/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef CRONMODEL_H
#define CRONMODEL_H

#include <QAbstractItemModel>

class Crontab;
class TCommand;

class CronModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CronModel(QList<Crontab *> *cron, QObject *parent = nullptr)
        : QAbstractItemModel(parent)
        , crontabs(cron)
    {
    }
    ~CronModel() { }

    enum Data { Time, User, Command };
    Qt::ItemFlags flags(const QModelIndex &) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &) const { return 4; }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &idx, int role) const;
    QModelIndex parent(const QModelIndex &index) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    void tCommandChanged(const QModelIndex &idx);
    void dragTCommand(const QModelIndex &idx);
    QModelIndex removeCComand(const QModelIndex &idx);
    QModelIndex insertTCommand(const QModelIndex &idx, TCommand *cmnd);
    QModelIndex searchTCommand(TCommand *cmnd) const;
    inline bool isOneUser() const { return (crontabs->count() == 1); }
    TCommand *getTCommand(const QModelIndex &idx) const;
    Crontab *getCrontab(const QModelIndex &idx) const;

    QList<Crontab *> *crontabs;
    TCommand *drag;

signals:
    void moveTCommand(TCommand *cmd);
};

#endif

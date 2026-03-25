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

#include <memory>
#include <vector>

class Crontab;
class TCommand;

class CronModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit CronModel(std::vector<std::unique_ptr<Crontab>> *cron, QObject *parent = nullptr)
        : QAbstractItemModel(parent),
          crontabs(cron)
    {
    }
    ~CronModel() override = default;

    enum Data { Time, User, Command };
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex & /*index*/) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/) const override
    {
        return 4;
    }
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                      const QModelIndex &parent) override;

    void tCommandChanged(const QModelIndex &idx);
    void dragTCommand(const QModelIndex &idx);
    QModelIndex removeCommand(const QModelIndex &idx);
    QModelIndex insertTCommand(const QModelIndex &idx, TCommand *cmnd);
    QModelIndex searchTCommand(TCommand *cmnd) const;
    [[nodiscard]] inline bool isOneUser() const
    {
        return (crontabs->size() == 1);
    }
    [[nodiscard]] TCommand *getTCommand(const QModelIndex &idx) const;
    [[nodiscard]] Crontab *getCrontab(const QModelIndex &idx) const;

private:
    std::vector<std::unique_ptr<Crontab>> *crontabs;
    TCommand *drag {};

signals:
    void moveTCommand(TCommand *cmd);
};

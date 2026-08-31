/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include "Crontab.h"
#include "ScrollableTreeView.h"

#include <memory>

class QPaintEvent;
class CronModel;

class CronView : public ScrollableTreeView
{
    Q_OBJECT

public:
    explicit CronView(CronModel *model, QWidget *parent = nullptr);
    ~CronView();

    void resetView();
    void hideUser(bool flag = true)
    {
        setColumnHidden(1, flag);
    }
    Crontab *getCurrentCrontab() const;
    TCommand *getCurrentTCommand() const;

protected:
    void startDrag(Qt::DropActions supportedActions) override;

public slots:
    void changeCurrent(TCommand *cmnd);
    void copyTCommand();
    void cutTCommand();
    void newTCommand();
    void pasteTCommand();
    void removeTCommand();
    void tCommandChanged();

private slots:
    void insertTCommand(TCommand *cmnd);
    void tCommandMoved(TCommand *cmnd);
    void selectChanged(const QModelIndex &cur, const QModelIndex &prev);

signals:
    void viewSelected(Crontab *, TCommand *);
    void pasted(bool flg = true);
    void dataChanged();

private:
    std::unique_ptr<TCommand> pasteData;
    CronModel *cronModel;
};

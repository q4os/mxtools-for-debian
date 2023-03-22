/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef CRONVIEW_H
#define CRONVIEW_H

#include <QTreeView>

class QPaintEvent;
class TCommand;
class Crontab;
class CronModel;

class CronView : public QTreeView
{
    Q_OBJECT

public:
    CronView(CronModel *model);

    void resetView();
    void hideUser(bool flag = true) { setColumnHidden(1, flag); }
    Crontab *getCurrentCrontab();
    TCommand *getCurrentTCommand();

protected:
    void startDrag(Qt::DropActions supportedActions);

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
    void TCommandMoved(TCommand *cmnd);
    void selectChanged(const QModelIndex &cur, const QModelIndex &prev);

signals:
    void viewSelected(Crontab *, TCommand *);
    void pasted(bool flg = true);
    void dataChanged();

private:
    void scrollTo(const QModelIndex &idx, ScrollHint hint);

    TCommand *pasteData;
    CronModel *cronModel;
};

#endif

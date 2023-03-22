/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef EXECUTEVIEW_H
#define EXECUTEVIEW_H

#include <QTreeView>

class QModelIndex;
class ExecuteModel;
class TCommand;

class ExecuteView : public QTreeView
{
    Q_OBJECT
public:
    ExecuteView(ExecuteModel *model);
    void resetView();
    void hideUser(bool flag) { setColumnHidden(2, flag); }

public slots:
    void selectChanged(const QModelIndex &idx, const QModelIndex &prev);

signals:
    void viewSelected(TCommand *cmnd);

private:
    void scrollTo(const QModelIndex &, ScrollHint);

    ExecuteModel *executeModel;
};

#endif

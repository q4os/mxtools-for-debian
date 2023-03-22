/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef VARIABLEVIEW_H
#define VARIABLEVIEW_H

#include <QTreeView>

class QModelIndex;
class Variable;
class VariableModel;

class VariableView : public QTreeView
{
    Q_OBJECT
public:
    VariableView(VariableModel *model);
    void resetView();
    void varDataChanged();
    void insertVariable();
    void removeVariable();

    VariableModel *variableModel;

private slots:
    void varSelected(const QModelIndex &cur, const QModelIndex &prev);

signals:
    void changeVar(Variable *var);

private:
    void scrollTo(const QModelIndex &, ScrollHint);

};
#endif

/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "ExecuteView.h"

#include <QHeaderView>
#include <ranges>

#include "Execute.h"
#include "ExecuteModel.h"

ExecuteView::ExecuteView(ExecuteModel *model, QWidget *parent)
    : ScrollableTreeView(parent),
      executeModel(model)
{
    setModel(executeModel);

    header()->setSortIndicatorShown(true);
    header()->setSectionsClickable(true);

    setRootIsDecorated(false);

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &ExecuteView::selectChanged);
}

void ExecuteView::resetView()
{
    for (int i : std::views::iota(0, 4)) {
        resizeColumnToContents(i);
    }
}

void ExecuteView::selectChanged(const QModelIndex &idx, const QModelIndex & /*unused*/)
{
    if (idx.isValid()) {
        auto *e = ExecuteModel::getExecute(idx);
        emit viewSelected(e->tCommands);
    }
}

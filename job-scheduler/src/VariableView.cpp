/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "VariableView.h"

#include <QScrollBar>
#include <QtGui>

#include <memory>

#include "Crontab.h"
#include "VariableModel.h"

VariableView::VariableView(VariableModel *model, QWidget *parent)
    : QTreeView(parent)
{
    variableModel = model;
    setModel(variableModel);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    setRootIsDecorated(false);

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &VariableView::varSelected);
}

void VariableView::resetView()
{
    clearSelection();
    reset();
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    if (variableModel->rowCount(QModelIndex()) > 0) {
        setCurrentIndex(variableModel->index(0, 0, QModelIndex()));
    } else {
        emit changeVar(nullptr);
    }
}

void VariableView::varSelected(const QModelIndex &cur, const QModelIndex & /*unused*/)
{
    if (cur.isValid()) {
        emit changeVar(VariableModel::getVariable(cur));
    } else {
        emit changeVar(nullptr);
    }
}

void VariableView::varDataChanged()
{
    QModelIndex index = currentIndex();
    if (index.isValid()) {
        variableModel->varDataChanged(index);
    }
}

void VariableView::insertVariable()
{
    auto v = std::make_unique<Variable>(QStringLiteral("*"), QStringLiteral("*"), QLatin1String(""));
    QModelIndex index = currentIndex();
    int pos = 0;
    if (index.isValid()) {
        pos = index.row();
    } else {
        pos = 0;
    }
    variableModel->insertVariable(pos, std::move(v));
    setCurrentIndex(variableModel->index(pos, 0, QModelIndex()));
}

void VariableView::removeVariable()
{
    QModelIndex index = currentIndex();
    if (index.isValid()) {
        variableModel->removeVariable(index.row());
        if (variableModel->rowCount(QModelIndex()) > 0) {
            setCurrentIndex(variableModel->index(0, 0, QModelIndex()));
        } else {
            emit changeVar(nullptr);
        }
    }
}

void VariableView::scrollTo(const QModelIndex &idx, ScrollHint /*hint*/)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(idx);
    if (rect.height() == 0) {
        return;
    }
    double step = 1.0 / rect.height();
    if (rect.top() < 0) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + static_cast<int>(rect.top() * step));
    } else if (rect.bottom() > area.bottom()) {
        verticalScrollBar()->setValue(verticalScrollBar()->value()
                                      + static_cast<int>((rect.bottom() - area.bottom()) * step) + 5);
    }
}

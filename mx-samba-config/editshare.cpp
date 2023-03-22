/**********************************************************************
 *  editshare.cpp
 **********************************************************************
 * Copyright (C) 2021 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          Dolphin_Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QFileDialog>
#include <QDebug>

#include "editshare.h"
#include "ui_editshare.h"

EditShare::EditShare(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditShare)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

EditShare::~EditShare()
{
    delete ui;
}

void EditShare::on_pushChooseDirectory_clicked()
{
    QFileDialog dialog;
    QString path = ui->textSharePath->text();
    if (path.isEmpty() || !QFileInfo::exists(path))
        path = QDir::homePath();
    const QString &selected = QFileDialog::getExistingDirectory(this, tr("Select directory to share"), path, QFileDialog::ShowDirsOnly);
    if (!selected.isEmpty())
        ui->textSharePath->setText(selected);
}

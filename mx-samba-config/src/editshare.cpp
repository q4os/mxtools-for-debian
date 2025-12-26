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
#include "editshare.h"
#include "ui_editshare.h"

#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

EditShare::EditShare(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::EditShare)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    connect(ui->pushChooseDirectory, &QPushButton::clicked, this, &EditShare::pushChooseDirectory_clicked);
}

EditShare::~EditShare()
{
    delete ui;
}

void EditShare::pushChooseDirectory_clicked()
{
    QString path = ui->textSharePath->text();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        path = QDir::homePath();
    }

    QString selected
        = QFileDialog::getExistingDirectory(this, tr("Select directory to share"), path, QFileDialog::ShowDirsOnly);
    if (!selected.isEmpty()) {
        ui->textSharePath->setText(selected);
    }
}

void EditShare::accept()
{
    const auto groupBoxes = ui->frameUsers->findChildren<QGroupBox *>(QString(), Qt::FindDirectChildrenOnly);
    bool anySelected = false;
    for (auto *groupBox : groupBoxes) {
        const auto radioButtons = groupBox->findChildren<QRadioButton *>(QString(), Qt::FindDirectChildrenOnly);

        for (auto *radio : radioButtons) {
            if (radio->isChecked()) {
                anySelected = true;
                break;
            }
        }

        if (anySelected) {
            break;
        }
    }

    if (!anySelected) {
        QMessageBox::warning(this, tr("Warning"), tr("Select access for at least one user before continuing."));
        return;
    }

    QDialog::accept();
}

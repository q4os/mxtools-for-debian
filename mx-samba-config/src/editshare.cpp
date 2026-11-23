/**********************************************************************
 *  editshare.cpp
 **********************************************************************
 * Copyright (C) 2021-2026 MX Authors
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

#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

EditShare::EditShare(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::EditShare)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    connect(ui->pushChooseDirectory, &QPushButton::clicked, this, &EditShare::pushChooseDirectory_clicked);
}

EditShare::~EditShare()
{
    delete ui;
}

QGroupBox *EditShare::addUser(const QString &principal)
{
    auto *groupBox = new QGroupBox(principal, ui->frameUsers);
    groupBox->setObjectName(principal);
    auto *layout = new QHBoxLayout(groupBox);
    const auto addRadio = [&](const QString &text, const QString &prefix) {
        auto *radio = new QRadioButton(text, groupBox);
        radio->setObjectName(prefix + principal);
        layout->addWidget(radio);
        connect(radio, &QRadioButton::pressed, radio, [radio]() { radio->setAutoExclusive(!radio->isChecked()); });
    };
    addRadio(QCoreApplication::translate("MainWindow", "&Deny"), "*Deny*");
    addRadio(QCoreApplication::translate("MainWindow", "&Read Only"), "*ReadOnly*");
    addRadio(QCoreApplication::translate("MainWindow", "&Full Access"), "*FullAccess*");
    layout->addStretch(1);
    auto *usersLayout = ui->frameUsers->layout();
    QLayoutItem *spacer = nullptr;
    if (usersLayout->count() > 0 && usersLayout->itemAt(usersLayout->count() - 1)->spacerItem()) {
        spacer = usersLayout->takeAt(usersLayout->count() - 1);
    }
    usersLayout->addWidget(groupBox);
    if (spacer) {
        usersLayout->addItem(spacer);
    }
    return groupBox;
}

void EditShare::addRemoveButton(QGroupBox *groupBox)
{
    auto *remove = new QPushButton(tr("Remove"), groupBox);
    remove->setAutoDefault(false);
    remove->setToolTip(tr("Remove this access rule"));
    groupBox->layout()->addWidget(remove);
    connect(remove, &QPushButton::clicked, groupBox, [groupBox]() {
        const auto radios = groupBox->findChildren<QRadioButton *>(QString(), Qt::FindDirectChildrenOnly);
        for (auto *radio : radios) {
            radio->setAutoExclusive(false);
            radio->setChecked(false);
            radio->setAutoExclusive(true);
        }
    });
}

QStringList EditShare::permissions() const
{
    const auto groupBoxes = ui->frameUsers->findChildren<QGroupBox *>(QString(), Qt::FindDirectChildrenOnly);
    QStringList order = permissionOrder;
    for (const auto *groupBox : groupBoxes) {
        if (!order.contains(groupBox->objectName())) {
            order << groupBox->objectName();
        }
    }

    QStringList result;
    for (const QString &name : order) {
        auto *groupBox = ui->frameUsers->findChild<QGroupBox *>(name, Qt::FindDirectChildrenOnly);
        if (!groupBox || name.isEmpty() || !groupBox->isEnabled()) {
            continue;
        }
        QString permission;
        if (groupBox->findChild<QRadioButton *>("*Deny*" + name)->isChecked()) {
            permission = "d";
        } else if (groupBox->findChild<QRadioButton *>("*ReadOnly*" + name)->isChecked()) {
            permission = "r";
        } else if (groupBox->findChild<QRadioButton *>("*FullAccess*" + name)->isChecked()) {
            permission = "f";
        } else {
            continue;
        }
        const QString principal = groupBox->property("principal").isValid()
                                      ? groupBox->property("principal").toString() : name;
        result << principal + ':' + permission;
    }
    return result;
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
    // net usershare add replaces a share of the same name, and share names
    // are case-insensitive
    const QString shareName = ui->textShareName->text();
    if (existingShareNames.contains(shareName, Qt::CaseInsensitive)) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("A share named '%1' already exists. Choose another name or edit the existing share.")
                                 .arg(shareName));
        return;
    }

    if (permissions().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Select access for at least one user before continuing."));
        return;
    }

    QDialog::accept();
}

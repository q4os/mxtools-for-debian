/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "SaveDialog.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QtGui>

SaveDialog::SaveDialog(const QString &user, const QString &text, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Save New Schedule"));
    setWindowIcon(QIcon(":/images/job-scheduler.svg"));

    QHBoxLayout *h = nullptr;
    QPushButton *okButton = nullptr;
    QPushButton *cancelButton = nullptr;
    QLabel *userLabel = nullptr;
    QString label = "  " + user + "  ";

    auto *mainLayout = new QVBoxLayout;
    {
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("User:")));
            h->addWidget((userLabel = new QLabel(label)));
            h->addStretch();
        }
        mainLayout->addWidget(cronText = new QTextEdit());
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addStretch();
            h->addWidget((okButton = new QPushButton(tr("&OK"))));
            h->addWidget((cancelButton = new QPushButton(tr("&Cancel"))));
        }
    }
    userLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    cronText->setPlainText(text);

    setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked, this, &SaveDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SaveDialog::reject);

    resize(410, 500);
}

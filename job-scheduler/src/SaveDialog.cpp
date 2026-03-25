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

#include "constants.h"
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
            h->addWidget(new QLabel(tr("User:"), this));
            h->addWidget((userLabel = new QLabel(label, this)));
            h->addStretch();
        }
        mainLayout->addWidget(cronText = new QTextEdit(this));
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addStretch();
            h->addWidget((okButton = new QPushButton(tr("&OK"), this)));
            h->addWidget((cancelButton = new QPushButton(tr("&Cancel"), this)));
        }
    }
    userLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    cronText->setPlainText(text);

    setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked, this, &SaveDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SaveDialog::reject);

    resize(JobScheduler::SAVE_DIALOG_WIDTH, JobScheduler::SAVE_DIALOG_HEIGHT);
}

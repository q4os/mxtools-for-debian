/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "TCommandEdit.h"

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtGui>
#include <ranges>

#include "Clib.h"
#include "CronTime.h"
#include "Crontab.h"
#include "TimeDialog.h"
#include "constants.h"
#include <chrono>

using namespace std::chrono_literals;

TCommandEdit::TCommandEdit(QWidget *parent)
    : QWidget(parent)
{

    QPushButton *timeButton = nullptr;
    QGroupBox *exeBox = nullptr;
    QHBoxLayout *h = nullptr;

    auto *mainLayout = new QVBoxLayout;
    {
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("User:"), this));
            h->addWidget((userCombo = new QComboBox(this)));
            h->addWidget((userLabel = new QLabel(QLatin1String(""), this)));
            h->addStretch();
        }
        mainLayout->addSpacing(5);
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("Time:"), this));
            h->addWidget((timeEdit = new QLineEdit(this)));
            h->addWidget((timeButton = new QPushButton(
                              QIcon::fromTheme(QStringLiteral("edit-symbolic"), QIcon(":/images/edit_small.png")),
                              tr("Time String E&ditor"), this)));
            timeButton->setMinimumSize(
                QSize(JobScheduler::COMMAND_TIME_BUTTON_MIN_WIDTH, timeButton->maximumHeight()));
        }
        mainLayout->addSpacing(5);
        mainLayout->addWidget(new QLabel(tr("Command:"), this));
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget((commandEdit = new QLineEdit(this)));
        }
        mainLayout->addSpacing(5);
        mainLayout->addWidget(new QLabel(tr("Comment:"), this));
        mainLayout->addWidget((commentEdit = new QTextEdit(this)));
        mainLayout->addSpacing(5);
        mainLayout->addWidget((exeBox = new QGroupBox(tr("Job Schedule:"), this)));
        {
            exeBox->setLayout((h = new QHBoxLayout));
            {
                h->addWidget((exeLabel = new QLabel(QStringLiteral("\n\n\n\n\n\n\n"), this)));
            }
        }
    }
    setLayout(mainLayout);

    exeLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    userLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    userCombo->addItems(Clib::allUsers());
    userCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    userLabel->hide();

    commentEdit->setAutoFormatting(QTextEdit::AutoNone);

    viewChanging = true;

    connect(commandEdit, &QLineEdit::textEdited, this, &TCommandEdit::commandEdited);
    connect(timeEdit, &QLineEdit::textEdited, this, &TCommandEdit::timeEdited);
    connect(commentEdit, &QTextEdit::textChanged, this, &TCommandEdit::commentEdited);
    connect(userCombo, qOverload<int>(&QComboBox::activated), this, &TCommandEdit::userChanged);
    timer.start(1min);
    connect(&timer, &QTimer::timeout, this, &TCommandEdit::resetExeTime);
    connect(timeButton, &QPushButton::clicked, this, &TCommandEdit::doTimeDialog);
}

void TCommandEdit::changeCurrent(Crontab *cron, TCommand *cmnd)
{
    viewChanging = true;
    tCommand = cmnd;
    if (cmnd == nullptr) {
        setEnabled(false);
    } else {
        setEnabled(true);
        timeEdit->setText(tCommand->time);
        timeEdit->setCursorPosition(0);
        if (cron->cronOwner == QLatin1String("/etc/crontab")) {
            userCombo->setCurrentIndex(userCombo->findText(tCommand->user));
            userCombo->show();
            userLabel->hide();
        } else {
            userLabel->setText("  " + cron->cronOwner + "  ");
            userCombo->hide();
            userLabel->show();
        }
        commandEdit->setText(tCommand->command);
        commandEdit->setCursorPosition(0);
        commentEdit->setPlainText(tCommand->comment);
        setExecuteList(tCommand->time);
    }
    viewChanging = false;
}

void TCommandEdit::setExecuteList(const QString &time)
{

    CronTime cronTime(time);
    if (!cronTime.isValid()) {
        exeLabel->setText("\n\n   " + tr("Time Format Error") + "\n\n\n");
        return;
    }
    QDate today = QDate::currentDate();
    QDate tommorow = today.addDays(1);
    QDateTime cur(QDateTime::currentDateTime());
    QDateTime dt = cur;
    QString str;
    for (int i : std::views::iota(0, 7)) {
        if (!str.isEmpty()) {
            str += '\n';
        }
        dt = cronTime.getNextTime(dt);
        if (!dt.isValid()) {
            exeLabel->setText("\n\n   " + tr("No matching schedule") + "\n\n\n");
            return;
        }
        qint64 sec = cur.secsTo(dt);
        str += QStringLiteral("%1 - %2:%3 later")
                   .arg(dt.toString(QStringLiteral("yyyy-MM-dd(ddd) hh:mm")))
                   .arg(sec / (60 * 60))
                   .arg((sec / 60) % 60, 2, 10, QChar('0'));
        if (dt.date() == today) {
            str += QStringLiteral(" - %1").arg(tr("Today"));
        } else if (dt.date() == tommorow) {
            str += QStringLiteral(" - %1").arg(tr("Tomorrow"));
        }
    }
    exeLabel->setText(str);
}

void TCommandEdit::commandEdited(const QString &str)
{
    tCommand->command = str;
    emit dataChanged();
}

void TCommandEdit::timeEdited(const QString &str)
{
    tCommand->time = str;
    emit dataChanged();
    setExecuteList(str);
}

void TCommandEdit::commentEdited()
{
    if (!viewChanging) {
        tCommand->comment = commentEdit->toPlainText();
        emit dataChanged();
    }
}

void TCommandEdit::userChanged(int index)
{

    tCommand->user = userCombo->itemText(index);
    emit dataChanged();
}

void TCommandEdit::resetExeTime()
{
    if (timeEdit->text().isEmpty()) {
        return;
    }
    setExecuteList(timeEdit->text());
}

void TCommandEdit::doTimeDialog()
{
    TimeDialog dialog(timeEdit->text(), this);
    int ret = dialog.exec();
    if (ret == QDialog::Accepted) {
        QString s = dialog.time();
        if (timeEdit->text() != s) {
            timeEdit->setText(s);
            setExecuteList(s);
            tCommand->time = s;
            emit dataChanged();
        }
    }
}

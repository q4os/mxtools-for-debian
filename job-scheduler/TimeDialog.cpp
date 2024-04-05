/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "TimeDialog.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QStyle>
#include <QtGui>
#include <utility>

#include "CronTime.h"

TimeButton::TimeButton(const QString &label, QWidget *parent)
    : QPushButton(label, parent)
{
    init();
}
TimeButton::TimeButton(int label, QWidget *parent)
    : QPushButton(parent)
{
    setText(QString::number(label));
    setFixedSize(27, 27);
    init();
}
void TimeButton::init()
{
    setCheckable(true);
    connect(this, &TimeButton::toggled, this, &TimeButton::buttonToggled);
}

void TimeButton::buttonToggled(bool chk)
{
    if (chk) {
        QPalette plt = palette();
        plt.setColor(QPalette::Button, QColor(163, 194, 186));
        plt.setColor(QPalette::ButtonText, QColor(0, 66, 0));
        setPalette(plt);
    } else {
        setPalette(style()->standardPalette());
    }
}

const QStringList MonthName = {"January", "February", "March",     "April",   "May",      "June",
                               "July",    "August",   "September", "October", "November", "December"};
const QStringList WeekName = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const QStringList SimpleName = {"@hourly", "@daily", "@weekly", "@monthly", "@yearly", "@reboot"};

TimeDialog::TimeDialog(QString time, QWidget *parent)
    : QDialog(parent),
      inTime(std::move(time))
{
    QHBoxLayout *h = nullptr;
    QGridLayout *g = nullptr;
    QVBoxLayout *v = nullptr;
    QGroupBox *b = nullptr;
    QPushButton *resetButton = nullptr;
    QPushButton *okButton = nullptr;
    QPushButton *cancelButton = nullptr;

    auto *mainLayout = new QVBoxLayout;
    {
        mainLayout->addLayout((h = new QHBoxLayout)); // time text
        {
            h->addWidget(new QLabel(tr("time")));
            h->addWidget((timeEdit = new QLineEdit()));
        }
        mainLayout->addLayout((h = new QHBoxLayout)); // minutes, hour, day
        {
            h->addWidget((b = new QGroupBox(tr("Minute"))));
            b->setLayout((g = new QGridLayout()));
            minuteBGroup = new QButtonGroup();
            minuteBGroup->setExclusive(false);
            {
                g->setSpacing(0);
                for (int i = 0; i < 60; ++i) {
                    auto *btn = new TimeButton(i);
                    g->addWidget(btn, i / 10, i % 10, 1, 1);
                    minuteBGroup->addButton(btn);
                }
            }
            h->addWidget((b = new QGroupBox(tr("Hour"))));
            b->setLayout((g = new QGridLayout()));
            hourBGroup = new QButtonGroup();
            hourBGroup->setExclusive(false);
            {
                g->setSpacing(0);
                g->addWidget(new QLabel(tr("AM ")), 0, 0, 1, 6, Qt::AlignRight);
                for (int i = 0; i < 12; ++i) {
                    QPushButton *btn = new TimeButton(i);
                    g->addWidget(btn, i / 6 + 1, i % 6, 1, 1);
                    hourBGroup->addButton(btn);
                }
                g->addWidget(new QLabel(tr("PM ")), 3, 0, 1, 6, Qt::AlignRight);
                for (int i = 12; i < 24; ++i) {
                    QPushButton *btn = new TimeButton(i);
                    g->addWidget(btn, i / 6 + 2, i % 6, 1, 1);
                    hourBGroup->addButton(btn);
                }
            }
            h->addWidget((b = new QGroupBox(tr("Day"))));
            b->setLayout((g = new QGridLayout()));
            dayBGroup = new QButtonGroup();
            dayBGroup->setExclusive(false);
            {
                g->setSpacing(0);
                for (int i = 0; i < 31; ++i) {
                    QPushButton *btn = new TimeButton(i + 1);
                    g->addWidget(btn, i / 7, i % 7, 1, 1);
                    dayBGroup->addButton(btn);
                }
            }
        }
        mainLayout->addLayout((h = new QHBoxLayout)); // month, week, Simple
        {
            h->addWidget((b = new QGroupBox(tr("Month"))));
            b->setLayout((g = new QGridLayout()));
            monthBGroup = new QButtonGroup();
            monthBGroup->setExclusive(false);
            {
                g->setColumnMinimumWidth(0, 120);
                g->setColumnMinimumWidth(1, 120);
                for (int i = 0; i < 12; ++i) {
                    QString str = QStringLiteral("%1(%2)").arg(MonthName.at(i)).arg(i + 1);
                    QPushButton *btn = new TimeButton(str);
                    g->addWidget(btn, i % 6, i / 6);
                    monthBGroup->addButton(btn);
                }
            }
            h->addWidget((b = new QGroupBox(tr("Week"))));
            b->setLayout((v = new QVBoxLayout()));
            weekBGroup = new QButtonGroup();
            weekBGroup->setExclusive(false);
            {
                for (int i = 0; i < 7; ++i) {
                    QString str = QStringLiteral("%1(%2)").arg(WeekName.at(i)).arg(i);
                    auto *btn = new TimeButton(str);
                    v->addWidget(btn);
                    weekBGroup->addButton(btn);
                }
            }
            h->addWidget((b = new QGroupBox(tr("Simple"))));
            b->setLayout((v = new QVBoxLayout()));
            simpleBGroup = new QButtonGroup();
            simpleBGroup->setExclusive(false);
            {
                for (int i = 0; i < 6; ++i) {
                    QPushButton *btn = new TimeButton(SimpleName.at(i));
                    v->addWidget(btn);
                    simpleBGroup->addButton(btn);
                }
            }
            h->addLayout((v = new QVBoxLayout));
            {
                v->addStretch();
                v->addWidget((litCheckBox = new QCheckBox(tr("Enable Literal"))));
                v->addStretch();
                v->addWidget((resetButton = new QPushButton(tr("&Reset"))));
                v->addWidget((cancelButton = new QPushButton(tr("&Cancel"))));
                v->addWidget((okButton = new QPushButton(tr("&Ok"))));
            }
        }
    }
    setLayout(mainLayout);

    outTime = inTime;
    if (!CronTime(outTime).isValid()) {
        timeEdit->setText(QStringLiteral("Time format error"));
        outTime = QStringLiteral("* * * * *");
    } else {
        timeEdit->setText(outTime);
    }

    useLiteral = true;
    litCheckBox->setChecked(useLiteral);
    initButtons(outTime);

    connect(minuteBGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this,
            &TimeDialog::minuteButtonClicked);
    connect(hourBGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this,
            &TimeDialog::hourButtonClicked);
    connect(dayBGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &TimeDialog::dayButtonClicked);
    connect(monthBGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this,
            &TimeDialog::monthButtonClicked);
    connect(weekBGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this,
            &TimeDialog::weekButtonClicked);
    connect(simpleBGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this,
            &TimeDialog::simpleButtonClicked);
    connect(litCheckBox, &QCheckBox::stateChanged, this, &TimeDialog::litCheckBoxChanged);
    connect(resetButton, &QPushButton::clicked, this, &TimeDialog::resetClicked);
    connect(okButton, &QPushButton::clicked, this, &TimeDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &TimeDialog::reject);
}

void TimeDialog::initButtons(const QString &time)
{
    CronTime ctime(time);
    auto btn = minuteBGroup->buttons();
    if (CronTime::isFill(ctime.minute)) {
        for (auto *i : btn) {
            i->setChecked(false);
        }
    } else {
        for (int i = 0; i < btn.size(); ++i) {
            btn.at(i)->setChecked(ctime.minute.at(i));
        }
    }

    btn = hourBGroup->buttons();
    if (CronTime::isFill(ctime.hour)) {
        for (auto *i : qAsConst(btn)) {
            i->setChecked(false);
        }
    } else {
        for (int i = 0; i < btn.size(); ++i) {
            btn.at(i)->setChecked(ctime.hour.at(i));
        }
    }

    btn = dayBGroup->buttons();
    if (CronTime::isFill(ctime.day)) {
        for (auto *i : qAsConst(btn)) {
            i->setChecked(false);
        }
    } else {
        for (int i = 0; i < btn.size(); ++i) {
            btn.at(i)->setChecked(ctime.day.at(i));
        }
    }

    btn = monthBGroup->buttons();
    if (CronTime::isFill(ctime.month)) {
        for (auto *i : qAsConst(btn)) {
            i->setChecked(false);
        }
    } else {
        for (int i = 0; i < btn.size(); ++i) {
            btn.at(i)->setChecked(ctime.month.at(i));
        }
    }

    btn = weekBGroup->buttons();
    if (CronTime::isFill(ctime.week)) {
        for (auto *i : qAsConst(btn)) {
            i->setChecked(false);
        }
    } else {
        for (int i = 0; i < btn.size(); ++i) {
            btn.at(i)->setChecked(ctime.week.at(i));
        }
    }
}

void TimeDialog::minuteButtonClicked(QAbstractButton * /*unused*/)
{
    CronTime ctime(outTime);
    auto btn = minuteBGroup->buttons();
    int cnt = 0;
    for (auto *i : btn) {
        if (i->isChecked()) {
            cnt++;
        }
    }
    for (int i = 0; i < btn.size(); ++i) {
        if (cnt == 0) {
            ctime.minute[i] = true;
        } else {
            ctime.minute[i] = btn.at(i)->isChecked();
        }
    }

    btn = simpleBGroup->buttons();
    for (auto *i : qAsConst(btn)) {
        i->setChecked(false);
    }

    outTime = ctime.toString(useLiteral);
    timeEdit->setText(outTime);
}
void TimeDialog::hourButtonClicked(QAbstractButton * /*unused*/)
{
    CronTime ctime(outTime);
    auto btn = hourBGroup->buttons();
    int cnt = 0;
    for (auto *i : btn) {
        if (i->isChecked()) {
            cnt++;
        }
    }
    for (int i = 0; i < btn.size(); ++i) {
        if (cnt == 0) {
            ctime.hour[i] = true;
        } else {
            ctime.hour[i] = btn.at(i)->isChecked();
        }
    }

    btn = simpleBGroup->buttons();
    for (auto *i : qAsConst(btn)) {
        if (i->isChecked()) {
            i->setChecked(false);
        }
    }

    outTime = ctime.toString(useLiteral);
    timeEdit->setText(outTime);
}
void TimeDialog::dayButtonClicked(QAbstractButton * /*unused*/)
{
    CronTime ctime(outTime);
    auto btn = dayBGroup->buttons();
    int cnt = 0;
    for (auto *i : btn) {
        if (i->isChecked()) {
            cnt++;
        }
    }
    for (int i = 0; i < btn.size(); ++i) {
        if (cnt == 0) {
            ctime.day[i] = true;
        } else {
            ctime.day[i] = btn.at(i)->isChecked();
        }
    }

    btn = simpleBGroup->buttons();
    for (auto *i : qAsConst(btn)) {
        if (i->isChecked()) {
            i->setChecked(false);
        }
    }

    outTime = ctime.toString(useLiteral);
    timeEdit->setText(outTime);
}
void TimeDialog::monthButtonClicked(QAbstractButton * /*unused*/)
{
    CronTime ctime(outTime);
    auto btn = monthBGroup->buttons();
    int cnt = 0;
    for (auto *i : btn) {
        if (i->isChecked()) {
            cnt++;
        }
    }
    for (int i = 0; i < btn.size(); ++i) {
        if (cnt == 0) {
            ctime.month[i] = true;
        } else {
            ctime.month[i] = btn.at(i)->isChecked();
        }
    }

    btn = simpleBGroup->buttons();
    for (auto *i : qAsConst(btn)) {
        if (i->isChecked()) {
            i->setChecked(false);
        }
    }

    outTime = ctime.toString(useLiteral);
    timeEdit->setText(outTime);
}
void TimeDialog::weekButtonClicked(QAbstractButton * /*unused*/)
{
    CronTime ctime(outTime);
    auto btn = weekBGroup->buttons();
    int cnt = 0;
    for (auto *i : btn) {
        if (i->isChecked()) {
            cnt++;
        }
    }
    for (int i = 0; i < btn.size(); ++i) {
        if (cnt == 0) {
            ctime.week[i] = true;
        } else {
            ctime.week[i] = btn.at(i)->isChecked();
        }
    }

    if (ctime.week.at(0)) {
        ctime.week[7] = true;
    }

    btn = simpleBGroup->buttons();
    for (auto *i : qAsConst(btn)) {
        if (i->isChecked()) {
            i->setChecked(false);
        }
    }

    outTime = ctime.toString(useLiteral);
    timeEdit->setText(outTime);
}
void TimeDialog::simpleButtonClicked(QAbstractButton *b)
{
    auto btn = simpleBGroup->buttons();
    for (auto *i : btn) {
        if (i == b) {
            outTime = i->text();
        } else {
            i->setChecked(false);
        }
    }

    //	QString numeric;
    //	if ( outTime == "@Hourly" ) numeric = "0 * * * *";
    //	else if (outTime == "@Daily") numeric = "0 0 * * *";
    //	else if (outTime == "@Weekly") numeric = "0 0 * * 0";
    //	else if (outTime == "@Monthly") numeric = "0 0 1 * *";
    //	else if (outTime == "@Yearly") numeric = "0 0 1 1 *";
    //	else if (outTime == "@Reboot") numeric = "0 0 * * *";
    //
    //	initButtons(numeric);
    //	if (!useLiteral) {
    //		outTime = numeric;
    //	}
    if (!useLiteral) {
        outTime = CronTime(outTime).toString(false);
    }
    initButtons(outTime);

    timeEdit->setText(outTime);
}
void TimeDialog::litCheckBoxChanged(int state)
{
    useLiteral = (state == Qt::Checked);
    outTime = CronTime(outTime).toString(useLiteral);
    timeEdit->setText(outTime);
}
void TimeDialog::resetClicked()
{
    outTime = inTime;
    timeEdit->setText(outTime);
    initButtons(outTime);

    auto btn = simpleBGroup->buttons();
    for (auto *i : btn) {
        i->setChecked(false);
    }
}

/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include "ExecuteList.h"

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <memory>
#include <ranges>

#include "CronTime.h"
#include "Crontab.h"
#include "Execute.h"
#include "ExecuteModel.h"
#include "ExecuteView.h"
#include "constants.h"

ExecuteList::ExecuteList(int maxN, int maxD, std::vector<std::unique_ptr<Crontab>> *cron, QWidget *parent)
    : QWidget(parent),
      maxNum(maxN),
      maxDate(maxD),
      crontabs(cron)
{
    executeModel = new ExecuteModel(&executes, this);
    QHBoxLayout *h = nullptr;
    QPushButton *resetButton = nullptr;
    auto *numSpinBox = new QSpinBox(this);
    auto *dateSpinBox = new QSpinBox(this);

    auto *mainLayout = new QVBoxLayout;
    {
        mainLayout->addWidget(executeView = new ExecuteView(executeModel, this));
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("Max Item"), this));
            h->addWidget(numSpinBox);
            h->addWidget(new QLabel(tr("Max Date"), this));
            h->addWidget(dateSpinBox);
            h->addWidget(new QLabel(tr("Select"), this));
            h->addWidget(countLabel = new QLabel(QLatin1String(""), this));
            h->addStretch();
            h->addWidget(resetButton
                         = new QPushButton(QIcon::fromTheme(QStringLiteral("undo"), QIcon(":/images/undo_small.png")),
                                           tr("&Update"), this));
        }
    }
    //	mainLayout->setMargin(0);
    setLayout(mainLayout);

    numSpinBox->setRange(JobScheduler::EXECUTE_LIST_MIN, JobScheduler::EXECUTE_LIST_MAX);
    numSpinBox->setSingleStep(JobScheduler::EXECUTE_LIST_STEP);
    numSpinBox->setValue(maxNum);
    dateSpinBox->setRange(JobScheduler::EXECUTE_LIST_MIN, JobScheduler::EXECUTE_LIST_MAX);
    dateSpinBox->setValue(maxDate);
    countLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    connect(resetButton, &QPushButton::clicked, this, &ExecuteList::dataChanged);
    connect(numSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ExecuteList::numChanged);
    connect(dateSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ExecuteList::dateChanged);

    curCrontab = nullptr;
    curTCommand = nullptr;
}

ExecuteList::~ExecuteList() = default;

void ExecuteList::dataChanged()
{

    if (!isVisible()) {
        return;
    }

    QDateTime stopTime = QDateTime::currentDateTime().addDays(maxDate);

    executeView->clearSelection();
    executes.clear();
    QList<TCommand *> cmnd;
    QList<QDateTime> date;
    for (const auto &cron : std::as_const(*crontabs)) {
        for (const auto &cc : std::as_const(cron->tCommands)) {
            CronTime ct(cc->time);
            if (ct.isValid()) {
                QDateTime next = ct.getNextTime(QDateTime::currentDateTime());
                if (next.isValid()) {
                    cmnd << cc.get();
                    date << next;
                } else {
                    executes.push_back(std::make_unique<Execute>(cc.get(), QStringLiteral("No matching schedule"), -1));
                }
            } else {
                executes.push_back(std::make_unique<Execute>(cc.get(), QStringLiteral("Time Format Error"), -1));
            }
        }
    }
    itemCount = 0;
    if (!cmnd.isEmpty()) {
        for (int i : std::views::iota(0, maxNum)) {
            int p = 0;
            QDateTime cur = date.at(0);
            for (int j : std::views::iota(1, cmnd.size())) {
                if (cur > date.at(j)) {
                    cur = date.at(j);
                    p = j;
                }
            }
            if (cur > stopTime) {
                break;
            }
            executes.push_back(std::make_unique<Execute>(cmnd.at(p), cur.toString(QStringLiteral("yyyy-MM-dd(ddd) hh:mm"))));
            itemCount++;
            date[p] = CronTime(cmnd.at(p)->time).getNextTime(cur);
        }
    }

    executeView->hideUser(crontabs->size() == 1);

    changeCurrent(curCrontab, curTCommand);
    executeView->resetView();
}

void ExecuteList::changeCurrent(Crontab *cron, TCommand *cmnd)
{
    curCrontab = cron;
    curTCommand = cmnd;
    if (!isVisible()) {
        return;
    }

    for (const auto &e : executes) {
        e->sel = 0;
    }

    int sel = 0;
    if (crontabs->size() > 1 && cron != nullptr) {
        for (const auto &e : executes) {
            if (e->tCommands->parent == cron) {
                e->sel = 1;
            }
        }
    }

    if (cmnd != nullptr) {
        for (const auto &e : executes) {
            if (e->tCommands == cmnd) {
                e->sel = 2;
                sel++;
            }
        }
    }

    countLabel->setText(QStringLiteral("%1/%2").arg(sel).arg(itemCount));
    executeModel->doSort();
}

void ExecuteList::numChanged(int num)
{
    maxNum = num;
}

void ExecuteList::dateChanged(int num)
{
    maxDate = num;
}

void ExecuteList::setVisible(bool flag)
{
    QWidget::setVisible(flag);

    if (flag) {
        dataChanged();
    }
}

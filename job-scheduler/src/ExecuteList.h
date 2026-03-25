/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <memory>
#include <vector>

#include <QWidget>

class QLabel;
class Crontab;
class TCommand;
class ExecuteView;
class ExecuteModel;
class Execute;

class ExecuteList : public QWidget
{
    Q_OBJECT

public:
    ExecuteList(int maxN, int maxD, std::vector<std::unique_ptr<Crontab>> *cron, QWidget *parent = nullptr);
    ~ExecuteList() override;

    std::vector<std::unique_ptr<Execute>> executes;

    int maxNum;
    int maxDate;

    ExecuteView *executeView;

public slots:
    void dataChanged();
    void changeCurrent(Crontab *cron, TCommand *cmnd);
    void numChanged(int num);
    void dateChanged(int num);
    void setVisible(bool flag) override;

private:
    int itemCount {};
    QLabel *countLabel;
    ExecuteModel *executeModel;
    std::vector<std::unique_ptr<Crontab>> *crontabs;
    Crontab *curCrontab;
    TCommand *curTCommand;
};

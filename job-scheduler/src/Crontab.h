/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#pragma once

#include <QList>
#include <QString>
#include <memory>
#include <utility>
#include <vector>

class Crontab;

class CronType
{
public:
    enum DataType { CRON, COMMAND };
    CronType() = default;
    explicit CronType(const int t)
        : type(t)
    {
    }
    int type;
};

class TCommand : public CronType
{
public:
    TCommand() = default;
    TCommand(QString t, QString u, QString cmnd, QString cmnt, Crontab *p)
        : CronType(CronType::COMMAND),
          time(std::move(t)),
          user(std::move(u)),
          command(std::move(cmnd)),
          comment(std::move(cmnt)),
          parent(p)
    {
    }
    ~TCommand() = default;

    // private:
    QString time;
    QString user;
    QString command;
    QString comment;
    Crontab *parent {};
};

class Variable
{
public:
    Variable(QString n, QString v, QString c)
        : name(std::move(n)),
          value(std::move(v)),
          comment(std::move(c))
    {
    }
    ~Variable() = default;

    QString name;
    QString value;
    QString comment;
};

class Crontab : public CronType
{
public:
    Crontab() = default;
    explicit Crontab(const QString &user);
    ~Crontab();

    QString getCrontab(const QString &user);
    bool putCrontab(const QString &text);
    bool putCrontab()
    {
        return putCrontab(cronText());
    }

    void setup(const QString &str);
    QString writeTempFile(const QString &text, const QString &tmp);
    static QString list2String(const QStringList &list);
    QString cronText();

    QString estr{};

    // private:
    QString cronOwner{};
    QString comment{};
    bool changed = false;
    std::vector<std::unique_ptr<Variable>> variables;
    std::vector<std::unique_ptr<TCommand>> tCommands;
};

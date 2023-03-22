/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#ifndef CRONTAB_H
#define CRONTAB_H

#include <QList>
#include <QString>

class Crontab;

class CronType
{
public:
    enum DataType { CRON, COMMAND };
    CronType() { }
    CronType(const int t)
        : type(t)
    {
    }
    int type;
};

class TCommand : public CronType
{
public:
    TCommand() { }
    TCommand(const QString &t, const QString &u, const QString &cmnd, const QString &cmnt, Crontab *p)
        : CronType(CronType::COMMAND)
        , time(t)
        , user(u)
        , command(cmnd)
        , comment(cmnt)
        , parent(p)
    {
    }
    ~TCommand() { }

    // private:
    QString time;
    QString user;
    QString command;
    QString comment;
    Crontab *parent;
};

class Variable
{
public:
    Variable(const QString &n, const QString &v, const QString &c)
        : name(n)
        , value(v)
        , comment(c)
    {
    }
    ~Variable() { }

    QString name;
    QString value;
    QString comment;
};

class Crontab : public CronType
{
public:
    Crontab() { }
    Crontab(const QString &user);
    ~Crontab();

    QString getCrontab(const QString &user);
    bool putCrontab(const QString &text);
    bool putCrontab() { return putCrontab(cronText()); }

    void setup(const QString &str);
    QString writeTempFile(const QString &test, const QString &tmp);
    static QString list2String(const QStringList &list);
    QString cronText();

    QString estr;

    // private:
    QString cronOwner;
    QString comment;
    bool changed;
    QList<Variable *> variables;
    QList<TCommand *> tCommands;
};

#endif

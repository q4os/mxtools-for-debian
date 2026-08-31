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

    [[nodiscard]] const QString &getTime() const
    {
        return time;
    }
    void setTime(QString v)
    {
        time = std::move(v);
    }
    [[nodiscard]] const QString &getUser() const
    {
        return user;
    }
    void setUser(QString v)
    {
        user = std::move(v);
    }
    [[nodiscard]] const QString &getCommand() const
    {
        return command;
    }
    void setCommand(QString v)
    {
        command = std::move(v);
    }
    [[nodiscard]] const QString &getComment() const
    {
        return comment;
    }
    void setComment(QString v)
    {
        comment = std::move(v);
    }
    [[nodiscard]] Crontab *getParent() const
    {
        return parent;
    }
    void setParent(Crontab *p)
    {
        parent = p;
    }

private:
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

    [[nodiscard]] const QString &getName() const
    {
        return name;
    }
    void setName(QString v)
    {
        name = std::move(v);
    }
    [[nodiscard]] const QString &getValue() const
    {
        return value;
    }
    void setValue(QString v)
    {
        value = std::move(v);
    }
    [[nodiscard]] const QString &getComment() const
    {
        return comment;
    }
    void setComment(QString v)
    {
        comment = std::move(v);
    }

private:
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
    QString cronText() const;

    static bool isSystemCron(const QString &owner)
    {
        return owner == QLatin1String("/etc/crontab") || owner.startsWith(QLatin1String("/etc/cron.d/"));
    }

    QString estr{};

    [[nodiscard]] const QString &getCronOwner() const
    {
        return cronOwner;
    }
    [[nodiscard]] const QString &getComment() const
    {
        return comment;
    }
    void setComment(QString v)
    {
        comment = std::move(v);
    }
    [[nodiscard]] bool isChanged() const
    {
        return changed;
    }
    void setChanged(bool v)
    {
        changed = v;
    }
    [[nodiscard]] std::vector<std::unique_ptr<Variable>> &getVariables()
    {
        return variables;
    }
    [[nodiscard]] const std::vector<std::unique_ptr<Variable>> &getVariables() const
    {
        return variables;
    }
    [[nodiscard]] std::vector<std::unique_ptr<TCommand>> &getTCommands()
    {
        return tCommands;
    }
    [[nodiscard]] const std::vector<std::unique_ptr<TCommand>> &getTCommands() const
    {
        return tCommands;
    }

private:
    QString cronOwner{};
    QString comment{};
    bool changed = false;
    std::vector<std::unique_ptr<Variable>> variables;
    std::vector<std::unique_ptr<TCommand>> tCommands;
};

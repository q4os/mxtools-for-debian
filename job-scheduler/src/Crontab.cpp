/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include <QtCore>

#include "Clib.h"
#include "Crontab.h"

Crontab::Crontab(const QString &user)
    : CronType(CronType::CRON),
      cronOwner(user),
      changed(false)
{
    QString str = getCrontab(user);
    if (!str.isEmpty()) {
        setup(str);
    }
    if (!estr.isEmpty()) {
        qWarning().noquote() << "Crontab initialization error for user" << user << ":" << estr;
    }
}

Crontab::~Crontab() = default;

QString Crontab::getCrontab(const QString &user)
{
    QString ret;
    estr = QLatin1String("");
    if (isSystemCron(user)) {
        QFile f(user);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            estr = QStringLiteral("can't open %1\n\n%2").arg(user, f.errorString());
            return ret;
        }
        ret = QString::fromUtf8(f.readAll());

    } else {
        QProcess p;
        if (user == Clib::uName()) {
            p.start(QStringLiteral("crontab"), QStringList() << QStringLiteral("-l"));
        } else {
            p.start(QStringLiteral("crontab"), QStringList() << QStringLiteral("-u") << user << QStringLiteral("-l"));
        }

        if (!p.waitForStarted()) {
            estr = "can't get crontab\n\nQProcess::waitForStarted():" + QString::number(p.error());
            return ret;
        }

        if (!p.waitForFinished()) {
            estr = "can't read crontab\n\nQProcess::waitForFinished():" + QString::number(p.error());
            return ret;
        }

        QString err = QString::fromUtf8(p.readAllStandardError());
        if (p.exitCode() != 0) {
            estr = "crontab read error\n\n" + err;
            return ret;
        }

        ret = QString::fromUtf8(p.readAllStandardOutput());
    }
    return ret;
}

QString Crontab::writeTempFile(const QString &text, const QString &tmp)
{
    QString fdir = QDir::tempPath() + "/job-scheduler-" + Clib::uName();
    if (!QFileInfo::exists(fdir)) {
        if (!QDir(fdir).mkdir(fdir)) {
            estr = "can't create directory " + fdir;
            return {};
        }
    }
    QTemporaryFile f(fdir + "/" + tmp);
    f.setAutoRemove(false);
    if (!f.open()) {
        estr = "can't open temporary file\n\n" + f.errorString();
        return {};
    }
    QTextStream t(&f);
    t << text;
    qDebug() << "File Saved :" << f.fileName();
    return f.fileName();
}

bool Crontab::putCrontab(const QString &text)
{
    estr = QLatin1String("");
    if (isSystemCron(cronOwner)) {
        QString tmpName = cronOwner == QLatin1String("/etc/crontab")
                              ? QStringLiteral("etccron")
                              : QStringLiteral("cron_d_") + QFileInfo(cronOwner).fileName();
        writeTempFile(text, tmpName);
        QFile f(cronOwner);
        if (!f.open(QIODevice::WriteOnly)) {
            estr = QStringLiteral("can't open %1 for write\n\n%2").arg(cronOwner, f.errorString());
            return false;
        }
        QTextStream t(&f);
        t << text;
    } else {
        QString fname = writeTempFile(text, cronOwner);
        if (fname.isEmpty()) {
            return false;
        }

        QProcess p;
        if (Clib::uId() == 0) {
            p.start(QStringLiteral("crontab"), QStringList() << QStringLiteral("-u") << cronOwner << fname);
        } else {
            p.start(QStringLiteral("crontab"), QStringList() << fname);
        }

        if (!p.waitForStarted()) {
            estr = "can't update crontab\n\nQProcess::waitForStarted():" + QString::number(p.error());
            return false;
        }

        if (!p.waitForFinished()) {
            estr = "can't update crontab\n\nQProcess::waitForFinished():" + QString::number(p.error());
            return false;
        }

        QString err = QString::fromUtf8(p.readAllStandardError());
        if (p.exitCode() != 0) {
            estr = "crontab update error\n\n" + err;
            return false;
        }
        QFile::remove(fname);
    }

    return true;
}

QString Crontab::cronText()
{
    QString ret;

    if (!comment.isEmpty()) {
        QString s = comment;
        ret += QStringLiteral("# %1\n\n").arg(s.replace('\n', QLatin1String("\n# ")));
    }

    for (const auto &v : std::as_const(variables)) {
        if (!v->comment.isEmpty()) {
            ret += QStringLiteral("# %1\n").arg(v->comment.replace('\n', QLatin1String("\n# ")));
        }

        ret += QStringLiteral("%1=%2\n").arg(v->name, v->value);
    }

    ret += QLatin1String("\n");
    for (const auto &c : std::as_const(tCommands)) {
        if (!c->comment.isEmpty()) {
            ret += QStringLiteral("# %1\n").arg(c->comment.replace('\n', QLatin1String("\n# ")));
        }

        if (isSystemCron(cronOwner)) {
            ret += QStringLiteral("%1 %2 %3\n").arg(c->time, c->user, c->command);
        } else {
            ret += QStringLiteral("%1 %2\n").arg(c->time, c->command);
        }
    }

    return ret;
}

void Crontab::setup(const QString &str)
{
    QStringList slist = str.split('\n');

    if (!isSystemCron(cronOwner)) {
        if (!slist.isEmpty() && slist.at(0).contains(QLatin1String("# DO NOT EDIT THIS FILE"), Qt::CaseInsensitive)) {
            // Strip the auto-generated crontab header without hard-coding its
            // length: the "DO NOT EDIT" line followed by the parenthetical
            // metadata lines ("# (... installed ...)", "# (Cron version ...)").
            // Stop there so a user comment that immediately follows the header
            // (with no blank separator) isn't swallowed.
            slist.removeFirst();
            while (!slist.isEmpty() && slist.first().startsWith(QLatin1String("# ("))) {
                slist.removeFirst();
            }
        }
    }

    QStringList cmnt;
    QStringList head;
    int headflag = 0;
    for (QString s : slist) {
        s = s.simplified();
        if (s.isEmpty()) {
            if (headflag == 0) {
                if (!head.isEmpty()) {
                    head << s;
                }
                head << cmnt;
                cmnt.clear();
            } else {
                cmnt << s;
            }
        } else if (s.at(0) == '#') {
            if (s.size() > 1 && s.at(1) == ' ') {
                cmnt << s.mid(2);
            } else {
                cmnt << s.mid(1);
            }

        } else {
            if (headflag == 0) {
                headflag = 1;

                comment = list2String(head);
            }
            if (s.contains(QRegularExpression(QStringLiteral("^\\S+\\s*=\\s*\\S*$")))) {
                // Variable
                QRegularExpression sep(QStringLiteral("\\s*=\\s*"));
                QString name = s.section(sep, 0, 0);
                QString val = s.section(sep, 1, 1);
                variables.push_back(std::make_unique<Variable>(name, val, list2String(cmnt)));
            } else {
                // Command
                QRegularExpression sep(QStringLiteral("\\s+"));
                int n = 0;
                if (s.at(0) == '@') {
                    n = 0;
                } else {
                    n = 4;
                }

                QString time = s.section(sep, 0, n);
                QString user = cronOwner;
                n++;
                if (isSystemCron(cronOwner)) {
                    user = s.section(sep, n, n);
                    n++;
                }

                QString cmnd = s.section(sep, n);
                tCommands.push_back(std::make_unique<TCommand>(time, user, cmnd, list2String(cmnt), this));
            }
            cmnt.clear();
        }
    }
}

QString Crontab::list2String(const QStringList &list)
{
    QString ret(QLatin1String(""));
    bool flag = false;

    for (const QString &s : list) {
        if (flag) {
            ret += '\n';
        }
        ret += s;
        flag = true;
    }

    return ret.replace(QRegularExpression(QStringLiteral("^\\n\\n")), QStringLiteral("\n"));
}

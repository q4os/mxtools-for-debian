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
}

Crontab::~Crontab()
{
    for (auto &c : tCommands) {
        delete c;
    }
    for (auto &v : variables) {
        delete v;
    }
}

QString Crontab::getCrontab(const QString &user)
{
    QString ret;
    estr = QLatin1String("");
    if (user == QLatin1String("/etc/crontab")) {
        QFile f(user);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            estr = "can't open /etc/crontab\n\n" + f.errorString();
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
        if ((p.exitCode() != 0) || !err.isEmpty()) {
            estr = "crontab update error\n\n" + err;
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
    if (cronOwner == QLatin1String("/etc/crontab")) {
        writeTempFile(text, QStringLiteral("etccron"));
        QFile f(cronOwner);
        if (!f.open(QIODevice::WriteOnly)) {
            estr = "can't open /etc/crontab for write\n\n" + f.errorString();
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
        if ((p.exitCode() != 0) || !err.isEmpty()) {
            estr = "crontab update error\n\n" + err;
            return false;
        }
        //		QFile::remove(fname);
    }

    return true;
}

QString Crontab::cronText()
{
    QString ret;

    if (!comment.isEmpty()) {
        QString s = comment;
        ret += "# " + s.replace('\n', QLatin1String("\n# ")) + "\n\n";
    }

    for (Variable *v : qAsConst(variables)) {
        if (!v->comment.isEmpty()) {
            ret += "# " + v->comment.replace('\n', QLatin1String("\n# ")) + '\n';
        }

        ret += v->name + "=" + v->value + '\n';
    }

    ret += QLatin1String("\n");
    for (TCommand *c : qAsConst(tCommands)) {
        if (!c->comment.isEmpty()) {
            ret += "# " + c->comment.replace('\n', QLatin1String("\n# ")) + '\n';
        }

        if (cronOwner == QLatin1String("/etc/crontab")) {
            ret += c->time + " " + c->user + " " + c->command + '\n';
        } else {
            ret += c->time + " " + c->command + '\n';
        }
    }

    return ret;
}

void Crontab::setup(const QString &str)
{
    QStringList slist = str.split('\n');

    if (cronOwner != QLatin1String("/etc/crontab")) {
        if (slist.at(0).contains(QLatin1String("# DO NOT EDIT THIS FILE"), Qt::CaseInsensitive)) {
            slist.removeFirst();
            slist.removeFirst();
            slist.removeFirst();
        }
    }

    QStringList cmnt;
    QStringList head;
    int headflag = 0;
    for (QString s : slist) {
        s = s.simplified();
        if (s.isEmpty()) {
            if (headflag == 0) {
                if (head.count() > 0) {
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
                //				if (head.count() == 0)
                //					head << cmnt;

                comment = list2String(head);
            }
            if (s.contains(QRegularExpression(QStringLiteral("^\\S+\\s*=\\s*\\S*$")))) {
                // Variable
                QRegularExpression sep(QStringLiteral("\\s*=\\s*"));
                QString name = s.section(sep, 0, 0);
                QString val = s.section(sep, 1, 1);
                variables << new Variable(name, val, list2String(cmnt));
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
                if (cronOwner == QLatin1String("/etc/crontab")) {
                    user = s.section(sep, n, n);
                    n++;
                }

                QString cmnd = s.section(sep, n);
                tCommands << new TCommand(time, user, cmnd, list2String(cmnt), this);
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

/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include <QtCore>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "Clib.h"
#include "Crontab.h"

namespace {
// Bound how long we'll block the calling (GUI) thread waiting on the
// external crontab process, rather than the QProcess default of 30s.
constexpr int kCrontabProcessTimeoutMs = 5000;

void killAndReap(QProcess &p)
{
    if (p.state() != QProcess::NotRunning) {
        p.kill();
        p.waitForFinished(kCrontabProcessTimeoutMs);
    }
}
} // namespace

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

        if (!p.waitForStarted(kCrontabProcessTimeoutMs)) {
            estr = "can't get crontab\n\nQProcess::waitForStarted():" + QString::number(p.error());
            killAndReap(p);
            return ret;
        }

        if (!p.waitForFinished(kCrontabProcessTimeoutMs)) {
            estr = "can't read crontab\n\nQProcess::waitForFinished():" + QString::number(p.error());
            killAndReap(p);
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
        // Write to a temp file in the same directory (so the final rename is
        // an atomic same-filesystem replace) instead of truncating the live
        // file in place, which could leave it empty/partial on interruption.
        QFileInfo targetInfo(cronOwner);
        QTemporaryFile tmp(targetInfo.absolutePath() + "/." + targetInfo.fileName() + ".XXXXXX");
        tmp.setAutoRemove(false);
        if (!tmp.open()) {
            estr = QStringLiteral("can't open temporary file for %1\n\n%2").arg(cronOwner, tmp.errorString());
            return false;
        }
        QString tmpFileName = tmp.fileName();
        QTextStream::Status streamStatus = QTextStream::Ok;
        {
            QTextStream t(&tmp);
            t << text;
            t.flush();
            streamStatus = t.status();
        }
        const bool writeOk = streamStatus == QTextStream::Ok && tmp.flush() && tmp.error() == QFile::NoError;
        tmp.close();
        if (!writeOk) {
            estr = QStringLiteral("can't write temporary file for %1\n\n%2").arg(cronOwner, tmp.errorString());
            QFile::remove(tmpFileName);
            return false;
        }

        const QFile::Permissions perms
            = targetInfo.exists() ? QFile(cronOwner).permissions()
                                  : QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther;
        if (!QFile::setPermissions(tmpFileName, perms)) {
            estr = QStringLiteral("can't set permissions on temporary file for %1").arg(cronOwner);
            QFile::remove(tmpFileName);
            return false;
        }

        // QFile::rename() refuses to overwrite an existing destination, which
        // /etc/crontab and /etc/cron.d/* entries always are. Use the POSIX
        // rename(2) syscall directly, which atomically replaces the
        // destination on the same filesystem.
        const QByteArray tmpPath = QFile::encodeName(tmpFileName);
        const QByteArray targetPath = QFile::encodeName(cronOwner);
        if (::rename(tmpPath.constData(), targetPath.constData()) != 0) {
            estr = QStringLiteral("can't replace %1\n\n%2").arg(cronOwner, QString::fromLocal8Bit(strerror(errno)));
            QFile::remove(tmpFileName);
            return false;
        }
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

        if (!p.waitForStarted(kCrontabProcessTimeoutMs)) {
            estr = "can't update crontab\n\nQProcess::waitForStarted():" + QString::number(p.error());
            killAndReap(p);
            return false;
        }

        if (!p.waitForFinished(kCrontabProcessTimeoutMs)) {
            estr = "can't update crontab\n\nQProcess::waitForFinished():" + QString::number(p.error());
            killAndReap(p);
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

QString Crontab::cronText() const
{
    QString ret;

    if (!comment.isEmpty()) {
        QString s = comment;
        ret += QStringLiteral("# %1\n\n").arg(s.replace('\n', QLatin1String("\n# ")));
    }

    for (const auto &v : std::as_const(variables)) {
        if (!v->getComment().isEmpty()) {
            QString c = v->getComment();
            ret += QStringLiteral("# %1\n").arg(c.replace('\n', QLatin1String("\n# ")));
        }

        ret += QStringLiteral("%1=%2\n").arg(v->getName(), v->getValue());
    }

    ret += QLatin1String("\n");
    for (const auto &c : std::as_const(tCommands)) {
        if (!c->getComment().isEmpty()) {
            QString cmt = c->getComment();
            ret += QStringLiteral("# %1\n").arg(cmt.replace('\n', QLatin1String("\n# ")));
        }

        if (isSystemCron(cronOwner)) {
            ret += QStringLiteral("%1 %2 %3\n").arg(c->getTime(), c->getUser(), c->getCommand());
        } else {
            ret += QStringLiteral("%1 %2\n").arg(c->getTime(), c->getCommand());
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
            } else if (!cmnt.isEmpty()) {
                // Only preserve a blank line once a comment block has actually
                // started; otherwise a bare section-separator blank line (e.g.
                // between the variables block and the next command) would be
                // misread as a leading blank line of that command's comment.
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

/**********************************************************************
 *  cmd.cpp
 **********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "cmd.h"

#include <QDebug>
#include <QEventLoop>

Cmd::Cmd(QObject *parent)
    : QProcess(parent)
{
    connect(this, &Cmd::readyReadStandardOutput, [this]() { emit outputAvailable(readAllStandardOutput()); });
    connect(this, &Cmd::readyReadStandardError, [this]() { emit errorAvailable(readAllStandardError()); });
    connect(this, &Cmd::outputAvailable, [this](const QString &out) { out_buffer += out; });
    connect(this, &Cmd::errorAvailable, [this](const QString &out) { out_buffer += out; });
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Cmd::done);
}

Cmd::~Cmd()
{
    if (state() != QProcess::NotRunning) {
        qDebug() << "Cmd::~Cmd(): Terminating running process:" << program() << arguments();
        terminate();
        if (!waitForFinished(3000)) {
            qDebug() << "Cmd::~Cmd(): Force killing process";
            kill();
            waitForFinished(1000);
        }
    }
}

bool Cmd::runImpl(const QString &prog, const QStringList &args, QString &output, bool trim, bool quiet)
{
    out_buffer.clear();
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (!quiet) {
        if (args.size() == 2 && prog == "/bin/bash" && args[0] == "-c") {
            qDebug().noquote() << args[1]; // Show the actual command for shell calls
        } else {
            qDebug().noquote() << prog << args;
        }
    }
    QEventLoop loop;
    connect(this, &Cmd::done, &loop, &QEventLoop::quit);
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(30000); // 30-second timeout to prevent UI freeze on hung processes
    start(prog, args);
    loop.exec();
    const bool finishedInTime = timer.isActive(); // active => process finished before timeout
    timer.stop();
    if (!finishedInTime) {
        qWarning() << "Cmd: command timed out after 30s, killing:" << prog << args;
        kill();
        waitForFinished(1000);
        output = trim ? out_buffer.trimmed() : out_buffer;
        return false;
    }
    output = trim ? out_buffer.trimmed() : out_buffer;
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

// --- Shell-string overloads (existing) ---

bool Cmd::run(const QString &cmd, bool quiet)
{
    QString output;
    return run(cmd, output, quiet);
}

bool Cmd::run(const QString &cmd, QString &output, bool quiet)
{
    return runImpl("/bin/bash", {QStringLiteral("-c"), cmd}, output, true, quiet);
}

bool Cmd::runUntrimmed(const QString &cmd, QString &output, bool quiet)
{
    return runImpl("/bin/bash", {QStringLiteral("-c"), cmd}, output, false, quiet);
}

QString Cmd::getCmdOut(const QString &cmd, bool quiet)
{
    QString output;
    run(cmd, output, quiet);
    return output;
}

QString Cmd::getCmdOutUntrimmed(const QString &cmd, bool quiet)
{
    QString output;
    runUntrimmed(cmd, output, quiet);
    return output;
}

// --- Argument-array overloads (new) ---

bool Cmd::run(const QString &program, const QStringList &args, bool quiet)
{
    QString output;
    return run(program, args, output, quiet);
}

bool Cmd::run(const QString &program, const QStringList &args, QString &output, bool quiet)
{
    return runImpl(program, args, output, true, quiet);
}

bool Cmd::runUntrimmed(const QString &program, const QStringList &args, QString &output, bool quiet)
{
    return runImpl(program, args, output, false, quiet);
}

QString Cmd::getCmdOut(const QString &program, const QStringList &args, bool quiet)
{
    QString output;
    run(program, args, output, quiet);
    return output;
}

QString Cmd::getCmdOutUntrimmed(const QString &program, const QStringList &args, bool quiet)
{
    QString output;
    runUntrimmed(program, args, output, quiet);
    return output;
}

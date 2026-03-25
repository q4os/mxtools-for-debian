/**********************************************************************
 *  cmd.cpp
 **********************************************************************
 * Copyright (C) 2024-2025 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QWidget>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent)
{
    const QStringList elevationCommands = {"/usr/bin/pkexec", "/usr/bin/gksu"};
    for (const QString &command : elevationCommands) {
        if (QFile::exists(command)) {
            elevationCommand = command;
            break;
        }
    }

    if (elevationCommand.isEmpty()) {
        qWarning() << "No suitable elevation command found (pkexec or gksu)";
    }

    helper = QString("/usr/lib/%1/helper").arg(QApplication::applicationName());
    helperLibrary = QString("/usr/lib/%1/uefimanager-lib").arg(QApplication::applicationName());

    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Cmd::done,
            Qt::UniqueConnection);
    connect(this, &Cmd::readyReadStandardOutput, this, &Cmd::handleStandardOutput);
    connect(this, &Cmd::readyReadStandardError, this, &Cmd::handleStandardError);
}

void Cmd::handleStandardOutput()
{
    const QString output = readAllStandardOutput();
    outBuffer += output;
    emit outputAvailable(output);
}

void Cmd::handleStandardError()
{
    const QString error = readAllStandardError();
    outBuffer += error;
    emit errorAvailable(error);
}

bool Cmd::proc(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input, QuietMode quiet,
               Elevation elevation)
{
    if (elevation == Elevation::Yes) {
        QStringList helperArgs {"exec", cmd};
        helperArgs += args;
        return helperProc(helperArgs, output, input, quiet);
    }

    outBuffer.clear();

    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }

    if (quiet == QuietMode::No) {
        qDebug() << cmd << args;
    }

    QEventLoop loop;
    bool processError = false;
    auto doneConn = connect(this, &Cmd::done, &loop, &QEventLoop::quit);
    auto errorConn = connect(this, &QProcess::errorOccurred, &loop, [&loop, &processError] {
        processError = true;
        loop.quit();
    });

    start(cmd, args);

    if (input && !input->isEmpty()) {
        waitForStarted();
        write(*input);
    }
    closeWriteChannel();
    loop.exec();
    disconnect(doneConn);
    disconnect(errorConn);

    if (processError) {
        qWarning() << "Process error:" << errorString();
        return false;
    }

    if (output) {
        *output = outBuffer.trimmed();
    }

    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::procAsRoot(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input,
                     QuietMode quiet)
{
    return proc(cmd, args, output, input, quiet, Elevation::Yes);
}

bool Cmd::helperProc(const QStringList &helperArgs, QString *output, const QByteArray *input, QuietMode quiet)
{
    if (elevationFailed) {
        return false;
    }

    if (getuid() != 0 && elevationCommand.isEmpty()) {
        qWarning() << "No elevation helper available";
        handleElevationError();
        return false;
    }

    const QString program = (getuid() == 0) ? helper : elevationCommand;
    QStringList programArgs = helperArgs;
    if (getuid() != 0) {
        programArgs.prepend(helper);
    }

    const bool result = proc(program, programArgs, output, input, quiet, Elevation::No);
    if (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND) {
        handleElevationError();
    }
    return result;
}

bool Cmd::procElevated(const QString &cmd, const QStringList &args, QString *output, QuietMode quiet)
{
    if (getuid() == 0) {
        return proc(cmd, args, output, nullptr, quiet);
    }

    if (cmd != helperLibrary) {
        qWarning() << "Refusing to elevate unexpected command directly:" << cmd;
        return false;
    }

    QStringList helperArgs {"lib"};
    helperArgs += args;
    return helperProc(helperArgs, output, nullptr, quiet);
}

void Cmd::handleElevationError()
{
    elevationFailed = true;
    QWidget *parentWidget = qobject_cast<QWidget *>(qApp->activeWindow());
    QMessageBox::critical(parentWidget, tr("Administrator Access Required"),
                          tr("This operation requires administrator privileges."));
}

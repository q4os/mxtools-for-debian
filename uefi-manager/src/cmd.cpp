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
    // Determine the appropriate elevation command
    const QStringList elevationCommands = {"/usr/bin/pkexec", "/usr/bin/gksu", "/usr/bin/sudo"};
    for (const QString &command : elevationCommands) {
        if (QFile::exists(command)) {
            elevationCommand = command;
            break;
        }
    }

    if (elevationCommand.isEmpty()) {
        qWarning() << "No suitable elevation command found (pkexec, gksu, or sudo)";
    }

    helper = QString("/usr/lib/%1/helper").arg(QApplication::applicationName());

    // Connect signals for output handling
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
    outBuffer.clear();

    // Skip if elevation has already failed in this action chain
    if (elevationFailed && elevation == Elevation::Yes) {
        return false;
    }

    // Check if process is already running
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }

    // Log command if not quiet
    if (quiet == QuietMode::No) {
        qDebug() << cmd << args;
    }

    // Fail fast if elevation is needed but no elevation command is available
    if (elevation == Elevation::Yes && getuid() != 0 && elevationCommand.isEmpty()) {
        qWarning() << "Elevation required but no pkexec/gksu found";
        handleElevationError();
        return false;
    }

    // Set up event loop for synchronous execution
    QEventLoop loop;
    bool processError = false;
    auto doneConn = connect(this, &Cmd::done, &loop, &QEventLoop::quit);
    auto errorConn = connect(this, &QProcess::errorOccurred, &loop, [&loop, &processError] {
        processError = true;
        loop.quit();
    });

    // Start the process with appropriate elevation
    if (elevation == Elevation::Yes && getuid() != 0) {
        QStringList cmdAndArgs = QStringList() << helper << cmd << args;
        start(elevationCommand, {cmdAndArgs});
    } else {
        start(cmd, args);
    }

    // Handle input if provided
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

    // Check for permission denied or command not found errors
    // These can occur when elevation fails (canceled dialog or incorrect password)
    if (elevation == Elevation::Yes
        && (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND)) {
        handleElevationError();
        return false;
    }

    // Provide output if requested
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

bool Cmd::procElevated(const QString &cmd, const QStringList &args, QString *output, QuietMode quiet)
{
    if (getuid() == 0) {
        return proc(cmd, args, output, nullptr, quiet);
    }
    if (elevationCommand.isEmpty()) {
        qWarning() << "Elevation required but no pkexec/gksu found";
        return false;
    }
    QStringList elevatedArgs = QStringList() << cmd << args;
    return proc(elevationCommand, elevatedArgs, output, nullptr, quiet);
}

void Cmd::handleElevationError()
{
    elevationFailed = true;
    QWidget *parentWidget = qobject_cast<QWidget *>(qApp->activeWindow());
    QMessageBox::critical(parentWidget, tr("Administrator Access Required"),
                          tr("This operation requires administrator privileges."));
}

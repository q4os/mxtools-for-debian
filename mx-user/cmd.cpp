#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QTimer>

#include "mainwindow.h"

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

    connect(this, &Cmd::readyReadStandardOutput, this, &Cmd::handleStandardOutput);
    connect(this, &Cmd::readyReadStandardError, this, &Cmd::handleStandardError);
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Cmd::done);
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

QString Cmd::getOut(const QString &cmd, QuietMode quiet)
{
    QString output;
    run(cmd, &output, nullptr, quiet);
    return output;
}

bool Cmd::runHelper(const QStringList &actionArgs, QString *output, const QByteArray *input, QuietMode quiet)
{
    return helperProc(actionArgs, output, input, quiet);
}

bool Cmd::helperProc(const QStringList &helperArgs, QString *output, const QByteArray *input, QuietMode quiet)
{
    // Once elevation has been refused in the current operation, do not run (and
    // re-prompt for) any further helper calls. This keeps multi-step operations
    // atomic and ensures the user sees the failure message only once.
    if (elevationFailed) {
        return false;
    }

    if (getuid() != 0 && elevationCommand.isEmpty()) {
        qWarning() << "No elevation helper available";
        return false;
    }

    const QString program = (getuid() == 0) ? helper : elevationCommand;
    QStringList programArgs = helperArgs;
    if (getuid() != 0) {
        programArgs.prepend(helper);
    }

    const bool result = proc(program, programArgs, output, input, quiet);
    if (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND) {
        elevationFailed = true;
        handleElevationError();
        return false;
    }
    return result;
}

bool Cmd::proc(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input, QuietMode quiet)
{
    outBuffer.clear();
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }

    if (quiet == QuietMode::No) {
        qDebug() << cmd << args;
    }

    start(cmd, args);
    if (!waitForStarted()) {
        // The process never started (e.g. the binary is missing or not
        // executable): finished() will never fire, so return now instead of
        // blocking forever in the event loop below.
        if (output) {
            *output = outBuffer.trimmed();
        }
        return false;
    }

    QEventLoop loop;
    connect(this, &Cmd::done, &loop, &QEventLoop::quit);

    if (input && !input->isEmpty()) {
        write(*input);
    }
    closeWriteChannel();
    // Block GUI re-entrancy while the command runs: exclude user input events so
    // the user cannot change selections, re-trigger Apply, or close the window
    // mid-operation. Process/timer/paint events still run, so progress updates
    // and repaints continue; deferred input is delivered once the command ends.
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    if (output) {
        *output = outBuffer.trimmed();
    }

    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::run(const QString &cmd, QString *output, const QByteArray *input, QuietMode quiet)
{
    return proc("/bin/bash", {"-c", cmd}, output, input, quiet);
}

void Cmd::handleElevationError()
{
    if (qobject_cast<MainWindow *>(qApp->activeWindow())) {
        QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                              tr("This operation requires administrator privileges. The change was not applied. "
                                 "Please try again and enter your password when prompted."));
    }
}

QString Cmd::readAllOutput() const
{
    return outBuffer.trimmed();
}

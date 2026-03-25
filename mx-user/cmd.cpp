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

QString Cmd::getOutAsRoot(const QString &cmd, const QStringList &args, QuietMode quiet)
{
    QString output;
    procAsRoot(cmd, args, &output, nullptr, quiet);
    return output;
}

bool Cmd::helperProc(const QStringList &helperArgs, QString *output, const QByteArray *input, QuietMode quiet)
{
    if (getuid() != 0 && elevationCommand.isEmpty()) {
        qWarning() << "No elevation helper available";
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
    connect(this, &Cmd::done, &loop, &QEventLoop::quit);

    start(cmd, args);
    if (input && !input->isEmpty()) {
        write(*input);
    }
    closeWriteChannel();
    loop.exec();

    if (output) {
        *output = outBuffer.trimmed();
    }

    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::procAsRoot(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input,
                     QuietMode quiet)
{
    if (cmd == QLatin1String("passwd")) {
        QStringList helperArgs {"passwd"};
        helperArgs += args;
        return helperProc(helperArgs, output, input, quiet);
    }
    return proc(cmd, args, output, input, quiet, Elevation::Yes);
}

bool Cmd::run(const QString &cmd, QString *output, const QByteArray *input, QuietMode quiet)
{
    return proc("/bin/bash", {"-c", cmd}, output, input, quiet);
}

void Cmd::handleElevationError()
{
    if (qobject_cast<MainWindow *>(qApp->activeWindow())) {
        QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                              tr("This operation requires administrator privileges. Please restart the application "
                                 "and enter your password when prompted."));
    }
    exit(EXIT_FAILURE);
}

QString Cmd::readAllOutput() const
{
    return outBuffer.trimmed();
}

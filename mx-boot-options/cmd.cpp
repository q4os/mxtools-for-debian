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
    // Determine the appropriate elevation command
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

    // Connect signals for output handling
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

QString Cmd::getOut(const QString &cmd, QuietMode quiet, Elevation elevation)
{
    QString output;
    run(cmd, &output, nullptr, quiet, elevation);
    return output;
}

QString Cmd::getOutAsRoot(const QString &cmd, QuietMode quiet)
{
    return getOut(cmd, quiet, Elevation::Yes);
}

bool Cmd::proc(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input, QuietMode quiet,
               Elevation elevation)
{
    outBuffer.clear();

    // Check if process is already running
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Cmd::done);
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }

    // Log command if not quiet
    if (quiet == QuietMode::No) {
        qDebug() << cmd << args;
    }

    // Set up event loop for synchronous execution
    QEventLoop loop;
    connect(this, &Cmd::done, &loop, &QEventLoop::quit);

    // Start the process with appropriate elevation
    if (elevation == Elevation::Yes && getuid() != 0) {
        QStringList cmdAndArgs = QStringList() << helper << cmd << args;
        start(elevationCommand, {cmdAndArgs});
    } else {
        start(cmd, args);
    }

    // Handle input if provided
    if (input && !input->isEmpty()) {
        write(*input);
    }
    closeWriteChannel();
    loop.exec();

    // Check for permission denied or command not found errors
    // These can occur when elevation fails (canceled dialog or incorrect password)
    if (elevation == Elevation::Yes
        && (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND)) {
        handleElevationError();
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

bool Cmd::run(const QString &cmd, QString *output, const QByteArray *input, QuietMode quiet, Elevation elevation)
{
    if (elevation == Elevation::Yes && getuid() != 0) {
        bool result = proc(elevationCommand, {helper, cmd}, output, input, quiet);
        // Command-not-found is returned when password is entered incorrectly
        if (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND) {
            handleElevationError();
        }
        return result;
    }
    return proc("/bin/bash", {"-c", cmd}, output, input, quiet);
}

bool Cmd::runAsRoot(const QString &cmd, QString *output, const QByteArray *input, QuietMode quiet)
{
    return run(cmd, output, input, quiet, Elevation::Yes);
}

void Cmd::handleElevationError()
{
    if (qobject_cast<MainWindow *>(qApp->activeWindow())) {
        QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                              tr("This operation requires administrator privileges. Please restart the application "
                                 "and enter your password when prompted."));
    }
    QTimer::singleShot(0, qApp, &QApplication::quit);
    exit(EXIT_FAILURE);
}

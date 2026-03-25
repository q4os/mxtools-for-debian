#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
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

QStringList Cmd::helperRootArgs(const QString &rootPath)
{
    if (rootPath.isEmpty()) {
        return {};
    }
    return {"--root", rootPath};
}

QStringList Cmd::helperExecArgs(const QString &cmd, const QStringList &args, const QString &rootPath) const
{
    QStringList helperArgs {"exec"};
    helperArgs += helperRootArgs(rootPath);
    helperArgs << cmd;
    helperArgs += args;
    return helperArgs;
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

QString Cmd::getOutAsRootInTarget(const QString &rootPath, const QString &cmd, const QStringList &args, QuietMode quiet)
{
    QString output;
    procAsRootInTarget(rootPath, cmd, args, &output, nullptr, quiet);
    return output;
}

QString Cmd::readFileAsRoot(const QString &path, QuietMode quiet, const QString &rootPath)
{
    QString output;
    QStringList helperArgs {"read-file"};
    helperArgs += helperRootArgs(rootPath);
    helperArgs << path;
    helperProc(helperArgs, &output, nullptr, quiet);
    return output;
}

bool Cmd::isPackageInstalledAsRoot(const QString &manager, const QString &package, const QString &rootPath, QuietMode quiet)
{
    QStringList helperArgs {"package-installed"};
    helperArgs += helperRootArgs(rootPath);
    helperArgs << manager << package;
    return helperProc(helperArgs, nullptr, nullptr, quiet);
}

bool Cmd::appendToFileAsRootIfMissing(const QString &path, const QString &needle, const QString &content, QuietMode quiet,
                                      const QString &rootPath)
{
    QStringList helperArgs {"append-if-missing"};
    helperArgs += helperRootArgs(rootPath);
    helperArgs << path << needle << content;
    return helperProc(helperArgs, nullptr, nullptr, quiet);
}

bool Cmd::previewPlymouthAsRoot(QuietMode quiet)
{
    return helperProc({"preview-plymouth"}, nullptr, nullptr, quiet);
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
        return helperProc(helperExecArgs(cmd, args), output, input, quiet);
    }

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

    start(cmd, args);

    // Handle input if provided
    if (input && !input->isEmpty()) {
        write(*input);
    }
    closeWriteChannel();
    loop.exec();

    // Check for permission denied or command not found errors
    // These can occur when elevation fails (canceled dialog or incorrect password)
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

bool Cmd::procAsRootInTarget(const QString &rootPath, const QString &cmd, const QStringList &args, QString *output,
                             const QByteArray *input, QuietMode quiet)
{
    return helperProc(helperExecArgs(cmd, args, rootPath), output, input, quiet);
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
    QTimer::singleShot(0, qApp, &QApplication::quit);
    exit(EXIT_FAILURE);
}

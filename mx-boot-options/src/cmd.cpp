#include "cmd.h"

#include "common.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent)
{
    // Determine the appropriate elevation command via PATH lookup
    elevationCommand = QStandardPaths::findExecutable(QStringLiteral("pkexec"));
    if (elevationCommand.isEmpty()) {
        elevationCommand = QStandardPaths::findExecutable(QStringLiteral("gksu"));
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

bool Cmd::writeFileAsRoot(const QString &path, const QByteArray &content, const QString &rootPath, QuietMode quiet,
                          bool *durabilityUncertain)
{
    QStringList helperArgs {"write-file"};
    helperArgs += helperRootArgs(rootPath);
    helperArgs << path;
    const bool result = helperProc(helperArgs, nullptr, &content, quiet);
    if (durabilityUncertain) {
        *durabilityUncertain = !result && exitCode() == EXIT_CODE_WRITE_FILE_DURABILITY_UNCERTAIN;
    }
    return result;
}

bool Cmd::previewPlymouthAsRoot(QuietMode quiet)
{
    return helperProc({"preview-plymouth"}, nullptr, nullptr, quiet);
}

bool Cmd::helperProc(const QStringList &helperArgs, QString *output, const QByteArray *input, QuietMode quiet, int timeoutMs)
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
    constexpr int helperShutdownGraceMs = 30000;
    const int outerTimeoutMs = timeoutMs == NoTimeoutMs ? NoTimeoutMs : timeoutMs + helperShutdownGraceMs;
    const bool result = proc(program, programArgs, output, input, quiet, Elevation::No, outerTimeoutMs);
    if (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND) {
        handleElevationError();
    }
    return result;
}

bool Cmd::proc(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input, QuietMode quiet,
               Elevation elevation, int timeoutMs)
{
    if (cancelRequested) {
        qDebug() << "Skipping command after cancellation:" << cmd << args;
        return false;
    }

    if (elevation == Elevation::Yes) {
        return helperProc(helperExecArgs(cmd, args), output, input, quiet, timeoutMs);
    }

    outBuffer.clear();

    // Relay QProcess::finished to Cmd::done, disconnecting any previous
    // relay to avoid accumulating duplicate signal-slot connections.
    disconnect(this, &QProcess::finished, this, &Cmd::done);
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
    bool processError = false;
    connect(this, &QProcess::errorOccurred, &loop, [&, this](QProcess::ProcessError error) {
        processError = true;
        qWarning() << "Command error:" << error << program() << arguments();
        loop.quit();
    });

    bool timedOut = false;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, &QTimer::timeout, &loop, [this, &timedOut] {
        timedOut = true;
        qWarning() << "Command timed out; terminating:" << program() << arguments();
        terminateRunningProcess();
    });

    start(cmd, args);
    if (timeoutMs != NoTimeoutMs) {
        timeoutTimer.start(timeoutMs);
    }

    // Handle input if provided
    if (input && !input->isEmpty()) {
        write(*input);
    }
    closeWriteChannel();
    loop.exec();
    timeoutTimer.stop();

    // Check for permission denied or command not found errors
    // These can occur when elevation fails (canceled dialog or incorrect password)
    // Provide output if requested
    if (output) {
        *output = outBuffer.trimmed();
    }

    return !timedOut && !processError && exitStatus() == QProcess::NormalExit && exitCode() == 0;
}

bool Cmd::procAsRoot(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input,
                     QuietMode quiet, int timeoutMs)
{
    return proc(cmd, args, output, input, quiet, Elevation::Yes, timeoutMs);
}

bool Cmd::procAsRootInTarget(const QString &rootPath, const QString &cmd, const QStringList &args, QString *output,
                             const QByteArray *input, QuietMode quiet, int timeoutMs)
{
    return helperProc(helperExecArgs(cmd, args, rootPath), output, input, quiet, timeoutMs);
}

bool Cmd::run(const QString &cmd, QString *output, const QByteArray *input, QuietMode quiet, int timeoutMs)
{
    return proc("/bin/bash", {"-c", cmd}, output, input, quiet, Elevation::No, timeoutMs);
}

void Cmd::cancel()
{
    cancelRequested = true;
    terminateRunningProcess();
}

void Cmd::terminateRunningProcess()
{
    if (state() == QProcess::NotRunning) {
        return;
    }
    const qint64 runningPid = processId();
    terminate();
    QTimer::singleShot(5000, this, [this, runningPid] {
        if (processId() == runningPid && state() != QProcess::NotRunning) {
            qWarning() << "Command did not terminate; killing:" << program() << arguments();
            kill();
        }
    });
}

void Cmd::clearCancelRequest()
{
    cancelRequested = false;
}

bool Cmd::isCancelRequested() const
{
    return cancelRequested;
}

void Cmd::handleElevationError()
{
    if (qApp->activeWindow()) {
        QMessageBox::critical(qApp->activeWindow(), tr("Administrator Access Required"),
                              tr("This operation requires administrator privileges. Please restart the application "
                                 "and enter your password when prompted."));
    }
    QTimer::singleShot(0, qApp, &QApplication::quit);
    exit(EXIT_FAILURE);
}

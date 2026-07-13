#include "cmd.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QUuid>

#include <unistd.h>

namespace
{
// Directory for the auth-success marker. Prefer the caller's private runtime
// dir (/run/user/<uid>, mode 0700): the root helper can create the marker there
// and this unprivileged process can still remove it afterwards. In the
// world-writable /tmp fallback a root-owned marker cannot be unlinked from here,
// so it may briefly linger, but that only affects the check's cleanup.
QString markerDirectory()
{
    const QString runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (!runtimeDir.isEmpty() && QDir(runtimeDir).exists()) {
        return runtimeDir;
    }
    return QDir::tempPath();
}

// Set by handleElevationError() whenever an elevated call anywhere is dismissed
// by the user, and cleared by resetElevationDismissed(). GUI is single-threaded,
// so a plain static is enough to let unrelated Cmd instances share this state.
bool elevationWasDismissed = false;
} // namespace

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {elevationTool()},
      helper {"/usr/lib/mx-packageinstaller/helper"}
{
    connect(this, &Cmd::readyReadStandardOutput, [this] { emit outputAvailable(readAllStandardOutput()); });
    connect(this, &Cmd::readyReadStandardError, [this] { emit errorAvailable(readAllStandardError()); });
    connect(this, &Cmd::outputAvailable, [this](const QString &out) { outBuffer += out; });
    connect(this, &Cmd::errorAvailable, [this](const QString &err) { outBuffer += err; });
}

QString Cmd::elevationTool()
{
    if (QFile::exists("/usr/bin/pkexec")) return QStringLiteral("/usr/bin/pkexec");
    if (QFile::exists("/usr/bin/gksu")) return QStringLiteral("/usr/bin/gksu");
    if (QFile::exists("/usr/bin/sudo")) return QStringLiteral("/usr/bin/sudo");
    return QStringLiteral("/usr/bin/sudo"); // fallback
}

QString Cmd::getOut(const QString &cmd, QuietMode quiet)
{
    outBuffer.clear();
    run(cmd, quiet);
    return outBuffer.trimmed();
}

QString Cmd::getOut(const QString &cmd, const QStringList &args, QuietMode quiet)
{
    QString output;
    proc(cmd, args, &output, nullptr, quiet);
    return output;
}

QStringList Cmd::helperExecArgs(const QString &cmd, const QStringList &args, const QHash<QString, QString> &environment) const
{
    QStringList helperArgs {"exec"};
    for (auto it = environment.cbegin(); it != environment.cend(); ++it) {
        helperArgs << "--env" << (it.key() + '=' + it.value());
    }
    helperArgs << cmd;
    helperArgs += args;
    return helperArgs;
}

bool Cmd::helperProc(const QStringList &helperArgs, QString *output, const QByteArray *input, QuietMode quiet)
{
    outBuffer.clear();
    if (getuid() != 0 && elevate.isEmpty()) {
        qWarning() << "No elevation helper available";
        return false;
    }

    const QString program = (getuid() == 0) ? helper : elevate;
    QStringList programArgs = helperArgs;
    if (getuid() != 0) {
        programArgs.prepend(helper);
    }
    return startAndWait(program, programArgs, output, input, quiet, getuid() != 0);
}

bool Cmd::proc(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input, QuietMode quiet)
{
    return startAndWait(cmd, args, output, input, quiet, false);
}

bool Cmd::procAsRoot(const QString &cmd, const QStringList &args, QString *output, const QByteArray *input, QuietMode quiet)
{
    return helperProc(helperExecArgs(cmd, args), output, input, quiet);
}

bool Cmd::procAsRootWithEnv(const QHash<QString, QString> &environment, const QString &cmd, const QStringList &args,
                            QString *output, const QByteArray *input, QuietMode quiet)
{
    return helperProc(helperExecArgs(cmd, args, environment), output, input, quiet);
}

bool Cmd::procScriptAsRoot(const QString &path, const QStringList &args, QString *output, const QByteArray *input,
                           QuietMode quiet)
{
    if (getuid() == 0) {
        return proc(path, args, output, input, quiet);
    }

    QStringList elevatedArgs {path};
    elevatedArgs += args;
    return startAndWait(elevationTool(), elevatedArgs, output, input, quiet, true);
}

bool Cmd::run(const QString &cmd, QuietMode quiet)
{
    return startAndWait("/bin/bash", {"-c", cmd}, nullptr, nullptr, quiet, false);
}

bool Cmd::runHookAsRoot(const QString &script, QuietMode quiet)
{
    return helperProc({"run-hook", script}, nullptr, nullptr, quiet);
}

QString Cmd::lockingProcessAsRoot(const QString &path, QuietMode quiet)
{
    QString output;
    if (!helperProc({"locking-process", path}, &output, nullptr, quiet)) {
        if (Cmd::elevationDismissed()) {
            return {};
        }
        return output.trimmed().isEmpty() ? QStringLiteral("unknown process") : output.trimmed();
    }
    return output.trimmed();
}

bool Cmd::writeFileAsRoot(const QString &path, const QString &content, QuietMode quiet)
{
    return helperProc({"write-file", path, content}, nullptr, nullptr, quiet);
}

bool Cmd::startAndWait(const QString &program, const QStringList &arguments, QString *output, const QByteArray *input,
                       QuietMode quiet, bool elevated, const QString &shellCommand)
{
    outBuffer.clear();
    helperMarkerPath.clear();
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << this->program() << this->arguments();
        return false;
    }

    if (quiet == QuietMode::No) {
        if (shellCommand.isEmpty()) {
            qDebug() << program << arguments;
        } else {
            qDebug().noquote() << shellCommand;
        }
    }

    setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    QStringList launchArgs = arguments;
    if (elevated) {
        helperMarkerPath = markerDirectory() + QStringLiteral("/mx-pkg-helper-")
                           + QUuid::createUuid().toString(QUuid::Id128) + QStringLiteral(".marker");
        // pkexec sanitizes the environment, so hand the marker path to the target
        // as explicit leading arguments.
        if (!launchArgs.isEmpty()) {
            launchArgs.insert(1, helperMarkerPath);
            launchArgs.insert(1, QStringLiteral("--marker"));
        }
    }

    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    start(program, launchArgs);
    if (!waitForStarted()) {
        if (elevated) {
            QFile::remove(helperMarkerPath);
            helperMarkerPath.clear();
        }
        if (output) {
            *output = outBuffer.trimmed();
        }
        emit done();
        return false;
    }
    if (input) {
        if (!input->isEmpty()) {
            write(*input);
        }
        closeWriteChannel();
    }
    loop.exec();

    if (elevated) {
        if (isAuthenticationDismissed()) {
            handleElevationError();
        }
        QFile::remove(helperMarkerPath);
        helperMarkerPath.clear();
    }

    if (output) {
        *output = outBuffer.trimmed();
    }

    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

// Return true when process is killed or not running
bool Cmd::terminateAndKill()
{
    if (state() != QProcess::NotRunning) {
        terminate();
        if (!waitForFinished(TerminateTimeoutMs)) {
            kill();
        }
    }
    return state() == QProcess::NotRunning;
}

QString Cmd::readAllOutput() const
{
    return outBuffer.trimmed();
}

bool Cmd::isAuthenticationDismissed() const
{
    if (exitStatus() != QProcess::NormalExit || helperMarkerPath.isEmpty()) {
        return false;
    }
    // pkexec returns 126 or 127 when auth is dismissed (varies by version).
    const int code = exitCode();
    if (code != 126 && code != 127) {
        return false;
    }
    // The helper creates a marker file when it starts. If the file exists, auth succeeded
    // (helper ran). If it doesn't exist with exit code 126/127, auth was dismissed.
    return !QFile::exists(helperMarkerPath);
}

void Cmd::handleElevationError()
{
    elevationWasDismissed = true;
    QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                          tr("This operation requires administrator privileges. Try the action again and "
                             "enter your password when prompted."));
}

bool Cmd::elevationDismissed()
{
    return elevationWasDismissed;
}

void Cmd::resetElevationDismissed()
{
    elevationWasDismissed = false;
}

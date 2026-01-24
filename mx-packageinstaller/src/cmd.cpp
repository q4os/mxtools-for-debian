#include "cmd.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QUuid>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {elevationTool()},
      helper {"/usr/lib/mx-packageinstaller/helper"}
{
    connect(this, &Cmd::readyReadStandardOutput, [this] { emit outputAvailable(readAllStandardOutput()); });
    connect(this, &Cmd::readyReadStandardError, [this] { emit errorAvailable(readAllStandardError()); });
    connect(this, &Cmd::outputAvailable, [this](const QString &out) { out_buffer += out; });
    connect(this, &Cmd::errorAvailable, [this](const QString &err) { out_buffer += err; });
}

QString Cmd::elevationTool()
{
    if (QFile::exists("/usr/bin/pkexec")) return QStringLiteral("/usr/bin/pkexec");
    if (QFile::exists("/usr/bin/gksu")) return QStringLiteral("/usr/bin/gksu");
    if (QFile::exists("/usr/bin/sudo")) return QStringLiteral("/usr/bin/sudo");
    return QStringLiteral("/usr/bin/sudo"); // fallback
}

QString Cmd::getOut(const QString &cmd, QuietMode quiet, Elevation elevation)
{
    out_buffer.clear();
    run(cmd, quiet, elevation);
    return out_buffer.trimmed();
}

QString Cmd::getOutAsRoot(const QString &cmd, QuietMode quiet)
{
    return getOut(cmd, quiet, Elevation::Yes);
}

bool Cmd::run(const QString &cmd, QuietMode quiet, Elevation elevation)
{
    out_buffer.clear();
    helperMarkerPath.clear();
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (quiet == QuietMode::No) {
        qDebug().noquote() << cmd;
    }
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    const bool elevated = elevation == Elevation::Yes && getuid() != 0;
    if (elevated) {
        // Set up marker file path for auth dismissal detection
        helperMarkerPath = QDir::tempPath() + QStringLiteral("/mx-pkg-helper-")
                           + QUuid::createUuid().toString(QUuid::Id128) + QStringLiteral(".marker");
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("MX_PKG_HELPER_MARKER"), helperMarkerPath);
        setProcessEnvironment(env);
        start(elevate, {helper, cmd});
    } else {
        start("/bin/bash", {"-c", cmd});
    }
    loop.exec();
    if (elevated) {
        // Reset environment and clean up marker file
        setProcessEnvironment(QProcessEnvironment::systemEnvironment());
        if (isAuthenticationDismissed()) {
            handleElevationError();
        }
        QFile::remove(helperMarkerPath);
        helperMarkerPath.clear();
    }
    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QString &cmd, QuietMode quiet)
{
    return run(cmd, quiet, Elevation::Yes);
}

// Return true when process is killed or not running
bool Cmd::terminateAndKill()
{
    if (state() != QProcess::NotRunning) {
        terminate();
        if (!waitForFinished(2000)) {
            kill();
        }
    }
    return state() == QProcess::NotRunning;
}

QString Cmd::readAllOutput() const
{
    return out_buffer.trimmed();
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
    QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                          tr("This operation requires administrator privileges. Please restart the "
                             "application and enter your password when prompted."));
    QCoreApplication::exit(EXIT_FAILURE);
}

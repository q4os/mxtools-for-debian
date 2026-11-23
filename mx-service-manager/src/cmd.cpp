#define QT_USE_QSTRINGBUILDER
#include "cmd.h"

#include <QDebug>
#include <QEventLoop>
#include <QFile>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {QStringLiteral(HELPER_PATH)}
{
}

QString Cmd::getOut(const QStringList &command, bool quiet, bool waitForFinish)
{
    run(command, quiet, waitForFinish);
    return readAll();
}

QString Cmd::getOutAsRoot(const QStringList &helperArgs, bool quiet, bool waitForFinish)
{
    runAsRoot(helperArgs, quiet, waitForFinish);
    return readAll();
}

bool Cmd::run(const QStringList &command, bool quiet, bool waitForFinish)
{
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (command.isEmpty() || command.first().isEmpty()) {
        return false;
    }
    if (!quiet) {
        qDebug().noquote() << command;
    }
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    connect(this, &QProcess::errorOccurred, &loop, [&loop](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            loop.quit();
        }
    });
    start(command.first(), command.mid(1));
    bool completed = true;
    if (state() != QProcess::NotRunning) {
        if (waitForFinish) {
            completed = waitForFinished();
        } else {
            loop.exec();
        }
    }
    emit done();
    return completed && state() == QProcess::NotRunning && error() != QProcess::FailedToStart
           && exitStatus() == QProcess::NormalExit && exitCode() == 0;
}

bool Cmd::runAsRoot(const QStringList &helperArgs, bool quiet, bool waitForFinish)
{
    QStringList command {helper};
    if (getuid() != 0) {
        command.prepend(elevate);
    }
    const bool ok = run(command + helperArgs, quiet, waitForFinish);
    if (elevationUnavailable()) {
        qWarning().noquote() << "Elevation failed for" << program() << arguments()
                             << "exitCode:" << exitCode()
                             << "stderr:" << QString::fromUtf8(readAllStandardError()).trimmed();
    }
    return ok;
}

bool Cmd::elevationUnavailable() const
{
    return program() == elevate && state() == QProcess::NotRunning && error() != QProcess::FailedToStart
           && exitStatus() == QProcess::NormalExit
           && (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND);
}

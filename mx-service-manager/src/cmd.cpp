#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {"/usr/lib/" + QApplication::applicationName() + "/helper"}
{
}

QString Cmd::getOut(const QString &cmd, bool quiet, bool asRoot, bool waitForFinish)
{
    run(cmd, quiet, asRoot, waitForFinish);
    return readAll();
}

QString Cmd::getOutAsRoot(const QString &cmd, bool quiet, bool waitForFinish)
{
    return getOut(cmd, quiet, true, waitForFinish);
}

bool Cmd::run(const QString &cmd, bool quiet, bool asRoot, bool waitForFinish)
{
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (!quiet) {
        qDebug().noquote() << cmd;
    }
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    if (asRoot && getuid() != 0) {
        start(elevate, {helper, cmd});
    } else {
        start("/bin/bash", {"-c", cmd});
    }
    if (!waitForFinish) {
        loop.exec();
    } else {
        waitForFinished();
    }

    // Check for permission denied
    if (asRoot && getuid() != 0) {
        if (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND) {
            handleElevationError();
        }
    }

    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QString &cmd, bool quiet)
{
    return run(cmd, quiet, true);
}

void Cmd::handleElevationError()
{
    QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                          tr("This operation requires administrator privileges. Please restart the application "
                             "and enter your password when prompted."));
    QTimer::singleShot(0, qApp, &QApplication::quit);
    exit(EXIT_FAILURE);
}

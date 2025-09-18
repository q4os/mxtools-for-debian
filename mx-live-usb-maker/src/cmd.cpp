#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {elevationTool()},
      helper {"/usr/lib/" + QApplication::applicationName() + "/helper"}
{
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
    run(cmd, quiet, elevation);
    return readAll();
}

QString Cmd::getOutAsRoot(const QString &cmd, QuietMode quiet)
{
    return getOut(cmd, quiet, Elevation::Yes);
}

bool Cmd::run(const QString &cmd, QuietMode quiet, Elevation elevation)
{
    cmdStr = cmd;
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (quiet == QuietMode::No) {
        qDebug().noquote() << cmd;
    }
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    if (elevation == Elevation::Yes && getuid() != 0) {
        start(elevate, {helper, cmd});
    } else {
        start("/bin/bash", {"-c", cmd});
    }
    loop.exec();
    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QString &cmd, QuietMode quiet)
{
    return run(cmd, quiet, Elevation::Yes);
}

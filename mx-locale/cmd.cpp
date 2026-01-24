#include "cmd.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>

#include "common.h"
#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {QDir(Paths::usrLib).filePath(QCoreApplication::applicationName() + "/helper")}
{
    connect(this, &Cmd::readyReadStandardOutput, [this] { emit outputAvailable(readAllStandardOutput()); });
    connect(this, &Cmd::readyReadStandardError, [this] { emit errorAvailable(readAllStandardError()); });
    connect(this, &Cmd::outputAvailable, [this](const QString &out) { outBuffer += out; });
    connect(this, &Cmd::errorAvailable, [this](const QString &out) { outBuffer += out; });
}

QString Cmd::getOut(const QString &cmd, bool quiet, bool asRoot)
{
    outBuffer.clear();
    run(cmd, quiet, asRoot);
    return outBuffer.trimmed();
}

QString Cmd::getOutAsRoot(const QString &cmd, bool quiet)
{
    return getOut(cmd, quiet, true);
}

bool Cmd::run(const QString &cmd, bool quiet, bool asRoot)
{
    outBuffer.clear();
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
        start(elevate, {helper, "export LC_ALL=C.UTF-8; " + cmd});
    } else {
        start("/bin/bash", {"-c", "export LC_ALL=C.UTF-8; " + cmd});
    }
    loop.exec();
    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QString &cmd, bool quiet)
{
    return run(cmd, quiet, true);
}

QString Cmd::readAllOutput()
{
    return outBuffer.trimmed();
}

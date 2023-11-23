#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFileInfo>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {"/usr/lib/" + QApplication::applicationName() + "/helper"}
{
}

QString Cmd::getOut(const QString &cmd, bool quiet, bool asRoot, bool gui_block)
{
    run(cmd, quiet, asRoot, gui_block);
    return readAll();
}

QString Cmd::getOutAsRoot(const QString &cmd, bool quiet, bool gui_block)
{
    return getOut(cmd, quiet, true, gui_block);
}

bool Cmd::run(const QString &cmd, bool quiet, bool asRoot, bool gui_block)
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
    if (!gui_block) {
        loop.exec();
    } else {
        waitForFinished();
    }
    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QString &cmd, bool quiet)
{
    return run(cmd, quiet, true);
}

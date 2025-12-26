#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {"/usr/lib/mx-repo-manager/helper"}
{
}

QString Cmd::getOut(const QString &cmd, bool quiet, bool asRoot)
{
    run(cmd, quiet, asRoot);
    return readAll();
}

QString Cmd::getOutAsRoot(const QString &cmd, bool quiet)
{
    return getOut(cmd, quiet, true);
}

bool Cmd::run(const QString &cmd, bool quiet, bool asRoot)
{
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (!quiet) {
        qDebug().noquote() << cmd;
    }
    QEventLoop loop;
    QProcess::ProcessError processError = QProcess::UnknownError;
    const QMetaObject::Connection finishedConn
        = connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    const QMetaObject::Connection errorConn = connect(this, &QProcess::errorOccurred, this,
                                                      [&](QProcess::ProcessError error) {
                                                          processError = error;
                                                          loop.quit();
                                                      });
    if (asRoot && getuid() != 0) {
        start(elevate, {helper, cmd});
    } else {
        start("/bin/bash", {"-c", cmd});
    }
    loop.exec();
    disconnect(finishedConn);
    disconnect(errorConn);
    emit done();
    return (processError == QProcess::UnknownError && exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QString &cmd, bool quiet)
{
    return run(cmd, quiet, true);
}

QString Cmd::shellQuote(const QString &arg)
{
    QString escaped = arg;
    escaped.replace("'", "'\\''");
    return "'" + escaped + "'";
}

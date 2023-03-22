#include "cmd.h"

#include <QDebug>
#include <QEventLoop>

Cmd::Cmd(QObject *parent)
    : QProcess(parent)
{
}

bool Cmd::run(const QString &cmd, bool quiet)
{
    QString output;
    return run(cmd, output, quiet);
}

QString Cmd::getCmdOut(const QString &cmd, bool quiet)
{
    QString output;
    run(cmd, output, quiet);
    return output;
}

bool Cmd::run(const QString &cmd, QString &output, bool quiet)
{
    if (this->state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << this->program() << this->arguments();
        return false;
    }
    if (!quiet)
        qDebug().noquote() << cmd;
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Cmd::done);
    setProcessChannelMode(QProcess::MergedChannels);
    start(QStringLiteral("/bin/bash"), {QStringLiteral("-c"), cmd});
    loop.exec();
    output = readAll().trimmed();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

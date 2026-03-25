#include "cmd.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include "common.h"
#include <unistd.h>

namespace {
QString resolveExecutable(const QString &program)
{
    if (QFileInfo(program).isAbsolute()) {
        return program;
    }

    const QString resolved = QStandardPaths::findExecutable(program, {"/usr/sbin", "/usr/bin", "/sbin", "/bin"});
    return resolved.isEmpty() ? program : resolved;
}
} // namespace

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {[&] {
          const QString localHelper = QDir(QCoreApplication::applicationDirPath()).filePath("helper");
          return QFileInfo::exists(localHelper) ? localHelper
                                                : QDir(Paths::usrLib).filePath(QCoreApplication::applicationName()
                                                                               + "/helper");
      }()}
{
    connect(this, &Cmd::readyReadStandardOutput, [this] { emit outputAvailable(readAllStandardOutput()); });
    connect(this, &Cmd::readyReadStandardError, [this] { emit errorAvailable(readAllStandardError()); });
    connect(this, &Cmd::outputAvailable, [this](const QString &out) { outBuffer += out; });
    connect(this, &Cmd::errorAvailable, [this](const QString &out) { outBuffer += out; });
}

QString Cmd::getOut(const QString &program, const QStringList &arguments, bool quiet)
{
    outBuffer.clear();
    run(program, arguments, quiet);
    return outBuffer.trimmed();
}

QString Cmd::getOutAsRoot(const QString &command, const QStringList &arguments, bool quiet)
{
    outBuffer.clear();
    runAsRoot(command, arguments, quiet);
    return outBuffer.trimmed();
}

bool Cmd::run(const QString &program, const QStringList &arguments, bool quiet, const QByteArray &stdinData)
{
    outBuffer.clear();
    lastRunSucceeded = false;
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << this->program() << this->arguments();
        return false;
    }
    const QString resolvedProgram = resolveExecutable(program);
    setProgram(resolvedProgram);
    setArguments(arguments);
    if (!quiet) {
        qDebug().noquote() << this->program() << this->arguments();
    }
    return execProcess(stdinData);
}

bool Cmd::runAsRoot(const QString &command, const QStringList &arguments, bool quiet, const QByteArray &stdinData)
{
    outBuffer.clear();
    lastRunSucceeded = false;
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << this->program() << this->arguments();
        return false;
    }

    if (getuid() != 0) {
        setProgram(elevate);
        QStringList helperArguments {helper, command};
        helperArguments += arguments;
        setArguments(helperArguments);
    } else {
        setProgram(helper);
        QStringList helperArguments {command};
        helperArguments += arguments;
        setArguments(helperArguments);
    }

    if (!quiet) {
        qDebug().noquote() << this->program() << this->arguments();
    }
    return execProcess(stdinData);
}

bool Cmd::execProcess(const QByteArray &stdinData)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C.UTF-8");
    setProcessEnvironment(env);

    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit,
            Qt::SingleShotConnection);
    connect(this, &QProcess::errorOccurred, &loop, &QEventLoop::quit, Qt::SingleShotConnection);
    start();
    if (!stdinData.isEmpty()) {
        write(stdinData);
        closeWriteChannel();
    }
    loop.exec();
    emit done();
    lastRunSucceeded = (exitStatus() == QProcess::NormalExit && exitCode() == 0);
    return lastRunSucceeded;
}

QString Cmd::readAllOutput()
{
    return outBuffer.trimmed();
}

bool Cmd::succeeded() const
{
    return lastRunSucceeded;
}

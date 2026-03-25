#define QT_USE_QSTRINGBUILDER
#include "cmd.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QTimer>

#include <unistd.h>

Cmd::Cmd(QObject *parent)
    : QProcess(parent),
      elevate {QFile::exists("/usr/bin/pkexec") ? "/usr/bin/pkexec" : "/usr/bin/gksu"},
      helper {"/usr/lib/" + QApplication::applicationName() + "/helper"}
{
}

QString Cmd::getOut(const QString &cmd, bool quiet, bool waitForFinish)
{
    run(cmd, quiet, waitForFinish);
    return readAll();
}

QString Cmd::getOutAsRoot(const QStringList &helperArgs, bool quiet, bool waitForFinish)
{
    runAsRoot(helperArgs, quiet, waitForFinish);
    return readAll();
}

bool Cmd::run(const QString &cmd, bool quiet, bool waitForFinish)
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
    start(QStringLiteral("/bin/bash"), {QStringLiteral("-c"), cmd});
    if (!waitForFinish) {
        loop.exec();
    } else {
        waitForFinished();
    }
    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

bool Cmd::runAsRoot(const QStringList &helperArgs, bool quiet, bool waitForFinish)
{
    if (state() != QProcess::NotRunning) {
        qDebug() << "Process already running:" << program() << arguments();
        return false;
    }
    if (!quiet) {
        qDebug().noquote() << helperArgs.join(QLatin1Char(' '));
    }
    QEventLoop loop;
    connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
    if (getuid() != 0) {
        start(elevate, QStringList{helper} + helperArgs);
    } else {
        start(helper, helperArgs);
    }
    if (!waitForFinish) {
        loop.exec();
    } else {
        waitForFinished();
    }
    if (getuid() != 0
        && (exitCode() == EXIT_CODE_PERMISSION_DENIED || exitCode() == EXIT_CODE_COMMAND_NOT_FOUND)) {
        const QByteArray errOut = readAllStandardError();
        const QByteArray stdOut = readAllStandardOutput();
        qWarning().noquote() << "Elevation failed for" << program() << arguments()
                             << "exitCode:" << exitCode()
                             << "stderr:" << QString::fromUtf8(errOut).trimmed()
                             << "stdout:" << QString::fromUtf8(stdOut).trimmed();
        handleElevationError();
    }
    emit done();
    return (exitStatus() == QProcess::NormalExit && exitCode() == 0);
}

void Cmd::handleElevationError()
{
    QMessageBox::critical(nullptr, tr("Administrator Access Required"),
                          tr("This operation requires administrator privileges. Please restart the application "
                             "and enter your password when prompted."));
    QTimer::singleShot(0, qApp, &QApplication::quit);
    exit(EXIT_FAILURE);
}

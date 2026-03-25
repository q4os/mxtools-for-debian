#pragma once

#include <QProcess>
#include <QStringList>

class QTextStream;

class Cmd : public QProcess
{
    Q_OBJECT

public:
    explicit Cmd(QObject *parent = nullptr);

    [[nodiscard]] QString getOut(const QString &program, const QStringList &arguments = {}, bool quiet = false);
    [[nodiscard]] QString getOutAsRoot(const QString &command, const QStringList &arguments = {}, bool quiet = false);
    [[nodiscard]] QString readAllOutput();
    [[nodiscard]] bool succeeded() const;
    bool run(const QString &program, const QStringList &arguments = {}, bool quiet = false,
             const QByteArray &stdinData = {});
    bool runAsRoot(const QString &command, const QStringList &arguments = {}, bool quiet = false,
                   const QByteArray &stdinData = {});

signals:
    void done();
    void errorAvailable(const QString &err);
    void outputAvailable(const QString &out);

private:
    bool execProcess(const QByteArray &stdinData);

    QString elevate;
    QString helper;
    QString outBuffer;
    bool lastRunSucceeded {false};
};

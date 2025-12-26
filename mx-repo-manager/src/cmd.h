#pragma once

#include <QProcess>
#include <QString>

class QTextStream;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    [[nodiscard]] QString getOut(const QString &cmd, bool quiet = false, bool asRoot = false);
    [[nodiscard]] QString getOutAsRoot(const QString &cmd, bool quiet = false);
    bool run(const QString &cmd, bool quiet = false, bool asRoot = false);
    bool runAsRoot(const QString &cmd, bool quiet = false);
    static QString shellQuote(const QString &arg);

signals:
    void done();

private:
    QString elevate;
    QString helper;
};

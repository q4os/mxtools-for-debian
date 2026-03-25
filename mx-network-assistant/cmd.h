#pragma once

#include <QProcess>

class QTextStream;

enum struct QuietMode { No, Yes };
enum struct StderrMode { Inherit, Suppress };

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOutAsRoot(const QString &program, const QStringList &args = {},
                                       QuietMode quiet = QuietMode::No,
                                       StderrMode stderrMode = StderrMode::Inherit);
    bool proc(const QString &program, const QStringList &args = {}, QString *output = nullptr,
              const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procAsRoot(const QString &program, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No,
                    StderrMode stderrMode = StderrMode::Inherit);
    bool run(const QString &cmd, QuietMode quiet = QuietMode::No);
    bool runAsRoot(const QString &program, const QStringList &args = {}, QuietMode quiet = QuietMode::No,
                   StderrMode stderrMode = StderrMode::Inherit);
    bool appendLineAsRoot(const QString &path, const QString &line, QuietMode quiet = QuietMode::No);
    bool writeFileAsRoot(const QString &path, const QByteArray &content, QuietMode quiet = QuietMode::No);

signals:
    void done();

private:
    bool helperProc(const QStringList &helperArgs, QString *output = nullptr, const QByteArray *input = nullptr,
                    QuietMode quiet = QuietMode::No);
    QString elevate;
    QString helper;
};

#pragma once

#include <QProcess>

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    enum class QuietMode {
        No,
        Yes,
    };

    static QString elevationTool();

    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOutAsRoot(const QString &programPath, const QStringList &args = {},
                                       QuietMode quiet = QuietMode::No);
    bool proc(const QString &programPath, const QStringList &args = {}, QString *output = nullptr,
              const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procAsRoot(const QString &programPath, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool run(const QString &cmd, QuietMode quiet = QuietMode::No);

signals:
    void done();

private:
    bool helperProc(const QString &programPath, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);

    QString elevationCommand;
    QString helperPath;
};

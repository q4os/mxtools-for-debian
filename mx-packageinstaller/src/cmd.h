#pragma once

#include <QHash>
#include <QProcess>

class QTextStream;

constexpr int TerminateTimeoutMs = 2000;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    enum class QuietMode { No, Yes };

    static QString elevationTool();

    bool proc(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
              const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procAsRoot(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procAsRootWithEnv(const QHash<QString, QString> &environment, const QString &cmd,
                           const QStringList &args = {}, QString *output = nullptr,
                           const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString readAllOutput() const;
    bool run(const QString &cmd, QuietMode quiet = QuietMode::No);
    bool runHookAsRoot(const QString &script, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString lockingProcessAsRoot(const QString &path, QuietMode quiet = QuietMode::No);
    bool writeFileAsRoot(const QString &path, const QString &content, QuietMode quiet = QuietMode::No);
    [[nodiscard]] bool terminateAndKill();

signals:
    void done();
    void errorAvailable(const QString &err);
    void outputAvailable(const QString &out);

private:
    QString elevate;
    QString helper;
    QString outBuffer;
    QString helperMarkerPath;

    [[nodiscard]] QStringList helperExecArgs(const QString &cmd, const QStringList &args,
                                             const QHash<QString, QString> &environment = {}) const;
    bool helperProc(const QStringList &helperArgs, QString *output = nullptr, const QByteArray *input = nullptr,
                    QuietMode quiet = QuietMode::No);
    bool startAndWait(const QString &program, const QStringList &arguments, QString *output = nullptr,
                      const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No, bool elevated = false,
                      const QString &shellCommand = {});
    [[nodiscard]] bool isAuthenticationDismissed() const;
    void handleElevationError();
};

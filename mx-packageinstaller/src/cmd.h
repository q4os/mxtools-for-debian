#pragma once

#include <QHash>
#include <QList>
#include <QProcess>

class QTextStream;

constexpr int TerminateTimeoutMs = 2000;
constexpr int ElevatedTerminateTimeoutMs = 12000;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    enum class QuietMode { No, Yes };

    // One stage of a pipeline() call: a plain argv program+args pair, plus any
    // environment variables that stage alone needs overridden (e.g. LANG=C on a
    // producer, DEBIAN_FRONTEND on an apt-get). Mirrors procWithEnv()'s environment
    // convention rather than inventing a second one.
    struct PipelineStage
    {
        QString program;
        QStringList args;
        QHash<QString, QString> environment = {};
    };

    static QString elevationTool();

    bool proc(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
              const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No, bool discardStderr = false);
    bool procWithEnv(const QHash<QString, QString> &environment, const QString &cmd, const QStringList &args = {},
                     QString *output = nullptr, const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No,
                     bool discardStderr = false);
    bool procAsRoot(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procAsRootWithEnv(const QHash<QString, QString> &environment, const QString &cmd,
                           const QStringList &args = {}, QString *output = nullptr,
                           const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procScriptAsRoot(const QString &path, const QStringList &args = {}, QString *output = nullptr,
                          const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    // Runs `cmd args...` attached to a pty via `script`, e.g. so a package tool
    // that only colorizes/shows progress bars when it thinks it's interactive
    // behaves the same as it did when invoked through a hand-composed shell
    // string. `script -c` inherently execs its argument through a shell (there is
    // no argv-vector variant), so that one inner string is still built and quoted
    // here -- but unlike the callers this replaces, no extra "/bin/bash -c" layer
    // wraps the whole thing, and callers no longer need to quote anything themselves.
    bool procOnPty(const QString &cmd, const QStringList &args = {}, QuietMode quiet = QuietMode::No);
    // Runs stages connected stdout-to-stdin, like a shell `stage1 | stage2 | ...`
    // pipeline, but as real argv-vector child processes linked with Qt's own
    // QProcess::setStandardOutputProcess() -- no shell, so no argument ever needs
    // shell-quoting. All but the last stage run as disposable, unmonitored
    // QProcess helpers; the last stage runs as `this`, so this Cmd's usual
    // signals/state/output-capture/terminateAndKill() apply to it exactly as for
    // any other call. Matching a shell pipeline's default (non-"pipefail") exit
    // status -- which several existing call sites rely on (e.g. a `grep -q` tail
    // stage exiting 0 after only reading the first match, upstream included) --
    // the returned bool and *output reflect only the last stage; earlier stages
    // exiting non-zero (including from SIGPIPE once a short-circuiting tail stage
    // stops reading) is not itself treated as pipeline failure. Note also that
    // terminateAndKill() reaches every stage, not just the last: while a
    // pipeline() call is in flight, its upstream QProcess helpers are tracked in
    // activePipelineUpstream so a reentrant terminateAndKill() call (e.g. from a
    // Cancel button's slot, while this function's own nested QEventLoop below is
    // pumping events) can signal their process groups too, not just this one's.
    bool pipeline(const QList<PipelineStage> &stages, QString *output = nullptr, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOut(const QString &cmd, const QStringList &args, QuietMode quiet = QuietMode::No,
                                 bool discardStderr = false);
    [[nodiscard]] QString getOut(const QHash<QString, QString> &environment, const QString &cmd,
                                 const QStringList &args, QuietMode quiet = QuietMode::No,
                                 bool discardStderr = false);
    [[nodiscard]] QString readAllOutput() const;
    bool run(const QString &cmd, QuietMode quiet = QuietMode::No);
    bool runHookAsRoot(const QString &script, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString lockingProcessAsRoot(const QString &path, QuietMode quiet = QuietMode::No);
    bool writeFileAsRoot(const QString &path, const QString &content, QuietMode quiet = QuietMode::No);
    [[nodiscard]] bool terminateAndKill();

    // Whether any elevated call anywhere was dismissed by the user (as opposed to
    // failing for a real reason) since the last reset. Callers use this to abort
    // the whole operation and revert quietly instead of reporting a generic error.
    // Deliberately sticky: a dismissal whose return value goes unchecked (e.g. a
    // postinstall hook) is still caught by the next check in the same operation.
    [[nodiscard]] static bool elevationDismissed();
    static void resetElevationDismissed();

    // Pure classification logic behind isAuthenticationDismissed(), pulled out so
    // it can be unit-tested without actually spawning pkexec: pkexec returns 126
    // or 127 when auth is dismissed (varies by version), but a real command
    // failure can produce the same exit code. The helper atomically creates the
    // marker file the instant it starts running as root, so its absence
    // alongside one of those exit codes is what actually distinguishes "auth was
    // dismissed" from "the elevated command itself exited 126/127."
    [[nodiscard]] static bool classifyAuthDismissed(QProcess::ExitStatus exitStatus, int exitCode,
                                                    bool markerPathEmpty, bool markerExists);

signals:
    void done();
    void errorAvailable(const QString &err);
    void outputAvailable(const QString &out);

private:
    QString elevate;
    QString helper;
    QString outBuffer;
    QString helperMarkerPath;
    bool elevatedOperation = false;
    // Non-owning raw pointers to pipeline()'s upstream QProcess helpers, populated
    // only while a pipeline() call is actually in flight (empty otherwise), so
    // terminateAndKill() -- called reentrantly while pipeline()'s own nested
    // QEventLoop is pumping events, e.g. from a Cancel button's slot -- can reach
    // and signal their process groups too, not just the last stage's (`this`).
    // pipeline() still owns the actual QProcess objects via its local vector.
    QList<QProcess *> activePipelineUpstream;

    [[nodiscard]] QStringList helperExecArgs(const QString &cmd, const QStringList &args,
                                             const QHash<QString, QString> &environment = {}) const;
    bool helperProc(const QStringList &helperArgs, QString *output = nullptr, const QByteArray *input = nullptr,
                    QuietMode quiet = QuietMode::No);
    bool startAndWait(const QString &program, const QStringList &arguments, QString *output = nullptr,
                      const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No, bool elevated = false,
                      const QString &shellCommand = {}, bool protectedOperation = false,
                      const QHash<QString, QString> &environmentOverrides = {}, bool discardStderr = false);
    [[nodiscard]] bool isAuthenticationDismissed() const;
    void handleElevationError();
};

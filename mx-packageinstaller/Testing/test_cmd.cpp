#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

#include <cerrno>
#include <csignal>

#include "src/cmd.h"

class CmdTest : public QObject
{
    Q_OBJECT

private slots:
    void cancellationTerminatesProcessGroup();
    void authDismissalClassification();
    void procWithEnvSetsVariable();
    void getOutWithEnvironmentOverload();
    void discardStderrExcludesStandardError();
    void discardStderrDefaultsToCapturingStandardError();
    void discardStderrDoesNotPersistToNextCall();
    void procOnPtyRunsProgramThroughScript();
    void pipelineChainsStdoutToStdin();
    void pipelinePassesPerStageEnvironment();
    void pipelineUsesLastStageExitStatus();
    void pipelineRejectsEmptyStageList();
    void pipelineCancellationTerminatesUpstreamStage();
};

void CmdTest::cancellationTerminatesProcessGroup()
{
    Cmd cmd;
    bool stopped = false;
    bool cancellationRequested = false;
    const auto cancel = [&cmd, &stopped, &cancellationRequested] {
        if (!cancellationRequested && cmd.state() != QProcess::NotRunning) {
            cancellationRequested = true;
            stopped = cmd.terminateAndKill();
        }
    };
    connect(&cmd, &Cmd::outputAvailable, &cmd, [&cancel](const QString &output) {
        if (!output.trimmed().isEmpty()) {
            cancel();
        }
    });
    QTimer::singleShot(1000, &cmd, cancel);

    QVERIFY(!cmd.run(QStringLiteral("sleep 30 & child=$!; echo $child; wait")));
    QVERIFY(cancellationRequested);
    QVERIFY(stopped);

    bool validPid = false;
    const qint64 childPid = cmd.readAllOutput().toLongLong(&validPid);
    QVERIFY(validPid);
    QVERIFY(childPid > 0);

    QTest::qWait(50);
    errno = 0;
    QCOMPARE(::kill(static_cast<pid_t>(childPid), 0), -1);
    QCOMPARE(errno, ESRCH);
}

// Pure classification logic behind Cmd::isAuthenticationDismissed() -- see
// Cmd::classifyAuthDismissed() in src/cmd.cpp. This is what lets the GUI tell
// "the user dismissed the polkit prompt" (revert quietly) apart from "the
// elevated command itself genuinely exited 126/127" (report a real failure),
// without ever needing to actually invoke pkexec.
void CmdTest::authDismissalClassification()
{
    // Dismissed: pkexec's own 126/127 exit, and the helper never got to create
    // its marker because it was never authorized to run.
    QVERIFY(Cmd::classifyAuthDismissed(QProcess::NormalExit, 126, /*markerPathEmpty=*/false, /*markerExists=*/false));
    QVERIFY(Cmd::classifyAuthDismissed(QProcess::NormalExit, 127, /*markerPathEmpty=*/false, /*markerExists=*/false));

    // Not dismissed: same exit code, but the marker exists -- the helper did
    // run (auth succeeded) and the command itself is what exited 126/127.
    QVERIFY(!Cmd::classifyAuthDismissed(QProcess::NormalExit, 126, /*markerPathEmpty=*/false, /*markerExists=*/true));
    QVERIFY(!Cmd::classifyAuthDismissed(QProcess::NormalExit, 127, /*markerPathEmpty=*/false, /*markerExists=*/true));

    // Not dismissed: an unrelated exit code, regardless of the marker.
    QVERIFY(!Cmd::classifyAuthDismissed(QProcess::NormalExit, 1, /*markerPathEmpty=*/false, /*markerExists=*/false));
    QVERIFY(!Cmd::classifyAuthDismissed(QProcess::NormalExit, 0, /*markerPathEmpty=*/false, /*markerExists=*/false));

    // Not dismissed: a crash rather than a normal exit.
    QVERIFY(!Cmd::classifyAuthDismissed(QProcess::CrashExit, 126, /*markerPathEmpty=*/false, /*markerExists=*/false));

    // Not dismissed: this call was never elevated in the first place (no
    // marker path was ever set up), regardless of exit code.
    QVERIFY(!Cmd::classifyAuthDismissed(QProcess::NormalExit, 126, /*markerPathEmpty=*/true, /*markerExists=*/false));
}

// procWithEnv()/getOut(environment, ...) exist so LANG=C-style overrides no longer
// need a shell "VAR=val cmd" prefix; verify the child process actually observes them.
void CmdTest::procWithEnvSetsVariable()
{
    Cmd cmd;
    QString output;
    QVERIFY(cmd.procWithEnv({{"MXPI_CMD_TEST_VAR", "sentinel-value"}}, QStringLiteral("printenv"),
                            {"MXPI_CMD_TEST_VAR"}, &output));
    QCOMPARE(output.trimmed(), QStringLiteral("sentinel-value"));
}

void CmdTest::getOutWithEnvironmentOverload()
{
    Cmd cmd;
    const QString output
        = cmd.getOut({{"MXPI_CMD_TEST_VAR", "other-value"}}, QStringLiteral("printenv"), {"MXPI_CMD_TEST_VAR"});
    QCOMPARE(output, QStringLiteral("other-value"));
}

// Several call sites (flatpak/snap listings) rely on discardStderr to drop expected
// warnings from parsed output without a shell "2>/dev/null" redirection.
void CmdTest::discardStderrExcludesStandardError()
{
    Cmd cmd;
    QString output;
    QVERIFY(cmd.proc(QStringLiteral("sh"), {"-c", "echo on-stdout; echo on-stderr 1>&2"}, &output, nullptr,
                     Cmd::QuietMode::Yes, /*discardStderr=*/true));
    QCOMPARE(output.trimmed(), QStringLiteral("on-stdout"));
}

void CmdTest::discardStderrDefaultsToCapturingStandardError()
{
    Cmd cmd;
    QString output;
    QVERIFY(cmd.proc(QStringLiteral("sh"), {"-c", "echo on-stdout; echo on-stderr 1>&2"}, &output));
    QVERIFY(output.contains(QLatin1String("on-stdout")));
    QVERIFY(output.contains(QLatin1String("on-stderr")));
}

// Regression test: QProcess's stderr-redirection-to-a-file setting persists across
// start() calls on the same object unless explicitly reset. A Cmd is frequently
// reused for several sequential calls (e.g. MainWindow's shared `cmd` member), so a
// single discardStderr=true call must not silently keep discarding stderr on every
// later call made on that same instance.
void CmdTest::discardStderrDoesNotPersistToNextCall()
{
    Cmd cmd;
    QString discarded;
    QVERIFY(cmd.proc(QStringLiteral("sh"), {"-c", "echo on-stdout; echo on-stderr 1>&2"}, &discarded, nullptr,
                     Cmd::QuietMode::Yes, /*discardStderr=*/true));
    QCOMPARE(discarded.trimmed(), QStringLiteral("on-stdout"));

    QString captured;
    QVERIFY(cmd.proc(QStringLiteral("sh"), {"-c", "echo on-stdout; echo on-stderr 1>&2"}, &captured));
    QVERIFY(captured.contains(QLatin1String("on-stdout")));
    QVERIFY2(captured.contains(QLatin1String("on-stderr")),
             "stderr redirection from a prior discardStderr=true call leaked into this one");
}

// procOnPty() replaces the old "cmd.run(flatpakPtyCommand(shellCommandFromArgs(args)))"
// pattern; confirm it actually drives a program through `script` and captures its output.
void CmdTest::procOnPtyRunsProgramThroughScript()
{
    if (!QFile::exists(QStringLiteral("/usr/bin/script"))) {
        QSKIP("script (util-linux) is not installed; procOnPty() requires it");
    }
    Cmd cmd;
    QVERIFY(cmd.procOnPty(QStringLiteral("printf"), {"hello %s", "pty"}));
    QCOMPARE(cmd.readAllOutput().trimmed(), QStringLiteral("hello pty"));
}

// The core of pipeline(): stages must be linked stdout-to-stdin like a real shell `|`,
// entirely via argv-vector child processes (no shell string involved anywhere).
void CmdTest::pipelineChainsStdoutToStdin()
{
    Cmd cmd;
    QString output;
    QVERIFY(cmd.pipeline({
                             {"printf", {"3\n1\n2\n"}},
                             {"sort", {}},
                         },
                         &output));
    QCOMPARE(output, QStringLiteral("1\n2\n3"));
}

// Per-stage environment (e.g. DEBIAN_FRONTEND/LANG on just the producer stage) must
// reach that stage's child process specifically.
void CmdTest::pipelinePassesPerStageEnvironment()
{
    Cmd cmd;
    QString output;
    QVERIFY(cmd.pipeline({
                             {"env", {}, {{"MXPI_PIPELINE_TEST_VAR", "sentinel-value"}}},
                             {"grep", {"-c", "^MXPI_PIPELINE_TEST_VAR=sentinel-value$"}},
                         },
                         &output));
    QCOMPARE(output, QStringLiteral("1"));
}

// Matches a shell pipeline's default (non-"pipefail") behaviour: several real call
// sites rely on a tail stage like `grep -m1 -q` exiting 0 after reading only the first
// match, even though the producer stage (here an unbounded `yes`) never got to finish
// and is killed off once nothing is reading it anymore. This also exercises pipeline()'s
// upstream-cleanup path -- if it didn't terminate the runaway producer, this test would
// leak a `yes` process running forever.
void CmdTest::pipelineUsesLastStageExitStatus()
{
    Cmd cmd;
    QVERIFY(cmd.pipeline({
        {"yes", {"pipeline-test-line"}},
        {"grep", {"-m1", "-q", "pipeline-test-line"}},
    }));
}

void CmdTest::pipelineRejectsEmptyStageList()
{
    Cmd cmd;
    QVERIFY(!cmd.pipeline({}));
}

// Regression test: terminateAndKill() must reach a pipeline()'s upstream stage(s)
// too, not just the last stage -- each stage runs in its own session (see
// makeNewSession()), so a plain signal to the last stage's group alone would leave
// an upstream producer (and anything it spawned) running after cancellation.
void CmdTest::pipelineCancellationTerminatesUpstreamStage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString pidFile = dir.filePath(QStringLiteral("child.pid"));

    Cmd cmd;
    QTimer::singleShot(1000, &cmd, [&cmd] { [[maybe_unused]] const bool stopped = cmd.terminateAndKill(); });

    // The upstream stage backgrounds a grandchild `sleep` and records its pid to a
    // file (out-of-band: its stdout feeds the pipeline chain, and its stderr is
    // only forwarded, not captured, so neither can carry this back to the test),
    // then blocks in `wait` so the pipeline stays running until cancelled. The
    // last stage (`cat`) never receives any actual bytes and simply blocks too,
    // giving the timer above time to fire.
    const QString upstreamScript = QStringLiteral("sleep 30 & echo $! > %1; wait").arg(pidFile);
    QVERIFY(!cmd.pipeline({
        {"sh", {"-c", upstreamScript}},
        {"cat", {}},
    }));

    QVERIFY(QFile::exists(pidFile));
    QFile file(pidFile);
    QVERIFY(file.open(QIODevice::ReadOnly));
    bool validPid = false;
    const qint64 grandchildPid = QString::fromUtf8(file.readAll()).trimmed().toLongLong(&validPid);
    QVERIFY(validPid);
    QVERIFY(grandchildPid > 0);

    QTest::qWait(200);
    errno = 0;
    QCOMPARE(::kill(static_cast<pid_t>(grandchildPid), 0), -1);
    QCOMPARE(errno, ESRCH);
}

QTEST_GUILESS_MAIN(CmdTest)

#include "test_cmd.moc"

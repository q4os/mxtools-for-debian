/**********************************************************************
 *  test_main.cpp
 **********************************************************************
 * Copyright (C) 2026 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <cstdio>
#include <cstdlib>

#include "../src/cmd.h"

// ── Mini test framework ──────────────────────────────────────────────────
static int tests_run = 0;
static int asserts_failed = 0;
static const char *current_test = nullptr;

#define TEST(name)                                                          \
    static void test_##name();                                              \
    static void test_##name()

#define RUN_TEST(name)                                                      \
    do {                                                                    \
        current_test = #name;                                               \
        tests_run++;                                                        \
        std::fprintf(stdout, "[%d] %s ... ", tests_run, #name);             \
        std::fflush(stdout);                                                \
        int before = asserts_failed;                                        \
        test_##name();                                                      \
        std::fprintf(stdout, "%s\n", asserts_failed == before ? "OK" : "FAILED"); \
    } while (false)

#define ASSERT_TRUE(cond)                                                   \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::fprintf(stderr, "  FAIL [%s:%d] %s: %s\n",                 \
                         __FILE__, __LINE__, current_test, #cond);           \
            asserts_failed++;                                               \
        }                                                                   \
    } while (false)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ_QSTR(a, b)                                                \
    do {                                                                    \
        auto _a = (a);                                                      \
        auto _b = (b);                                                      \
        if (_a != _b) {                                                     \
            std::fprintf(stderr, "  FAIL [%s:%d] %s: '%s' == '%s'\n"        \
                         "    expected: %s\n    actual:   %s\n",            \
                         __FILE__, __LINE__, current_test, #a, #b,           \
                         _b.toUtf8().constData(),                            \
                         _a.toUtf8().constData());                          \
            asserts_failed++;                                               \
        }                                                                   \
    } while (false)

#define ASSERT_EQ_INT(a, b)                                                 \
    do {                                                                    \
        auto _a = (a);                                                      \
        auto _b = (b);                                                      \
        if (_a != _b) {                                                     \
            std::fprintf(stderr, "  FAIL [%s:%d] %s: '%s' == '%s'\n"        \
                         "    expected: %lld\n    actual:   %lld\n",         \
                         __FILE__, __LINE__, current_test, #a, #b,           \
                         static_cast<long long>(_b),                         \
                         static_cast<long long>(_a));                       \
            asserts_failed++;                                               \
        }                                                                   \
    } while (false)

// ── Cmd: shell-based run() ───────────────────────────────────────────────

TEST(cmd_shell_echo)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(cmd.run("echo hello", &out));
    ASSERT_EQ_QSTR(out, QStringLiteral("hello"));
}

TEST(cmd_shell_empty_output)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(cmd.run("true", &out));
    ASSERT_EQ_QSTR(out, QString());
}

TEST(cmd_shell_false_returns_false)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(!cmd.run("false", &out));
}

TEST(cmd_shell_getOut)
{
    Cmd cmd;
    ASSERT_EQ_QSTR(cmd.getOut("echo hello"), QStringLiteral("hello"));
}

TEST(cmd_shell_multiple_words)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(cmd.run("echo hello world", &out));
    ASSERT_EQ_QSTR(out, QStringLiteral("hello world"));
}

TEST(cmd_shell_multiline)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(cmd.run("printf 'line1\\nline2\\nline3'", &out));
    ASSERT_TRUE(out.contains(QStringLiteral("line1")));
    ASSERT_TRUE(out.contains(QStringLiteral("line2")));
    ASSERT_TRUE(out.contains(QStringLiteral("line3")));
}

// ── Cmd: arg-based proc() ─────────────────────────────────────────────────

TEST(cmd_args_echo)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(cmd.proc("echo", {"hello"}, &out));
    ASSERT_EQ_QSTR(out, QStringLiteral("hello"));
}

TEST(cmd_args_false_returns_false)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(!cmd.proc("false", {}, &out));
}

TEST(cmd_failed_start_after_success_returns_false)
{
    Cmd cmd;
    ASSERT_TRUE(cmd.proc("true"));
    ASSERT_FALSE(cmd.proc("/definitely/not/a/command"));
}

TEST(cmd_timeout_terminates_process)
{
    Cmd cmd;
    QElapsedTimer elapsed;
    elapsed.start();
    ASSERT_FALSE(cmd.proc("sh", {"-c", "sleep 2"}, nullptr, nullptr, QuietMode::Yes, Elevation::No, 100));
    ASSERT_TRUE(elapsed.elapsed() < 1500);
    ASSERT_TRUE(cmd.proc("true"));
}

TEST(cmd_cancel_terminates_process)
{
    Cmd cmd;
    QElapsedTimer elapsed;
    elapsed.start();
    QTimer::singleShot(100, &cmd, &Cmd::cancel);
    ASSERT_FALSE(cmd.proc("sh", {"-c", "sleep 2"}, nullptr, nullptr, QuietMode::Yes));
    ASSERT_TRUE(elapsed.elapsed() < 1500);
    ASSERT_FALSE(cmd.proc("true"));
    cmd.clearCancelRequest();
    ASSERT_TRUE(cmd.proc("true"));
}

TEST(cmd_args_merged_channels)
{
    Cmd cmd;
    QString out;
    ASSERT_TRUE(!cmd.proc("sh", {"-c", "echo stdout; echo stderr >&2; false"}, &out));
    ASSERT_TRUE(!out.isEmpty());
}

// ── Cmd: reuse / accumulation ────────────────────────────────────────────

TEST(cmd_reuse_repeated_calls)
{
    Cmd cmd;
    for (int i = 0; i < 5; ++i) {
        QString out;
        ASSERT_TRUE(cmd.run("echo iteration", &out));
        ASSERT_FALSE(out.isEmpty());
    }
}

TEST(cmd_reuse_mixed_shell_and_args)
{
    Cmd cmd;
    for (int i = 0; i < 3; ++i) {
        QString out;
        ASSERT_TRUE(cmd.run("echo shell", &out));
        ASSERT_EQ_QSTR(out, QStringLiteral("shell"));

        ASSERT_TRUE(cmd.proc("echo", {"args"}, &out));
        ASSERT_EQ_QSTR(out, QStringLiteral("args"));
    }
}

// ── Cmd: quiet mode ──────────────────────────────────────────────────────

TEST(cmd_quiet_suppresses_debug)
{
    Cmd cmd;
    QString out;
    // Quiet mode should not affect functionality, only debug output
    ASSERT_TRUE(cmd.run("echo quiet", &out, nullptr, QuietMode::Yes));
    ASSERT_EQ_QSTR(out, QStringLiteral("quiet"));
}

// ── Main ─────────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    RUN_TEST(cmd_shell_echo);
    RUN_TEST(cmd_shell_empty_output);
    RUN_TEST(cmd_shell_false_returns_false);
    RUN_TEST(cmd_shell_getOut);
    RUN_TEST(cmd_shell_multiple_words);
    RUN_TEST(cmd_shell_multiline);
    RUN_TEST(cmd_args_echo);
    RUN_TEST(cmd_args_false_returns_false);
    RUN_TEST(cmd_failed_start_after_success_returns_false);
    RUN_TEST(cmd_timeout_terminates_process);
    RUN_TEST(cmd_cancel_terminates_process);
    RUN_TEST(cmd_args_merged_channels);
    RUN_TEST(cmd_reuse_repeated_calls);
    RUN_TEST(cmd_reuse_mixed_shell_and_args);
    RUN_TEST(cmd_quiet_suppresses_debug);

    std::printf("\nResults: %d tests, %d failed\n", tests_run, asserts_failed);
    return asserts_failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

/**********************************************************************
 *  test_main.cpp
 **********************************************************************
 * Copyright (C) 2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "cmd.h"

#include <QCoreApplication>
#include <QDebug>

#define CHECK(cond, msg)                                 \
    do {                                                 \
        if (!(cond)) {                                   \
            qCritical() << "FAIL:" << msg;               \
            return 1;                                    \
        }                                                \
    } while (0)

#define CHECK_EQ(a, b, msg)                              \
    do {                                                 \
        if ((a) != (b)) {                                \
            qCritical() << "FAIL:" << msg                \
                        << "— got:" << (a)               \
                        << "expected:" << (b);           \
            return 1;                                    \
        }                                                \
    } while (0)

#define RUN_TEST(name)                                   \
    do {                                                 \
        qDebug() << "  Running:" << #name;               \
        if (int r = test_##name()) {                     \
            qCritical() << "FAILED:" << #name;           \
            return r;                                    \
        }                                                \
    } while (0)

static int test_cmd_echo()
{
    Cmd cmd;
    QString output = cmd.getCmdOut("echo hello world");
    CHECK_EQ(output, QStringLiteral("hello world"), "echo output");
    return 0;
}

static int test_cmd_exit_code()
{
    Cmd cmd;
    CHECK(cmd.run("true"), "true should succeed");
    CHECK(!cmd.run("false"), "false should fail");
    return 0;
}

static int test_cmd_multiline()
{
    Cmd cmd;
    QString output;
    CHECK(cmd.runUntrimmed("printf 'line1\\nline2\\n'", output), "printf");
    CHECK_EQ(output, QStringLiteral("line1\nline2\n"), "multiline output");
    return 0;
}

static int test_cmd_untrimmed()
{
    Cmd cmd;
    QString output = cmd.getCmdOutUntrimmed("printf '  spaced  \\n'");
    CHECK_EQ(output, QStringLiteral("  spaced  \n"), "untrimmed output");
    return 0;
}

static int test_cmd_trimmed()
{
    Cmd cmd;
    QString output = cmd.getCmdOut("printf '\\n  trimmed  \\n'");
    CHECK_EQ(output, QStringLiteral("trimmed"), "trimmed output");
    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qDebug() << "mx-conky unit tests";

    RUN_TEST(cmd_echo);
    RUN_TEST(cmd_exit_code);
    RUN_TEST(cmd_multiline);
    RUN_TEST(cmd_untrimmed);
    RUN_TEST(cmd_trimmed);

    qDebug() << "All tests passed.";
    return 0;
}

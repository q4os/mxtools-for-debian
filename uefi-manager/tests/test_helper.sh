#!/bin/bash
#
# Unit tests for the compiled helper command validation.
#

set -euo pipefail

HELPER="${1:-$(dirname "$0")/../build/helper}"
PASS=0
FAIL=0

# Run a test case.
#   expect_ok  <description> <args...>  — expect exit 0
#   expect_err <description> <args...>  — expect non-zero exit
expect_ok() {
    local desc="$1"; shift
    if "$HELPER" "$@" >/dev/null 2>&1; then
        ((++PASS))
    else
        echo "FAIL (expected ok): $desc" >&2
        ((++FAIL))
    fi
}

expect_err() {
    local desc="$1"; shift
    if "$HELPER" "$@" >/dev/null 2>&1; then
        echo "FAIL (expected error): $desc" >&2
        ((++FAIL))
    else
        ((++PASS))
    fi
}

# Check that a specific error message appears on stderr.
#   expect_err_msg <description> <pattern> <args...>
expect_err_msg() {
    local desc="$1"; shift
    local pattern="$1"; shift
    local stderr
    stderr="$("$HELPER" "$@" 2>&1 >/dev/null || true)"
    if [[ "$stderr" == *"$pattern"* ]]; then
        ((++PASS))
    else
        echo "FAIL (expected '$pattern' in stderr): $desc — got: $stderr" >&2
        ((++FAIL))
    fi
}

echo "=== Exec action tests ==="

expect_ok   "allowed command: lsblk"          exec lsblk --version
expect_ok   "allowed command: grep"           exec grep --version
expect_ok   "allowed command: efibootmgr"     exec efibootmgr --version
expect_err  "disallowed command: bash"        exec bash -c "echo hi"
expect_err  "disallowed command: sh"          exec sh -c "echo hi"
expect_err  "disallowed command: cat"         exec cat /etc/passwd
expect_err  "disallowed command: python"      exec python3 -c "print(1)"
expect_err  "disallowed command: chmod"       exec chmod 777 /tmp
expect_err  "disallowed command: chown"       exec chown root /tmp

echo "=== No arguments ==="

expect_err_msg "no args" "Missing helper action"
expect_err_msg "exec without command" "exec requires a command name" exec

echo "=== Lib action tests ==="

expect_err_msg "lib without subcommand" "lib requires a subcommand" lib
expect_err_msg "invalid lib subcommand" "lib subcommand is not allowed" lib bad_subcommand

stderr="$("$HELPER" lib copy_log 2>&1 || true)"
if [[ "$stderr" == *"lib subcommand is not allowed"* ]]; then
    echo "FAIL: allowed lib subcommand rejected" >&2
    ((++FAIL))
else
    ((++PASS))
fi

echo "=== Single-string shell commands are rejected ==="

expect_err_msg "single-arg pipeline string" "Command is not allowed" exec 'grep --version | cut -d" " -f1'
expect_err_msg "single-arg command substitution" "Command is not allowed" exec 'echo $(whoami)'
expect_err_msg "single-arg redirection" "Command is not allowed" exec 'grep foo /dev/null > /tmp/out'
expect_err_msg "single-quoted command in one arg" "Command is not allowed" exec "'grep' --version"
expect_err_msg "double-quoted command in one arg" "Command is not allowed" exec '"grep" --version'

echo "=== Empty/whitespace command arguments ==="

expect_err_msg "empty string command" "Command is not allowed" exec ""
expect_err_msg "whitespace command" "Command is not allowed" exec "   "

echo "=== Unsupported helper actions ==="

expect_err_msg "unsupported action" "Unsupported helper action" unknown

echo ""
echo "Results: $PASS passed, $FAIL failed"
if ((FAIL > 0)); then
    exit 1
fi

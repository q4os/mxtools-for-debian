#!/bin/bash
#
# Unit tests for scripts/helper command validation.
#

set -euo pipefail

HELPER="$(dirname "$0")/../scripts/helper"
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

echo "=== Fast path (multi-arg) tests ==="

expect_ok   "allowed command: lsblk"          lsblk --version
expect_ok   "allowed command: grep"           grep --version
expect_ok   "allowed command: efibootmgr"     efibootmgr --version
expect_err  "disallowed command: bash"         bash -c "echo hi"
expect_err  "disallowed command: sh"           sh -c "echo hi"
expect_err  "disallowed command: cat"          cat /etc/passwd
expect_err  "disallowed command: python"       python3 -c "print(1)"
expect_err  "disallowed command: chmod"        chmod 777 /tmp
expect_err  "disallowed command: chown"        chown root /tmp

echo "=== No arguments ==="

expect_err_msg "no args" "no command provided"

echo "=== Allowed full-path command ==="

# This path won't exist in test env, so it will fail to exec, but it should
# pass validation (exit code from missing binary, not from permission check).
# We just verify it doesn't say "not permitted".
stderr="$("$HELPER" /usr/lib/uefi-manager/uefimanager-lib 2>&1 || true)"
if [[ "$stderr" == *"not permitted"* ]]; then
    echo "FAIL: full-path command rejected" >&2
    ((++FAIL))
else
    ((++PASS))
fi

echo "=== Single-string shell commands are rejected ==="

expect_err_msg "single-arg pipeline string" "not permitted" 'grep --version | cut -d" " -f1'
expect_err_msg "single-arg command substitution" "not permitted" 'echo $(whoami)'
expect_err_msg "single-arg redirection" "not permitted" 'grep foo /dev/null > /tmp/out'
expect_err_msg "single-quoted command in one arg" "not permitted" "'grep' --version"
expect_err_msg "double-quoted command in one arg" "not permitted" '"grep" --version'

echo "=== Empty/whitespace command arguments ==="

expect_err_msg "empty string command" "not permitted" ""
expect_err_msg "whitespace command" "not permitted" "   "

echo ""
echo "Results: $PASS passed, $FAIL failed"
if ((FAIL > 0)); then
    exit 1
fi

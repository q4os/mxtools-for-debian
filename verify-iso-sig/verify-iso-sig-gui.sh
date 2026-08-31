#!/usr/bin/bash
#
# Copyright (C) 2026  fehlix <fehlix@mxlinux.org>
#                     MX Linux Development Team <https://mxlinux.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# shellcheck disable=SC2034
# Many of this file's variables are read only via eval_gettext/
# safe_eval_gettext's own envsubst-based substitution - it dynamically
# exports whatever variable names appear in the msgid text, a mechanism
# this static checker can't trace, so it flags every one of them as
# unused. Real dead variables are still worth checking for by hand.
# yad GUI for verify-iso-sig - two modes in one process: the picker
# (pick an ISO/signature file and check it - the default) and the
# trusted-keys manager (--manage-keys - list/forget/export/import keys
# saved in ~/.gnupg/trustedkeys.gpg, also reachable from the picker's
# own "Manage Trusted Keys" button). Both modes share the yad/gettext/
# pango helpers below; GUI_MODE picks which top-level flow runs.
#
# -e: exit on any error. -u: error on unset vars. pipefail: a pipeline
# fails if any stage does, not just the last.
set -euo pipefail

# Same literal value as verify-iso-sig, kept in sync by hand.
VERSION="2026.08.01"

# Literal, never-translated command reference used inside one
# translatable message in manager mode below.
CMD_LIST_KNOWN_KEYS="verify-iso-sig --list-known-keys"

# readlink -f, not a plain dirname, so this works even when this file
# itself is a symlink.
SELF=$(readlink -f -- "${BASH_SOURCE[0]}" 2>/dev/null) || SELF=${BASH_SOURCE[0]}
SCRIPT_DIR=$(dirname "$SELF")
# VERIFY needs verify-iso-sig as a sibling - in a real install this file
# lives in usr/lib/verify-iso-sig/ while verify-iso-sig lives in usr/bin/,
# so detect that shape (mirrors verify-iso-sig's own SCRIPT_DIR detection).
if [ "$(basename "$SCRIPT_DIR")" = "verify-iso-sig" ] && [ "$(basename "$(dirname "$SCRIPT_DIR")")" = "lib" ] && [ "$(basename "$(dirname "$(dirname "$SCRIPT_DIR")")")" = "usr" ]; then
    USR_DIR=$(dirname "$(dirname "$SCRIPT_DIR")")
    VERIFY="$USR_DIR/bin/verify-iso-sig"
else
    VERIFY="$SCRIPT_DIR/verify-iso-sig"
fi
# The packaged dispatcher exports DISPLAY_NAME=verify-iso-sig before
# exec-ing here; falls back to this file's own name for direct execution.
DISPLAY_NAME=${DISPLAY_NAME:-$(basename "$SELF")}

# A leading "--manage-keys" switches into the trusted-keys-manager flow
# instead of the default picker, consumed here before either mode's own
# argument handling runs.
GUI_MODE=picker
if [ "${1:-}" = "--manage-keys" ]; then
    GUI_MODE=manage
    shift
fi

# Manager mode's own -h/--help/-V short-circuit, deliberately this early
# and gettext-independent, before ever sourcing gettext.sh or checking
# for yad/$VERIFY. Picker mode's own -h/--help/-V/--debug handling stays
# at its own, later position further below.
if [ "$GUI_MODE" = manage ]; then
    for arg in "$@"; do
        case "$arg" in
            -h|--help)
                cat <<EOF
Usage: $DISPLAY_NAME --manage-keys

GUI to list and remove keys saved in ~/.gnupg/trustedkeys.gpg. Open it
via 'verify-iso-sig --manage-keys' from a terminal, or the "Manage
Trusted Keys" button in the main window. See 'verify-iso-sig --man'
for the full manual.

  -h, --help      this help
  -V, --version   show version and exit
EOF
                exit 0
                ;;
            -V|--version) printf '%s %s\n' "$DISPLAY_NAME" "$VERSION"; exit 0 ;;
        esac
    done
fi

# gettext.sh (gettext-base package) provides gettext()/eval_gettext() for
# this script's own user-facing strings. TEXTDOMAIN must match the .mo
# filename; both TEXTDOMAIN/TEXTDOMAINDIR must be exported - the external
# gettext/eval_gettext calls are subprocesses that silently don't see
# unexported variables. One shared catalog covers both modes.
. /usr/bin/gettext.sh
export TEXTDOMAIN="verify-iso-sig"
export TEXTDOMAINDIR="$SCRIPT_DIR/locale"

# eval_gettext() pipes through envsubst, so a broken translation can
# never execute code - only leave a literal unsubstituted "${VAR}"/"$VAR"
# visible (checked below by looking for the real value in the result,
# which also catches a mismatched brace or typo'd name a plain
# placeholder match would miss).
#
# Separately, a translator can break Pango markup tags (mistyped,
# dropped, or extra) - yad/GTK then fails to parse the markup and blanks
# the entire dialog text, not just cosmetic. pango_tags_ok() compares the
# exact tag-name sequence against the English original.
#
# Either kind of breakage falls back to the English original (still
# through eval_gettext, so values still substitute) with a one-time stderr warning.
pango_tag_tokens() {
    grep -oE '</?[A-Za-z][A-Za-z0-9]*' <<< "$1" | sed 's/^<//'
}

# True (exit 0) if $1 (a captured verify-iso-sig output variable, e.g.
# $OUTPUT) contains a "[VERIFY-ISO-SIG:] $2" tag line - see
# verify-iso-sig's own status_out()/--status-fd for what emits these.
# Takes the output as an explicit argument rather than reading a fixed
# global, so this same helper works regardless of mode. Only used by
# picker mode, but harmless to define unconditionally.
gui_status_has() {
    printf '%s\n' "$1" | grep -q "^\[VERIFY-ISO-SIG:\] $2\$\|^\[VERIFY-ISO-SIG:\] $2 "
}

# Echoes the value part (fields 3 onward - may itself contain spaces) of
# the first "[VERIFY-ISO-SIG:] $2 ..." tag line in $1.
gui_status_field() {
    # A tag not being present is normal, not an error - must never
    # propagate grep's no-match exit under `set -o pipefail`/`set -e`.
    printf '%s\n' "$1" | grep -m1 "^\[VERIFY-ISO-SIG:\] $2 " | cut -d' ' -f3- || true
}

pango_tags_ok() {
    local candidate=$1 reference=$2
    local -a stack=()
    local tag name
    while IFS= read -r tag; do
        [ -z "$tag" ] && continue
        if [[ $tag == /* ]]; then
            name=${tag#/}
            [ "${#stack[@]}" -gt 0 ] && [ "${stack[-1]}" = "$name" ] || return 1
            unset 'stack[-1]'
        else
            stack+=("$tag")
        fi
    done < <(pango_tag_tokens "$candidate")
    [ "${#stack[@]}" -eq 0 ] || return 1
    [ "$(pango_tag_tokens "$candidate")" = "$(pango_tag_tokens "$reference")" ]
}

safe_eval_gettext() {
    local msgid=$1 result var value broken=0
    shift
    result=$(eval_gettext "$msgid")
    for var in "$@"; do
        value=${!var}
        if [[ "$result" != *"$value"* ]]; then
            echo "warning: translation for \"$msgid\" is missing the expected \$$var value (broken placeholder in the .po?) - showing English instead" >&2
            broken=1
            break
        fi
    done
    if [ "$broken" -eq 0 ] && ! pango_tags_ok "$result" "$msgid"; then
        echo "warning: translation for \"$msgid\" has broken or mismatched markup tags (a translator likely mistyped or dropped a <tag> in the .po) - showing English instead" >&2
        broken=1
    fi
    [ "$broken" -eq 1 ] && result=$(LC_ALL=C LANGUAGE= eval_gettext "$msgid")
    printf '%s' "$result"
}

# TITLE and the mode-specific button labels are set per-mode: picker's
# own values here as plain globals; the manager's own are set as locals
# inside run_manage_mode() itself. YAD_ERROR_WIDTH="" means yad_error()
# emits no --width flag in picker mode.
case "$GUI_MODE" in
    picker)
        TITLE=$(safe_eval_gettext "Verify ISO signature")
        # Translated once here, then referenced everywhere this button
        # label is also quoted inline inside a longer sentence, so both
        # can't drift out of sync via two separately-translated strings.
        BTN_TRUST_THIS_KEY=$(safe_eval_gettext "Trust this key")
        BTN_CHECK_ANOTHER_FILE=$(safe_eval_gettext "Check Another File")
        YAD_ERROR_WIDTH=""
        ;;
esac
ICON_FILE="$SCRIPT_DIR/verify-iso-sig.svg"

# Help button target - a local copy of the README (built from it by
# debian/rules, English-only for now) if installed, else the GitHub
# page. The local copy avoids the full GitHub web UI chrome and works
# offline; it's skipped in a from-source checkout, where help/ was
# never built.
HELP_FILE="$SCRIPT_DIR/help/help.html"
if [ -f "$HELP_FILE" ]; then
    HELP_URL="file://$HELP_FILE"
else
    HELP_URL="https://github.com/MX-Linux/verify-iso-sig/blob/main/README.md"
fi

# --class is a standard GTK option, setting WM_CLASS so the window
# manager/taskbar can group and icon this window - matches
# verify-iso-sig.desktop's StartupWMClass=. Same class for both modes -
# a distinct manager class left the taskbar showing a generic icon
# instead of the real one.
WM_CLASS="verify-iso-sig"

# GUI apps shouldn't run as root - and this one specifically reads/
# writes the real ~/.gnupg (verify-iso-sig's own $TRUSTED_GPG/
# $PUBRING_KBX, both under $HOME - not this tool's own throwaway
# per-run $GNUPGHOME), so under sudo/pkexec, $HOME can resolve to
# root's own home (fine but pointless - a fresh, empty trust store
# every time) or, if the invoking user's environment gets preserved,
# to THAT user's real ~/.gnupg - written to with root ownership,
# breaking their own later, unprivileged use of this tool. CLI-only
# use is deliberately NOT guarded the same way here - a scripted or
# headless root context may have no other user to run as, and that's
# the caller's own call to make.
if [ "$EUID" -eq 0 ]; then
    safe_eval_gettext "error: refusing to run as root - run this as your normal desktop user instead" >&2
    printf '\n' >&2
    exit 1
fi

command -v yad >/dev/null || { safe_eval_gettext "error: yad is not installed" >&2; printf '\n' >&2; exit 1; }
# TRANSLATORS: ${VERIFY} is a file path - keep the placeholder as-is.
[ -x "$VERIFY" ] || { safe_eval_gettext "error: \${VERIFY} not found or not executable" VERIFY >&2; printf '\n' >&2; exit 1; }

# Sourced in-process instead of invoked as a subprocess - LIB_MODE=1
# makes its own die() return 1 instead of exiting this whole GUI process.
# lib_init_defaults sets every global the mode functions below implicitly
# depend on.
LIB_MODE=1
# shellcheck source=./verify-iso-sig
. "$VERIFY"
lib_init_defaults

# `--selectable-labels` pre-highlights the first selectable label's text
# on old yad (0.40.0/GTK+ 3.24.38) but not current yad (14.1/GTK+
# 3.24.49) - yad's versioning jumped from "0.NN.N" to "14.N", so a plain
# major-version check (>= 14) distinguishes them. `${var%%[!0-9]*}`
# strips from the first non-digit onward, deliberately not `cut -d'.'`
# which would cut at the wrong dot in a future "16 (GTK+ 4.0.1)". `||
# true`: this yad build exits `--version` with status 252 despite
# printing the version correctly, which `set -e` would otherwise abort on.
YAD_MAJOR=$(yad --version 2>/dev/null | head -n1) || true
YAD_MAJOR=${YAD_MAJOR%%[!0-9]*}
[ -n "$YAD_MAJOR" ] || YAD_MAJOR=0
if [ "$YAD_MAJOR" -ge 14 ]; then
    SELECTABLE_LABELS_ARGS=(--selectable-labels)
else
    SELECTABLE_LABELS_ARGS=()
fi

# Runs verify-iso-sig's own --keep-key mode directly (in-process). Emits
# a synthetic "[VERIFY-ISO-SIG:] KEPT_IN_TRUSTED_GPG" tag into
# $KEEPKEY_OUTPUT on success so the existing `gui_status_has` check below
# keeps working. Resets KEEP_KEY back to 0 afterwards so it can't leak
# into a later loop iteration's verify_iso() call.
run_keep_key() {
    local fpr=$1 from_ring=$2
    KEEP_KEY=1
    FPR=$fpr
    FROM_RING_OVERRIDE=$from_ring
    if KEEPKEY_OUTPUT=$(verify_iso 2>&1 >/dev/null); then
        KEEPKEY_OUTPUT="$KEEPKEY_OUTPUT
[VERIFY-ISO-SIG:] KEPT_IN_TRUSTED_GPG"
        KEEP_KEY=0
        return 0
    else
        KEEP_KEY=0
        return 1
    fi
}

# Shared by both modes - the only difference (picker: no --width;
# manager: --width=480) is whether $YAD_ERROR_WIDTH is empty.
yad_error() {
    local ARGS=(
        --error
        "${SELECTABLE_LABELS_ARGS[@]}"
        --center
        --title="$TITLE"
        --class="$WM_CLASS"
        --window-icon="$ICON_FILE"
    )
    [ -n "$YAD_ERROR_WIDTH" ] && ARGS+=(--width="$YAD_ERROR_WIDTH")
    ARGS+=(--text="$1" --button="$(safe_eval_gettext "OK"):0")
    yad "${ARGS[@]}"
}

# Manager-mode-only, but harmless to define unconditionally.
yad_info() {
    local ARGS=(
        --info
        "${SELECTABLE_LABELS_ARGS[@]}"
        --center
        --title="$TITLE"
        --class="$WM_CLASS"
        --window-icon="$ICON_FILE"
        # A short one-line message otherwise renders in yad's much wider
        # default window, looking sparse - a longer message just wraps.
        --width=480
        --text="$1"
        --button="$(safe_eval_gettext "OK"):0"
    )
    yad "${ARGS[@]}"
}

# The picker's own "About" button. yad's own native --about dialog
# (--pname/--pversion/--copyright/--comments/--license/--authors)
# auto-localizes its own chrome using yad's own translations - only
# available since new-scheme yad 9, reusing the YAD_MAJOR>=14 cliff
# already used for --selectable-labels. Older yad falls back to a
# hand-built dialog with the same content via the general --image option -
# force that fallback on any yad version by setting
# VERIFY_ISO_SIG_ABOUT_FALLBACK (any non-empty value) for debugging.
run_about_mode() {
    local about_comments about_comments_wrapped about_text
    # Distinct from $DISPLAY_NAME (the technical script name, still used
    # for --class etc.) - a punchy product name, shared with
    # verify-iso-sig.desktop.in's own Name= (one translation for both).
    ABOUT_TITLE=$(safe_eval_gettext "ISO Signature Verifier")
    about_comments=$(safe_eval_gettext "Check the GPG signature of a downloaded ISO (built-in support for MX Linux/antiX signing keys)")
    # Separate msgid (not a runtime split of $about_comments) so the line
    # break is a translator-visible choice in the .po itself. Kept apart
    # from $about_comments (shared with .desktop's Comment=, which must
    # stay single-line) rather than reusing one string for both.
    if [ "$YAD_MAJOR" -ge 14 ] && [ -z "${VERIFY_ISO_SIG_ABOUT_FALLBACK:-}" ]; then
        # --window-icon needs the installed icon-theme NAME here, not a
        # file path, or --about keeps yad's default icon; --image (the
        # big in-dialog logo) takes the file path fine either way.
        yad --about \
            --class="$WM_CLASS" \
            --window-icon="verify-iso-sig" \
            --image="$ICON_FILE" \
            --pname="$ABOUT_TITLE" \
            --pversion="$(safe_eval_gettext "Version \${VERSION}" VERSION)" \
            --copyright=$'\u00a9'" MX Linux" \
            --comments="$about_comments" \
            --license=GPL3 \
            --authors="fehlix" \
            --website="https://mxlinux.org/" \
            --website-label="https://mxlinux.org" \
            >/dev/null 2>&1 || true
    else
        # Fixed 500/50 (not a per-language formula) - a length-driven
        # formula produced an unreasonably wide window on some languages.
        # --text-width alone doesn't reliably force a wrap on every yad
        # version, so $about_comments_wrapped's own real "\n" gives a
        # version-independent break instead. --fixed locks the
        # negotiated width/height instead of letting yad grow past them.
        about_comments_wrapped=$(safe_eval_gettext "Check the GPG signature of a downloaded ISO\n(built-in support for MX Linux/antiX signing keys)")
        # <a href> renders as a real clickable link even in a plain
        # --text dialog - no --html needed. Line order matches the
        # native --about dialog's own: comments, website, copyright, license.
        about_text="<span size='x-large'><b>$(pango_escape "$ABOUT_TITLE")</b></span>\n<span size='small'>$(pango_escape "$(safe_eval_gettext "Version \${VERSION}" VERSION)")</span>\n\n$(pango_escape "$about_comments_wrapped")\n\n<a href='https://mxlinux.org/'>https://mxlinux.org</a>\n\n$(printf '%b' '\u00a9') MX Linux\n\n<a href='https://www.gnu.org/licenses/gpl-3.0.html'>$(safe_eval_gettext "License: GNU GPL v3 or later")</a>\n"
        yad --center \
            --fixed \
            --title="$(safe_eval_gettext "About \${ABOUT_TITLE}" ABOUT_TITLE)" \
            --class="$WM_CLASS" \
            --window-icon="$ICON_FILE" \
            --image="$ICON_FILE" \
            --text-align=center \
            --width=500 \
            --text-width=50 \
            --text="$about_text" \
            --button="$(safe_eval_gettext "Close"):0"
    fi
}

# yad list columns and --text both parse Pango markup, so a raw UID like
# "Name <user@host>" breaks rendering unless escaped.
pango_escape() {
    local s=$1
    # NOTE: an unescaped & in the replacement of ${var//pat/repl} means
    # "the matched text" (like sed) - \& is needed for a literal &.
    s=${s//&/\&amp;}
    s=${s//</\&lt;}
    s=${s//>/\&gt;}
    printf '%s' "$s"
}

# Shared helpers for the signature naming convention. sig_for_iso() only
# knows .sig/.asc/.gpg - it guesses an ISO's own direct signature,
# matching verify-iso-sig's 3-extension direct-sig detection (excludes
# .sign on purpose). iso_for_sig() has one more extension, .sign, since
# the file handed in could be a checksum-listing's own signature
# (Debian's convention) instead of a direct-ISO one.
sig_for_iso() {
    local iso=$1 ext
    for ext in sig asc gpg; do
        [ -r "${iso}.${ext}" ] && { printf '%s' "${iso}.${ext}"; return 0; }
    done
    printf '%s' "${iso}.sig"
}

iso_for_sig() {
    local f=$1
    case "$f" in
        *.sig) printf '%s' "${f%.sig}" ;;
        *.asc) printf '%s' "${f%.asc}" ;;
        *.gpg) printf '%s' "${f%.gpg}" ;;
        *.sign) printf '%s' "${f%.sign}" ;;
        *)     printf '%s' "$f" ;;
    esac
}

# Formats a 40-hex-char fingerprint into gpg's own traditional display
# grouping - 4-char blocks space-separated, extra space after the 5th
# block, matching `gpg --fingerprint`'s own output.
format_fingerprint() {
    local f=$1 out="" i grp
    for ((i = 0; i < 40; i += 4)); do
        grp=${f:i:4}
        out+="$grp"
        if [ "$i" -eq 16 ]; then
            out+="  "
        elif [ "$i" -ne 36 ]; then
            out+=" "
        fi
    done
    printf '%s' "$out"
}

# The picker's own "Manage Trusted Keys" button calls this in-process -
# both modes share the same already-sourced, already-LIB_MODE=1 process.
# TITLE/BTN_FORGET_SELECTED/BTN_IMPORT_SELECTED/YAD_ERROR_WIDTH are
# `local` so a picker-triggered call shadows the picker's own globals
# only for this call's duration. Every failure `return`s rather than
# `exit`s, so a picker-triggered call can't kill the whole GUI process -
# the bottom dispatch converts a direct-entry call's return into a real exit.
run_manage_mode() {
    local TITLE BTN_FORGET_SELECTED BTN_IMPORT_SELECTED YAD_ERROR_WIDTH
    TITLE=$(safe_eval_gettext "Manage Trusted Keys")
    BTN_FORGET_SELECTED=$(safe_eval_gettext "Forget Selected")
    BTN_IMPORT_SELECTED=$(safe_eval_gettext "Import Selected")
    YAD_ERROR_WIDTH="480"

    # Loops so removing several keys is one continuous flow.
    while :; do
        SELECTED=""
        LISTING=$(list_trusted) || { yad_error "$(safe_eval_gettext "Could not read the list of trusted keys:")\n$LISTING"; return 1; }

        if [ -z "$LISTING" ]; then
            EMPTY_LIST_LINE1=$(safe_eval_gettext "No signing keys are currently saved in trustedkeys.gpg.")
            # TRANSLATORS: "Trust"/"Keep" here are informal short references to
            # the main GUI's "Trust this key"/"Keep this key" button labels, not
            # verbatim quotes - no need to match those translated labels exactly.
            # The straight quotes around each word are just English's own
            # convention - please use your language's own quotation marks
            # instead (e.g. « », „", etc.) if that reads more naturally.
            EMPTY_LIST_LINE2=$(safe_eval_gettext "A key ends up there after you choose to \"Trust\"/\"Keep\" it while verifying an ISO.")
            EMPTY_ARGS=(
                --info
                --center
                --title="$TITLE"
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
                --width=480
                --text="$EMPTY_LIST_LINE1\n\n$EMPTY_LIST_LINE2"
                # Same button code (3) as "Import from File" below, so it
                # falls into that same handling further down - Forget/
                # Export make no sense with nothing saved yet, so neither
                # is offered here.
                --button="$(safe_eval_gettext "Import from File"):3"
                --button="$(safe_eval_gettext "Close"):1"
            )
            set +e
            yad "${EMPTY_ARGS[@]}"
            LIST_RC=$?
            set -e
        else
            # Scale the window height to the key count - 320 fits ~5-6 rows.
            KEY_COUNT=$(printf '%s\n' "$LISTING" | grep -c .)
            if [ "$KEY_COUNT" -ge 6 ]; then
                LIST_HEIGHT=480
            else
                LIST_HEIGHT=320
            fi

            LIST_ARGS=(
                --list
                --checklist
                --center
                --title="$TITLE"
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
                # 940, not 860 - a long real-world UID (e.g. a full name plus
                # email) could otherwise force a horizontal scrollbar across
                # the Key ID/User ID/Status/Expiration Date columns.
                --width=940
                --height="$LIST_HEIGHT"
                --column="$(safe_eval_gettext "Remove")"
                --column="$(safe_eval_gettext "Fingerprint")"
                --column="$(safe_eval_gettext "Key ID")"
                --column="$(safe_eval_gettext "User ID")"
                --column="$(safe_eval_gettext "Status")"
                --column="$(safe_eval_gettext "Expiration Date")"
                # GTK's own type-ahead search defaults to column 1 (the Remove
                # checkbox, useless for typed text) - point it at User ID instead.
                --search-column=4
                # Fingerprint stays in the list model for --untrust-key below,
                # but isn't shown - the Key ID column is friendlier at a glance.
                --hide-column=2
                --print-column=2
                --separator=$'\n'
                # TRANSLATORS: ${BTN_FORGET_SELECTED} is the translated button label - keep the placeholder as-is.
                --text="<span size='large'><b>$TITLE</b></span>\n$(safe_eval_gettext "Keys saved in ~/.gnupg/trustedkeys.gpg - check any you want to forget, then click \"\${BTN_FORGET_SELECTED}\"." BTN_FORGET_SELECTED)"
                --button="$BTN_FORGET_SELECTED:0"
                # Even/odd per yad's own EXIT STATUS rule: Export Selected needs
                # the checked rows (even, like Forget Selected); Import from
                # File doesn't (odd).
                --button="$(safe_eval_gettext "Export Selected"):2"
                --button="$(safe_eval_gettext "Import from File"):3"
                # Close last/rightmost - same convention as the result
                # dialog's own button order (yad's last button is the
                # Enter-key default, "Close" is the safe one to land on).
                --button="$(safe_eval_gettext "Close"):1"
            )

            # FPR|VALIDITY|UID|KNOWN|EXPIRE, one line per key. KNOWN flags a
            # fingerprint also on the hardcoded MX/antiX allow-list (removing it
            # here doesn't stop recognition, since that never depends on
            # trustedkeys.gpg). EXPIRE is a raw Unix epoch, formatted here since
            # date formatting is a display concern. Key ID shown bare (no "0x").
            ROWS=()
            while IFS='|' read -r fpr vstr uid known expire; do
                [ -n "$fpr" ] || continue
                case "$vstr" in
                    # TRANSLATORS: shown as-is in the list's "Status" column (a
                    # gpg key's trust validity), one word each.
                    valid)   status=$(safe_eval_gettext "valid") ;;
                    # TRANSLATORS: same "Status" column as "valid" above.
                    expired) status=$(safe_eval_gettext "expired") ;;
                    # TRANSLATORS: same "Status" column as "valid" above.
                    revoked) status=$(safe_eval_gettext "revoked") ;;
                    *)       status="$vstr" ;;
                esac
                if [ -n "$expire" ]; then
                    expire_display=$(date -d "@$expire" '+%Y-%m-%d' 2>/dev/null || printf '%s' "$expire")
                else
                    # TRANSLATORS: shown in the "Expires" column when the key has
                    # no expiration date set.
                    expire_display=$(safe_eval_gettext "never")
                fi
                ROWS+=(FALSE "$fpr" "${fpr: -16}" "$(pango_escape "$uid")" "$status" "$expire_display")
            done <<< "$LISTING"

            set +e
            SELECTED=$(yad "${LIST_ARGS[@]}" "${ROWS[@]}")
            LIST_RC=$?
        fi
        set -e

        # 0 = Forget Selected, handled straight after this case. 2/3 are
        # handled inline here, then loop back. Anything else (1 = Close,
        # Escape) returns to the caller.
        case "$LIST_RC" in
            0) : ;;
            2)
                # Export Selected.
                [ -n "$SELECTED" ] || { yad_error "$(safe_eval_gettext "No key was checked - nothing to export.")"; continue; }
                # yad's checklist print-column output has a blank line between
                # each checked row's value - guard against that per-line.
                SELECTED_FPRS=()
                while IFS= read -r fpr; do
                    [ -n "$fpr" ] || continue
                    SELECTED_FPRS+=("$fpr")
                done <<< "$SELECTED"
                EXPORT_COUNT=${#SELECTED_FPRS[@]}
                SAVE_ARGS=(
                    --file --save --confirm-overwrite
                    "${SELECTABLE_LABELS_ARGS[@]}"
                    --center
                    --title="$(safe_eval_gettext "Export Trusted Keys")"
                    --class="$WM_CLASS"
                    --window-icon="$ICON_FILE"
                    --width=860
                    --height=480
                    # Date+time so two same-day exports don't suggest the same filename.
                    --filename="trustedkeys-$(date +%Y-%m-%d_%H%M%S).asc"
                )
                set +e
                EXPORT_PATH=$(yad "${SAVE_ARGS[@]}" 2>/dev/null)
                SAVE_RC=$?
                set -e
                [ "$SAVE_RC" -eq 0 ] && [ -n "$EXPORT_PATH" ] || continue
                EXPORT_TRUSTED_KEYS="$EXPORT_PATH"
                EXPORT_TRUSTED_KEYS_FPRS=("${SELECTED_FPRS[@]}")
                if OUT=$(export_trusted_keys 2>&1 >/dev/null); then
                    EXPORT_PATH_SAFE=$(pango_escape "$EXPORT_PATH")
                    yad_info "$(safe_eval_gettext "Exported \${EXPORT_COUNT} key(s) to \${EXPORT_PATH_SAFE}." EXPORT_COUNT EXPORT_PATH_SAFE)"
                else
                    yad_error "$(safe_eval_gettext "Could not export keys:")\n$OUT"
                fi
                continue
                ;;
            3)
                # Import from File.
                OPEN_ARGS=(
                    --file
                    "${SELECTABLE_LABELS_ARGS[@]}"
                    --center
                    --title="$(safe_eval_gettext "Import Trusted Keys")"
                    --class="$WM_CLASS"
                    --window-icon="$ICON_FILE"
                    --width=860
                    --height=480
                    --file-filter="$(safe_eval_gettext "OpenPGP key files") | *.asc *.gpg *.pgp *.key"
                    --file-filter="$(safe_eval_gettext "All files") | *"
                )
                set +e
                IMPORT_PATH=$(yad "${OPEN_ARGS[@]}" 2>/dev/null)
                OPEN_RC=$?
                set -e
                [ "$OPEN_RC" -eq 0 ] && [ -n "$IMPORT_PATH" ] || continue

                INSPECT_KEY_FILE="$IMPORT_PATH"
                if ! INSPECT_OUT=$(inspect_key_file 2>&1); then
                    yad_error "$(safe_eval_gettext "Could not read that key file:")\n$INSPECT_OUT"
                    continue
                fi
                [ -n "$INSPECT_OUT" ] || { yad_error "$(safe_eval_gettext "No keys were found in that file.")"; continue; }

                # Cross-reference the current listing so the preview can mark
                # which keys are already trusted vs genuinely new.
                ALREADY_TRUSTED_FPRS=$(printf '%s\n' "$LISTING" | cut -d'|' -f1)

                # Same key-count-based height logic as the main list above.
                IMPORT_KEY_COUNT=$(printf '%s\n' "$INSPECT_OUT" | grep -c .)
                if [ "$IMPORT_KEY_COUNT" -ge 6 ]; then
                    IMPORT_LIST_HEIGHT=480
                else
                    IMPORT_LIST_HEIGHT=320
                fi

                IMPORT_LIST_ARGS=(
                    --list --checklist
                    "${SELECTABLE_LABELS_ARGS[@]}"
                    --center
                    --title="$(safe_eval_gettext "Import Trusted Keys")"
                    --class="$WM_CLASS"
                    --window-icon="$ICON_FILE"
                    # See the identical comment on the main list's own --width above.
                    --width=940
                    --height="$IMPORT_LIST_HEIGHT"
                    --column="$(safe_eval_gettext "Import")"
                    --column="$(safe_eval_gettext "Fingerprint")"
                    --column="$(safe_eval_gettext "Key ID")"
                    --column="$(safe_eval_gettext "User ID")"
                    --column="$(safe_eval_gettext "Status")"
                    --column="$(safe_eval_gettext "Expiration Date")"
                    --search-column=4
                    --hide-column=2
                    --print-column=2
                    --separator=$'\n'
                    # TRANSLATORS: ${BTN_IMPORT_SELECTED} is the translated button label - keep the placeholder as-is.
                    --text="<span size='large'><b>$(safe_eval_gettext "Import Trusted Keys")</b></span>\n$(safe_eval_gettext "Keys found in this file - check any you want to import, then click \"\${BTN_IMPORT_SELECTED}\"." BTN_IMPORT_SELECTED)"
                    # Independent 0/1 response-code space, local to this inner dialog only.
                    --button="$BTN_IMPORT_SELECTED:0"
                    --button="$(safe_eval_gettext "Cancel"):1"
                )
                IMPORT_ROWS=()
                while IFS='|' read -r fpr vstr uid known expire; do
                    [ -n "$fpr" ] || continue
                    case "$vstr" in
                        valid)   status=$(safe_eval_gettext "valid") ;;
                        expired) status=$(safe_eval_gettext "expired") ;;
                        revoked) status=$(safe_eval_gettext "revoked") ;;
                        *)       status="$vstr" ;;
                    esac
                    if printf '%s\n' "$ALREADY_TRUSTED_FPRS" | grep -qxF "$fpr"; then
                        # TRANSLATORS: appended to the Status cell in the
                        # import preview when this key is already present in
                        # trustedkeys.gpg - lets the user tell "already
                        # trusted" apart from "genuinely new" at a glance.
                        status="$status $(safe_eval_gettext "(already trusted)")"
                    fi
                    if [ -n "$expire" ]; then
                        expire_display=$(date -d "@$expire" '+%Y-%m-%d' 2>/dev/null || printf '%s' "$expire")
                    else
                        expire_display=$(safe_eval_gettext "never")
                    fi
                    # Pre-checked TRUE by default (opposite of the main
                    # list's FALSE default above) - importing is presumably
                    # why the user picked this file in the first place.
                    IMPORT_ROWS+=(TRUE "$fpr" "${fpr: -16}" "$(pango_escape "$uid")" "$status" "$expire_display")
                done <<< "$INSPECT_OUT"

                set +e
                IMPORT_CHECKED=$(yad "${IMPORT_LIST_ARGS[@]}" "${IMPORT_ROWS[@]}")
                IMPORT_LIST_RC=$?
                set -e
                [ "$IMPORT_LIST_RC" -eq 0 ] || continue
                [ -n "$IMPORT_CHECKED" ] || { yad_error "$(safe_eval_gettext "No key was checked - nothing to import.")"; continue; }

                # Same blank-line-between-entries quirk as Export Selected
                # above - filter them out rather than counting/passing them.
                IMPORT_CHECKED_FPRS=()
                while IFS= read -r fpr; do
                    [ -n "$fpr" ] || continue
                    IMPORT_CHECKED_FPRS+=("$fpr")
                done <<< "$IMPORT_CHECKED"
                IMPORT_TRUSTED_KEYS="$IMPORT_PATH"
                IMPORT_TRUSTED_KEYS_FPRS=("${IMPORT_CHECKED_FPRS[@]}")
                # Called directly, not via "$(import_trusted_keys ...)" -
                # command substitution forks a subshell, which would lose the
                # real PROCESSED/IMPORTED/CHANGED/UNCHANGED_COUNT globals it
                # sets. stderr goes to a temp file instead, since a bare
                # redirection doesn't fork a subshell.
                IMPORT_ERR_FILE=$(mktemp "$SESSION_TMPDIR/import-err.XXXXXXXXXX")
                if import_trusted_keys 2>"$IMPORT_ERR_FILE" >/dev/null; then
                    # Copied to IMPORT_*-prefixed local names so the
                    # translated message keeps its ${IMPORT_...} placeholder names.
                    IMPORT_PROCESSED=$PROCESSED
                    IMPORT_NEW=$IMPORTED
                    IMPORT_UPDATED=$CHANGED
                    IMPORT_UNCHANGED=$UNCHANGED_COUNT
                    rm -f "$IMPORT_ERR_FILE"
                    yad_info "$(safe_eval_gettext "Processed \${IMPORT_PROCESSED} key(s): \${IMPORT_NEW} new, \${IMPORT_UPDATED} updated, \${IMPORT_UNCHANGED} unchanged." IMPORT_PROCESSED IMPORT_NEW IMPORT_UPDATED IMPORT_UNCHANGED)"
                else
                    OUT=$(cat "$IMPORT_ERR_FILE")
                    rm -f "$IMPORT_ERR_FILE"
                    yad_error "$(safe_eval_gettext "Could not import keys:")\n$OUT"
                fi
                continue
                ;;
            *) return 0 ;;
        esac

        [ -n "$SELECTED" ] || { yad_error "$(safe_eval_gettext "No key was checked - nothing to forget.")"; continue; }

        # Same defensive parsing as the export/import paths - reused for
        # both the confirmation listing and the removal loop below.
        SELECTED_FPRS=()
        while IFS= read -r fpr; do
            [ -n "$fpr" ] || continue
            SELECTED_FPRS+=("$fpr")
        done <<< "$SELECTED"
        COUNT=${#SELECTED_FPRS[@]}

        # Spells out exactly which keys are about to be forgotten (Key ID -
        # User ID) rather than just a bare count.
        FORGET_LIST=""
        for fpr in "${SELECTED_FPRS[@]}"; do
            uid=$(printf '%s\n' "$LISTING" | awk -F'|' -v f="$fpr" '$1 == f { print $3; exit }')
            FORGET_LIST="$FORGET_LIST${fpr: -16}  $(pango_escape "$uid")
"
        done

        # A fixed-size --text-info (scrollable), not the --question dialog's
        # own --text label, whose window otherwise grows/wraps unpredictably
        # across yad versions for a variable number of key lines - a longer
        # list just scrolls instead. Height scales with COUNT so the common
        # case shows every row without scrolling, capped so a large
        # selection scrolls rather than growing indefinitely. "+ 2" pads for
        # a heading that wraps to 4 lines in some locales (e.g. French)
        # instead of 3.
        CONFIRM_HEIGHT=$(( 190 + (COUNT + 1) * 18 ))
        [ "$CONFIRM_HEIGHT" -gt 500 ] && CONFIRM_HEIGHT=500

        CONFIRM_ARGS=(
            --text-info
            --formatted
            "${SELECTABLE_LABELS_ARGS[@]}"
            --center
            --title="$TITLE"
            --class="$WM_CLASS"
            --window-icon="$ICON_FILE"
            --width=720
            --height="$CONFIRM_HEIGHT"
            # TRANSLATORS: ${COUNT}/${CMD_LIST_KNOWN_KEYS} are literal - keep placeholders as-is.
            --text="<b>$(safe_eval_gettext "Forget \${COUNT} selected key(s) from trustedkeys.gpg?" COUNT)</b>\n\n$(safe_eval_gettext "If any of them is also one of this tool's built-in recognized keys (run '\${CMD_LIST_KNOWN_KEYS}' to see the full list), it will still be recognized automatically next time - this only removes the local cache entry, not that recognition." CMD_LIST_KNOWN_KEYS)"
            --button="$(safe_eval_gettext "Forget"):0"
            --button="$(safe_eval_gettext "Cancel"):1"
        )
        if ! printf '%s' "$FORGET_LIST" | yad "${CONFIRM_ARGS[@]}"; then
            continue
        fi

        ERRORS=""
        for fpr in "${SELECTED_FPRS[@]}"; do
            UNTRUST_KEY="$fpr"
            if ! OUT=$(untrust_key 2>&1); then
                ERRORS="$ERRORS
$OUT"
            fi
        done

        [ -n "$ERRORS" ] && yad_error "$(safe_eval_gettext "Some keys could not be removed:")$ERRORS"
    done
}

# --debug is a plain command-line flag for this script (meant for
# launching from a terminal, e.g. `./verify-iso-sig-gui.sh --debug` or
# `./verify-iso-sig-gui.sh --debug some.iso`) rather than a form checkbox -
# if you're already at a terminal to see debug output, typing the flag is
# no extra hassle, and it avoids a checkbox whose GUI wiring is one more
# thing that can silently break. Picker-mode-only (manage mode's own
# early -h/--help/-V handling above never reaches here at all).
if [ "$GUI_MODE" = picker ]; then
    DEBUG=0
    DND_MODE=0
    FILE_ARGS=()
    for arg in "$@"; do
        case "$arg" in
            --debug) DEBUG=1 ;;
            # Opt-in, not default - needs yad's --paned/--plug (X11-only,
            # fixed-size geometry, see run_picker_mode() below). Reached
            # via the app menu's "Drag & Drop" action.
            --drag-and-drop) DND_MODE=1 ;;
            # Without this, --help falls through to the "*" branch below
            # and becomes the picker's prefill value instead - the GUI
            # opens with a blank field and blocks the terminal until
            # closed, reading like a hang to a caller expecting text
            # output. Kept in plain English, unwrapped by gettext,
            # matching the CLI's own --help convention.
            -h|--help)
                cat <<EOF
Usage: $DISPLAY_NAME [--debug] [--drag-and-drop] [iso-file] [sig-file]

Picks an ISO or signature file and checks its GPG signature, with a
graphical trust/keep flow for unrecognized keys. Normally opens
automatically (no arguments needed) whenever a desktop is available;
see 'verify-iso-sig --man' for the full manual.

  --debug           print every gpg/gpgv command before running it
  --drag-and-drop   also show the drag-and-drop pane (X11 only)
  -h, --help        this help
  -V, --version     show version and exit

Given one file, it's prefilled into the picker so the naming convention
(<iso>.sig/.asc/.gpg) can find its counterpart. Given two (an ISO and
its signature file, same order as the CLI's own <iso-file> [sig-file]),
both are used exactly as given - no naming convention, no picker form -
after a plain confirmation dialog naming both files.
EOF
                exit 0
                ;;
            -V|--version) printf '%s %s\n' "$DISPLAY_NAME" "$VERSION"; exit 0 ;;
            *) FILE_ARGS+=("$arg") ;;
        esac
    done

    # "Run me only once": any launch - a bare one (the desktop icon) or
    # one with a file argument ("Open With" on a specific ISO) - doesn't
    # open a second window while another instance is already running.
    # A file argument is NOT special-cased to always open its own
    # window: there's no IPC here to hand a freshly-picked file to an
    # already-running instance, so treating it differently would mean
    # either silently dropping the file with zero feedback, or letting a
    # file-argument launch skip the lock entirely - which would mean a
    # *later* bare launch finds the lock still free and opens yet
    # another window, since the file-launch never took it either.
    # Blocking + notifying is the honest answer given what this tool can
    # actually do without adding real inter-process communication.
    #
    # Detected via a flock'd lock file, not wmctrl/xdotool - portable,
    # and works identically on Wayland (unlike scanning wmctrl -lx,
    # which can't see any windows there at all). Also self-cleaning: the
    # kernel drops the lock the instant this process exits, crash
    # included, so there's no stale-lock cleanup to worry about (unlike
    # a plain PID file, which would need a "is that PID still actually
    # alive" check). Raising/focusing the existing window on top of
    # that is still an X11+wmctrl-only bonus - Wayland has no portable
    # way for one app to raise another's window at all, so there (or
    # without wmctrl) this just notifies (if possible) and exits
    # quietly instead, without spawning a second window.
    LOCK_FILE="${XDG_RUNTIME_DIR:-/tmp}/verify-iso-sig-$UID.lock"
    exec {LOCK_FD}>"$LOCK_FILE"
    if ! flock -n "$LOCK_FD"; then
        RAISED=0
        if [ -z "${WAYLAND_DISPLAY:-}" ] && command -v wmctrl >/dev/null 2>&1; then
            EXISTING_WID=$(wmctrl -lx 2>/dev/null | awk -v cls="$WM_CLASS" '$3 ~ cls {print $1; exit}') || true
            if [ -n "$EXISTING_WID" ]; then
                wmctrl -ia "$EXISTING_WID" >/dev/null 2>&1
                RAISED=1
            fi
        fi
        if [ "${#FILE_ARGS[@]}" -gt 0 ]; then
            # A file was picked but can't be handed to the running
            # instance - always say so, even if the window was raised,
            # since raising alone doesn't explain why the file itself
            # didn't open.
            NOTIFY_MSG=$(safe_eval_gettext "Already running - close it first to check this file.")
        elif [ "$RAISED" -eq 0 ]; then
            # No way to raise the existing window either - a
            # notification is the only feedback left that anything
            # happened at all; without it, a second click on the
            # desktop icon looks like nothing happened.
            NOTIFY_MSG=$(safe_eval_gettext "Already running.")
        else
            NOTIFY_MSG=""
        fi
        if [ -n "$NOTIFY_MSG" ] && command -v notify-send >/dev/null 2>&1; then
            # --app-name: without it, notify-send defaults the app-name
            # field to its own program name ("notify-send") - some
            # notification popups (e.g. Plasma) show that field
            # prominently, so without this it looks like the alert came
            # from a tool called "notify-send" instead of this one.
            notify-send --app-name="$TITLE" --icon="$ICON_FILE" "$TITLE" "$NOTIFY_MSG" >/dev/null 2>&1 || true
        fi
        exit 0
    fi

    # The picker's own form takes a single file (the .iso, or its
    # .sig/.asc/.gpg directly) - prefill is a straight passthrough of
    # whatever one path is already known; classification into ISO/SIG
    # happens once the form is submitted (see pick_files()).
    #
    # Two explicit file arguments means the caller already knows which
    # file is which (like the CLI's own <iso-file> [sig-file]) -
    # run_picker_mode() uses EXPLICIT_ISO/EXPLICIT_SIG directly, skipping
    # pick_files() and the naming convention, but still shows a
    # confirmation dialog before verifying. Three explicit arguments
    # (ISO plus checksum plus signature) works the same way. Four or
    # more has no sensible interpretation - a plain usage error.
    EXPLICIT_ISO=""
    EXPLICIT_SIG=""
    EXPLICIT_CHECKSUM_FILE=""
    EXPLICIT_CHECKSUM_ALGO=""
    PREFILL_FILE=""
    case "${#FILE_ARGS[@]}" in
        0) : ;;
        1) PREFILL_FILE=${FILE_ARGS[0]} ;;
        # classify_checksum_listing_pair()/classify_checksum_signature_
        # pair()/classify_plain_checksum_signature_pair()/classify_plain_
        # checksum_pair()/classify_iso_sig_pair() (verify-iso-sig's own
        # functions) classify the pair order-independent, same as main()'s
        # own <iso-file> [sig-file] positional args. EXPLICIT_NO_DIRECT_SIG/
        # EXPLICIT_CHECKSUM_SIG/EXPLICIT_CHECKSUM_ALGO carry each
        # classifier's own NO_DIRECT_SIG/CHECKSUM_SIG_PREVIEW/CHECKSUM_
        # ALGO_OVERRIDE through to run_picker_mode() and its confirmation dialog.
        2)
            EXPLICIT_CHECKSUM_FILE=""
            EXPLICIT_CHECKSUM_SIG=""
            EXPLICIT_CHECKSUM_ALGO=""
            EXPLICIT_NO_DIRECT_SIG=0
            EXPLICIT_NEITHER_SIG=0
            EXPLICIT_SIG_NAMED_BUT_NOT_SIG=""
            EXPLICIT_ISO_NOT_PLAUSIBLE=0
            # && iso_result_plausible: see verify-iso-sig's own dispatch.
            if classify_checksum_listing_pair "${FILE_ARGS[0]}" "${FILE_ARGS[1]}" && iso_result_plausible; then
                EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
            elif classify_checksum_signature_pair "${FILE_ARGS[0]}" "${FILE_ARGS[1]}" && iso_result_plausible; then
                EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
            elif classify_plain_checksum_signature_pair "${FILE_ARGS[0]}" "${FILE_ARGS[1]}" && iso_result_plausible; then
                EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
            elif classify_plain_checksum_pair "${FILE_ARGS[0]}" "${FILE_ARGS[1]}" && iso_result_plausible; then
                EXPLICIT_NO_DIRECT_SIG=$NO_DIRECT_SIG
                EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
            else
                classify_iso_sig_pair "${FILE_ARGS[0]}" "${FILE_ARGS[1]}"
                EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                EXPLICIT_NEITHER_SIG=$NEITHER_LOOKS_LIKE_SIGNATURE
                EXPLICIT_SIG_NAMED_BUT_NOT_SIG=$SIG_NAMED_BUT_NOT_SIGNATURE
                if [ "$NEITHER_LOOKS_LIKE_SIGNATURE" -eq 0 ] && ! iso_result_plausible; then
                    EXPLICIT_ISO_NOT_PLAUSIBLE=1
                fi
            fi
            EXPLICIT_ISO=$ISO
            EXPLICIT_SIG=$SIG
            ;;
        # Three explicit arguments - an ISO plus its signature/checksum
        # files, for files not all in the same folder. Falls back to
        # treating any two of them as an ISO+SIG pair (ignoring the
        # third) when the triple itself doesn't match a known shape.
        3)
            EXPLICIT_CHECKSUM_FILE=""
            EXPLICIT_CHECKSUM_SIG=""
            EXPLICIT_CHECKSUM_ALGO=""
            EXPLICIT_NO_DIRECT_SIG=0
            EXPLICIT_NEITHER_SIG=0
            EXPLICIT_SIG_NAMED_BUT_NOT_SIG=""
            EXPLICIT_ISO_NOT_PLAUSIBLE=0
            if ! classify_iso_checksum_signature_triple "${FILE_ARGS[0]}" "${FILE_ARGS[1]}" "${FILE_ARGS[2]}" \
                && ! classify_iso_triple_ignoring_one "${FILE_ARGS[0]}" "${FILE_ARGS[1]}" "${FILE_ARGS[2]}"; then
                # A GUI popup, not just stderr - no visible terminal when
                # launched via a file manager's "Open With". Names the
                # three files given, since the user may no longer see
                # what was selected.
                triple_base1=$(basename "${FILE_ARGS[0]}")
                triple_base2=$(basename "${FILE_ARGS[1]}")
                triple_base3=$(basename "${FILE_ARGS[2]}")
                if [ "$TRIPLE_AMBIGUOUS" -eq 1 ]; then
                    if [ -n "$TRIPLE_AMBIGUOUS_ANCHOR" ]; then
                        anchor_base=$(basename "$TRIPLE_AMBIGUOUS_ANCHOR")
                        cand1_base=$(basename "$TRIPLE_AMBIGUOUS_CANDIDATE1")
                        cand2_base=$(basename "$TRIPLE_AMBIGUOUS_CANDIDATE2")
                        yad_error "$(safe_eval_gettext "Cannot detect what '\${anchor_base}' belongs to:" anchor_base)\n$cand1_base\n$cand2_base"
                    else
                        yad_error "$(safe_eval_gettext "Cannot detect how these three files belong together:")\n$triple_base1\n$triple_base2\n$triple_base3"
                    fi
                else
                    yad_error "$(safe_eval_gettext "Cannot detect an ISO, a checksum listing, and its signature among these three files:")\n$triple_base1\n$triple_base2\n$triple_base3"
                fi
                exit 1
            fi
            EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
            EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
            EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
            EXPLICIT_ISO=$ISO
            EXPLICIT_SIG=$SIG
            ;;
        *)
            # Same reasoning as above - a GUI popup, not just stderr.
            yad_error "$(safe_eval_gettext "Too many files were given - at most three are supported.")"
            exit 1
            ;;
    esac
fi

# No --keep checkbox here - whether to remember a key locally is asked
# after a successful verification, not guessed upfront.
#
# In --drag-and-drop mode, the picker is a Form pane (top) and a
# drag-and-drop pane (bottom) swallowed into one --paned window via
# yad's --plug mechanism. Buttons belong to the outer --paned dialog,
# not the plugs themselves (a plug's own --button is simply not
# rendered). The default (no --drag-and-drop) mode is a single plain
# --form window instead - see the branch below.
#
# yad's --paned runs two fully independent processes with no live
# channel between them, so a single-file drop kills the whole paned
# window and relaunches it with the Form pane pre-filled (PREFILL_FILE
# above) - a background watcher polls the DnD pane's own output file for
# this. A two-file drop is classified order-independent via
# classify_iso_sig_pair() and handed to the caller as
# EXPLICIT_ISO/EXPLICIT_SIG, same as the explicit-two-argument entry point.

# Polls for a window matching $WM_CLASS to appear, then iconifies it;
# returns once it has (or after ~2s if the window never appears). X11
# only (wmctrl can't see/control real windows under Wayland). Called
# synchronously by the Help handler below, BEFORE the browser opens -
# so the browser is confirmed to be the last thing taking focus, with
# nothing after it to steal focus back.
minimize_next_own_window() {
    local wid tries=0
    while [ "$tries" -lt 20 ]; do
        # $3 only (the WM_CLASS field), never $0 (the whole line) - a
        # window's title can coincidentally contain the app's own
        # class name too.
        wid=$(wmctrl -lx 2>/dev/null | awk -v cls="$WM_CLASS" '$3 ~ cls {print $1; exit}') || true
        if [ -n "$wid" ]; then
            break
        fi
        sleep 0.1
        tries=$((tries + 1))
    done
    if [ -n "$wid" ]; then
        wmctrl -ir "$wid" -b add,hidden >/dev/null 2>&1
    fi
}

pick_files() {
    local key res_dnd res_form watcher_pid dnd_pid form_pid form dropped
    local -a DROPPED_LINES DROPPED_PATHS
    local FORM_PLUG_ARGS DND_PLUG_ARGS PANED_ARGS
    local FORM_TITLE_LINE FORM_INTRO_LINE FORM_LINE1 FORM_LINE2 FORM_LINE2_PLAIN
    local PICKER_MIN_WIDTH PICKER_MAX_WIDTH PICKER_WIDTH title_px intro_px line1_px line2_px
    local FORM_OUT yad_pid
    # Set to 1 by the Help handler below - makes the *next* picker
    # re-show start minimized (X11 only) and, once confirmed hidden,
    # open the browser. Consumed (reset to 0) right after each yad call
    # regardless of whether it fired.
    local OPEN_HELP_ON_NEXT_PICKER=0

    # Dynamic width: estimate each header line's rendered pixel width
    # from its character count, and widen the picker just enough to keep
    # all 3 lines on one physical line each - a fixed width wraps
    # differently per language. Pango uses a proportional font, so this
    # char-count heuristic is only an approximation. `${#var}` counts
    # characters, not bytes, correct for UTF-8 locales.
    FORM_TITLE_LINE=$TITLE
    # First-time-user context, above the how-to instructions below - a
    # user opening this from the menu with nothing picked yet may not
    # know what the tool is even for.
    FORM_INTRO_LINE=$(safe_eval_gettext "This tool checks that a downloaded ISO is authentic and undamaged, using its signature file or a signed checksum listing.")
    FORM_LINE1=$(safe_eval_gettext "Pick the .iso file, or its .sig/.asc/.gpg/.sign signature file directly.")
    # TRANSLATORS: keep the <b>/</b> tags exactly as-is (they render as
    # bold text, not literal characters).
    FORM_LINE2=$(safe_eval_gettext "The other one is found automatically - <b>but only if it's in the same folder</b>.")
    # Pango tags aren't rendered text, so they'd skew the length
    # estimate - stripped for measurement only; the actual --text=
    # below still uses the tagged $FORM_LINE2 so <b> still renders bold.
    FORM_LINE2_PLAIN=${FORM_LINE2//<b>/}
    FORM_LINE2_PLAIN=${FORM_LINE2_PLAIN//<\/b>/}

    # Empirically-picked per-character pixel averages for this GTK
    # theme/font (title uses Pango size='large', ~20% bigger than body
    # lines); +80 is a fixed margin for window decoration/padding.
    # 820 gives the FL field below room for a long real filename without
    # ellipsizing - unlike CONFIRM_WIDTH/TRUST_WIDTH, this can't be
    # computed from the actual filename since none has been picked yet;
    # 820 is just a calibrated floor.
    PICKER_MIN_WIDTH=820
    PICKER_MAX_WIDTH=900
    title_px=$(( ${#FORM_TITLE_LINE} * 9 + 80 ))
    intro_px=$(( ${#FORM_INTRO_LINE} * 7 + 80 ))
    line1_px=$(( ${#FORM_LINE1} * 7 + 80 ))
    line2_px=$(( ${#FORM_LINE2_PLAIN} * 7 + 80 ))
    PICKER_WIDTH=$title_px
    [ "$intro_px" -gt "$PICKER_WIDTH" ] && PICKER_WIDTH=$intro_px
    [ "$line1_px" -gt "$PICKER_WIDTH" ] && PICKER_WIDTH=$line1_px
    [ "$line2_px" -gt "$PICKER_WIDTH" ] && PICKER_WIDTH=$line2_px
    [ "$PICKER_WIDTH" -lt "$PICKER_MIN_WIDTH" ] && PICKER_WIDTH=$PICKER_MIN_WIDTH
    [ "$PICKER_WIDTH" -gt "$PICKER_MAX_WIDTH" ] && PICKER_WIDTH=$PICKER_MAX_WIDTH

    while :; do
        # The drag-and-drop pane (yad's --paned/--plug) is opt-in via
        # --drag-and-drop ($DND_MODE), not default - unavailable under
        # Wayland regardless ("this mode not supported on wayland").
        # PANED_ARGS below uses fixed-size geometry, calibrated to this
        # form's exact content - change with care.
        if [ -n "${WAYLAND_DISPLAY:-}" ] || [ "$DND_MODE" -ne 1 ]; then
            dropped=""
            FORM_ONLY_ARGS=(
                --form
                --center
                --title="$TITLE"
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
                # No --fixed here (unlike the paned one below) - just a
                # --height hint for breathing room below the file field,
                # before the button row; the window still auto-grows if a
                # language's wrapped text needs more room than this.
                --text="<span size='large'><b>$FORM_TITLE_LINE</b></span>\n\n$FORM_INTRO_LINE\n\n$FORM_LINE1\n$FORM_LINE2\n"
                --field="$(safe_eval_gettext "ISO or signature file"):FL"
                --width="$PICKER_WIDTH"
                --height=280
                # Help/Manage Trusted Keys/About grouped left (none is
                # part of the actual accept/reject decision for this
                # file) - yad has no GTK-style "secondary" slot to give
                # Help a real gap, but grouping keeps the decision pair
                # (Cancel/Verify) visually separate on the right, with
                # Verify last/rightmost as yad's Enter-key default.
                --button="$(safe_eval_gettext "Help"):6"
                --button="$(safe_eval_gettext "Manage Trusted Keys"):2"
                --button="$(safe_eval_gettext "About"):4"
                --button="$(safe_eval_gettext "Cancel"):1"
                --button="$(safe_eval_gettext "Verify"):0"
            )
            if [ "$OPEN_HELP_ON_NEXT_PICKER" -eq 1 ]; then
                OPEN_HELP_ON_NEXT_PICKER=0
                FORM_OUT=$(mktemp "$SESSION_TMPDIR/form-out.XXXXXXXXXX")
                set +e
                yad "${FORM_ONLY_ARGS[@]}" "$PREFILL_FILE" >"$FORM_OUT" &
                yad_pid=$!
                minimize_next_own_window
                xdg-open "$HELP_URL" >/dev/null 2>&1 &
                disown
                wait "$yad_pid"
                paned_rc=$?
                set -e
                form=$(cat "$FORM_OUT")
                rm -f "$FORM_OUT"
            else
                set +e
                form=$(yad "${FORM_ONLY_ARGS[@]}" "$PREFILL_FILE")
                paned_rc=$?
                set -e
            fi
        else

            # Fresh key each attempt - reusing one too soon after killing the
            # previous attempt's yad processes hits "cannot create shared
            # memory for key N: file already exists" (cleanup isn't instant).
            key=$((SRANDOM % 900000 + 100000))
            res_dnd=$(mktemp "$SESSION_TMPDIR/dnd.XXXXXXXXXX")
            res_form=$(mktemp "$SESSION_TMPDIR/form.XXXXXXXXXX")

            FORM_PLUG_ARGS=(
                --plug="$key"
                --tabnum=1
                --form
                # Single \n after the title (not \n\n): this fixed-size
                # splitter has no room for a blank line without pushing the
                # title off the top, clipped behind the window decoration.
                --text="<span size='large'><b>$FORM_TITLE_LINE</b></span>\n$FORM_INTRO_LINE $FORM_LINE1\n$FORM_LINE2"
                --field="$(safe_eval_gettext "ISO or signature file"):FL"
                # Kept for consistency, but doesn't actually fix the FL
                # field's own file-chooser popup: yad never sets
                # _NET_WM_ICON on that internal dialog, so the WM falls
                # back to WM_CLASS's res_name ("yad", hardcoded - --class
                # only controls res_class) and shows yad's own icon
                # instead - a yad limitation, not fixable from here.
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
            )
            # stdout ONLY into res_form, not "2>&1": this file is read back as
            # the plug's submitted data, so stderr noise (e.g. a GTK-WARNING
            # from an invalid LANG) must never land in it - it would make the
            # `[ -s "$res_form" ]` check below see it as real content and
            # relaunch in a tight infinite loop.
            yad "${FORM_PLUG_ARGS[@]}" "$PREFILL_FILE" > "$res_form" &
            form_pid=$!

            DND_PLUG_ARGS=(
                --plug="$key"
                --tabnum=2
                --dnd
                --text="$(safe_eval_gettext "Or drag a .iso or signature file here")"
            )
            # Same reasoning as $res_form above - stderr must not land here,
            # since the watcher below treats ANY content as "a file was dropped".
            yad "${DND_PLUG_ARGS[@]}" > "$res_dnd" &
            dnd_pid=$!

            (
                while :; do
                    if [ -s "$res_dnd" ]; then
                        sleep 0.3
                        pkill -f "yad --paned --key=$key" 2>/dev/null || true
                        break
                    fi
                    if ! kill -0 "$form_pid" 2>/dev/null; then
                        break
                    fi
                    sleep 0.2
                done
            ) &
            watcher_pid=$!

            PANED_ARGS=(
                --paned
                --key="$key"
                --center
                --title="$TITLE"
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
                --tab="$(safe_eval_gettext "Pick Files")"
                --tab="$(safe_eval_gettext "Drag a File")"
                --width="$PICKER_WIDTH"
                --height=490
                --orient=vert
                --splitter=340
                # GtkPaned's divider is an absolute pixel offset, not a
                # fraction - shrinking then growing the window can leave it
                # clamped near zero, hiding the Form pane. This window's
                # content is fixed and small, so --fixed avoids the glitch.
                # The result dialog stays resizable, unlike this picker.
                # Calibrated to fit the translated (not just English) form
                # text without clipping - change with care.
                --fixed
                # Help/Manage Trusted Keys/About grouped left (none is
                # part of the actual accept/reject decision for this
                # file) - yad has no GTK-style "secondary" slot to give
                # Help a real gap, but grouping keeps the decision pair
                # (Cancel/Verify) visually separate on the right, with
                # Verify last/rightmost as yad's Enter-key default.
                --button="$(safe_eval_gettext "Help"):6"
                --button="$(safe_eval_gettext "Manage Trusted Keys"):2"
                # Even exit code (4, not 3) - "even means print result" per
                # `man yad`'s own EXIT STATUS section, needed so $form still
                # holds whatever was already picked when this fires, matching
                # "Manage Trusted Keys"'s own code 2 for the same reason.
                --button="$(safe_eval_gettext "About"):4"
                --button="$(safe_eval_gettext "Cancel"):1"
                --button="$(safe_eval_gettext "Verify"):0"
            )
            if [ "$OPEN_HELP_ON_NEXT_PICKER" -eq 1 ]; then
                OPEN_HELP_ON_NEXT_PICKER=0
                set +e
                yad "${PANED_ARGS[@]}" &
                yad_pid=$!
                minimize_next_own_window
                xdg-open "$HELP_URL" >/dev/null 2>&1 &
                disown
                wait "$yad_pid"
                paned_rc=$?
                set -e
            else
                set +e
                yad "${PANED_ARGS[@]}"
                paned_rc=$?
                set -e
            fi

            kill "$watcher_pid" "$dnd_pid" "$form_pid" 2>/dev/null || true
            wait "$watcher_pid" 2>/dev/null || true
            wait "$dnd_pid" 2>/dev/null || true
            wait "$form_pid" 2>/dev/null || true

            dropped=$(cat "$res_dnd" 2>/dev/null || true)
            form=$(cat "$res_form" 2>/dev/null || true)
            rm -f "$res_dnd" "$res_form"
        fi

        if [ -n "$dropped" ]; then
            # One "file://..." URI per dropped file, one per line (see
            # this function's own header comment) - strip the prefix
            # from each line independently, not the whole blob at once
            # (a plain "${dropped#file://}" would only strip the very
            # first occurrence, leaving a second dropped file's own
            # "file://" prefix embedded mid-string instead of a second,
            # separate path).
            mapfile -t DROPPED_LINES <<< "$dropped"
            DROPPED_PATHS=()
            for dropped_line in "${DROPPED_LINES[@]}"; do
                [ -n "$dropped_line" ] || continue
                DROPPED_PATHS+=("${dropped_line#file://}")
            done
            case "${#DROPPED_PATHS[@]}" in
                1)
                    PREFILL_FILE=${DROPPED_PATHS[0]}
                    ;;
                2)
                    # See the top-level two-argument entry point's own
                    # comment (above, in this file) for why each
                    # classify_*_pair() is tried in this order.
                    EXPLICIT_CHECKSUM_FILE=""
                    EXPLICIT_CHECKSUM_SIG=""
                    EXPLICIT_CHECKSUM_ALGO=""
                    EXPLICIT_NO_DIRECT_SIG=0
                    EXPLICIT_NEITHER_SIG=0
                    EXPLICIT_SIG_NAMED_BUT_NOT_SIG=""
                    EXPLICIT_ISO_NOT_PLAUSIBLE=0
                    if classify_checksum_listing_pair "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}" && iso_result_plausible; then
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                    elif classify_checksum_signature_pair "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}" && iso_result_plausible; then
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                    elif classify_plain_checksum_signature_pair "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}" && iso_result_plausible; then
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                        EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
                    elif classify_plain_checksum_pair "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}" && iso_result_plausible; then
                        EXPLICIT_NO_DIRECT_SIG=$NO_DIRECT_SIG
                        # See the top-level two-argument entry point's own
                        # comment (above, in this file) for why this is needed.
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                        EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
                    else
                        classify_iso_sig_pair "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}"
                        # See the top-level two-argument entry point's own
                        # comment (above, in this file) for why this is needed.
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_NEITHER_SIG=$NEITHER_LOOKS_LIKE_SIGNATURE
                        EXPLICIT_SIG_NAMED_BUT_NOT_SIG=$SIG_NAMED_BUT_NOT_SIGNATURE
                        if [ "$NEITHER_LOOKS_LIKE_SIGNATURE" -eq 0 ] && ! iso_result_plausible; then
                            EXPLICIT_ISO_NOT_PLAUSIBLE=1
                        fi
                    fi
                    EXPLICIT_ISO=$ISO
                    EXPLICIT_SIG=$SIG
                    return 0
                    ;;
                3)
                    # Same triple-classification as the top-level three-
                    # argument entry point (see run_picker_mode()'s own
                    # comment on its "3)" case) - reused here rather than
                    # duplicated, since drag-and-drop is just another way
                    # the same three files can arrive.
                    EXPLICIT_CHECKSUM_FILE=""
                    EXPLICIT_CHECKSUM_SIG=""
                    EXPLICIT_CHECKSUM_ALGO=""
                    EXPLICIT_NO_DIRECT_SIG=0
                    EXPLICIT_NEITHER_SIG=0
                    EXPLICIT_SIG_NAMED_BUT_NOT_SIG=""
                    EXPLICIT_ISO_NOT_PLAUSIBLE=0
                    if ! classify_iso_checksum_signature_triple "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}" "${DROPPED_PATHS[2]}" \
                        && ! classify_iso_triple_ignoring_one "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}" "${DROPPED_PATHS[2]}"; then
                        triple_base1=$(basename "${DROPPED_PATHS[0]}")
                        triple_base2=$(basename "${DROPPED_PATHS[1]}")
                        triple_base3=$(basename "${DROPPED_PATHS[2]}")
                        if [ "$TRIPLE_AMBIGUOUS" -eq 1 ]; then
                            if [ -n "$TRIPLE_AMBIGUOUS_ANCHOR" ]; then
                                anchor_base=$(basename "$TRIPLE_AMBIGUOUS_ANCHOR")
                                cand1_base=$(basename "$TRIPLE_AMBIGUOUS_CANDIDATE1")
                                cand2_base=$(basename "$TRIPLE_AMBIGUOUS_CANDIDATE2")
                                yad_error "$(safe_eval_gettext "Cannot detect what '\${anchor_base}' belongs to:" anchor_base)\n$cand1_base\n$cand2_base"
                            else
                                yad_error "$(safe_eval_gettext "Cannot detect how these three files belong together:")\n$triple_base1\n$triple_base2\n$triple_base3"
                            fi
                        else
                            yad_error "$(safe_eval_gettext "Cannot detect an ISO, a checksum listing, and its signature among these three files:")\n$triple_base1\n$triple_base2\n$triple_base3"
                        fi
                    else
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                        EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
                        EXPLICIT_ISO=$ISO
                        EXPLICIT_SIG=$SIG
                        return 0
                    fi
                    ;;
                *)
                    # No sensible interpretation for 4+ at once.
                    DROP_COUNT=${#DROPPED_PATHS[@]}
                    yad_error "$(safe_eval_gettext "Drop at most three files (the ISO plus its checksum listing and/or signature file) - \${DROP_COUNT} were dropped." DROP_COUNT)"
                    ;;
            esac
            continue
        fi

        # "Manage Trusted Keys" closes this whole picker assembly (yad
        # can't keep it open while running another dialog flow). Code 2 is
        # even, so $form already holds whatever was picked in the field -
        # carry it into PREFILL_FILE before running the manager, then loop
        # back to re-show this picker with it still filled in.
        if [ "$paned_rc" -eq 2 ]; then
            if [ -n "$form" ]; then
                PREFILL_FILE=$(printf '%s' "$form" | cut -d'|' -f1)
            fi
            run_manage_mode
            continue
        fi

        # "About" - same reasoning as "Manage Trusted Keys" just above.
        if [ "$paned_rc" -eq 4 ]; then
            if [ -n "$form" ]; then
                PREFILL_FILE=$(printf '%s' "$form" | cut -d'|' -f1)
            fi
            run_about_mode
            continue
        fi

        # "Help" - opens the README (this tool's help content for now) in
        # the default browser. Where possible (X11, needs wmctrl), the
        # picker is reshown minimized and confirmed hidden BEFORE the
        # browser opens (see OPEN_HELP_ON_NEXT_PICKER above) - so the
        # browser is the last thing to take focus, with nothing after
        # it to steal focus back. Without wmctrl (or on Wayland) there's
        # no way to do that, so the browser just opens directly and the
        # picker may steal focus back when it reappears. Falls back to
        # a copyable error dialog with the URL if xdg-open itself isn't
        # installed.
        if [ "$paned_rc" -eq 6 ]; then
            if [ -n "$form" ]; then
                PREFILL_FILE=$(printf '%s' "$form" | cut -d'|' -f1)
            fi
            if command -v xdg-open >/dev/null 2>&1; then
                if [ -z "${WAYLAND_DISPLAY:-}" ] && command -v wmctrl >/dev/null 2>&1; then
                    OPEN_HELP_ON_NEXT_PICKER=1
                else
                    xdg-open "$HELP_URL" >/dev/null 2>&1 &
                    disown
                fi
            else
                yad_error "$(safe_eval_gettext "Could not find a way to open a web browser (xdg-open is missing). Open this address yourself:") $HELP_URL"
            fi
            continue
        fi

        # Cancel or closing the window leaves $form truly empty; a real
        # submission (even with the field left blank) still yields a lone "|".
        [ -n "$form" ] || exit 0
        FILE_PICKED=$(printf '%s' "$form" | cut -d'|' -f1)
        return 0
    done
}

# Runs verify_iso() in a setsid'd subprocess (own process group,
# killable as one unit). State passed via RV_* exported vars. Sets
# $OUTPUT/$RC, or $INTERRUPTED=1 if the dialog closes first. $1:
# progress text.
run_verify() {
    local progress_text=$1
    local PROGRESS_ARGS=(
        --progress
        --pulsate
        # --hide-text: hides the unused "0%" label.
        --hide-text
        --no-buttons
        --center
        --title="$TITLE"
        --class="$WM_CLASS"
        --window-icon="$ICON_FILE"
        --text="$progress_text"
    )

    # Relayed to the subprocess via the environment.
    local RV_ISO=$ISO RV_SIG=$SIG RV_EXPORT_KEY_TO=$KEY_EXPORT_FILE RV_DEBUG=$DEBUG
    local RV_CHECKSUM_FILE=${PINNED_CHECKSUM_FILE:-} RV_CHECKSUM_ALGO=${PINNED_CHECKSUM_ALGO:-}
    local RV_ISO_DERIVED=${PINNED_ISO_DERIVED_FROM_SIG:-0} RV_NO_DIRECT_SIG
    if [ "${PINNED_NO_DIRECT_SIG:-0}" -eq 1 ]; then
        RV_NO_DIRECT_SIG=1
    else
        [ -n "$SIG" ] && RV_NO_DIRECT_SIG=0 || RV_NO_DIRECT_SIG=1
    fi

    RV_VERIFY=$VERIFY RV_ISO=$RV_ISO RV_SIG=$RV_SIG RV_EXPORT_KEY_TO=$RV_EXPORT_KEY_TO \
    RV_CHECKSUM_FILE=$RV_CHECKSUM_FILE RV_CHECKSUM_ALGO=$RV_CHECKSUM_ALGO \
    RV_NO_DIRECT_SIG=$RV_NO_DIRECT_SIG RV_ISO_DERIVED=$RV_ISO_DERIVED \
    RV_OUT_FILE=$OUT_FILE RV_RC_FILE=$RC_FILE RV_DEBUG=$RV_DEBUG \
    setsid bash -c '
        set -euo pipefail
        LIB_MODE=1
        . "$RV_VERIFY"
        lib_init_defaults
        KEEP_KEY=0
        IS_CACHED=0
        ALLOW_UNKNOWN=1
        TRUST_KEY=0
        KEEP=0
        # lib_init_defaults reset this to 0 - restore --debug from the parent.
        DEBUG=$RV_DEBUG
        EXPORT_KEY_TO=$RV_EXPORT_KEY_TO
        FROM_RING_OVERRIDE=""
        VERIFY_AS_CHECKSUM_FILE=0
        NO_CHECKSUM_FALLBACK=0
        CHECKSUM_FILE_OVERRIDE=$RV_CHECKSUM_FILE
        CHECKSUM_ALGO_OVERRIDE=$RV_CHECKSUM_ALGO
        KEYSERVER_OPT=""
        STATUS_FD=""
        NO_DIRECT_SIG=$RV_NO_DIRECT_SIG
        ISO_DERIVED_FROM_SIG=$RV_ISO_DERIVED
        ISO=$RV_ISO
        SIG=$RV_SIG
        # Full output to $RV_OUT_FILE; stderr copy drops tag lines.
        # set +e: avoid errexit before $RV_RC_FILE is written.
        set +e
        verify_iso 2>&1 | tee "$RV_OUT_FILE" | grep -v "^\[VERIFY-ISO-SIG:\]" >&2
        rc=${PIPESTATUS[0]}
        set -e
        echo "$rc" > "$RV_RC_FILE"
    ' &
    local producer_pid=$!

    # FIFO heartbeat: --pulsate only advances on new stdin lines.
    local progress_fifo
    progress_fifo=$(mktemp -u "$SESSION_TMPDIR/progress-stdin.XXXXXXXXXX")
    mkfifo "$progress_fifo"
    ( while :; do echo; sleep 0.3; done ) > "$progress_fifo" &
    local heartbeat_pid=$!
    trap 'kill "$heartbeat_pid" 2>/dev/null || true; rm -f "$progress_fifo"' RETURN

    yad "${PROGRESS_ARGS[@]}" < "$progress_fifo" 2>/dev/null &
    local yad_pid=$!

    local finished_pid=""
    wait -n -p finished_pid "$producer_pid" "$yad_pid" 2>/dev/null || true

    INTERRUPTED=0
    if [ "$finished_pid" = "$yad_pid" ] && kill -0 "$producer_pid" 2>/dev/null; then
        # Kill the whole process group: SIGTERM, then SIGKILL.
        INTERRUPTED=1
        kill -TERM -- "-$producer_pid" 2>/dev/null || true
        sleep 0.2
        kill -KILL -- "-$producer_pid" 2>/dev/null || true
        wait "$producer_pid" 2>/dev/null || true
        return 0
    fi

    # Work finished first - close the stale progress window.
    kill "$yad_pid" 2>/dev/null || true
    wait "$yad_pid" 2>/dev/null || true
    wait "$producer_pid" 2>/dev/null || true
    OUTPUT=$(cat "$OUT_FILE")
    RC=$(cat "$RC_FILE")
}

# Shared "Trust this key?" dialog, used both for an unrecognized-but-GOOD
# direct-ISO signature and for an unrecognized-but-GOOD checksum-file
# signature (checksum-mode) - the wording differs by scenario (built by
# the caller), but accepting/declining is handled identically either way:
# accept -> --keep-key (no re-verification needed, the check already
# succeeded); decline -> RC forced to 1, since "verified" here means
# verified AND trusted, not just cryptographically self-consistent.
# Operates on the OUTPUT/RC globals, same as run_verify(). Picker-mode-only.
offer_trust_unrecognized_key() {
    local dialog_text=$1 dialog_width=${2:-520} dialog_textwidth=${3:-0}
    # --text-width keeps yad's own auto-height guess honest for a widened
    # dialog (same fix/reasoning as CONFIRM_WIDTH's own call site).
    QUESTION_ARGS=(
        --question
        "${SELECTABLE_LABELS_ARGS[@]}"
        --center
        --title="$TITLE"
        --class="$WM_CLASS"
        --window-icon="$ICON_FILE"
        --width="$dialog_width"
    )
    [ "$dialog_textwidth" -gt 0 ] && QUESTION_ARGS+=(--text-width="$dialog_textwidth")
    QUESTION_ARGS+=(
        --text="$dialog_text"
        --button="$BTN_TRUST_THIS_KEY:0"
        --button="$(safe_eval_gettext "Cancel"):1"
    )
    if yad "${QUESTION_ARGS[@]}"; then
        if run_keep_key "$FPR" "$KEY_EXPORT_FILE"; then
            # The key is now trusted, so the earlier "unrecognized key"
            # notes no longer describe the current state - strip them
            # from the displayed technical body (not the CLI's own
            # historical output on disk/stderr). Deletes by the stable,
            # never-localized UNRECOGNIZED_KEY_WARNINGS_BEGIN/END tags, not
            # by matching the (localized) info() text itself.
            OUTPUT=$(printf '%s\n' "$OUTPUT" | sed '/^\[VERIFY-ISO-SIG:\] UNRECOGNIZED_KEY_WARNINGS_BEGIN$/,/^\[VERIFY-ISO-SIG:\] UNRECOGNIZED_KEY_WARNINGS_END$/d')
            OUTPUT="$OUTPUT

$KEEPKEY_OUTPUT"
        else
            RC=1
            OUTPUT="$OUTPUT

$KEEPKEY_OUTPUT

[VERIFY-ISO-SIG:] GUI_KEEP_FAILED"
        fi
    else
        RC=1
        OUTPUT="$OUTPUT

[VERIFY-ISO-SIG:] GUI_DECLINED_TRUST"
    fi
}

# Wraps the picker's whole pick-verify-show-result flow. Only ever called
# once, so it's fine for this one to end the process directly via `exit "$RC"`.
run_picker_mode() {
    # Live inside $SESSION_TMPDIR (a real global, already set by the time
    # this line runs - verify-iso-sig creates it, and installs its own
    # `trap 'rm -rf "$SESSION_TMPDIR"' EXIT`, unconditionally at source time -
    # see its own comment there). That one trap already cleans these up too,
    # on both normal completion and an external kill mid-operation (e.g.
    # SIGTERM during a keyserver fetch) - no separate trap needed in this
    # script at all.
    local OUT_FILE RC_FILE KEY_EXPORT_FILE
    OUT_FILE=$(mktemp "$SESSION_TMPDIR/out.XXXXXXXXXX")
    RC_FILE=$(mktemp "$SESSION_TMPDIR/rc.XXXXXXXXXX")
    # Holds a copy of the signing key exported right after a normal verify
    # run resolves it - reused by a later --keep-key --from-ring call
    # ("Trust this key") so trusting it doesn't need a second keyserver round-trip.
    KEY_EXPORT_FILE=$(mktemp "$SESSION_TMPDIR/keyexport.XXXXXXXXXX")

    # The whole pick-verify-show-result cycle is wrapped in a loop so the
    # result dialog's "Check Another File" button can return here with the
    # ISO/SIG pre-filled, instead of exiting the whole script.
    while :; do
    # EXPLICIT_ISO can be non-empty two ways: set before this loop's first
    # pass (two positional arguments), or set by pick_files() itself (a
    # two-file drag-and-drop) - only call pick_files() when neither has
    # happened yet. Cleared once consumed so a later "Check Another File"
    # round falls through to the normal picker.
    if [ -z "$EXPLICIT_ISO" ]; then
        FROM_EXPLICIT_PAIR=0
        pick_files
    fi

    if [ -n "$EXPLICIT_ISO" ]; then
        FROM_EXPLICIT_PAIR=1
        ISO=$EXPLICIT_ISO
        SIG=$EXPLICIT_SIG
        # PINNED_CHECKSUM_FILE survives into run_verify()'s own subshell via
        # "${PINNED_CHECKSUM_FILE:-}" instead of a blind "". Empty except
        # for a recognized checksum-listing pairing.
        PINNED_CHECKSUM_FILE=$EXPLICIT_CHECKSUM_FILE
        # PINNED_CHECKSUM_ALGO: only classify_plain_checksum_signature_
        # pair() sets EXPLICIT_CHECKSUM_ALGO (a "<iso>.<suffix>" checksum
        # file isn't a fixed CHECKSUM_FILES literal try_checksum_fallback()
        # can resolve a hashcmd from on its own) - empty for every other shape.
        PINNED_CHECKSUM_ALGO=$EXPLICIT_CHECKSUM_ALGO
        # PINNED_NO_DIRECT_SIG: for a plain-per-file-checksum pairing whose
        # ISO has no real direct signature either, $SIG is still non-empty
        # (a placeholder), so the usual "$SIG empty means no direct sig"
        # inference would get this sub-case wrong without this override.
        PINNED_NO_DIRECT_SIG=$EXPLICIT_NO_DIRECT_SIG
        # Always 0 here - both files were explicit, not derived - reset
        # explicitly so a stale 1 can't survive from an earlier round.
        PINNED_ISO_DERIVED_FROM_SIG=0
        # Display-only, for the confirmation dialog below - never read by verify_iso().
        CONFIRM_CHECKSUM_SIG=$EXPLICIT_CHECKSUM_SIG
        NEITHER_IS_SIG=$EXPLICIT_NEITHER_SIG
        SIG_NAMED_BUT_NOT_SIG=$EXPLICIT_SIG_NAMED_BUT_NOT_SIG
        ISO_NOT_PLAUSIBLE=$EXPLICIT_ISO_NOT_PLAUSIBLE
        EXPLICIT_ISO=""
        EXPLICIT_SIG=""
        EXPLICIT_CHECKSUM_FILE=""
        EXPLICIT_CHECKSUM_SIG=""
        EXPLICIT_CHECKSUM_ALGO=""
        EXPLICIT_NO_DIRECT_SIG=0
        EXPLICIT_NEITHER_SIG=0
        EXPLICIT_SIG_NAMED_BUT_NOT_SIG=""
        EXPLICIT_ISO_NOT_PLAUSIBLE=0

        if [ -d "$ISO" ]; then
            yad_error "$(safe_eval_gettext "That's a folder, not a file - pick the .iso file itself (or its .sig/.asc/.gpg/.sign signature file) directly.")"
            continue
        fi

        # Neither file was confirmed a signature by name or content -
        # refuse rather than showing a confirmation for an unvalidated
        # pairing. YAD_ERROR_WIDTH forces wrapping for a long basename.
        if [ "$NEITHER_IS_SIG" -eq 1 ]; then
            if [ -n "$SIG_NAMED_BUT_NOT_SIG" ]; then
                named_base=$(basename "$SIG_NAMED_BUT_NOT_SIG")
                YAD_ERROR_WIDTH=480 yad_error "$(safe_eval_gettext "'\${named_base}' looks like a signature file by its name, but its content doesn't look like a real one." named_base)"
            else
                neither_iso_base=$(basename "$ISO")
                neither_sig_base=$(basename "$SIG")
                YAD_ERROR_WIDTH=480 yad_error "$(safe_eval_gettext "Neither '\${neither_iso_base}' nor '\${neither_sig_base}' looks like a real ISO signature file, by name or content." neither_iso_base neither_sig_base)"
            fi
            continue
        fi

        if [ "$ISO_NOT_PLAUSIBLE" -eq 1 ]; then
            not_plausible_iso_base=$(basename "$ISO")
            not_plausible_sig_base=$(basename "$SIG")
            YAD_ERROR_WIDTH=480 yad_error "$(safe_eval_gettext "'\${not_plausible_iso_base}' and '\${not_plausible_sig_base}' both look like signature or checksum-listing files - pick the actual ISO, plus its checksum listing or signature file." not_plausible_iso_base not_plausible_sig_base)"
            continue
        fi
    else
        PINNED_CHECKSUM_FILE=""
        PINNED_CHECKSUM_ALGO=""
        PINNED_NO_DIRECT_SIG=0
        PINNED_ISO_DERIVED_FROM_SIG=0
        # A directory passes a plain [ -r ] check like a readable regular
        # file - caught here, before extension classification, so it isn't
        # mistaken for a checksum listing.
        if [ -d "$FILE_PICKED" ]; then
            yad_error "$(safe_eval_gettext "That's a folder, not a file - pick the .iso file itself (or its .sig/.asc/.gpg/.sign signature file) directly.")"
            continue
        fi

        # The picker returns a single path - the .iso, or a signature file
        # directly (.sig/.asc/.gpg, or a checksum-listing's own .sign).
        # Classify by extension and derive the counterpart via the naming
        # convention. When no direct signature exists, PINNED_NO_DIRECT_SIG
        # lets the checksum-file fallback take over - SIG itself still gets
        # sig_for_iso()'s own ".sig" placeholder name (never left blank),
        # so a later "cannot read signature file" error names an actual
        # path instead of an empty string, matching the CLI's own main().
        # is_clearsigned_file() checked first: a full clearsigned message
        # needs no separate signature, whatever its name/extension - used
        # exactly as picked, not a derived sibling name.
        if is_clearsigned_file "$FILE_PICKED"; then
            ISO=$FILE_PICKED
            CANDIDATE_SIG=$(sig_for_iso "$FILE_PICKED")
            # $CANDIDATE_SIG can itself be clearsigned - use it as that instead.
            if [ -r "$CANDIDATE_SIG" ] && is_clearsigned_file "$CANDIDATE_SIG"; then
                PINNED_CHECKSUM_FILE=$CANDIDATE_SIG
                SIG=""
            else
                SIG=$CANDIDATE_SIG
                { [ -f "$CANDIDATE_SIG" ] && [ -r "$CANDIDATE_SIG" ]; } || PINNED_NO_DIRECT_SIG=1
            fi
        else
            case "$FILE_PICKED" in
                *.sig|*.asc|*.gpg|*.sign)
                    SIG=$FILE_PICKED
                    ISO=$(iso_for_sig "$FILE_PICKED")
                    # Only our own guess from the signature file, not user-given.
                    PINNED_ISO_DERIVED_FROM_SIG=1
                    ;;
                *)
                    ISO=$FILE_PICKED
                    CANDIDATE_SIG=$(sig_for_iso "$FILE_PICKED")
                    # $CANDIDATE_SIG can itself be clearsigned - use it as that instead.
                    if [ -r "$CANDIDATE_SIG" ] && is_clearsigned_file "$CANDIDATE_SIG"; then
                        PINNED_CHECKSUM_FILE=$CANDIDATE_SIG
                        SIG=""
                    else
                        SIG=$CANDIDATE_SIG
                        { [ -f "$CANDIDATE_SIG" ] && [ -r "$CANDIDATE_SIG" ]; } || PINNED_NO_DIRECT_SIG=1
                    fi
                    ;;
            esac
        fi
    fi

    # Preserved so the precheck call below (run directly, not in a
    # subshell) can be undone afterward: if $FILE_PICKED was a checksum-
    # listing's own signature, resolve_checksum_listing_as_iso() reassigns
    # the global $ISO/$SIG to the resolved .iso plus a guessed, nonexistent
    # "$ISO.sig" - without restoring these, the real run below would
    # inherit that wrong $SIG instead of the signature file actually picked.
    ORIG_ISO=$ISO
    ORIG_SIG=$SIG

    # Easy to hit by mistake - re-show the picker instead of exiting.
    if [ -z "$ISO" ]; then
        yad_error "$(safe_eval_gettext "No ISO or signature file was selected.")"
        continue
    fi

    # -f (not just -r) also rejects a FIFO/device/socket masquerading as a readable path.
    # Skip when PINNED_ISO_DERIVED_FROM_SIG - verify_iso()'s own
    # SIG_WITHOUT_ISO tag handles that case better than a blunt error here.
    if [ "${PINNED_ISO_DERIVED_FROM_SIG:-0}" -ne 1 ] && ! { [ -f "$ISO" ] && [ -r "$ISO" ]; }; then
        yad_error "$(safe_eval_gettext "Cannot read ISO file:")\n$ISO"
        continue
    fi

    # "${PINNED_NO_DIRECT_SIG:-0}" -ne 1: for a plain-per-file-checksum
    # pairing with no real direct ISO signature, $SIG is a deliberately
    # nonexistent "${ISO}.sig" placeholder (see PINNED_NO_DIRECT_SIG's own
    # comment above) - this precheck must not reject that as a missing
    # signature file, since the checksum-file fallback handles it instead.
    if [ "${PINNED_NO_DIRECT_SIG:-0}" -ne 1 ] && [ -n "$SIG" ] \
       && ! { [ -f "$SIG" ] && [ -r "$SIG" ]; }; then
        yad_error "$(safe_eval_gettext "Cannot find the matching signature file:")\n$SIG"
        continue
    fi

    if [ -n "$PINNED_CHECKSUM_FILE" ] && ! { [ -f "$PINNED_CHECKSUM_FILE" ] && [ -r "$PINNED_CHECKSUM_FILE" ]; }; then
        yad_error "$(safe_eval_gettext "Cannot read the checksum-listing file:")\n$PINNED_CHECKSUM_FILE"
        continue
    fi

    # The explicit-two-arguments entry point skips pick_files() entirely -
    # this dialog is the "look before you leap" step it would otherwise
    # miss, shown once both files pass the same checks as any other path.
    # A recognized checksum-listing pairing shows "SHA:" instead of
    # "SIG:" - there's no direct signature file in this shape at all.
    # When the listing's own signature is known too (CONFIRM_CHECKSUM_SIG),
    # a third line names it as well.
    if [ "$FROM_EXPLICIT_PAIR" -eq 1 ]; then
        LABEL_ISO=$(safe_eval_gettext "ISO:")
        ISO_BASE=$(basename "$ISO")
        CONFIRM_LONGEST_LINE=$(( ${#LABEL_ISO} + ${#ISO_BASE} ))
        if [ -n "$PINNED_CHECKSUM_FILE" ]; then
            LABEL_CHECKSUM_FILE=$(safe_eval_gettext "SHA:")
            CHECKSUM_FILE_BASE=$(basename "$PINNED_CHECKSUM_FILE")
            CONFIRM_VERIFY_TEXT="$(safe_eval_gettext "About to verify:")\n<b>$LABEL_ISO</b> $(pango_escape "$ISO_BASE")\n<b>$LABEL_CHECKSUM_FILE</b> $(pango_escape "$CHECKSUM_FILE_BASE")"
            [ $(( ${#LABEL_CHECKSUM_FILE} + ${#CHECKSUM_FILE_BASE} )) -gt "$CONFIRM_LONGEST_LINE" ] \
                && CONFIRM_LONGEST_LINE=$(( ${#LABEL_CHECKSUM_FILE} + ${#CHECKSUM_FILE_BASE} ))
            if [ -n "$CONFIRM_CHECKSUM_SIG" ]; then
                LABEL_CHECKSUM_SIG=$(safe_eval_gettext "SIG:")
                CHECKSUM_SIG_BASE=$(basename "$CONFIRM_CHECKSUM_SIG")
                CONFIRM_VERIFY_TEXT="$CONFIRM_VERIFY_TEXT\n<b>$LABEL_CHECKSUM_SIG</b> $(pango_escape "$CHECKSUM_SIG_BASE")"
                [ $(( ${#LABEL_CHECKSUM_SIG} + ${#CHECKSUM_SIG_BASE} )) -gt "$CONFIRM_LONGEST_LINE" ] \
                    && CONFIRM_LONGEST_LINE=$(( ${#LABEL_CHECKSUM_SIG} + ${#CHECKSUM_SIG_BASE} ))
            fi
        else
            LABEL_SIG=$(safe_eval_gettext "SIG:")
            SIG_BASE=$(basename "$SIG")
            CONFIRM_VERIFY_TEXT="$(safe_eval_gettext "About to verify:")\n<b>$LABEL_ISO</b> $(pango_escape "$ISO_BASE")\n<b>$LABEL_SIG</b> $(pango_escape "$SIG_BASE")"
            [ $(( ${#LABEL_SIG} + ${#SIG_BASE} )) -gt "$CONFIRM_LONGEST_LINE" ] \
                && CONFIRM_LONGEST_LINE=$(( ${#LABEL_SIG} + ${#SIG_BASE} ))
        fi
        # Trailing blank line - breathing room so the buttons don't sit
        # flush against the last filename line (--text-width below sizes
        # the dialog to fit the text exactly, so this needs to be a real
        # extra line, not just assumed padding).
        CONFIRM_VERIFY_TEXT="$CONFIRM_VERIFY_TEXT\n"
        # ~7px/char is a rough, deliberately generous per-char estimate
        # for this dialog's bold-label text, +140 fixed chrome margin -
        # clamped to [480, 900] so a short filename keeps the original
        # compact size and a long one doesn't balloon unreasonably.
        CONFIRM_WIDTH=$(( CONFIRM_LONGEST_LINE * 7 + 140 ))
        [ "$CONFIRM_WIDTH" -lt 480 ] && CONFIRM_WIDTH=480
        [ "$CONFIRM_WIDTH" -gt 900 ] && CONFIRM_WIDTH=900
        CONFIRM_VERIFY_ARGS=(
            --question
            "${SELECTABLE_LABELS_ARGS[@]}"
            --center
            --title="$TITLE"
            --class="$WM_CLASS"
            --window-icon="$ICON_FILE"
            --width="$CONFIRM_WIDTH"
            # yad's own auto-height guess for --text ignores the actual
            # --width given, assuming a narrower wrap - leaving a big
            # empty gap below the text on a widened dialog. Passing the
            # real longest line's char count as --text-width keeps it honest.
            --text-width="$CONFIRM_LONGEST_LINE"
            --text="$CONFIRM_VERIFY_TEXT"
            --button="$(safe_eval_gettext "Verify"):0"
            --button="$(safe_eval_gettext "Cancel"):1"
        )
        yad "${CONFIRM_VERIFY_ARGS[@]}" || exit 0
    fi

    # Fresh each iteration - no reason to let the file grow across rounds.
    : > "$KEY_EXPORT_FILE"

    # Quick, network-free precheck (no gpgv, no fetch) just to pick
    # accurate progress-dialog wording - doesn't affect the real run. A
    # key that's cached but expired/revoked still counts as "not cached"
    # here, matching --is-cached's own exit status. Called directly
    # in-process, so every global verify_iso() reads is reset here rather
    # than relying on lib_init_defaults' one-time values.
    KEEP_KEY=0; IS_CACHED=1; ALLOW_UNKNOWN=0; TRUST_KEY=0; KEEP=0
    EXPORT_KEY_TO=""; FROM_RING_OVERRIDE=""; VERIFY_AS_CHECKSUM_FILE=0
    # "${PINNED_CHECKSUM_FILE:-}"/"${PINNED_CHECKSUM_ALGO:-}" here too - see run_verify()'s identical reset.
    NO_CHECKSUM_FALLBACK=0; CHECKSUM_FILE_OVERRIDE=${PINNED_CHECKSUM_FILE:-}; CHECKSUM_ALGO_OVERRIDE=${PINNED_CHECKSUM_ALGO:-}
    KEYSERVER_OPT=""; STATUS_FD=""
    # Mirrors main()'s own ISO/SIG classification: 1 when no .sig/.asc/.gpg
    # was found next to the ISO, letting verify_iso() try the checksum-file
    # fallback. "${PINNED_NO_DIRECT_SIG:-0}" overrides this inference for
    # the one sub-case it gets wrong.
    if [ "${PINNED_NO_DIRECT_SIG:-0}" -eq 1 ]; then
        NO_DIRECT_SIG=1
    else
        [ -n "$SIG" ] && NO_DIRECT_SIG=0 || NO_DIRECT_SIG=1
    fi
    # Same reasoning as NO_DIRECT_SIG just above.
    ISO_DERIVED_FROM_SIG=${PINNED_ISO_DERIVED_FROM_SIG:-0}
    if verify_iso >/dev/null 2>&1; then
        VERIFYING_TEXT=$(safe_eval_gettext "Verifying signature - this can take a while for a large ISO...")
    else
        VERIFYING_TEXT_LINE1=$(safe_eval_gettext "Verifying signature - this can take a few seconds")
        # TRANSLATORS: continues the previous line's sentence ("Verifying
        # signature - this can take a few seconds") onto a second line -
        # the two need to read as one continuous sentence together.
        VERIFYING_TEXT_LINE2=$(safe_eval_gettext "if the signing key needs to be fetched from a keyserver...")
        VERIFYING_TEXT="$VERIFYING_TEXT_LINE1\n$VERIFYING_TEXT_LINE2"
    fi

    # Undo whatever the precheck call just above did to $ISO/$SIG - the
    # real run needs the same freshly-classified values, not whatever the
    # precheck left behind.
    ISO=$ORIG_ISO
    SIG=$ORIG_SIG

    # Always let the actual cryptographic check run, even for an
    # unrecognized key - it's harmless on its own, and it's the only way
    # to learn the signer's claimed identity. What gates on recognition is
    # whether the result gets accepted as "verified".
    run_verify "$VERIFYING_TEXT"

    if [ "$INTERRUPTED" -eq 1 ]; then
        yad_error "$(safe_eval_gettext "Process was interrupted - the check did not finish, so nothing here has been verified.")"
        PREFILL_FILE=$ISO
        continue
    fi

    FPR=$(gui_status_field "$OUTPUT" SIGNATURE_FPR)

    # Non-empty only when the checksum-file fallback kicked in (no direct
    # ISO signature found) - shows different HEADING/NOTE detail below.
    CHECKSUM_INFO=$(gui_status_field "$OUTPUT" CHECKSUM_FILE)
    CHECKSUM_SIG_INFO=$(gui_status_field "$OUTPUT" CHECKSUM_SIG_INFO)

    # Non-empty only when a checksum-listing file was picked directly and
    # resolve_checksum_listing_as_iso() redirected to the ISO it actually
    # mentions - lets the result heading show that ISO instead of the
    # checksum listing's own name.
    RESOLVED_ISO=$(gui_status_field "$OUTPUT" RESOLVED_ISO)
    # Same idea as RESOLVED_ISO, for the resolved direct signature file.
    RESOLVED_SIG=$(gui_status_field "$OUTPUT" RESOLVED_SIG)

    # DISPLAY_ISO/DISPLAY_ISO_PATH prefer RESOLVED_ISO over the GUI's own
    # possibly-stale $ISO - computed here (not just below, right before
    # the HEADING) so the trust-unrecognized-key dialog just below can
    # also name which file is actually being asked about, not just the
    # final result.
    if [ -n "$RESOLVED_ISO" ]; then
        DISPLAY_ISO_PATH="$(dirname "$ISO")/$RESOLVED_ISO"
    else
        DISPLAY_ISO_PATH=$ISO
    fi
    DISPLAY_ISO=${RESOLVED_ISO:-$(basename "$ISO")}

    # Covers SIG left blank (verify-iso-sig defaulted to whichever of
    # .sig/.asc/.gpg exists) - computed here, not just before the final
    # HEADING, so the trust dialog's direct-sig branch can name it too.
    # RESOLVED_SIG takes priority when present.
    if [ -n "$RESOLVED_SIG" ]; then
        DISPLAY_SIG="$(dirname "$ISO")/$RESOLVED_SIG"
    else
        DISPLAY_SIG=${SIG:-$(sig_for_iso "$DISPLAY_ISO_PATH")}
    fi

    # Two scenarios can produce a GOOD-but-unrecognized-key result: a
    # direct ISO signature, or a checksum-file's own signature - same
    # neutral wording either way (SIGCHECK_LINE below is the one
    # legitimate difference: what was actually checked).
    if [ "$RC" -eq 0 ] && gui_status_has "$OUTPUT" UNRECOGNIZED_KEY; then
        CLAIMED_ID=$(gui_status_field "$OUTPUT" CLAIMED_IDENTITY)
        CLAIMED_ID_SAFE=$(pango_escape "$CLAIMED_ID")
        # Last 16 hex chars of the fingerprint - what gpg shows as
        # rsa3072/0x..., shown alongside the full fingerprint.
        KEY_ID="0x${FPR: -16}"
        FPR_PRETTY=$(format_fingerprint "$FPR")
        # TRANSLATORS: ${BTN_TRUST_THIS_KEY} is the translated button label - keep the placeholder as-is.
        TRUST_CONFIRM_SENTENCE=$(safe_eval_gettext "By clicking \"\${BTN_TRUST_THIS_KEY}\" you confirm you've checked the Key ID or fingerprint yourself and want to trust and remember this key for future checks." BTN_TRUST_THIS_KEY)
        # UNRECOGNIZED_KEY's own value is $VERIFY_AS_CHECKSUM_FILE (0/1)
        # from the CLI - "0" means direct-ISO-signature wording, else checksum-listing.
        if [ "$(gui_status_field "$OUTPUT" UNRECOGNIZED_KEY)" = "0" ]; then
            # TRANSLATORS: keep the <b>/</b> tags exactly as-is (they render
            # as bold text, not literal characters).
            SIGCHECK_LINE=$(safe_eval_gettext "Signature check: <b>GOOD</b> - the ISO exactly matches this key.")
            NOTE_SELF_DECLARED=$(safe_eval_gettext "Note: that identity is self-declared by whoever created the key - it is NOT independently verified, unlike the Key ID or fingerprint. Only trust this key if you've confirmed the Key ID or fingerprint yourself (e.g. from the respin/distro's own official site, or its keyserver listing).")
            TRUST_LABEL_ISO=$(safe_eval_gettext "ISO:")
            TRUST_LABEL_SIG=$(safe_eval_gettext "SIG:")
            DISPLAY_SIG_BASE=$(basename "$DISPLAY_SIG")
            TRUST_FILE_LINE="$TRUST_LABEL_ISO <b>$(pango_escape "$DISPLAY_ISO")</b>\n$TRUST_LABEL_SIG <b>$(pango_escape "$DISPLAY_SIG_BASE")</b>"
            TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_ISO} + ${#DISPLAY_ISO} ))
            [ $(( ${#TRUST_LABEL_SIG} + ${#DISPLAY_SIG_BASE} )) -gt "$TRUST_LONGEST_LINE" ] \
                && TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_SIG} + ${#DISPLAY_SIG_BASE} ))
            TRUST_TEXT="$(safe_eval_gettext "This ISO's signing key isn't one this tool already recognizes.")\n\n$TRUST_FILE_LINE\n\n$(safe_eval_gettext "Key ID:") <b>$KEY_ID</b>\n$(safe_eval_gettext "Claimed identity:") <b>$CLAIMED_ID_SAFE</b>\n$(safe_eval_gettext "Fingerprint:") <b>$FPR_PRETTY</b>\n$SIGCHECK_LINE\n\n$NOTE_SELF_DECLARED\n\n$TRUST_CONFIRM_SENTENCE"
        else
            # TRANSLATORS: keep the <b>/</b> tags exactly as-is (they render
            # as bold text, not literal characters).
            SIGCHECK_LINE=$(safe_eval_gettext "Signature check: <b>GOOD</b> - the checksum listing exactly matches this key (the ISO's own hash is checked separately, next).")
            NOTE_SELF_DECLARED=$(safe_eval_gettext "Note: that identity is self-declared by whoever created the key - it is NOT independently verified, unlike the Key ID or fingerprint. Only trust this key if you've confirmed it yourself (e.g. from the distro's own official website or keyserver listing).")
            TRUST_LABEL_ISO=$(safe_eval_gettext "ISO:")
            TRUST_LABEL_SHA=$(safe_eval_gettext "SHA:")
            # ISO/SHA/SIG order matches the confirm and final result dialogs.
            TRUST_FILE_LINE="$TRUST_LABEL_ISO <b>$(pango_escape "$DISPLAY_ISO")</b>\n$TRUST_LABEL_SHA <b>$(pango_escape "$CHECKSUM_INFO")</b>"
            TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_ISO} + ${#DISPLAY_ISO} ))
            [ $(( ${#TRUST_LABEL_SHA} + ${#CHECKSUM_INFO} )) -gt "$TRUST_LONGEST_LINE" ] \
                && TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_SHA} + ${#CHECKSUM_INFO} ))
            # No separate line for an inline-signed checksum listing - it has
            # no detached signature file to name (matches the final result
            # dialog's own CHECKSUM_SIG_INFO guard just below).
            if [ -n "$CHECKSUM_SIG_INFO" ]; then
                TRUST_LABEL_SIG=$(safe_eval_gettext "SIG:")
                TRUST_FILE_LINE="$TRUST_FILE_LINE\n$TRUST_LABEL_SIG <b>$(pango_escape "$CHECKSUM_SIG_INFO")</b>"
                [ $(( ${#TRUST_LABEL_SIG} + ${#CHECKSUM_SIG_INFO} )) -gt "$TRUST_LONGEST_LINE" ] \
                    && TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_SIG} + ${#CHECKSUM_SIG_INFO} ))
            fi
            TRUST_TEXT="$(safe_eval_gettext "This checksum-listing file's signing key isn't one this tool already recognizes.")\n\n$TRUST_FILE_LINE\n\n$(safe_eval_gettext "Key ID:") <b>$KEY_ID</b>\n$(safe_eval_gettext "Claimed identity:") <b>$CLAIMED_ID_SAFE</b>\n$(safe_eval_gettext "Fingerprint:") <b>$FPR_PRETTY</b>\n$SIGCHECK_LINE\n\n$NOTE_SELF_DECLARED\n\n$TRUST_CONFIRM_SENTENCE"
        fi
        # Same width formula/clamp as CONFIRM_WIDTH (see its own comment) -
        # a floor of 520 keeps this dialog's existing compact look for a
        # normal-length filename (it carries more prose than the confirm
        # dialog, so it's never shrunk below its original default).
        TRUST_WIDTH=$(( TRUST_LONGEST_LINE * 7 + 140 ))
        [ "$TRUST_WIDTH" -lt 520 ] && TRUST_WIDTH=520
        [ "$TRUST_WIDTH" -gt 900 ] && TRUST_WIDTH=900
        offer_trust_unrecognized_key "$TRUST_TEXT" "$TRUST_WIDTH" "$TRUST_LONGEST_LINE"
    fi

    # For a recognized key that had to be fetched from a keyserver, or was
    # only found in pubring.kbx, offer to cache it in trustedkeys.gpg too,
    # so the next check needs no network. Skipped if the block above
    # already handled it (an unrecognized key just trusted is already kept).
    if [ "$RC" -eq 0 ] && ! gui_status_has "$OUTPUT" KEPT_IN_TRUSTED_GPG; then
        # Same Key ID/identity/fingerprint block as the unrecognized-key
        # trust dialog above - CLAIMED_IDENTITY is emitted on any
        # successful verification, not just the unrecognized-key path.
        CLAIMED_ID=$(gui_status_field "$OUTPUT" CLAIMED_IDENTITY)
        CLAIMED_ID_SAFE=$(pango_escape "$CLAIMED_ID")
        KEY_ID="0x${FPR: -16}"
        FPR_PRETTY=$(format_fingerprint "$FPR")
        KEEP_KEY_DETAILS="$(safe_eval_gettext "Key ID:") <b>$KEY_ID</b>\n$(safe_eval_gettext "Claimed identity:") <b>$CLAIMED_ID_SAFE</b>\n$(safe_eval_gettext "Fingerprint:") <b>$FPR_PRETTY</b>"
        if gui_status_has "$OUTPUT" ALREADY_IN_PUBRING; then
            KEEP_TEXT="$(safe_eval_gettext "This signing key currently only lives in your personal keyring (pubring.kbx).")\n\n$KEEP_KEY_DETAILS\n\n$(safe_eval_gettext "Copy it into ~/.gnupg/trustedkeys.gpg so future checks don't depend on it staying there?")"
        elif gui_status_has "$OUTPUT" FETCHING; then
            KEEP_TEXT="$(safe_eval_gettext "This signing key was just fetched from a keyserver.")\n\n$KEEP_KEY_DETAILS\n\n$(safe_eval_gettext "Remember it locally, in ~/.gnupg/trustedkeys.gpg, so future checks don't need the network again?")"
        else
            KEEP_TEXT=""
        fi
        if [ -n "$KEEP_TEXT" ]; then
            KEEP_Q_ARGS=(
                --question
                "${SELECTABLE_LABELS_ARGS[@]}"
                --center
                --title="$TITLE"
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
                # Matches the trust-unrecognized-key dialog's own floor,
                # now that this dialog shows the same key-details block.
                --width=520
                --text="$KEEP_TEXT"
                --button="$(safe_eval_gettext "Keep this key"):0"
                --button="$(safe_eval_gettext "No thanks"):1"
            )
            if yad "${KEEP_Q_ARGS[@]}"; then
                run_keep_key "$FPR" "$KEY_EXPORT_FILE" || true
                OUTPUT="$OUTPUT

$KEEPKEY_OUTPUT"
            fi
        fi
    fi

    # Crypto check already came back GOOD here - RC is non-zero only
    # because the key wasn't trusted/saved. Field lines stay the normal
    # ones below (every name here is a real, checked file).
    KEY_NOT_TRUSTED_CASE=""
    if gui_status_has "$OUTPUT" GUI_DECLINED_TRUST; then
        KEY_NOT_TRUSTED_CASE=GUI_DECLINED_TRUST
    elif gui_status_has "$OUTPUT" GUI_KEEP_FAILED; then
        KEY_NOT_TRUSTED_CASE=GUI_KEEP_FAILED
    fi

    # These tags mean no real check was ever attempted - only field
    # line(s) confirmed to exist get shown below.
    NOTHING_TO_VERIFY_CASE=""
    if gui_status_has "$OUTPUT" NOTHING_TO_VERIFY; then
        NOTHING_TO_VERIFY_CASE=NOTHING_TO_VERIFY
    elif gui_status_has "$OUTPUT" SIG_WITHOUT_ISO; then
        NOTHING_TO_VERIFY_CASE=SIG_WITHOUT_ISO
    elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_AMBIGUOUS; then
        NOTHING_TO_VERIFY_CASE=CHECKSUM_LISTING_AMBIGUOUS
    elif gui_status_has "$OUTPUT" PLAIN_CHECKSUM_TARGET_MISSING; then
        NOTHING_TO_VERIFY_CASE=PLAIN_CHECKSUM_TARGET_MISSING
    elif gui_status_has "$OUTPUT" PLAIN_CHECKSUM_NOTHING_VERIFIABLE; then
        NOTHING_TO_VERIFY_CASE=PLAIN_CHECKSUM_NOTHING_VERIFIABLE
    elif gui_status_has "$OUTPUT" NO_CHECKSUM_LISTING_MATCH; then
        NOTHING_TO_VERIFY_CASE=NO_CHECKSUM_LISTING_MATCH
    elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_SIG_MISSING; then
        NOTHING_TO_VERIFY_CASE=CHECKSUM_LISTING_SIG_MISSING
    elif gui_status_has "$OUTPUT" SIG_WITHOUT_CHECKSUM_LISTING; then
        NOTHING_TO_VERIFY_CASE=SIG_WITHOUT_CHECKSUM_LISTING
    elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_FOUND_UNSIGNED; then
        NOTHING_TO_VERIFY_CASE=CHECKSUM_LISTING_FOUND_UNSIGNED
    fi

    if gui_status_has "$OUTPUT" KEPT_IN_TRUSTED_GPG; then
        KEY_SOURCE=$(safe_eval_gettext "key saved locally - future checks won't need the network")
    elif gui_status_has "$OUTPUT" ALREADY_IN_TRUSTED_GPG; then
        KEY_SOURCE=$(safe_eval_gettext "used an already-cached key, no network needed")
    elif gui_status_has "$OUTPUT" ALREADY_IN_PUBRING; then
        KEY_SOURCE=$(safe_eval_gettext "used a key from your personal keyring (pubring.kbx)")
    elif gui_status_has "$OUTPUT" FETCHING; then
        KEY_SOURCE=$(safe_eval_gettext "fetched the signing key from a keyserver")
    else
        KEY_SOURCE=""
    fi
    # Shown alongside KEY_SOURCE regardless of outcome (PASS or FAIL) -
    # key_material_usable() in verify-iso-sig accepts an expired key for
    # the crypto check itself, but the GUI should still surface this
    # plainly rather than silently treating it like a normal, current key.
    if gui_status_has "$OUTPUT" KEY_EXPIRED; then
        KEY_EXPIRY_DATE=$(gui_status_field "$OUTPUT" KEY_EXPIRED)
        if [ -n "$KEY_EXPIRY_DATE" ]; then
            KEY_EXPIRY_DATE_SAFE=$(pango_escape "$KEY_EXPIRY_DATE")
            # TRANSLATORS: ${KEY_EXPIRY_DATE_SAFE} is a date (e.g. 2026-08-06) - keep the placeholder as-is.
            KEY_EXPIRED_NOTE=$(safe_eval_gettext "this key expired on \${KEY_EXPIRY_DATE_SAFE} - this does not affect the signature check" KEY_EXPIRY_DATE_SAFE)
        else
            KEY_EXPIRED_NOTE=$(safe_eval_gettext "this key has expired - this does not affect the signature check")
        fi
    else
        KEY_EXPIRED_NOTE=""
    fi

    # yad's --text uses Pango markup (needed for the <b> tags below), so any
    # dynamic value embedded in it - e.g. a UID like "Name <user@host>" - must
    # have &/</> escaped first, or the raw "<user@host>" is parsed as an
    # unknown markup tag and GTK silently renders the whole label empty.
    # $'\uXXXX' is bash's ANSI-C-quoted Unicode escape - it expands to the
    # UTF-8 encoding of that code point at runtime, but keeps the script's
    # own source file itself pure ASCII (no literal multi-byte characters
    # in the file). U+2713 CHECK MARK / U+2717 BALLOT X - no plain-ASCII
    # equivalent exists for either.
    CHECK_MARK=$'\u2713'
    CROSS_MARK=$'\u2717'
    # U+26A0 WARNING SIGN had almost no contrast on a dark theme (thin
    # outline glyph) - U+26D4 NO ENTRY is a solid, filled shape instead,
    # checked against both themes directly, no color override needed.
    WARN_MARK=$'\u26d4'
    if [ "$RC" -eq 0 ]; then
        STATUS_LINE="<span size='x-large'><b>$CHECK_MARK $(safe_eval_gettext "Verified OK")</b></span>"
    elif [ -n "$KEY_NOT_TRUSTED_CASE" ]; then
        STATUS_LINE="<span size='x-large'><b>? $(safe_eval_gettext "Key not trusted")</b></span>"
    elif [ -n "$NOTHING_TO_VERIFY_CASE" ]; then
        STATUS_LINE="<span size='x-large'><b>$WARN_MARK $(safe_eval_gettext "Nothing to verify")</b></span>"
    else
        STATUS_LINE="<span size='x-large'><b>$CROSS_MARK $(safe_eval_gettext "Verification FAILED")</b></span>"
    fi
    # In checksum-mode there's no meaningful per-ISO "SIG:" - show which
    # checksum listing and its signature were used instead. DISPLAY_ISO/
    # DISPLAY_ISO_PATH/DISPLAY_SIG were already computed above, reused
    # here for the mismatch-check below and the HEADING itself.

    if [ -n "$NOTHING_TO_VERIFY_CASE" ]; then
        # Only show a field line for something confirmed to exist.
        HEADING="<span size='large'><b>$TITLE</b></span>\n\n$STATUS_LINE"
        case "$NOTHING_TO_VERIFY_CASE" in
            NOTHING_TO_VERIFY|PLAIN_CHECKSUM_NOTHING_VERIFIABLE)
                HEADING="$HEADING\n<b>$(safe_eval_gettext "ISO:")</b> $(pango_escape "$DISPLAY_ISO")"
                ;;
            CHECKSUM_LISTING_SIG_MISSING)
                # Show what was picked (the listing), not just the ISO.
                HEADING="$HEADING\n<b>$(safe_eval_gettext "ISO:")</b> $(pango_escape "$DISPLAY_ISO")\n<b>$(safe_eval_gettext "SHA:")</b> $(pango_escape "$(basename "${PINNED_CHECKSUM_FILE:-$ISO}")")"
                ;;
            CHECKSUM_LISTING_FOUND_UNSIGNED)
                # Auto-discovered (not picked/pinned) - the name comes
                # from the CLI's own status field, not $PINNED_CHECKSUM_FILE.
                HEADING="$HEADING\n<b>$(safe_eval_gettext "ISO:")</b> $(pango_escape "$DISPLAY_ISO")\n<b>$(safe_eval_gettext "SHA:")</b> $(pango_escape "$(gui_status_field "$OUTPUT" CHECKSUM_LISTING_FOUND_UNSIGNED)")"
                ;;
            SIG_WITHOUT_ISO|SIG_WITHOUT_CHECKSUM_LISTING)
                HEADING="$HEADING\n<b>$(safe_eval_gettext "SIG:")</b> $(basename "$DISPLAY_SIG")"
                ;;
            PLAIN_CHECKSUM_TARGET_MISSING)
                # $ISO is the checksum file itself - DISPLAY_ISO would
                # show the *missing* ISO's name instead.
                HEADING="$HEADING\n<b>$(safe_eval_gettext "SHA:")</b> $(basename "$ISO")"
                ;;
            CHECKSUM_LISTING_AMBIGUOUS|NO_CHECKSUM_LISTING_MATCH)
                if [ -n "$CHECKSUM_INFO" ]; then
                    HEADING="$HEADING\n<b>$(safe_eval_gettext "SHA:")</b> $(pango_escape "$CHECKSUM_INFO")"
                    [ -n "$CHECKSUM_SIG_INFO" ] && HEADING="$HEADING\n<b>$(safe_eval_gettext "SIG:")</b> $(pango_escape "$CHECKSUM_SIG_INFO")"
                else
                    HEADING="$HEADING\n<b>$(safe_eval_gettext "ISO:")</b> $(pango_escape "$DISPLAY_ISO")"
                fi
                ;;
        esac
    elif [ -n "$CHECKSUM_INFO" ]; then
        HEADING="<span size='large'><b>$TITLE</b></span>\n\n$STATUS_LINE\n<b>$(safe_eval_gettext "ISO:")</b> $(pango_escape "$DISPLAY_ISO")\n<b>$(safe_eval_gettext "SHA:")</b> $(pango_escape "$CHECKSUM_INFO")"
        # No separate line for an inline-signed checksum listing - it has no
        # detached signature file to name.
        [ -n "$CHECKSUM_SIG_INFO" ] && HEADING="$HEADING\n<b>$(safe_eval_gettext "SIG:")</b> $(pango_escape "$CHECKSUM_SIG_INFO")"
    else
        HEADING="<span size='large'><b>$TITLE</b></span>\n\n$STATUS_LINE\n<b>$(safe_eval_gettext "ISO:")</b> $(pango_escape "$DISPLAY_ISO")\n<b>$(safe_eval_gettext "SIG:")</b> $(basename "$DISPLAY_SIG")"
    fi
    if [ -z "$NOTHING_TO_VERIFY_CASE" ]; then
        # Neither is meaningful when no key/crypto step was ever reached.
        [ -n "$KEY_SOURCE" ] && HEADING="$HEADING\n<i>$KEY_SOURCE</i>"
        [ -n "$KEY_EXPIRED_NOTE" ] && HEADING="$HEADING\n<i>$KEY_EXPIRED_NOTE</i>"
    fi

    # On failure, translate the likely cause into one plain-language note.
    # Priority order matches likelihood/certainty: a declined or
    # failed-to-save trust decision is checked first - in that case the
    # cryptographic check already came back GOOD, "FAILED" here only means
    # "not trusted". The two checksum-mode-specific real-failure outcomes
    # come next, then a filename mismatch (far more often the real cause
    # of a direct-sig FAILED result than an actual corrupted ISO).
    if [ "$RC" -ne 0 ]; then
        if gui_status_has "$OUTPUT" GUI_DECLINED_TRUST; then
            # TRANSLATORS: ${BTN_CHECK_ANOTHER_FILE}/${BTN_TRUST_THIS_KEY} are translated button labels - keep placeholders as-is.
            NOTE=$(safe_eval_gettext "The cryptographic check itself already came back GOOD (see the technical details below) - you just haven't trusted this signing key yet. If you're confident this is the genuine key (e.g. you've checked its fingerprint against the distro's own official site or keyserver listing), click \"\${BTN_CHECK_ANOTHER_FILE}\" and choose \"\${BTN_TRUST_THIS_KEY}\" this time." BTN_CHECK_ANOTHER_FILE BTN_TRUST_THIS_KEY)
        elif gui_status_has "$OUTPUT" GUI_KEEP_FAILED; then
            NOTE=$(safe_eval_gettext "The cryptographic check itself already came back GOOD (see the technical details below) - the key just couldn't be saved for future trust (see the technical details for why). Feel free to try again.")
        elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_AMBIGUOUS; then
            # TRANSLATORS: ${BTN_CHECK_ANOTHER_FILE} is a translated button label - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "You pointed this tool at a checksum-listing file, not a single ISO - it can cover many ISOs at once, so there's no one file to check unless exactly one of the ones it mentions is actually present here (see the technical details below for which). Use \"\${BTN_CHECK_ANOTHER_FILE}\" and point at the actual .iso file instead." BTN_CHECK_ANOTHER_FILE)
        elif gui_status_has "$OUTPUT" NOTHING_TO_VERIFY; then
            NOTE=$(safe_eval_gettext "No signature file or signed checksum listing was found for this ISO - there's nothing here to check yet.")
        elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_SIG_MISSING; then
            # CHECKSUM_FILE_OVERRIDE itself is only set inside run_verify()'s
            # own subshell, never visible out here - $ISO/$PINNED_CHECKSUM_FILE instead.
            CHECKSUM_FILE_SAFE=$(pango_escape "$(basename "${PINNED_CHECKSUM_FILE:-$ISO}")")
            # TRANSLATORS: ${CHECKSUM_FILE_SAFE} is the checksum-listing file's name - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This checksum listing (\${CHECKSUM_FILE_SAFE}) has no signature file of its own (.sig/.asc/.gpg/.sign) next to it - it can't be trusted without one, so it was never checked against this ISO." CHECKSUM_FILE_SAFE)
        elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_FOUND_UNSIGNED; then
            # Auto-discovered, not picked/pinned - same note text as
            # CHECKSUM_LISTING_SIG_MISSING above, just a different source
            # for the filename (the CLI's own status field).
            CHECKSUM_FILE_SAFE=$(pango_escape "$(gui_status_field "$OUTPUT" CHECKSUM_LISTING_FOUND_UNSIGNED)")
            # TRANSLATORS: ${CHECKSUM_FILE_SAFE} is the checksum-listing file's name - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This checksum listing (\${CHECKSUM_FILE_SAFE}) has no signature file of its own (.sig/.asc/.gpg/.sign) next to it - it can't be trusted without one, so it was never checked against this ISO." CHECKSUM_FILE_SAFE)
        elif gui_status_has "$OUTPUT" SIG_WITHOUT_ISO; then
            DISPLAY_ISO_SAFE=$(pango_escape "$DISPLAY_ISO")
            # TRANSLATORS: ${DISPLAY_ISO_SAFE} is the ISO's filename - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This is a signature file, but its ISO ('\${DISPLAY_ISO_SAFE}') isn't in the same folder - this tool only looks for a counterpart next to the file you pick." DISPLAY_ISO_SAFE)
        elif gui_status_has "$OUTPUT" SIG_WITHOUT_CHECKSUM_LISTING; then
            # $ISO is the checksum listing's own guessed name here.
            LISTING_BASE_SAFE=$(pango_escape "$(basename "$ISO")")
            # TRANSLATORS: ${LISTING_BASE_SAFE} is the checksum-listing file's name - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This is a signature file for a checksum listing, but '\${LISTING_BASE_SAFE}' isn't in the same folder - this tool only looks for a counterpart next to the file you pick." LISTING_BASE_SAFE)
        elif gui_status_has "$OUTPUT" PLAIN_CHECKSUM_TARGET_MISSING; then
            DISPLAY_ISO_SAFE=$(pango_escape "$DISPLAY_ISO")
            # TRANSLATORS: ${DISPLAY_ISO_SAFE} is the ISO's filename - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "Neither '\${DISPLAY_ISO_SAFE}' nor a signature file for it were found in this folder - nothing to check yet." DISPLAY_ISO_SAFE)
        elif gui_status_has "$OUTPUT" PLAIN_CHECKSUM_NOTHING_VERIFIABLE; then
            NOTE=$(safe_eval_gettext "This checksum file isn't signed, and the ISO it describes has no signature of its own either - there's nothing here this tool can cryptographically verify. Check whether the distro provides a signed checksum listing or a direct .sig/.asc/.gpg file for this ISO.")
        elif gui_status_has "$OUTPUT" CHECKSUM_SIG_FAILED; then
            NOTE=$(safe_eval_gettext "The checksum file that lists this ISO's hash failed its own verification (bad/untrusted signature, or its key couldn't be confirmed) - nothing in it can be trusted. Re-download the checksum/signature files (and probably the ISO too), ideally from a different mirror.")
        elif gui_status_has "$OUTPUT" CHECKSUM_HASH_MISMATCH; then
            CHECKSUM_INFO_SAFE=$(pango_escape "$CHECKSUM_INFO")
            # TRANSLATORS: ${CHECKSUM_INFO_SAFE} is the checksum-listing file's name - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This ISO was checked against a signed checksum listing (\${CHECKSUM_INFO_SAFE}) that IS validly signed - but the ISO's own hash doesn't match what's listed. The checksum list itself is trustworthy; this particular download is not. Re-download the ISO, ideally from a different mirror." CHECKSUM_INFO_SAFE)
        elif gui_status_has "$OUTPUT" NO_CHECKSUM_LISTING_MATCH; then
            # TRANSLATORS: ${BTN_CHECK_ANOTHER_FILE} is a translated button label - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "No direct signature file was found for this ISO, and none of the checksum-listing files found nearby mention this exact ISO filename either. If this distro uses a different naming convention, use \"\${BTN_CHECK_ANOTHER_FILE}\" and point at the right file directly." BTN_CHECK_ANOTHER_FILE)
        elif [ "$DISPLAY_SIG" != "${DISPLAY_ISO_PATH}.sig" ] && [ "$DISPLAY_SIG" != "${DISPLAY_ISO_PATH}.asc" ] \
             && [ "$DISPLAY_SIG" != "${DISPLAY_ISO_PATH}.gpg" ] && [ "$DISPLAY_SIG" != "${DISPLAY_ISO_PATH}.sign" ]; then
            # Only reachable via CLI args or a two-file drag-and-drop -
            # the picker's own single-file field always auto-discovers a
            # correctly-named counterpart, never a mismatched one.
            # TRANSLATORS: ${BTN_CHECK_ANOTHER_FILE} is a translated button label - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This signature file's name doesn't match this ISO's name, so it likely belongs to a different download. Use \"\${BTN_CHECK_ANOTHER_FILE}\" and pick just this ISO, or just its own real signature file - the tool finds the matching one automatically." BTN_CHECK_ANOTHER_FILE)
        elif [ "$(gui_status_field "$OUTPUT" UNRECOGNIZED_KEY)" = "0" ]; then
            NOTE=$(safe_eval_gettext "On top of the signature not matching, this signing key also isn't one this tool already recognizes - treat this file as untrustworthy and re-download from an official source.")
        else
            NOTE=$(safe_eval_gettext "The ISO may be corrupted, or the download was incomplete or tampered with - try re-downloading it, ideally from a different mirror.")
        fi
        HEADING="$HEADING\n\n<i>$NOTE</i>"
    fi

    RESULT_ARGS=(
        --text-info
        # Without this, a long single line forces horizontal scrolling
        # instead of wrapping at the dialog's width.
        --wrap
        "${SELECTABLE_LABELS_ARGS[@]}"
        --center
        --title="$TITLE"
        --class="$WM_CLASS"
        # Not --fixed (unlike the picker) - just a friendlier default size,
        # the user can still resize it freely.
        --width=800
        --height=500
        --window-icon="$ICON_FILE"
        --text="$HEADING"
        # "Check Another File" first (left), "Close" last (right) - yad's
        # last button is the Enter-key default, and "Close" is the safer
        # one to trigger by an accidental Enter press.
        --button="$BTN_CHECK_ANOTHER_FILE:2"
        --button="$(safe_eval_gettext "Close"):0"
    )
    # Herestring, not a pipe: old yad mismanages a GLib IO-watch source ID
    # for a live anonymous pipe into --text-info; bash's `<<<` is backed by
    # a seekable temp file instead, avoiding it.
    # Tag lines stripped for the same reason as run_verify()'s live-stream
    # copy - an English-only machine protocol, not for display.
    DISPLAY_OUTPUT=$(printf '%s\n' "$OUTPUT" | grep -v '^\[VERIFY-ISO-SIG:\]' || true)
    set +e
    yad "${RESULT_ARGS[@]}" <<< "$DISPLAY_OUTPUT"
    RESULT_RC=$?
    set -e

    # "Check Another File" loops back to the same picker, pre-filled with
    # the ISO just used. Prefers RESOLVED_ISO over the GUI's own $ISO -
    # when a checksum-listing file was picked directly and redirected,
    # $ISO is still whatever was originally picked, not the one actually checked.
    if [ "$RESULT_RC" -eq 2 ]; then
        if [ -n "$RESOLVED_ISO" ]; then
            PREFILL_FILE="$(dirname "$ISO")/$RESOLVED_ISO"
        else
            PREFILL_FILE=$ISO
        fi
        continue
    fi

    exit "$RC"
    done
}

if [ "$GUI_MODE" = manage ]; then
    run_manage_mode
    exit $?
else
    run_picker_mode
fi

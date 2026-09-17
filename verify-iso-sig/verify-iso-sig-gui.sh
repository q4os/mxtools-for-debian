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
# Many variables here are read only via eval_gettext/safe_eval_gettext's
# envsubst-based substitution, which shellcheck can't trace.
# yad GUI for verify-iso-sig - two modes in one process: the picker
# (default) and the trusted-keys manager (--manage-keys).
set -euo pipefail

# Same literal value as verify-iso-sig, kept in sync by hand.
VERSION="2026.08.01"

# readlink -f, not dirname, so this still works if this file is a symlink.
SELF=$(readlink -f -- "${BASH_SOURCE[0]}" 2>/dev/null) || SELF=${BASH_SOURCE[0]}
SCRIPT_DIR=$(dirname "$SELF")
# A real install has this file in usr/lib/verify-iso-sig/ and
# verify-iso-sig itself in usr/bin/.
if [ "$(basename "$SCRIPT_DIR")" = "verify-iso-sig" ] && [ "$(basename "$(dirname "$SCRIPT_DIR")")" = "lib" ] && [ "$(basename "$(dirname "$(dirname "$SCRIPT_DIR")")")" = "usr" ]; then
    USR_DIR=$(dirname "$(dirname "$SCRIPT_DIR")")
    VERIFY="$USR_DIR/bin/verify-iso-sig"
else
    VERIFY="$SCRIPT_DIR/verify-iso-sig"
fi
DISPLAY_NAME=${DISPLAY_NAME:-$(basename "$SELF")}

GUI_MODE=picker
if [ "${1:-}" = "--manage-keys" ]; then
    GUI_MODE=manage
    shift
fi

# Deliberately before sourcing gettext.sh or checking for yad/$VERIFY.
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

# TEXTDOMAIN must match the .mo filename; both TEXTDOMAIN/TEXTDOMAINDIR
# must be exported - eval_gettext runs as a subprocess and won't see
# unexported variables otherwise.
. /usr/bin/gettext.sh
export TEXTDOMAIN="verify-iso-sig"
export TEXTDOMAINDIR="$SCRIPT_DIR/locale"

# safe_eval_gettext() below falls back to the English original if a
# translator broke a ${VAR} placeholder, or broke Pango markup tags
# (yad/GTK blanks the whole dialog on a markup parse error otherwise).
pango_tag_tokens() {
    grep -oE '</?[A-Za-z][A-Za-z0-9]*' <<< "$1" | sed 's/^<//'
}

# True if $1 (a captured verify-iso-sig output) contains a
# "[VERIFY-ISO-SIG:] $2" tag line - see verify-iso-sig's own
# status_out()/--status-fd for what emits these.
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

# Manager mode's own button labels are set as locals inside
# run_manage_mode() itself. Both titles are computed unconditionally so
# acquire_single_instance_lock() can name either mode. YAD_ERROR_WIDTH=""
# means yad_error() emits no --width flag in picker mode.
PICKER_TITLE=$(safe_eval_gettext "Verify ISO signature")
MANAGE_TITLE=$(safe_eval_gettext "Manage Trusted Keys")
case "$GUI_MODE" in
    picker)
        TITLE=$PICKER_TITLE
        # Translated once here since this label is also quoted inline
        # inside longer sentences elsewhere.
        BTN_TRUST_THIS_KEY=$(safe_eval_gettext "Trust this key")
        BTN_CHECK_ANOTHER_FILE=$(safe_eval_gettext "Check Another File")
        YAD_ERROR_WIDTH=""
        ;;
    manage)
        TITLE=$MANAGE_TITLE
        ;;
esac
ICON_FILE="$SCRIPT_DIR/verify-iso-sig.svg"

# Help button target - a local copy of the README (built by
# debian/rules) if installed, else the GitHub page.
HELP_FILE="$SCRIPT_DIR/help/help.html"
if [ -f "$HELP_FILE" ]; then
    HELP_URL="file://$HELP_FILE"
else
    HELP_URL="https://github.com/MX-Linux/verify-iso-sig/blob/main/README.md"
fi

# Matches verify-iso-sig.desktop's StartupWMClass=. Same class for both
# modes - a distinct manager class left the taskbar showing a generic icon.
WM_CLASS="verify-iso-sig"

# This tool writes to the real ~/.gnupg ($TRUSTED_GPG/$PUBRING_KBX under
# $HOME) - under sudo/pkexec that can end up root-owned, breaking later
# unprivileged use. CLI-only use isn't guarded the same way - a headless
# root context may have no other user to run as.
if [ "$EUID" -eq 0 ]; then
    safe_eval_gettext "error: refusing to run as root - run this as your normal desktop user instead" >&2
    printf '\n' >&2
    exit 1
fi

command -v yad >/dev/null || { safe_eval_gettext "error: yad is not installed" >&2; printf '\n' >&2; exit 1; }
# TRANSLATORS: ${VERIFY} is a file path - keep the placeholder as-is.
[ -x "$VERIFY" ] || { safe_eval_gettext "error: \${VERIFY} not found or not executable" VERIFY >&2; printf '\n' >&2; exit 1; }

# LIB_MODE=1 makes verify-iso-sig's own die() return 1 instead of
# exiting this whole GUI process.
LIB_MODE=1
# shellcheck source=./verify-iso-sig
. "$VERIFY"
lib_init_defaults

# `--selectable-labels` pre-highlights the first label's text on old yad
# (0.40.0) but not current yad (14.1+, versioning jumped from "0.NN.N" to
# "14.N"). `${var%%[!0-9]*}` strips from the first non-digit onward
# (unlike `cut -d'.'`, safe against a future "16 (GTK+ 4.0.1)"). `|| true`:
# this yad build exits `--version` with status 252 despite printing it fine.
YAD_MAJOR=$(yad --version 2>/dev/null | head -n1) || true
YAD_MAJOR=${YAD_MAJOR%%[!0-9]*}
[ -n "$YAD_MAJOR" ] || YAD_MAJOR=0
if [ "$YAD_MAJOR" -ge 14 ]; then
    SELECTABLE_LABELS_ARGS=(--selectable-labels)
else
    SELECTABLE_LABELS_ARGS=()
fi

# Runs verify-iso-sig's own --keep-key mode in-process. Emits a
# synthetic KEPT_IN_TRUSTED_GPG tag on success for gui_status_has().
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
        # Fixed 500/50, not a length-driven formula - that produced an
        # unreasonably wide window on some languages. The explicit "\n"
        # gives a version-independent wrap since --text-width alone
        # doesn't reliably force one on every yad version.
        about_comments_wrapped=$(safe_eval_gettext "Check the GPG signature of a downloaded ISO\n(built-in support for MX Linux/antiX signing keys)")
        # <a href> renders as a clickable link even in a plain --text dialog.
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

# sig_for_iso() only knows .sig/.asc/.gpg (matches verify-iso-sig's
# direct-sig detection, excludes .sign on purpose). iso_for_sig() also
# accepts .sign, since the file could be a checksum-listing's signature.
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

# Runs refresh_trusted_keys() in the background with a progress dialog -
# same producer/heartbeat/race pattern as run_verify() below. Sets
# OUTPUT/RC, same convention as run_verify().
run_refresh_sweep() {
    local progress_text=$1
    local PROGRESS_ARGS=(
        --progress
        --pulsate
        --hide-text
        --no-buttons
        --center
        --title="$TITLE"
        --class="$WM_CLASS"
        --window-icon="$ICON_FILE"
        --text="$progress_text"
    )
    local RS_OUT_FILE
    RS_OUT_FILE=$(mktemp "$SESSION_TMPDIR/refresh-out.XXXXXXXXXX")

    RS_VERIFY=$VERIFY RS_OUT_FILE=$RS_OUT_FILE \
    setsid bash -c '
        set -euo pipefail
        LIB_MODE=1
        . "$RS_VERIFY"
        lib_init_defaults
        refresh_trusted_keys > "$RS_OUT_FILE" 2>&1
    ' {LOCK_FD}>&- &
    local producer_pid=$!

    # FIFO heartbeat: --pulsate only advances on new stdin lines.
    local progress_fifo
    progress_fifo=$(mktemp -u "$SESSION_TMPDIR/refresh-progress-stdin.XXXXXXXXXX")
    mkfifo "$progress_fifo"
    ( while :; do echo; sleep 0.3; done ) > "$progress_fifo" {LOCK_FD}>&- &
    local heartbeat_pid=$!

    setsid yad "${PROGRESS_ARGS[@]}" < "$progress_fifo" 2>/dev/null {LOCK_FD}>&- &
    local yad_pid=$!

    local finished_pid=""
    wait -n -p finished_pid "$producer_pid" "$yad_pid" 2>/dev/null || true

    if [ "$finished_pid" = "$yad_pid" ] && kill -0 "$producer_pid" 2>/dev/null; then
        # Progress window closed (e.g. via the WM's own close button)
        # before the sweep finished - kill the whole process group:
        # SIGTERM, then SIGKILL. Treat as a failure - whatever was
        # captured so far is incomplete.
        kill -TERM -- "-$producer_pid" 2>/dev/null || true
        sleep 0.2
        kill -KILL -- "-$producer_pid" 2>/dev/null || true
        wait "$producer_pid" 2>/dev/null || true
        OUTPUT=$(cat "$RS_OUT_FILE" 2>/dev/null || true)
        RC=1
        kill "$heartbeat_pid" 2>/dev/null || true
        rm -f "$progress_fifo"
        return 0
    fi

    # Work finished first - close the stale progress window.
    kill -- "-$yad_pid" 2>/dev/null || true
    wait "$yad_pid" 2>/dev/null || true
    wait "$producer_pid" 2>/dev/null
    RC=$?
    OUTPUT=$(cat "$RS_OUT_FILE")
    kill "$heartbeat_pid" 2>/dev/null || true
    rm -f "$progress_fifo"
}

# The picker's own "Manage Trusted Keys" button calls this in-process.
# TITLE/etc. are `local` so a picker-triggered call shadows the picker's
# own globals only for this call's duration. Every failure `return`s
# rather than `exit`s, so a picker-triggered call can't kill the whole
# GUI process - the bottom dispatch converts a direct-entry return into exit.
run_manage_mode() {
    local TITLE BTN_DETAILS_SELECTED BTN_UNTRUST_SELECTED BTN_IMPORT_SELECTED BTN_REFRESH_TRUSTED YAD_ERROR_WIDTH
    TITLE=$(safe_eval_gettext "Manage Trusted Keys")
    BTN_DETAILS_SELECTED=$(safe_eval_gettext "Details")
    BTN_UNTRUST_SELECTED=$(safe_eval_gettext "Untrust")
    BTN_IMPORT_SELECTED=$(safe_eval_gettext "Import Selected")
    BTN_REFRESH_TRUSTED=$(safe_eval_gettext "Refresh")
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
                # Same button code (3) as "Import" below, so it falls
                # into that handling further down.
                --button="$(safe_eval_gettext "Import"):3"
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
                # 940, not 860 - a long UID otherwise forces a horizontal
                # scrollbar across the columns.
                --width=940
                --height="$LIST_HEIGHT"
                --column="$(safe_eval_gettext "Select")"
                --column="$(safe_eval_gettext "Fingerprint")"
                --column="$(safe_eval_gettext "Key ID")"
                --column="$(safe_eval_gettext "User ID")"
                --column="$(safe_eval_gettext "Status")"
                --column="$(safe_eval_gettext "Expiration Date")"
                # GTK's type-ahead search defaults to column 1 (the Select
                # checkbox) - point it at User ID instead.
                --search-column=4
                # Fingerprint stays in the model for --untrust-key below,
                # but isn't shown - Key ID is friendlier at a glance.
                --hide-column=2
                --print-column=2
                --separator=$'\n'
                # TRANSLATORS: ${BTN_UNTRUST_SELECTED} is the translated button label - keep the placeholder as-is.
                --text="<span size='large'><b>$TITLE</b></span>\n$(safe_eval_gettext "Keys saved in ~/.gnupg/trustedkeys.gpg - check any you want to untrust, then click \"\${BTN_UNTRUST_SELECTED}\"." BTN_UNTRUST_SELECTED)"
                # Button order is array position; display order matches,
                # independent of exit code.
                --button="$BTN_DETAILS_SELECTED:4"
                --button="$BTN_UNTRUST_SELECTED:0"
                # Even/odd per yad's own EXIT STATUS rule: even needs the
                # checked rows (Untrust/Export/Details), odd doesn't
                # (Import/Refresh - Refresh operates on every saved key).
                --button="$(safe_eval_gettext "Export"):2"
                --button="$(safe_eval_gettext "Import"):3"
                --button="$BTN_REFRESH_TRUSTED:5"
                --button="$(safe_eval_gettext "Close"):1"
            )

            # FPR|VALIDITY|UID|KNOWN|EXPIRE, one line per key. KNOWN flags a
            # fingerprint also on the hardcoded MX/antiX allow-list.
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

        # 0 = Untrust Selected, handled straight after this case. 2/3/4
        # are handled inline here, then loop back. Anything else (1 =
        # Close, Escape) returns to the caller.
        case "$LIST_RC" in
            0) : ;;
            4)
                # Details.
                [ -n "$SELECTED" ] || { yad_error "$(safe_eval_gettext "No key was checked - nothing to show details for.")"; continue; }
                # Same yad checklist blank-line-between-entries quirk as Export/Import below.
                SELECTED_FPRS=()
                while IFS= read -r fpr; do
                    [ -n "$fpr" ] || continue
                    SELECTED_FPRS+=("$fpr")
                done <<< "$SELECTED"

                DETAILS_TEXT=""
                DETAILS_TOTAL=${#SELECTED_FPRS[@]}
                DETAILS_N=0
                for fpr in "${SELECTED_FPRS[@]}"; do
                    DETAILS_N=$((DETAILS_N + 1))
                    uid=$(printf '%s\n' "$LISTING" | awk -F'|' -v f="$fpr" '$1 == f { print $3; exit }')
                    # $uid appended in plain bash, not as a gettext
                    # placeholder - a raw UID's "<email>" would otherwise
                    # look like an unmatched markup tag to
                    # safe_eval_gettext()'s tag-consistency check.
                    # TRANSLATORS: ${DETAILS_N}/${DETAILS_TOTAL} are literal numbers - keep placeholders as-is.
                    DETAILS_TEXT="$DETAILS_TEXT$(safe_eval_gettext "Key \${DETAILS_N} of \${DETAILS_TOTAL}" DETAILS_N DETAILS_TOTAL) - $uid
$(printf '%.0s-' {1..70})
"
                    SHOW_KEY_DETAILS_FPRS=("$fpr")
                    if OUT=$(show_key_details 2>&1); then
                        DETAILS_TEXT="$DETAILS_TEXT$OUT

"
                    else
                        DETAILS_TEXT="$DETAILS_TEXT$(safe_eval_gettext "(could not read details: \${OUT})" OUT)

"
                    fi
                done

                DETAILS_HEIGHT=$(( 220 + DETAILS_TOTAL * 110 ))
                [ "$DETAILS_HEIGHT" -gt 600 ] && DETAILS_HEIGHT=600
                DETAILS_ARGS=(
                    --text-info
                    --wrap
                    "${SELECTABLE_LABELS_ARGS[@]}"
                    --center
                    --title="$(safe_eval_gettext "Key Details")"
                    --class="$WM_CLASS"
                    --window-icon="$ICON_FILE"
                    --width=760
                    --height="$DETAILS_HEIGHT"
                    --text="<b>$(safe_eval_gettext "Key Details")</b>"
                    --button="$(safe_eval_gettext "Close"):1"
                )
                # No --formatted: gpg's raw "Name <email>" text would
                # otherwise be parsed as (and break on) Pango markup.
                # Herestring, not a pipe: old yad mismanages a GLib
                # IO-watch source ID for a live anonymous pipe.
                set +e
                yad "${DETAILS_ARGS[@]}" <<< "$DETAILS_TEXT"
                set -e
                continue
                ;;
            2)
                # Export.
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
                # Import.
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

                ALREADY_TRUSTED_FPRS=$(printf '%s\n' "$LISTING" | cut -d'|' -f1)

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
                    # Pre-checked TRUE (opposite of the main list's default).
                    IMPORT_ROWS+=(TRUE "$fpr" "${fpr: -16}" "$(pango_escape "$uid")" "$status" "$expire_display")
                done <<< "$INSPECT_OUT"

                set +e
                IMPORT_CHECKED=$(yad "${IMPORT_LIST_ARGS[@]}" "${IMPORT_ROWS[@]}")
                IMPORT_LIST_RC=$?
                set -e
                [ "$IMPORT_LIST_RC" -eq 0 ] || continue
                [ -n "$IMPORT_CHECKED" ] || { yad_error "$(safe_eval_gettext "No key was checked - nothing to import.")"; continue; }

                IMPORT_CHECKED_FPRS=()
                while IFS= read -r fpr; do
                    [ -n "$fpr" ] || continue
                    IMPORT_CHECKED_FPRS+=("$fpr")
                done <<< "$IMPORT_CHECKED"
                IMPORT_TRUSTED_KEYS="$IMPORT_PATH"
                IMPORT_TRUSTED_KEYS_FPRS=("${IMPORT_CHECKED_FPRS[@]}")
                # Called directly, not via "$(import_trusted_keys ...)" -
                # command substitution forks a subshell, losing the real
                # PROCESSED/IMPORTED/CHANGED/UNCHANGED_COUNT globals it sets.
                IMPORT_ERR_FILE=$(mktemp "$SESSION_TMPDIR/import-err.XXXXXXXXXX")
                if import_trusted_keys 2>"$IMPORT_ERR_FILE" >/dev/null; then
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
            5)
                # Refresh - sweeps every key, not just checked rows.
                REFRESH_COUNT=$(printf '%s\n' "$LISTING" | grep -c .) || REFRESH_COUNT=0
                # Only names pubring.kbx/mx-gpg-keyring if each actually
                # exists here - naming an absent one would describe
                # something that was never checked.
                if [ -f "$PUBRING_KBX" ] && [ -r "$MX_GPG_KEYRING" ]; then
                    REFRESH_PROGRESS_TEXT=$(safe_eval_gettext "Checking \${REFRESH_COUNT} key(s) against pubring.kbx, mx-gpg-keyring, and a keyserver - this can take a while..." REFRESH_COUNT)
                elif [ -f "$PUBRING_KBX" ]; then
                    REFRESH_PROGRESS_TEXT=$(safe_eval_gettext "Checking \${REFRESH_COUNT} key(s) against pubring.kbx and a keyserver - this can take a while..." REFRESH_COUNT)
                elif [ -r "$MX_GPG_KEYRING" ]; then
                    REFRESH_PROGRESS_TEXT=$(safe_eval_gettext "Checking \${REFRESH_COUNT} key(s) against mx-gpg-keyring and a keyserver - this can take a while..." REFRESH_COUNT)
                else
                    REFRESH_PROGRESS_TEXT=$(safe_eval_gettext "Checking \${REFRESH_COUNT} key(s) against a keyserver - this can take a while..." REFRESH_COUNT)
                fi
                run_refresh_sweep "$REFRESH_PROGRESS_TEXT"
                SWEEP_SUMMARY=$(gui_status_field "$OUTPUT" REFRESH_TRUSTED_KEYS_DONE)
                SWEEP_CHECKED=$(printf '%s' "$SWEEP_SUMMARY" | cut -d' ' -f1)
                SWEEP_UPDATED=$(printf '%s' "$SWEEP_SUMMARY" | cut -d' ' -f2)
                SWEEP_REVOKED=$(printf '%s' "$SWEEP_SUMMARY" | cut -d' ' -f3)
                SWEEP_UNREACHABLE=$(printf '%s' "$SWEEP_SUMMARY" | cut -d' ' -f4)
                # yad's --text is always parsed as Pango markup (unlike
                # --text-info) - a raw UID's "<email>" would crash it unescaped.
                SWEEP_DISPLAY=$(pango_escape "$(printf '%s\n' "$OUTPUT" | grep -v '^\[VERIFY-ISO-SIG:\]')")
                if [ "$RC" -eq 0 ] && [ -n "$SWEEP_SUMMARY" ]; then
                    SWEEP_HEADLINE=$(safe_eval_gettext "Checked \${SWEEP_CHECKED} key(s): \${SWEEP_UPDATED} updated, \${SWEEP_REVOKED} revoked." SWEEP_CHECKED SWEEP_UPDATED SWEEP_REVOKED)
                    if [ "${SWEEP_UNREACHABLE:-0}" -gt 0 ]; then
                        SWEEP_HEADLINE="$SWEEP_HEADLINE $(safe_eval_gettext "\${SWEEP_UNREACHABLE} of \${SWEEP_CHECKED} key(s) could not be checked against any keyserver - results for those may be incomplete." SWEEP_UNREACHABLE SWEEP_CHECKED)"
                    fi
                    yad_info "$SWEEP_HEADLINE\n\n$SWEEP_DISPLAY"
                else
                    yad_error "$(safe_eval_gettext "Could not refresh trusted keys:")\n$SWEEP_DISPLAY"
                fi
                continue
                ;;
            *) return 0 ;;
        esac

        [ -n "$SELECTED" ] || { yad_error "$(safe_eval_gettext "No key was checked - nothing to untrust.")"; continue; }

        SELECTED_FPRS=()
        while IFS= read -r fpr; do
            [ -n "$fpr" ] || continue
            SELECTED_FPRS+=("$fpr")
        done <<< "$SELECTED"
        COUNT=${#SELECTED_FPRS[@]}

        UNTRUST_LIST=""
        for fpr in "${SELECTED_FPRS[@]}"; do
            uid=$(printf '%s\n' "$LISTING" | awk -F'|' -v f="$fpr" '$1 == f { print $3; exit }')
            UNTRUST_LIST="$UNTRUST_LIST${fpr: -16}  $(pango_escape "$uid")
"
        done

        # A fixed-size --text-info (scrollable), not --question's own
        # --text label, which grows/wraps unpredictably across yad
        # versions for a variable number of lines. Height scales with
        # COUNT, capped so a large selection scrolls instead.
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
            # TRANSLATORS: ${COUNT} is literal - keep the placeholder as-is.
            --text="<b>$(safe_eval_gettext "Untrust \${COUNT} selected key(s)?" COUNT)</b>\n\n$(safe_eval_gettext "This tool will stop automatically trusting the selected key(s) - whether they're one of its built-in recognized keys, or separately marked trusted in your own GnuPG keyring. This isn't permanent - trust a key again anytime (verify something signed by it and accept it, or import the key again) and it'll be trusted normally from then on.")"
            --button="$(safe_eval_gettext "Untrust"):0"
            --button="$(safe_eval_gettext "Cancel"):1"
        )
        if ! printf '%s' "$UNTRUST_LIST" | yad "${CONFIRM_ARGS[@]}"; then
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

# Used regardless of $GUI_MODE by the run-once lock check below.
FILE_ARGS=()

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
    for arg in "$@"; do
        case "$arg" in
            --debug) DEBUG=1 ;;
            # Opt-in, not default - needs yad's --paned/--plug (X11-only,
            # fixed-size geometry, see run_picker_mode() below). Reached
            # via the app menu's "Drag & Drop" action.
            --drag-and-drop) DND_MODE=1 ;;
            # Without this, --help falls through to "*" and becomes the
            # picker's prefill value instead. Plain English, unwrapped by
            # gettext, matching the CLI's own --help.
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
fi

# "Run me only once" - covers a bare launch, a file argument, and
# --manage-keys alike, via a flock'd lock file.

# Title for mode name $1 ("picker"/"manage") - may be the other mode.
mode_title() {
    case "$1" in
        picker) printf '%s' "$PICKER_TITLE" ;;
        manage) printf '%s' "$MANAGE_TITLE" ;;
    esac
}

# $1: "file" if a file argument was passed. Returns 1 (caller should
# exit) for a second instance, 0 otherwise. Opened with ">>", not ">" -
# a second process's own open must not truncate the first process's
# already-written mode before it's read.
acquire_single_instance_lock() {
    local has_file=${1:-} existing_mode other_title msg
    LOCK_FILE="${XDG_RUNTIME_DIR:-/tmp}/verify-iso-sig-$UID.lock"
    exec {LOCK_FD}>>"$LOCK_FILE"
    if flock -n "$LOCK_FD"; then
        printf '%s' "$GUI_MODE" > "$LOCK_FILE"
        return 0
    fi
    existing_mode=$(<"$LOCK_FILE")
    RAISED=0
    if [ -z "${WAYLAND_DISPLAY:-}" ] && command -v wmctrl >/dev/null 2>&1; then
        EXISTING_WID=$(wmctrl -lx 2>/dev/null | awk -v cls="$WM_CLASS" '$3 ~ cls {print $1; exit}') || true
        if [ -n "$EXISTING_WID" ]; then
            wmctrl -ia "$EXISTING_WID" >/dev/null 2>&1
            RAISED=1
        fi
    fi
    other_title=$(mode_title "$existing_mode")
    if [ "$has_file" = file ]; then
        if [ -n "$other_title" ]; then
            msg=$(safe_eval_gettext "'\${other_title}' is already open - close it first to check this file." other_title)
        else
            msg=$(safe_eval_gettext "Already running - close it first to check this file.")
        fi
    elif [ "$RAISED" -eq 1 ] && [ "$existing_mode" = "$GUI_MODE" ]; then
        msg=""
    elif [ -n "$other_title" ]; then
        msg=$(safe_eval_gettext "'\${other_title}' is already open." other_title)
    else
        msg=$(safe_eval_gettext "Already running.")
    fi
    if [ -n "$msg" ] && { [ "$RAISED" -eq 0 ] || [ "$has_file" = file ] || [ "$existing_mode" != "$GUI_MODE" ]; }; then
        # Also to stderr - a terminal launch has no notification daemon.
        printf '%s\n' "$msg" >&2
        # --app-name: without it, notify-send's own name shows instead.
        if command -v notify-send >/dev/null 2>&1; then
            notify-send --app-name="$TITLE" --icon="$ICON_FILE" "$TITLE" "$msg" >/dev/null 2>&1 || true
        fi
    fi
    return 1
}

if [ "${#FILE_ARGS[@]}" -gt 0 ]; then
    acquire_single_instance_lock file || exit 0
else
    acquire_single_instance_lock || exit 0
fi

if [ "$GUI_MODE" = picker ]; then
    # Two explicit file arguments means the caller already knows which
    # file is which - run_picker_mode() uses EXPLICIT_ISO/EXPLICIT_SIG
    # directly, skipping pick_files(), but still shows a confirmation
    # dialog before verifying. Three works the same way.
    EXPLICIT_ISO=""
    EXPLICIT_SIG=""
    EXPLICIT_CHECKSUM_FILE=""
    EXPLICIT_CHECKSUM_ALGO=""
    PREFILL_FILE=""
    case "${#FILE_ARGS[@]}" in
        0) : ;;
        1) PREFILL_FILE=${FILE_ARGS[0]} ;;
        # The classify_*_pair() functions (verify-iso-sig's own)
        # classify the pair order-independent, same as main()'s own
        # <iso-file> [sig-file] args.
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
                # launched via a file manager's "Open With".
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
            yad_error "$(safe_eval_gettext "Too many files were given - at most three are supported.")"
            exit 1
            ;;
    esac
fi

# In --drag-and-drop mode, the picker is a Form pane and a
# drag-and-drop pane swallowed into one --paned window via yad's
# --plug mechanism - buttons belong to the outer --paned dialog, not
# the plugs (a plug's own --button isn't rendered).
#
# yad's --paned runs two independent processes with no live channel
# between them, so a single-file drop kills the whole paned window and
# relaunches it with the Form pane pre-filled (PREFILL_FILE above) - a
# background watcher polls the DnD pane's own output file for this.

# Polls for a window matching $WM_CLASS, then iconifies it (X11 only).
# Called synchronously by the Help handler below, before the browser
# opens, so nothing steals focus back after it.
minimize_next_own_window() {
    local wid tries=0
    while [ "$tries" -lt 20 ]; do
        # $3 only (WM_CLASS), never $0 - a window title can
        # coincidentally contain the app's own class name too.
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
    # re-show start minimized (X11 only), then open the browser once
    # confirmed hidden. Reset to 0 right after each yad call regardless.
    local OPEN_HELP_ON_NEXT_PICKER=0

    # Dynamic width: estimate each header line's rendered pixel width
    # from its character count (Pango is proportional, so this is only
    # an approximation) and widen the picker to keep all 3 on one line -
    # a fixed width wraps differently per language.
    FORM_TITLE_LINE=$TITLE
    FORM_INTRO_LINE=$(safe_eval_gettext "This tool checks that a downloaded ISO is authentic and undamaged, using its signature file or a signed checksum listing.")
    FORM_LINE1=$(safe_eval_gettext "Pick the .iso file, or its .sig/.asc/.gpg/.sign signature file directly.")
    # TRANSLATORS: keep the <b>/</b> tags exactly as-is (they render as
    # bold text, not literal characters).
    FORM_LINE2=$(safe_eval_gettext "The other one is found automatically - <b>but only if it's in the same folder</b>.")
    # Pango tags would skew the length estimate - stripped for
    # measurement only; --text= below still uses tagged $FORM_LINE2.
    FORM_LINE2_PLAIN=${FORM_LINE2//<b>/}
    FORM_LINE2_PLAIN=${FORM_LINE2_PLAIN//<\/b>/}

    # Empirically-picked per-character pixel averages for this GTK
    # theme/font; +80 is a fixed margin for window decoration. 820 is a
    # calibrated floor giving the FL field room for a long filename.
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
                # Help/Manage Trusted Keys/About grouped left, Cancel/Verify
                # right (Verify last as yad's Enter-key default).
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
                yad "${FORM_ONLY_ARGS[@]}" "$PREFILL_FILE" >"$FORM_OUT" {LOCK_FD}>&- &
                yad_pid=$!
                minimize_next_own_window
                xdg-open "$HELP_URL" >/dev/null 2>&1 {LOCK_FD}>&- &
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
            # Fresh key each attempt - reusing one too soon after killing
            # the previous yad processes hits "cannot create shared
            # memory for key N: file already exists" (cleanup isn't instant).
            key=$((SRANDOM % 900000 + 100000))
            res_dnd=$(mktemp "$SESSION_TMPDIR/dnd.XXXXXXXXXX")
            res_form=$(mktemp "$SESSION_TMPDIR/form.XXXXXXXXXX")

            FORM_PLUG_ARGS=(
                --plug="$key"
                --tabnum=1
                --form
                # Single \n after the title (not \n\n): this fixed-size
                # splitter has no room for a blank line without clipping
                # the title behind the window decoration.
                --text="<span size='large'><b>$FORM_TITLE_LINE</b></span>\n$FORM_INTRO_LINE $FORM_LINE1\n$FORM_LINE2"
                --field="$(safe_eval_gettext "ISO or signature file"):FL"
                --class="$WM_CLASS"
                --window-icon="$ICON_FILE"
            )
            # stdout ONLY into res_form, not "2>&1": stderr noise (e.g. a
            # GTK-WARNING) would make `[ -s "$res_form" ]` below see it as
            # real content and relaunch in a tight infinite loop.
            yad "${FORM_PLUG_ARGS[@]}" "$PREFILL_FILE" > "$res_form" {LOCK_FD}>&- &
            form_pid=$!

            DND_PLUG_ARGS=(
                --plug="$key"
                --tabnum=2
                --dnd
                --text="$(safe_eval_gettext "Or drag a .iso or signature file here")"
            )
            # Same reasoning as $res_form above.
            yad "${DND_PLUG_ARGS[@]}" > "$res_dnd" {LOCK_FD}>&- &
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
            ) {LOCK_FD}>&- &
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
                # fraction - shrinking then growing the window can clamp
                # it near zero, hiding the Form pane. --fixed avoids that
                # (this window's content is fixed and small; the result
                # dialog stays resizable, unlike this one).
                --fixed
                # Help/Manage Trusted Keys/About grouped left (none is
                # part of the actual accept/reject decision for this
                # file) - yad has no GTK-style "secondary" slot to give
                # Help a real gap, but grouping keeps the decision pair
                # (Cancel/Verify) visually separate on the right, with
                # Verify last/rightmost as yad's Enter-key default.
                --button="$(safe_eval_gettext "Help"):6"
                --button="$(safe_eval_gettext "Manage Trusted Keys"):2"
                # Even exit code (per `man yad`'s EXIT STATUS: even means
                # print result) so $form still holds what was picked.
                --button="$(safe_eval_gettext "About"):4"
                --button="$(safe_eval_gettext "Cancel"):1"
                --button="$(safe_eval_gettext "Verify"):0"
            )
            if [ "$OPEN_HELP_ON_NEXT_PICKER" -eq 1 ]; then
                OPEN_HELP_ON_NEXT_PICKER=0
                set +e
                yad "${PANED_ARGS[@]}" {LOCK_FD}>&- &
                yad_pid=$!
                minimize_next_own_window
                xdg-open "$HELP_URL" >/dev/null 2>&1 {LOCK_FD}>&- &
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
            # One "file://..." URI per line - strip the prefix per line,
            # not the whole blob (a plain "${dropped#file://}" would only
            # strip the first occurrence).
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
                        EXPLICIT_CHECKSUM_FILE=$CHECKSUM_FILE_OVERRIDE
                        EXPLICIT_CHECKSUM_SIG=$CHECKSUM_SIG_PREVIEW
                        EXPLICIT_CHECKSUM_ALGO=$CHECKSUM_ALGO_OVERRIDE
                    else
                        classify_iso_sig_pair "${DROPPED_PATHS[0]}" "${DROPPED_PATHS[1]}"
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

        # Code 2 is even, so $form already holds whatever was picked -
        # carry it into PREFILL_FILE before running the manager, then
        # loop back to re-show this picker with it still filled in.
        if [ "$paned_rc" -eq 2 ]; then
            if [ -n "$form" ]; then
                PREFILL_FILE=$(printf '%s' "$form" | cut -d'|' -f1)
            fi
            run_manage_mode
            continue
        fi

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
                    xdg-open "$HELP_URL" >/dev/null 2>&1 {LOCK_FD}>&- &
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
# killable as one unit). Sets $OUTPUT/$RC, or $INTERRUPTED=1 if the
# dialog closes first. $1: progress text.
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
    ' {LOCK_FD}>&- &
    local producer_pid=$!

    # FIFO heartbeat: --pulsate only advances on new stdin lines.
    local progress_fifo
    progress_fifo=$(mktemp -u "$SESSION_TMPDIR/progress-stdin.XXXXXXXXXX")
    mkfifo "$progress_fifo"
    ( while :; do echo; sleep 0.3; done ) > "$progress_fifo" {LOCK_FD}>&- &
    local heartbeat_pid=$!

    # setsid: makes $yad_pid its own process group, so a group kill
    # below also reaches a no-exec yad wrapper's real child process.
    setsid yad "${PROGRESS_ARGS[@]}" < "$progress_fifo" 2>/dev/null {LOCK_FD}>&- &
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
        kill "$heartbeat_pid" 2>/dev/null || true
        rm -f "$progress_fifo"
        return 0
    fi

    # Work finished first - close the stale progress window.
    kill -- "-$yad_pid" 2>/dev/null || true
    wait "$yad_pid" 2>/dev/null || true
    wait "$producer_pid" 2>/dev/null || true
    OUTPUT=$(cat "$OUT_FILE")
    RC=$(cat "$RC_FILE")
    kill "$heartbeat_pid" 2>/dev/null || true
    rm -f "$progress_fifo"
}

# Shared "Trust this key?" dialog for an unrecognized-but-GOOD signature
# (direct-ISO or checksum-file) - accept runs --keep-key (no
# re-verification needed), decline forces RC=1 since "verified" here
# means verified AND trusted. Operates on OUTPUT/RC, same as run_verify().
offer_trust_unrecognized_key() {
    local dialog_text=$1 dialog_width=${2:-520} dialog_textwidth=${3:-0}
    # --text-width keeps yad's auto-height guess honest for a widened dialog.
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
            # The key is now trusted - strip the now-stale "unrecognized
            # key" notes from the displayed body, by the stable,
            # never-localized BEGIN/END tags, not the localized text.
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
    # Lives inside $SESSION_TMPDIR - verify-iso-sig's own
    # `trap 'rm -rf "$SESSION_TMPDIR"' EXIT` already cleans these up.
    local OUT_FILE RC_FILE KEY_EXPORT_FILE
    OUT_FILE=$(mktemp "$SESSION_TMPDIR/out.XXXXXXXXXX")
    RC_FILE=$(mktemp "$SESSION_TMPDIR/rc.XXXXXXXXXX")
    # Reused by a later --keep-key --from-ring call ("Trust this key")
    # so trusting it doesn't need a second keyserver round-trip.
    KEY_EXPORT_FILE=$(mktemp "$SESSION_TMPDIR/keyexport.XXXXXXXXXX")

    # Looped so the result dialog's "Check Another File" button can
    # return here with the ISO/SIG pre-filled.
    while :; do
    # EXPLICIT_ISO can be non-empty two ways: set before this loop's
    # first pass, or by pick_files() (a two-file drag-and-drop) - only
    # call pick_files() when neither has happened yet.
    if [ -z "$EXPLICIT_ISO" ]; then
        FROM_EXPLICIT_PAIR=0
        pick_files
    fi

    if [ -n "$EXPLICIT_ISO" ]; then
        FROM_EXPLICIT_PAIR=1
        ISO=$EXPLICIT_ISO
        SIG=$EXPLICIT_SIG
        PINNED_CHECKSUM_FILE=$EXPLICIT_CHECKSUM_FILE
        PINNED_CHECKSUM_ALGO=$EXPLICIT_CHECKSUM_ALGO
        # For a plain-per-file-checksum pairing, $SIG is a non-empty
        # placeholder even with no real direct signature, so the usual
        # "$SIG empty means no direct sig" inference needs this override.
        PINNED_NO_DIRECT_SIG=$EXPLICIT_NO_DIRECT_SIG
        PINNED_ISO_DERIVED_FROM_SIG=0
        # Display-only, for the confirmation dialog below.
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
        # A directory passes a plain [ -r ] check like a regular file -
        # caught here, before extension classification.
        if [ -d "$FILE_PICKED" ]; then
            yad_error "$(safe_eval_gettext "That's a folder, not a file - pick the .iso file itself (or its .sig/.asc/.gpg/.sign signature file) directly.")"
            continue
        fi

        # is_clearsigned_file() checked first: a full clearsigned message
        # needs no separate signature, used exactly as picked.
        if is_clearsigned_file "$FILE_PICKED"; then
            ISO=$FILE_PICKED
            CANDIDATE_SIG=$(sig_for_iso "$FILE_PICKED")
            # $CANDIDATE_SIG can itself be clearsigned.
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
                    if find_clearsigned_plain_checksum "$ISO"; then
                        PINNED_CHECKSUM_FILE=$CLEARSIGNED_PLAIN_CHECKSUM
                        SIG=""
                    else
                        CANDIDATE_SIG=$(sig_for_iso "$FILE_PICKED")
                        # $CANDIDATE_SIG can itself be clearsigned - use it as that instead.
                        if [ -r "$CANDIDATE_SIG" ] && is_clearsigned_file "$CANDIDATE_SIG"; then
                            PINNED_CHECKSUM_FILE=$CANDIDATE_SIG
                            SIG=""
                        else
                            SIG=$CANDIDATE_SIG
                            { [ -f "$CANDIDATE_SIG" ] && [ -r "$CANDIDATE_SIG" ]; } || PINNED_NO_DIRECT_SIG=1
                        fi
                    fi
                    ;;
            esac
        fi
    fi

    # Preserved so the precheck call below can be undone afterward:
    # resolve_checksum_listing_as_iso() may reassign the global
    # $ISO/$SIG to the resolved .iso plus a guessed "$ISO.sig" - the
    # real run below needs the signature file actually picked instead.
    ORIG_ISO=$ISO
    ORIG_SIG=$SIG

    if [ -z "$ISO" ]; then
        yad_error "$(safe_eval_gettext "No ISO or signature file was selected.")"
        continue
    fi

    # -f (not just -r) also rejects a FIFO/device/socket. Skipped for
    # PINNED_ISO_DERIVED_FROM_SIG - verify_iso()'s own SIG_WITHOUT_ISO
    # tag handles that case better.
    if [ "${PINNED_ISO_DERIVED_FROM_SIG:-0}" -ne 1 ] && ! { [ -f "$ISO" ] && [ -r "$ISO" ]; }; then
        yad_error "$(safe_eval_gettext "Cannot read ISO file:")\n$ISO"
        continue
    fi

    # PINNED_NO_DIRECT_SIG: a plain-per-file-checksum pairing with no
    # real direct signature still has a nonexistent "${ISO}.sig"
    # placeholder in $SIG - must not reject that here.
    if [ "${PINNED_NO_DIRECT_SIG:-0}" -ne 1 ] && [ -n "$SIG" ] \
       && ! { [ -f "$SIG" ] && [ -r "$SIG" ]; }; then
        yad_error "$(safe_eval_gettext "Cannot find the matching signature file:")\n$SIG"
        continue
    fi

    if [ -n "$PINNED_CHECKSUM_FILE" ] && ! { [ -f "$PINNED_CHECKSUM_FILE" ] && [ -r "$PINNED_CHECKSUM_FILE" ]; }; then
        yad_error "$(safe_eval_gettext "Cannot read the checksum-listing file:")\n$PINNED_CHECKSUM_FILE"
        continue
    fi

    # A recognized checksum-listing pairing shows "SHA:" instead of
    # "SIG:" - no direct signature file exists in this shape.
    if [ "$FROM_EXPLICIT_PAIR" -eq 1 ]; then
        LABEL_ISO=$(safe_eval_gettext "ISO:")
        ISO_BASE=$(basename "$ISO")
        CONFIRM_LONGEST_LINE=$(( ${#LABEL_ISO} + ${#ISO_BASE} ))
        if [ -n "$PINNED_CHECKSUM_FILE" ]; then
            LABEL_CHECKSUM_FILE=$(safe_eval_gettext "SHA:")
            CHECKSUM_FILE_BASE=$(basename "$PINNED_CHECKSUM_FILE")
            # Empty CONFIRM_CHECKSUM_SIG means signed inline - show the same
            # "(algo+sig)" the result screen shows, when the algo is known.
            CHECKSUM_FILE_DISPLAY=$CHECKSUM_FILE_BASE
            if [ -z "$CONFIRM_CHECKSUM_SIG" ] && [ -n "$PINNED_CHECKSUM_ALGO" ]; then
                CHECKSUM_FILE_DISPLAY="$CHECKSUM_FILE_BASE (${PINNED_CHECKSUM_ALGO%sum}+sig)"
            fi
            CONFIRM_VERIFY_TEXT="$(safe_eval_gettext "About to verify:")\n<b>$LABEL_ISO</b> $(pango_escape "$ISO_BASE")\n<b>$LABEL_CHECKSUM_FILE</b> $(pango_escape "$CHECKSUM_FILE_DISPLAY")"
            [ $(( ${#LABEL_CHECKSUM_FILE} + ${#CHECKSUM_FILE_DISPLAY} )) -gt "$CONFIRM_LONGEST_LINE" ] \
                && CONFIRM_LONGEST_LINE=$(( ${#LABEL_CHECKSUM_FILE} + ${#CHECKSUM_FILE_DISPLAY} ))
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
        # Trailing blank line for breathing room above the buttons
        # (--text-width below sizes the dialog to fit the text exactly).
        CONFIRM_VERIFY_TEXT="$CONFIRM_VERIFY_TEXT\n"
        # ~7px/char estimate, +140 fixed chrome margin, clamped to [480, 900].
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
            # yad's auto-height guess ignores --width, leaving a gap on
            # a widened dialog - --text-width keeps it honest.
            --text-width="$CONFIRM_LONGEST_LINE"
            --text="$CONFIRM_VERIFY_TEXT"
            --button="$(safe_eval_gettext "Verify"):0"
            --button="$(safe_eval_gettext "Cancel"):1"
        )
        yad "${CONFIRM_VERIFY_ARGS[@]}" || exit 0
    fi

    : > "$KEY_EXPORT_FILE"

    # Quick, network-free precheck just to pick accurate progress-dialog
    # wording - doesn't affect the real run. A key that's cached but
    # expired/revoked still counts as "not cached", matching
    # --is-cached's own exit status.
    KEEP_KEY=0; IS_CACHED=1; ALLOW_UNKNOWN=0; TRUST_KEY=0; KEEP=0
    EXPORT_KEY_TO=""; FROM_RING_OVERRIDE=""; VERIFY_AS_CHECKSUM_FILE=0
    NO_CHECKSUM_FALLBACK=0; CHECKSUM_FILE_OVERRIDE=${PINNED_CHECKSUM_FILE:-}; CHECKSUM_ALGO_OVERRIDE=${PINNED_CHECKSUM_ALGO:-}
    KEYSERVER_OPT=""; STATUS_FD=""
    if [ "${PINNED_NO_DIRECT_SIG:-0}" -eq 1 ]; then
        NO_DIRECT_SIG=1
    else
        [ -n "$SIG" ] && NO_DIRECT_SIG=0 || NO_DIRECT_SIG=1
    fi
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

    # Undo whatever the precheck just above did to $ISO/$SIG.
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
    PRIMARY_FPR=$(gui_status_field "$OUTPUT" PRIMARY_FPR)
    PRIMARY_FPR_LINE=""
    [ -n "$PRIMARY_FPR" ] && PRIMARY_FPR_LINE="\n$(safe_eval_gettext "Primary key:") <b>$(format_fingerprint "$PRIMARY_FPR")</b>"

    # Non-empty only when the checksum-file fallback kicked in.
    CHECKSUM_INFO=$(gui_status_field "$OUTPUT" CHECKSUM_FILE)
    CHECKSUM_SIG_INFO=$(gui_status_field "$OUTPUT" CHECKSUM_SIG_INFO)

    # Non-empty only when resolve_checksum_listing_as_iso() redirected to
    # the ISO the checksum listing actually mentions.
    RESOLVED_ISO=$(gui_status_field "$OUTPUT" RESOLVED_ISO)
    RESOLVED_SIG=$(gui_status_field "$OUTPUT" RESOLVED_SIG)

    # Prefer RESOLVED_ISO over the possibly-stale $ISO - computed here so
    # the trust-unrecognized-key dialog below can also use it.
    if [ -n "$RESOLVED_ISO" ]; then
        DISPLAY_ISO_PATH="$(dirname "$ISO")/$RESOLVED_ISO"
    else
        DISPLAY_ISO_PATH=$ISO
    fi
    DISPLAY_ISO=${RESOLVED_ISO:-$(basename "$ISO")}

    # Covers SIG left blank (defaulted to whichever of .sig/.asc/.gpg
    # exists); RESOLVED_SIG takes priority when present.
    if [ -n "$RESOLVED_SIG" ]; then
        DISPLAY_SIG="$(dirname "$ISO")/$RESOLVED_SIG"
    else
        DISPLAY_SIG=${SIG:-$(sig_for_iso "$DISPLAY_ISO_PATH")}
    fi

    if [ "$RC" -eq 0 ] && gui_status_has "$OUTPUT" UNRECOGNIZED_KEY; then
        CLAIMED_ID=$(gui_status_field "$OUTPUT" CLAIMED_IDENTITY)
        CLAIMED_ID_SAFE=$(pango_escape "$CLAIMED_ID")
        # Last 16 hex chars - what gpg shows as rsa3072/0x...
        KEY_ID="0x${FPR: -16}"
        FPR_PRETTY=$(format_fingerprint "$FPR")
        # TRANSLATORS: ${BTN_TRUST_THIS_KEY} is the translated button label - keep the placeholder as-is.
        TRUST_CONFIRM_SENTENCE=$(safe_eval_gettext "By clicking \"\${BTN_TRUST_THIS_KEY}\" you confirm you've checked the Key ID or fingerprint yourself and want to trust and remember this key for future checks." BTN_TRUST_THIS_KEY)
        if gui_status_has "$OUTPUT" EXPLICITLY_UNTRUSTED; then
            ISO_UNRECOGNIZED_HEADING=$(safe_eval_gettext "You've told this tool not to automatically trust this signing key anymore.")
            CHECKSUM_UNRECOGNIZED_HEADING=$(safe_eval_gettext "You've told this tool not to automatically trust this checksum-listing signing key anymore.")
        else
            ISO_UNRECOGNIZED_HEADING=$(safe_eval_gettext "This ISO's signing key isn't one this tool already recognizes.")
            CHECKSUM_UNRECOGNIZED_HEADING=$(safe_eval_gettext "This checksum-listing file's signing key isn't one this tool already recognizes.")
        fi
        # "0" means direct-ISO-signature wording, else checksum-listing.
        if [ "$(gui_status_field "$OUTPUT" UNRECOGNIZED_KEY)" = "0" ]; then
            # TRANSLATORS: keep the <b>/</b> tags exactly as-is (they render
            # as bold text, not literal characters).
            SIGCHECK_LINE=$(safe_eval_gettext "Signature check: <b>GOOD</b> - the ISO exactly matches this key.")
            NOTE_SELF_DECLARED=$(safe_eval_gettext "Note: that identity is just self-declared text - whoever made the key could have typed anything there. The Key ID and fingerprint are different: they can't be faked, so they're what you can actually check against an independent source. Only trust this key if you've confirmed the Key ID or fingerprint yourself (e.g. from the respin/distro's own official site, or its keyserver listing).")
            TRUST_LABEL_ISO=$(safe_eval_gettext "ISO:")
            TRUST_LABEL_SIG=$(safe_eval_gettext "SIG:")
            DISPLAY_SIG_BASE=$(basename "$DISPLAY_SIG")
            TRUST_FILE_LINE="$TRUST_LABEL_ISO <b>$(pango_escape "$DISPLAY_ISO")</b>\n$TRUST_LABEL_SIG <b>$(pango_escape "$DISPLAY_SIG_BASE")</b>"
            TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_ISO} + ${#DISPLAY_ISO} ))
            [ $(( ${#TRUST_LABEL_SIG} + ${#DISPLAY_SIG_BASE} )) -gt "$TRUST_LONGEST_LINE" ] \
                && TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_SIG} + ${#DISPLAY_SIG_BASE} ))
            TRUST_TEXT="$ISO_UNRECOGNIZED_HEADING\n\n$TRUST_FILE_LINE\n\n$(safe_eval_gettext "Key ID:") <b>$KEY_ID</b>\n$(safe_eval_gettext "Claimed identity:") <b>$CLAIMED_ID_SAFE</b>\n$(safe_eval_gettext "Fingerprint:") <b>$FPR_PRETTY</b>$PRIMARY_FPR_LINE\n$SIGCHECK_LINE\n\n$NOTE_SELF_DECLARED\n\n$TRUST_CONFIRM_SENTENCE"
        else
            # TRANSLATORS: keep the <b>/</b> tags exactly as-is (they render
            # as bold text, not literal characters).
            SIGCHECK_LINE=$(safe_eval_gettext "Signature check: <b>GOOD</b> - the checksum listing exactly matches this key (the ISO's own hash is checked separately, next).")
            NOTE_SELF_DECLARED=$(safe_eval_gettext "Note: that identity is just self-declared text - whoever made the key could have typed anything there. The Key ID and fingerprint are different: they can't be faked, so they're what you can actually check against an independent source. Only trust this key if you've confirmed it yourself (e.g. from the distro's own official website or keyserver listing).")
            TRUST_LABEL_ISO=$(safe_eval_gettext "ISO:")
            TRUST_LABEL_SHA=$(safe_eval_gettext "SHA:")
            TRUST_FILE_LINE="$TRUST_LABEL_ISO <b>$(pango_escape "$DISPLAY_ISO")</b>\n$TRUST_LABEL_SHA <b>$(pango_escape "$CHECKSUM_INFO")</b>"
            TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_ISO} + ${#DISPLAY_ISO} ))
            [ $(( ${#TRUST_LABEL_SHA} + ${#CHECKSUM_INFO} )) -gt "$TRUST_LONGEST_LINE" ] \
                && TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_SHA} + ${#CHECKSUM_INFO} ))
            # No separate line for an inline-signed checksum listing - it
            # has no detached signature file to name.
            if [ -n "$CHECKSUM_SIG_INFO" ]; then
                TRUST_LABEL_SIG=$(safe_eval_gettext "SIG:")
                TRUST_FILE_LINE="$TRUST_FILE_LINE\n$TRUST_LABEL_SIG <b>$(pango_escape "$CHECKSUM_SIG_INFO")</b>"
                [ $(( ${#TRUST_LABEL_SIG} + ${#CHECKSUM_SIG_INFO} )) -gt "$TRUST_LONGEST_LINE" ] \
                    && TRUST_LONGEST_LINE=$(( ${#TRUST_LABEL_SIG} + ${#CHECKSUM_SIG_INFO} ))
            fi
            TRUST_TEXT="$CHECKSUM_UNRECOGNIZED_HEADING\n\n$TRUST_FILE_LINE\n\n$(safe_eval_gettext "Key ID:") <b>$KEY_ID</b>\n$(safe_eval_gettext "Claimed identity:") <b>$CLAIMED_ID_SAFE</b>\n$(safe_eval_gettext "Fingerprint:") <b>$FPR_PRETTY</b>$PRIMARY_FPR_LINE\n$SIGCHECK_LINE\n\n$NOTE_SELF_DECLARED\n\n$TRUST_CONFIRM_SENTENCE"
        fi
        # Same formula as CONFIRM_WIDTH, floor of 520 (more prose than
        # the confirm dialog, so never shrunk below its original default).
        TRUST_WIDTH=$(( TRUST_LONGEST_LINE * 7 + 140 ))
        [ "$TRUST_WIDTH" -lt 520 ] && TRUST_WIDTH=520
        [ "$TRUST_WIDTH" -gt 900 ] && TRUST_WIDTH=900
        offer_trust_unrecognized_key "$TRUST_TEXT" "$TRUST_WIDTH" "$TRUST_LONGEST_LINE"
    fi

    # For a recognized key fetched from a keyserver or found only in
    # pubring.kbx, offer to cache it in trustedkeys.gpg too. Skipped if
    # the block above already handled it.
    if [ "$RC" -eq 0 ] && ! gui_status_has "$OUTPUT" KEPT_IN_TRUSTED_GPG; then
        CLAIMED_ID=$(gui_status_field "$OUTPUT" CLAIMED_IDENTITY)
        CLAIMED_ID_SAFE=$(pango_escape "$CLAIMED_ID")
        KEY_ID="0x${FPR: -16}"
        FPR_PRETTY=$(format_fingerprint "$FPR")
        KEEP_KEY_DETAILS="$(safe_eval_gettext "Key ID:") <b>$KEY_ID</b>\n$(safe_eval_gettext "Claimed identity:") <b>$CLAIMED_ID_SAFE</b>\n$(safe_eval_gettext "Fingerprint:") <b>$FPR_PRETTY</b>$PRIMARY_FPR_LINE"
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

    if gui_status_has "$OUTPUT" FETCHING && gui_status_has "$OUTPUT" KEPT_IN_TRUSTED_GPG; then
        KEY_SOURCE=$(safe_eval_gettext "key fetched and saved locally - future checks won't need the network")
    elif gui_status_has "$OUTPUT" ALREADY_IN_TRUSTED_GPG; then
        KEY_SOURCE=$(safe_eval_gettext "used an already-cached key, no network needed")
    elif gui_status_has "$OUTPUT" ALREADY_IN_PUBRING; then
        KEY_SOURCE=$(safe_eval_gettext "used a key from your personal keyring (pubring.kbx)")
    elif gui_status_has "$OUTPUT" ALREADY_IN_MX_GPG_KEYRING; then
        KEY_SOURCE=$(safe_eval_gettext "used a key from mx-gpg-keyring")
    elif gui_status_has "$OUTPUT" KEPT_IN_TRUSTED_GPG; then
        KEY_SOURCE=$(safe_eval_gettext "key saved locally for faster future checks")
    elif [ "$RC" -eq 0 ] && gui_status_has "$OUTPUT" FETCHING; then
        # FETCHING only means a fetch was attempted, not that it
        # succeeded - the RC==0 guard confirms a usable key was found.
        KEY_SOURCE=$(safe_eval_gettext "fetched the signing key from a keyserver")
    else
        KEY_SOURCE=""
    fi
    # Shown regardless of outcome - an expired key still passes the
    # crypto check, but the GUI should surface this plainly.
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

    # yad's --text uses Pango markup, so a raw UID like "Name <user@host>"
    # embedded in it must be escaped first, or GTK renders the label empty.
    # $'\uXXXX' keeps the source file itself pure ASCII.
    CHECK_MARK=$'\u2713'
    CROSS_MARK=$'\u2717'
    # U+26A0 WARNING SIGN had almost no contrast on a dark theme -
    # U+26D4 NO ENTRY is a solid shape instead.
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
    # checksum listing and its signature were used instead.
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
    if [ "$RC" -ne 0 ]; then
        if gui_status_has "$OUTPUT" KEY_REVOKED; then
            NOTE=$(safe_eval_gettext "This signing key has been revoked - refusing to use it, whatever the reason (a real key compromise, or the owner deliberately retiring it). Do not trust this file.")
        elif gui_status_has "$OUTPUT" KEY_FETCH_FAILED; then
            NOTE=$(safe_eval_gettext "The signing key could not be fetched from any keyserver - this looks like a network problem, not necessarily a bad signature. Try again once you have a working connection.")
        elif gui_status_has "$OUTPUT" GUI_DECLINED_TRUST; then
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
            # CHECKSUM_FILE_OVERRIDE is only set inside run_verify()'s own
            # subshell - use $ISO/$PINNED_CHECKSUM_FILE instead.
            CHECKSUM_FILE_SAFE=$(pango_escape "$(basename "${PINNED_CHECKSUM_FILE:-$ISO}")")
            # TRANSLATORS: ${CHECKSUM_FILE_SAFE} is the checksum-listing file's name - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This checksum listing (\${CHECKSUM_FILE_SAFE}) has no signature file of its own (.sig/.asc/.gpg/.sign) next to it - it can't be trusted without one, so it was never checked against this ISO." CHECKSUM_FILE_SAFE)
        elif gui_status_has "$OUTPUT" CHECKSUM_LISTING_FOUND_UNSIGNED; then
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
            NOTE=$(safe_eval_gettext "This checksum file isn't signed, and the ISO it describes has no signature of its own either - there's nothing here this tool can cryptographically verify. Check whether the distro provides a signed checksum listing, a direct .sig/.asc/.gpg file, or a self-contained clearsigned checksum (e.g. '.sha512.asc') for this ISO.")
        elif gui_status_has "$OUTPUT" CHECKSUM_KEY_FETCH_FAILED; then
            NOTE=$(safe_eval_gettext "The checksum listing's signing key could not be fetched from any keyserver - nothing in it could be checked. This looks like a network problem, not necessarily a bad signature. Try again once you have a working connection.")
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
            # Only reachable via CLI args or a two-file drag-and-drop.
            # TRANSLATORS: ${BTN_CHECK_ANOTHER_FILE} is a translated button label - keep the placeholder as-is.
            NOTE=$(safe_eval_gettext "This signature file's name doesn't match this ISO's name, so it likely belongs to a different download. Use \"\${BTN_CHECK_ANOTHER_FILE}\" and pick just this ISO, or just its own real signature file - the tool finds the matching one automatically." BTN_CHECK_ANOTHER_FILE)
        elif [ "$(gui_status_field "$OUTPUT" UNRECOGNIZED_KEY)" = "0" ]; then
            NOTE=$(safe_eval_gettext "On top of the signature not matching, this signing key also isn't one this tool already recognizes - treat this file as untrustworthy and re-download from an official source.")
        else
            NOTE=$(safe_eval_gettext "The ISO may be corrupted, or the download was incomplete or tampered with - try re-downloading it, ideally from a different mirror.")
        fi
        HEADING="$HEADING\n\n<i>$NOTE</i>"
    fi

    if gui_status_has "$OUTPUT" WEAK_CHECKSUM_ALGO; then
        WEAK_ALGO_SAFE=$(pango_escape "$(gui_status_field "$OUTPUT" WEAK_CHECKSUM_ALGO)")
        # TRANSLATORS: ${WEAK_ALGO_SAFE} is a literal algorithm name (e.g. "md5") - keep the placeholder as-is.
        WEAK_ALGO_NOTE=$(safe_eval_gettext "This relies on \${WEAK_ALGO_SAFE}, a broken checksum algorithm - a matching hash doesn't prove the ISO is genuine. Prefer a real signature or a stronger checksum (SHA256/SHA512) when available." WEAK_ALGO_SAFE)
        HEADING="$HEADING\n\n<i>$WEAK_ALGO_NOTE</i>"
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
    # for a live anonymous pipe into --text-info.
    DISPLAY_OUTPUT=$(printf '%s\n' "$OUTPUT" | grep -v '^\[VERIFY-ISO-SIG:\]' || true)
    set +e
    yad "${RESULT_ARGS[@]}" <<< "$DISPLAY_OUTPUT"
    RESULT_RC=$?
    set -e

    # "Check Another File" loops back pre-filled with the ISO just used.
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

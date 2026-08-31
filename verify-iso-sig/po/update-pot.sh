#!/usr/bin/bash
# Regenerates verify-iso-sig.pot from both scripts' gettext calls plus
# verify-iso-sig.desktop.in's own Name=/Comment=/Keywords= fields, then
# merges into every po/*.po (msgmerge marks changed entries "fuzzy",
# never overwrites a translation).
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR/.."

# --keyword=safe_eval_gettext: xgettext's Shell backend doesn't know our
# own wrapper by default - without it, every string passed through
# safe_eval_gettext() is silently dropped, no warning.
# --language=Shell forced explicitly: verify-iso-sig (the dispatcher)
# has no .sh suffix for extension-based auto-detection to key off.
# --no-location: suppresses "#: file:line" comments - pure diff noise
# on every regen, since line numbers shift with unrelated code edits.
xgettext --language=Shell --from-code=UTF-8 --add-comments=TRANSLATORS: \
    --no-location \
    --keyword=safe_eval_gettext \
    --package-name=verify-iso-sig \
    -o po/verify-iso-sig.pot verify-iso-sig-gui.sh verify-iso-sig

# --join-existing: xgettext only extracts one language per invocation -
# merges the .desktop.in template's Name=/Comment=/Keywords= (main entry
# and every [Desktop Action] stanza) into the same .pot instead of
# overwriting the pass above.
xgettext --join-existing --language=Desktop --no-location \
    -o po/verify-iso-sig.pot verify-iso-sig.desktop.in

for po in po/*.po; do
    [ -e "$po" ] || continue
    msgmerge --update --backup=off "$po" po/verify-iso-sig.pot
done

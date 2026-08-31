#!/usr/bin/bash
# Regenerates the .pot/.po catalogs via update-pot.sh, then reports new
# msgids, removed msgids, and any fuzzy/untranslated msgstr per language.
#
# Usage: po/check-translations.sh
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR/.."

OLD_POT=$(mktemp)
cp po/verify-iso-sig.pot "$OLD_POT"

bash po/update-pot.sh >/dev/null

python3 - "$OLD_POT" po/verify-iso-sig.pot <<'PYEOF'
import re
import sys


def extract_msgids(path):
    text = open(path, encoding="utf-8").read()
    ids = set()
    # Each entry: one or more consecutive quoted lines right after the
    # msgid keyword, concatenated - matches how xgettext wraps long strings.
    for m in re.finditer(r'^msgid((?:\s+"(?:[^"\\]|\\.)*")+)', text, re.M):
        parts = re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(1))
        ids.add("".join(parts))
    ids.discard("")  # the header entry's own empty msgid
    return ids


old_ids = extract_msgids(sys.argv[1])
new_ids = extract_msgids(sys.argv[2])

added = sorted(new_ids - old_ids)
removed = sorted(old_ids - new_ids)

if added:
    print(f"=== {len(added)} new msgid(s) (need translating) ===")
    for s in added:
        print(f"  + {s!r}")
if removed:
    print(f"\n=== {len(removed)} removed msgid(s) (no longer used anywhere) ===")
    for s in removed:
        print(f"  - {s!r}")
    print(
        "\n  Review before assuming these are safe to lose - a msgid changed"
        "\n  by even a typo shows up here as a remove+add pair, not a single"
        "\n  \"changed\" entry. If one of the added strings above is really the"
        "\n  same string with a small fix, consider re-pointing that language's"
        "\n  existing translation instead of starting fresh from scratch."
    )
if not added and not removed:
    print("No msgid changes.")
PYEOF

rm -f "$OLD_POT"

echo
echo "=== per-language status ==="
STATUS=0
for po in po/*.po; do
    lang=$(basename "$po" .po)
    fuzzy=$(grep -c '^#, fuzzy' "$po" || true)
    # msgattrib --untranslated always includes the header entry's own
    # msgid "" - subtract 1 to get the real gap count.
    untranslated=$(msgattrib --untranslated "$po" 2>/dev/null | grep -c '^msgid "' || true)
    real_untranslated=$(( untranslated > 0 ? untranslated - 1 : 0 ))
    if [ "$fuzzy" -gt 0 ] || [ "$real_untranslated" -gt 0 ]; then
        STATUS=1
        echo "$lang: $fuzzy fuzzy, $real_untranslated untranslated - NEEDS ATTENTION"
    else
        echo "$lang: clean"
    fi
done

exit "$STATUS"

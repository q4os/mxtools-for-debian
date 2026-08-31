#!/usr/bin/bash
# Regenerates verify-iso-sig.desktop (the installed file) from
# verify-iso-sig.desktop.in plus every po/*.po. Requires po/LINGUAS -
# without it, msgfmt silently produces an untranslated copy, no error.
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR/.."

msgfmt --desktop --template=verify-iso-sig.desktop.in -d po \
    -o verify-iso-sig.desktop

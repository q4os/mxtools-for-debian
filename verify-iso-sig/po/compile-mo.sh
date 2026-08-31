#!/usr/bin/bash
# Compiles every po/<lang>.po into locale/<lang>/LC_MESSAGES/verify-iso-sig.mo.
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "$SCRIPT_DIR/.."

for po in po/*.po; do
    [ -e "$po" ] || continue
    lang=$(basename "$po" .po)
    mkdir -p "locale/$lang/LC_MESSAGES"
    msgfmt "$po" -o "locale/$lang/LC_MESSAGES/verify-iso-sig.mo"
done

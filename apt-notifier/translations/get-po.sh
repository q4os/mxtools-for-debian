#!/bin/bash

echodo() {
  echo "${@}"
  ${@}
}

[ -x tx_bin/tx ] && TXBIN=tx_bin/tx
: ${TXBIN:=$(which tx)}

[ ! -x "$TXBIN" ] && echo "Error: transifex client not found!" && exit 1

# prepare transifex 
mkdir -p .tx
cat <<EOF > .tx/config
[main]
host = https://www.transifex.com

[o:anticapitalista:p:antix-development:r:apt-notifier]

file_filter = po/<lang>.po
minimum_perc = 0
source_file = apt-notifier.pot
source_lang = en
type = PO

EOF

# backup existing
[ -d po ] && echodo mv po po_$(date '+%Y-%m-%d_%H%M%S').bak
mkdir po

# get all translations
RESOURCE="antix-development.apt-notifier"
echodo ${TXBIN} pull --force  --all "$RESOURCE"


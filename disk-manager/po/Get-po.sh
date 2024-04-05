#!/bin/bash


# set transifex organization and project name - if not set in environment already
PACKAGE="disk-manager"
POTFILE=en.pot
PODIR=.
MINIMUM_PERC=5
: ${ORGANIZATION:=anticapitalista}
: ${PROJECT:=antix-development}
: ${RESOURCE:=$PACKAGE}
RESOUCE_ID="${PROJECT}.${RESOURCE}"

# prepare transifex 
if ! command -v tx >/dev/null; then
   echo "Need: transifex client 'tx' - exit"; exit 1
fi
[ -d .tx ] || mkdir -p .tx
[ -f .tx/config ] &&  rm .tx/config

cat <<EOF > .tx/config
[main]
host = https://www.transifex.com

[o:${ORGANIZATION}:p:${PROJECT}:r:${RESOURCE}]

file_filter   = ${PODIR}/<lang>.po
minimum_perc  = ${MINIMUM_PERC}
resource_name = ${RESOURCE}
source_file   = ${POTFILE}
source_lang   = en
type          = PO

EOF

echodo() { echo "${@}"; ${@}; }

# backup existing
DATE=$(date '+%Y-%m-%d_%H%M%S')
for PO in $(ls $PODIR/*.po 2>/dev/null); do mv $PO $PO.$DATE.bak~; done

# get all translations
echodo tx pull --translations --all "$RESOUCE_ID"

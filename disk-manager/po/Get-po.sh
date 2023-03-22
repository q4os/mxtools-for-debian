#!/bin/bash


# prepare transifex 
#if [ ! -s  .tx/config ]; then
   mkdir -p .tx
   cat <<EOF > .tx/config
[main]
host = https://www.transifex.com

[antix-development.disk-manager]
file_filter = <lang>.po
minimum_perc = 1
source_file = en.pot
source_lang = en
type = PO
EOF
#fi    

echodo() { echo "${@}"; ${@}; }

# backup existing
[ -d po ] && echodo mv po po_$(date '+%Y-%m-%d_%H%M%S').bak

# get all translations
# remove existing
rm *.po 
if command -v tx >/dev/null; then
   #rm en.pot 
   #echodo tx pull -r antix-development.disk-manager  -s --all
   echodo tx pull -r antix-development.disk-manager  --all
fi


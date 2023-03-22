#!/bin/bash

# creates apt-notifer.pot file 

MODULES=(
    ../lib/modules/apt-notifier.py  
    ../lib/modules/aptnotifier_xlate.py
    )

DESKTOPS=(
    ../xdg/apt-notifier-autostart.desktop.in
    ../xdg/apt-notifier.desktop.in
    )

POLICIES=(
    ../lib/policy/org.mxlinux.apt-notifier.auto-update-enable.policy
    ../lib/policy/org.mxlinux.apt-notifier.auto-update-disable.policy
    )

PKGNAME=apt-notifier
POTFILE=en.pot

echodo() { local run="$1"; shift; echo "$run" "${@@Q}"; "$run" "$@"; }

OPTS="--no-wrap --sort-output --no-location --package-name=$PKGNAME"
echodo xgettext $OPTS -L Python -cTRANSLATORS: -o $POTFILE "${MODULES[@]}"

OPTS="--no-wrap --join-existing --no-location --package-name=$PKGNAME"
echodo xgettext $OPTS --add-comments -L Desktop -o $POTFILE "${DESKTOPS[@]}"
sed -i 's/charset=CHARSET/charset=UTF-8/'  $POTFILE

for P in "${POLICIES[@]}"; do
    msgid="$(grep -m1 -oP '<message[^>]*>\K[^<]+' $P)"
    [[ -n $msgid ]] &&  printf '\nmsgid "%s"\nmsgstr ""\n' "$msgid" | tee -a $POTFILE
done

# put addtionaly msgid's into en-extra.pot
EXTRA=en-extra.pot
TEMPO=$EXTRA.tmp

OPTS="--no-wrap --sort-output --no-location --package-name=$PKGNAME"
echodo xgettext $OPTS --add-comments -kComment -L Desktop -o $TEMPO "${DESKTOPS[@]}"
sed -i 's/charset=CHARSET/charset=UTF-8/'  $TEMPO

for P in "${POLICIES[@]}"; do
    msgid="$(grep -m1 -oP '<message[^>]*>\K[^<]+' $P)"
    [[ -n $msgid ]] &&  printf '\nmsgid "%s"\nmsgstr ""\n' "$msgid" | tee -a $TEMPO
done

msggrep -v --no-wrap -K -e '^MX Updater$' -o $EXTRA $TEMPO
rm $TEMPO

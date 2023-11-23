#!/bin/bash
# Filename:      conkytoggleflux.sh
# Purpose:       toggle conky on/off from fluxbox menu
# Authors:       Kerry and anticapitalista, secipolla for antiX
# Authors:       modified for mx linux version 17 by dolphin oracle
# Latest change: Sun December 10, 2017.
################################################################################
# Adjusted by fehlix, December 2022 

main()
{
    local user=$(id -nu)
    if [ $(pgrep -c -u "$user" -x conky) != 0 ];  then
        /usr/bin/killall -u "$user" conky
        autostart_off
    else
        if grep -q "conky -c" "$HOME"/.conky/conky-startup.sh 2>/dev/null; then
            launch_conky
            autostart_on
        else
            if command -v conky-manager2 >/dev/null; then
                conky-manager2 &
            elif command -v conky-manager >/dev/null; then
                conky-manager &
            fi
        fi
    fi
}

launch_conky()
{

CONKY_TEMP=$(mktemp --tmpdir=${XDG_RUNTIME_DIR:-/tmp} conky-startup.sh.XXXXXXXXXXXX)

/usr/bin/sed -e 's/^[[:space:]]*sleep.*/sleep 1s/' "$HOME"/.conky/conky-startup.sh > $CONKY_TEMP

sh $CONKY_TEMP

rm $CONKY_TEMP

}

autostart_off()
{
if [ ! -e "$HOME"/.config/autostart/conky.desktop ]; then
    /usr/bin/cp /usr/share/conky-toggle/conky.desktop "$HOME"/.config/autostart/conky.desktop
fi
/usr/bin/sed -i -r s/Hidden=.*/Hidden=true/ "$HOME"/.config/autostart/conky.desktop

}

autostart_on()
{
if [ ! -e "$HOME"/.config/autostart/conky.desktop ]; then
   /usr/bin/cp /usr/share/conky-toggle/conky.desktop "$HOME"/.config/autostart/conky.desktop
fi
/usr/bin/sed -i -r s/Hidden=.*/Hidden=false/ "$HOME"/.config/autostart/conky.desktop

}

main
exit 0

#!/bin/bash
# Filename:      conkytoggleflux.sh
# Purpose:       toggle conky on/off from fluxbox menu
# Authors:       Kerry and anticapitalista, secipolla for antiX
# Authors:       modified for mx linux version 17 by dolphin oracle
# Latest change: Sun December 10, 2017.
################################################################################
# Adjusted by fehlix, December 2022 
# Adjusted for new mx-conky July 2025

main()
{
#migrate desktop autostart file if necessary
    if [ -f "$HOME/.config/autostart/conky.desktop" ]; then
        sed -i 's#.conky/conky-startup.sh#/usr/share/mx-conky-data/conky-startup.sh#' "$HOME/.config/autostart/conky.desktop"
    fi
    local user=$(id -nu)
    if [ $(pgrep -c -u "$user" -x conky) != 0 ];  then
        /usr/bin/killall -u "$user" conky
        autostart_off
        exit 0
    fi
    #check for empty user .conky/conky-startup.sh
    if [ -f "$HOME"/.conky/conky-startup.sh ]; then
        if grep -q "conky -c" "$HOME"/.conky/conky-startup.sh 2>/dev/null; then
            launch_conky
            autostart_on
        else
			version_check=$(dpkg-query -W -f='${Version}\n' mx-conky |cut -d"." -f1)
			echo $version_check
			min_version=25  # Target as an integer

			if [[ $version_check -ge $min_version ]]; then
				if command -v mx-conky >/dev/null; then
					mx-conky &
					exit 0
				fi    
            else
            	if command -v conky-manager2 >/dev/null; then
                	conky-manager2 &
                	exit 0
            	elif command -v conky-manager >/dev/null; then
                	conky-manager &
                	exit 0
            	fi
        	fi
    	fi
    else
		echo "launch"
		autostart_on
		launch_conky
	fi
}

launch_conky()
{

#check for existing conky-startup.sh file

FILE="$HOME/.conky/conky-startup.sh"

if [ ! -f "$HOME/.conky/conky-startup.sh" ]; then
	FILE="/usr/share/mx-conky-data/conky-startup.sh"
fi

echo $FILE
CONKY_TEMP=$(mktemp --tmpdir=${XDG_RUNTIME_DIR:-/tmp} conky-startup.sh.XXXXXXXXXXXX)

/usr/bin/sed -e 's/^[[:space:]]*sleep.*/sleep 1s/' "$FILE" > $CONKY_TEMP

sh $CONKY_TEMP

rm $CONKY_TEMP

}

autostart_off()
{
if [ ! -f "$HOME"/.config/autostart/conky.desktop ]; then
    /usr/bin/cp /usr/share/conky-toggle/conky.desktop "$HOME"/.config/autostart/conky.desktop
fi
/usr/bin/sed -i -r s/Hidden=.*/Hidden=true/ "$HOME"/.config/autostart/conky.desktop

}

autostart_on()
{
if [ ! -f "$HOME"/.config/autostart/conky.desktop ]; then
   /usr/bin/cp /usr/share/conky-toggle/conky.desktop "$HOME"/.config/autostart/conky.desktop
fi
/usr/bin/sed -i -r s/Hidden=.*/Hidden=false/ "$HOME"/.config/autostart/conky.desktop

}

main
exit 0

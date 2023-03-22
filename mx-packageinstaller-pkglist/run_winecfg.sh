#!/bin/bash

#run winecfg as part of mx-packageinstaller
#must run winecfg as user

launch_user=$(logname)
echo "run wine config as user:$launch_user"
#run winecfg as user
if [ "$launch_user" = "" ]; then
    yad --window-icon=/usr/share/pixmaps/mx-packageinstaller.png --width=300 --align=center --form --title="MX-Packageinstaller" --button=gtk-ok --buttons-layout=center --field="<b>Run winecfg from terminal as regular user to finish installation</b>":LBL ""
    exit 0
else
    su $launch_user -c 'bash -c "/usr/bin/winecfg  2>/dev/null 1>/dev/null & disown"'
fi
exit 0

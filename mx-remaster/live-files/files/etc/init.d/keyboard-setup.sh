#!/bin/sh
### BEGIN INIT INFO
# Provides:          keyboard-setup.sh
# Required-Start:    mountkernfs
# Required-Stop:
# X-Start-Before:    checkroot
# Default-Start:     S
# Default-Stop:
# X-Interactive:     true
# Short-Description: Set the console keyboard layout
# Description:       Set the console keyboard as early as possible
#                    so during the file systems checks the administrator
#                    can interact.  At this stage of the boot process
#                    only the ASCII symbols are supported.
### END INIT INFO

live_files_dir="/usr/share/live-files/general-files"
def_files="keyboard"

# return true if the files are the same
no_diff() { diff -q "$@" >/dev/null 2>&1 ; return $? ;}

case $1 in start)
    if mountpoint -q /live/aufs; then

        # First, fix the console font ASAP
        prog=/live/bin/set-console-width
        test -x $prog && $prog --auto keyboard-setup.sh

        # Always run setupcon -k if the keyboard file is not the default
        for file in $def_files; do
            a=/etc/default/$file
            b=$live_files_dir$a
            [ -e $a -a -e $b ] || continue
            #echo no_diff $a $b
            no_diff $a $b && continue

            . /lib/lsb/init-functions

	        log_action_begin_msg "Setting up live keyboard layout"
            /bin/setupcon -k
	        log_action_end_msg $?

            break
        done
        exit 0
    fi
esac


if [ -f /bin/setupcon ]; then
    case "$1" in
        stop|status)
        # console-setup isn't a daemon
        ;;
        start|force-reload|restart|reload)
            if [ -f /lib/lsb/init-functions ]; then
                . /lib/lsb/init-functions
            else
                log_action_begin_msg () {
	            echo -n "$@... "
                }

                log_action_end_msg () {
	            if [ "$1" -eq 0 ]; then
	                echo done.
	            else
	                echo failed.
	            fi
                }
            fi
	    log_action_begin_msg "Setting up keyboard layout"
            if /lib/console-setup/keyboard-setup.sh; then
	        log_action_end_msg 0
	    else
	        log_action_end_msg $?
	    fi
	    ;;
        *)
            echo 'Usage: /etc/init.d/keyboard-setup {start|reload|restart|force-reload|stop|status}'
            exit 3
            ;;
    esac
fi

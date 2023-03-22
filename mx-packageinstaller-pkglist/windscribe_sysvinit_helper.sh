#!/bin/bash

#  windscribe_systemctl_helper

echodo() {
	echo "$@"
	"$@"
}

systemctl() {
	local ret
	if [ -d /run/systemd/system ]; then
	   echodo /usr/bin/systemctl "$@"
	   ret=$?
	   return $ret
	fi
	local action="$1"
	shift $(($# - 1))
	local service="$1"
	case "$action" in
	   start|stop|status) 
			echodo service $service $action
			ret=$?
		    ;;
	   disable)
			echodo update-rc.d $service remove
			ret=$?
			;;
	   enable)
			echodo update-rc.d $service remove
			echodo update-rc.d $service defaults
			ret=$?
			;;
	   is-active)
			sysv-rc-conf  --list $service | grep -q ':on'
			ret=$?
			;;
		*)
			echodo /usr/bin/systemctl "$@"
			ret=$?
			;;
	esac
    return $ret
}

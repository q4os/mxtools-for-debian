#!/bin/bash

#  windscribe_systemctl_helper

if [ -d /run/systemd/system ]; then
    :
else

echodo() {
    echo "$@"
    "$@"
}

systemctl() {
    local ret
    local arg=( "$@" ) 
    local action="$1"
    shift $(($# - 1))
    local service="$1"
    case "$action" in
       start|stop|status) 
            echodo service $service $action || :
            ret=$?
            ;;
       disable)
            echodo update-rc.d $service remove || :
            ret=$?
            ;;
       enable|preset)
            echodo update-rc.d $service remove    || :
            echodo update-rc.d $service defaults  || :
            ret=$?
            ;;
       is-active)
            sysv-rc-conf  --list $service | grep -q ':on'
            ret=$?
            ;;
        *)
            echodo /usr/bin/systemctl "${arg[@]}"
            ret=$?
            ;;
    esac
    return $ret
}

fi

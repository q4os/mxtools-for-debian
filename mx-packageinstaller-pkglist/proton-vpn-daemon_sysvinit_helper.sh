#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: systemctl shim providing sysvinit compatibility for
# me.proton.vpn.split_tunneling.service, sourced into proton-vpn-daemon's
# maintainer scripts at install time.

function systemctl {
local action=$1
shift
local service="${1:-}"
local initservice="${service##*/}"
initservice="${initservice%.service}"
local unitfile="/usr/lib/systemd/system/${initservice}.service"
local wantedby

case $action in
  disable)
     if [ -x /usr/bin/systemctl ]; then
        /usr/bin/systemctl --root=/ disable "${initservice}.service" 2>/dev/null || true
     fi
     _SKIP_SYSTEMD_NATIVE=1 update-rc.d "$initservice" remove || true
     rm /etc/rc[S0-6].d/[SK][0-9][0-9]"$initservice" 2>/dev/null || true
     wantedby=$(sed -n "s/^WantedBy=//p" "$unitfile" 2>/dev/null | head -1)
     if [ -n "$wantedby" ]; then
        rm -f "/etc/systemd/system/${wantedby}.wants/${initservice}.service" 2>/dev/null || true
        rmdir "/etc/systemd/system/${wantedby}.wants" 2>/dev/null || true
     fi
     if [ -L "/etc/systemd/system/${initservice}.service" ]; then
        rm -f "/etc/systemd/system/${initservice}.service" 2>/dev/null || true
     fi
     if [ -d /run/systemd/system ] && [ -x /usr/bin/systemctl ]; then
        /usr/bin/systemctl daemon-reload 2>/dev/null || true
     fi
     return 0
     ;;
  enable)
     _SKIP_SYSTEMD_NATIVE=1 update-rc.d "$initservice" remove
     rm /etc/rc[S0-6].d/[SK][0-9][0-9]"$initservice" 2>/dev/null || true
     _SKIP_SYSTEMD_NATIVE=1 update-rc.d "$initservice" defaults
     if [ -L "/etc/systemd/system/${initservice}.service" ]; then
        rm -f "/etc/systemd/system/${initservice}.service" 2>/dev/null || true
     fi
     if [ -x /usr/bin/systemctl ]; then
        /usr/bin/systemctl --root=/ enable "$unitfile" 2>/dev/null || true
     fi
     wantedby=$(sed -n "s/^WantedBy=//p" "$unitfile" 2>/dev/null | head -1)
     if [ -n "$wantedby" ]; then
        mkdir -p "/etc/systemd/system/${wantedby}.wants"
        ln -sf "$unitfile" "/etc/systemd/system/${wantedby}.wants/${initservice}.service"
     fi
     if [ -d /run/systemd/system ] && [ -x /usr/bin/systemctl ]; then
        /usr/bin/systemctl daemon-reload 2>/dev/null || true
     fi
     return 0
     ;;
  is-system-running)
     if [ -d /run/systemd/system ] && [ -x /usr/bin/systemctl ]; then
        /usr/bin/systemctl is-system-running
     else
        echo "running"
     fi
     return 0
     ;;
esac

if [ -d /run/systemd/system ] && [ -x /usr/bin/systemctl ]; then
   /usr/bin/systemctl "$action" "$@"
   return $?
else
   case $action in
      start|stop|status) invoke-rc.d "$initservice" $action
      ;;
   esac
   return $?
fi
}

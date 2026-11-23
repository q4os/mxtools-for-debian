#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: shared install/uninstall helper logic for the "PIAVPN" entry.
# Called by piavpn.pm with a stage name; not meant to be run manually.

usage() {
    cat <<'EOF'
piavpn.sh - install/uninstall helper for Private Internet Access VPN (MX Package Installer)

Usage: piavpn.sh preinstall
       piavpn.sh postinstall
       piavpn.sh postuninstall

Called by MX Package Installer's "PIAVPN" entry (piavpn.pm). Not intended
to be run directly.
EOF
}

do_preinstall() {

# Neutralize the old package's prerm before purging - avoids running its
# real uninstaller as a side effect.
local pkg="piavpn-downloader-installer"
local prerm=/var/lib/dpkg/info/piavpn-downloader-installer.prerm
local state=

[ -f "$prerm" ] && rm -f "$prerm"
state=$(dpkg-query -W -f='${db:Status-Abbrev}' "$pkg" 2>/dev/null) || :

if [ -n "$state" ] && [ "${state:2:1}" != "n" ]; then
    apt-get -y purge "$pkg" 2>/dev/null || :
fi

}

do_postinstall() {

if command -v update-desktop-database >/dev/null 2>&1; then
   update-desktop-database -q /usr/share/applications 2>/dev/null || true
fi

if [ -x /usr/share/pia-vpn-installer-helper/run-pia-installer.sh ]; then
   echo "Open 'PIA VPN Installer' from your application menu to finish installing PIA VPN."

   # Also drop a copy on the user's Desktop - the menu entry is easy to miss.
   REAL_USER=$(id -un "${PKEXEC_UID:-${SUDO_UID:-$(id -u)}}" 2>/dev/null)
   if [ -z "$REAL_USER" ] || [ "$REAL_USER" = "root" ]; then
      REAL_USER=$(logname 2>/dev/null)
   fi
   if [ -n "$REAL_USER" ] && [ "$REAL_USER" != "root" ] && command -v xdg-user-dir >/dev/null 2>&1; then
      REAL_HOME=$(getent passwd "$REAL_USER" | cut -d: -f6)
      if [ -n "$REAL_HOME" ] && [ -d "$REAL_HOME" ]; then
         DESKTOP_DIR=$(HOME="$REAL_HOME" XDG_CONFIG_HOME="$REAL_HOME/.config" xdg-user-dir DESKTOP 2>/dev/null)
         if [ -n "$DESKTOP_DIR" ] && [ -d "$DESKTOP_DIR" ]; then
            cp /usr/share/applications/pia-vpn-installer.desktop "$DESKTOP_DIR/pia-vpn-installer.desktop" 2>/dev/null
            chmod +x "$DESKTOP_DIR/pia-vpn-installer.desktop" 2>/dev/null
            chown "$REAL_USER":"$(id -gn "$REAL_USER" 2>/dev/null)" "$DESKTOP_DIR/pia-vpn-installer.desktop" 2>/dev/null
         fi
      fi
   fi
fi

}

do_postuninstall() {

echo
echo "Uninstalling PIA VPN..."
sleep 1

# Matches pia-uninstall.sh's checkmark/cross; kept colorless since this
# isn't piped through strip_color.
echo_done() {
  printf '\xE2\x9C\x94 %s\n' "$@"
}
echo_fail() {
  printf '\xE2\x9C\x98 %s\n' "$@"
}

stop_daemon() {
  # pia-client isn't a service, so the daemon checks below can't see it -
  # but it can still hold the net_cls cgroup open.
  pkill -x pia-client 2>/dev/null || true
  sleep 1

  if command -v systemctl >/dev/null 2>&1 && [ -d /run/systemd/system ]; then
     TEXTDOMAIN=cli-installer gettext "This may take some time. Please wait...."; echo
     systemctl stop piavpn 2>/dev/null || true
     systemctl disable piavpn 2>/dev/null || true
     # systemctl stop blocks until inactive, but cgroup teardown can lag
     # slightly - poll, then a settle buffer.
     for i in $(seq 1 20); do
        systemctl is-active --quiet piavpn 2>/dev/null || break
        sleep 1
     done
     sleep 1
     if systemctl is-active --quiet piavpn 2>/dev/null; then
        echo_fail "PIA daemon did not stop in time"
     else
        echo_done "PIA daemon stopped"
     fi
     systemctl daemon-reload 2>/dev/null || true
  elif [ -x /etc/init.d/piavpn ]; then
     TEXTDOMAIN=cli-installer gettext "This may take some time. Please wait...."; echo
     service piavpn stop 2>/dev/null || true
     # piavpn's own "status" action always exits 0 regardless of state
     # (do_status() never propagates start-stop-daemon's real exit code) -
     # call start-stop-daemon --status directly instead.
     for i in $(seq 1 20); do
        start-stop-daemon --status --quiet --pidfile /var/run/piavpn.pid 2>/dev/null || break
        sleep 1
     done
     sleep 1
     if start-stop-daemon --status --quiet --pidfile /var/run/piavpn.pid 2>/dev/null; then
        echo_fail "PIA daemon did not stop in time"
     else
        echo_done "PIA daemon stopped"
     fi
  fi

  # Daemon status alone won't catch an orphaned child still attached to
  # the cgroup - check its task list directly.
  local cgroup_tasks=/opt/piavpn/etc/cgroup/net_cls/tasks
  if [ -f "$cgroup_tasks" ]; then
     for i in $(seq 1 10); do
        [ -s "$cgroup_tasks" ] || break
        sleep 1
     done
     if [ -s "$cgroup_tasks" ]; then
        echo_fail "PIA cgroup still has attached processes"
     else
        # Empty of tasks doesn't mean unmounted - pia-uninstall.sh's own
        # rm -rf can't remove an active mount point.
        umount /opt/piavpn/etc/cgroup/net_cls 2>/dev/null || true
     fi
  fi
}

remove_daemon() {
  local daemon=piavpn
  case $(readlink /proc/1/exe) in
    *init)
      if [ -f /etc/systemd/system/${daemon}.service ] || ls /etc/systemd/system/*/${daemon}.service >/dev/null 2>&1; then
         rm /etc/systemd/system/${daemon}.service   2>/dev/null
         rm /etc/systemd/system/*/${daemon}.service 2>/dev/null
         echo_done "Removed unused systemd service registration"
      fi
      ;;
    *systemd)
      if [ -f /etc/init.d/${daemon} ] || ls /etc/rc[0-6S].d/[SK][0-9][0-9]${daemon} >/dev/null 2>&1; then
         # Run before deleting the init.d script - insserv needs it
         # present for a precise, name-targeted removal.
         command -v update-rc.d >/dev/null 2>&1 && update-rc.d "$daemon" remove >/dev/null 2>&1
         rm /etc/init.d/${daemon}           2>/dev/null
         rm /etc/init.d/${daemon}.dpkg-dist 2>/dev/null
         # Safety net for any symlink update-rc.d didn't pick up.
         rm /etc/rc[0-6S].d/[SK][0-9][0-9]${daemon} 2>/dev/null
         echo_done "Removed unused sysvinit service registration"
      fi
      ;;
  esac
}

strip_color() {
  sed -r -e "s/\x1B\[[0-9;]+[fhHmlpKABCDj]|\x1B\[[suK]|\x08//g"
}

uninstall_app() {
  local uninstaller=/opt/piavpn/bin/pia-uninstall.sh
  [ -x $uninstaller ] || return
  # stderr deliberately not suppressed - doing so once also hid a real
  # cgroup removal error along with the harmless "no process found" line.
  echo "Y" | $uninstaller | strip_color
}

stop_daemon
remove_daemon
uninstall_app

# dpkg/pia-uninstall.sh don't track per-user copies (menu-editor overrides,
# autostart entries, Desktop shortcuts) of our installer entry or PIA's own
# app entry - sweep every real account via getent instead (not a /home/*
# glob) and resolve each one's Desktop folder via xdg-user-dir. PIA's own
# autostart cleanup is also broken: wrong filename, and ~ never expands
# inside double quotes.
while IFS=: read -r _ _ uid _ _ uhome _; do
    [ "$uid" -ge 1000 ] && [ "$uid" -lt 65534 ] || continue
    [ -d "$uhome" ] || continue
    rm -f "$uhome/.local/share/applications/pia-vpn-installer.desktop" 2>/dev/null
    rm -f "$uhome/.local/share/applications/piavpn.desktop" 2>/dev/null
    rm -f "$uhome/.config/autostart/pia-client.desktop" 2>/dev/null
    rm -f "$uhome/.config/autostart/piavpn.desktop" 2>/dev/null
    if command -v xdg-user-dir >/dev/null 2>&1; then
        udesktop=$(HOME="$uhome" XDG_CONFIG_HOME="$uhome/.config" xdg-user-dir DESKTOP 2>/dev/null)
        if [ -n "$udesktop" ]; then
           rm -f "$udesktop/pia-vpn-installer.desktop" 2>/dev/null
           # KDE's "Add to Desktop" drops a symlink here - rm -f is fine either way.
           rm -f "$udesktop/piavpn.desktop" 2>/dev/null
        fi
    fi
done < <(getent passwd)

# root (UID 0) is outside the loop's normal-account range above.
rm -f /root/.local/share/applications/pia-vpn-installer.desktop 2>/dev/null
rm -f /root/.local/share/applications/piavpn.desktop 2>/dev/null
rm -f /root/.config/autostart/pia-client.desktop 2>/dev/null
rm -f /root/.config/autostart/piavpn.desktop 2>/dev/null
if command -v xdg-user-dir >/dev/null 2>&1; then
    rdesktop=$(HOME=/root XDG_CONFIG_HOME=/root/.config xdg-user-dir DESKTOP 2>/dev/null)
    if [ -n "$rdesktop" ]; then
       rm -f "$rdesktop/pia-vpn-installer.desktop" 2>/dev/null
       rm -f "$rdesktop/piavpn.desktop" 2>/dev/null
    fi
fi

}

case "${1:-}" in
    preinstall)     do_preinstall ;;
    postinstall)    do_postinstall ;;
    postuninstall)  do_postuninstall ;;
    *) usage; exit 1 ;;
esac

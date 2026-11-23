#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: shared repo/dual-init/cleanup logic for the "NordVPN CLI"/"NordVPN
# GUI" entries - NordVPN's own .deb self-registers only the currently active
# init; do_postinstall below fills in whichever side it skipped.

usage() {
    cat <<'EOF'
nordvpn.sh - install/upgrade/uninstall helper for NordVPN (MX Package Installer)

Usage: nordvpn.sh {preinstall|postinstall}
       nordvpn.sh postuninstall [gui]

"postuninstall gui" removes only nordvpn-gui; any other/no argument does a
full removal of everything NordVPN-related. Called by MX Package Installer's
"NordVPN CLI"/"NordVPN GUI" entries (nordvpn-cli.pm/nordvpn-gui.pm). Not
intended to be run directly.
EOF
}

REPO_BASE=https://repo.nordvpn.com/deb/nordvpn/debian

do_preinstall() {

export DEBIAN_FRONTEND=noninteractive

dpkg --purge nordvpn-sysvinit-compat 2>/dev/null
dpkg --purge nordvpn-release 2>/dev/null
rm -f /etc/apt/sources.list.d/nordvpn* 2>/dev/null
rm -f /etc/apt/trusted.gpg.d/nordvpn*  2>/dev/null
rm -f /etc/apt/keyrings/nordvpn*       2>/dev/null

TMP_DIR=$(mktemp -d /tmp/mxpi-nordvpn-installer.XXXXXXXXXX)
tidy_up() { rm -r /tmp/mxpi-nordvpn-installer.* 2>/dev/null ; }
trap tidy_up EXIT
chown _apt "$TMP_DIR"
pushd "$TMP_DIR" >/dev/null

# Look up the current nordvpn-release filename rather than hardcoding a
# version; sort -rV picks the highest if more than one is ever listed.
RELPATH=$(curl -sL "$REPO_BASE/dists/stable/main/binary-all/Packages" | grep -E "^Filename: .*/nordvpn-release_.*\.deb" | cut -d' ' -f2 | sort -rV | head -n1)
if [ -z "$RELPATH" ]; then
   echo "ERROR: could not determine nordvpn-release package from $REPO_BASE/dists/stable/main/binary-all/Packages"
   popd >/dev/null
   exit 1
fi
curl -sL -o nordvpn-release.deb "$REPO_BASE/$RELPATH"
if [ ! -s nordvpn-release.deb ]; then
   echo "ERROR: could not download nordvpn-release"
   popd >/dev/null
   exit 1
fi
apt-get install -y ./nordvpn-release.deb
popd >/dev/null

apt-get update

}

do_postinstall() {

# nordvpn-gui depends on nordvpn; mark both manual so removing one never
# autoremoves the other as a side effect.
apt-mark manual nordvpn nordvpn-gui 2>/dev/null || true

# nordvpn's own postinst uses $SUDO_USER/logname, which pkexec never sets -
# prefer $PKEXEC_UID/$SUDO_UID instead, falling back to logname.
REAL_USER=$(id -un "${PKEXEC_UID:-${SUDO_UID:-$(id -u)}}" 2>/dev/null)
if [ -z "$REAL_USER" ] || [ "$REAL_USER" = "root" ]; then
   REAL_USER=$(logname 2>/dev/null)
fi
if [ -n "$REAL_USER" ] && [ "$REAL_USER" != "root" ]; then
   usermod -aG nordvpn "$REAL_USER" 2>/dev/null || true
fi

# Registers whichever init side nordvpn's own postinst skipped.
if [ -x /etc/init.d/nordvpn ]; then
   _SKIP_SYSTEMD_NATIVE=1 update-rc.d nordvpn defaults 2>/dev/null || true
fi
if [ -x /usr/bin/systemctl ] && [ -f /usr/lib/systemd/system/nordvpnd.service ]; then
   systemctl --root=/ enable nordvpnd.socket 2>/dev/null || true
   systemctl --root=/ enable nordvpnd-killswitch.service 2>/dev/null || true
   systemctl --root=/ enable nordvpnd.service 2>/dev/null || true
fi
if [ -d /run/systemd/system ] && [ -x /usr/bin/systemctl ]; then
   systemctl daemon-reload 2>/dev/null || true
fi

echo "--------------------------------"
echo "NOTE: log out and back in before using the NordVPN GUI/CLI - group"
echo "membership changes don't apply to your already-running session."
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"

}

# "gui" scope removes only nordvpn-gui; anything else does a full removal.
do_postuninstall() {
SCOPE="${1:-full}"

# MXPI already removed the target; defensive purge only (no autoremove/autopurge - policy).
if [ "$SCOPE" = "gui" ]; then
   apt-get -y purge nordvpn-gui 2>/dev/null
else
   apt-get -y purge 'nordvpn*' 2>/dev/null
   rm -f /etc/apt/sources.list.d/nordvpn* 2>/dev/null
   rm -f /etc/apt/trusted.gpg.d/nordvpn*  2>/dev/null
   rm -f /etc/apt/keyrings/nordvpn*       2>/dev/null
   delgroup nordvpn 2>/dev/null
fi

echo "--------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"

}

case "${1:-}" in
    preinstall)     do_preinstall ;;
    postinstall)    do_postinstall ;;
    postuninstall)  do_postuninstall "${2:-}" ;;
    *) usage; exit 1 ;;
esac

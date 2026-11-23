#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: install/upgrade/uninstall logic for the "Mullvad browser" entry
# (mullvad-browser.pm). Called by the .pm with a stage name; not meant to be
# run manually.

usage() {
    cat <<'EOF'
mullvad-browser.sh - install/upgrade/uninstall helper for Mullvad browser (MX Package Installer)

Usage: mullvad-browser.sh {preinstall|postinstall|postuninstall}

Called by MX Package Installer's "Mullvad browser" entry (mullvad-browser.pm).
Not intended to be run directly.
EOF
}

do_preinstall() {

export DEBIAN_FRONTEND=noninteractive

# remove existing repo and keyring for mullvad
rm -f /etc/apt/keyrings/*mullvad*               2>/dev/null
rm -f /etc/apt/trusted.gpg.d/*mullvad*          2>/dev/null
rm -f /usr/share/keyrings/*mullvad*             2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.list    2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.sources 2>/dev/null
rm -f /var/lib/extrepo/keys/mullvad.asc         2>/dev/null
rm -f /usr/share/xfce4/helpers/mullvad-browser.desktop 2>/dev/null

apt-get update
apt-get install extrepo --yes
extrepo enable mullvad

if [ -f /etc/apt/sources.list.d/extrepo_mullvad.sources ]; then
# Enable explicitly by adding "Enabled: yes"
sed -i -e '$aEnabled: yes' -e '/^Enabled/Id' /etc/apt/sources.list.d/extrepo_mullvad.sources
# Removing not needed architectures
case "$(dpkg --print-architecture)" in
  amd64) sed -i '/^Architectures/IcArchitectures: amd64' /etc/apt/sources.list.d/extrepo_mullvad.sources
  ;;
esac
fi

# Fixing the CTYPE error in apt for certain locales such as tr_TR.UTF-8
for SRC in $(grep -sHi ^uris /etc/apt/sources.list.d/*.sources | grep -v ':URIs' | sed 's/:URIs.*//I' | sort -u); do
env LC_CTYPE=C.UTF-8 sed -i 's/^uris/URIs/I' "$SRC"
done

apt-get update

}

do_postinstall() {

# Add Xfce helpers
if [ -d /usr/share/xfce4/helpers ] && [ ! -f /usr/share/xfce4/helpers/mullvad-browser.desktop ]; then
cat <<'EOF' > /usr/share/xfce4/helpers/mullvad-browser.desktop
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Icon=mullvad-browser
Name=Mullvad Browser
NoDisplay=true
StartupNotify=false
StartupWMClass=Mullvad Browser
Terminal=false
Type=X-XFCE-Helper
X-XFCE-Binaries=/usr/bin/mullvad-browser;
X-XFCE-Category=WebBrowser
X-XFCE-Commands=%B --detach;
X-XFCE-CommandsWithParameter=%B --detach "%s";
EOF
fi

# Disable repo unless sysvinit is unreachable on this system.
DISABLE_MULLVAD_REPO=0
if [ -e /usr/lib/sysvinit/init ] && [ -e /usr/lib/systemd/systemd ]; then
   DISABLE_MULLVAD_REPO=1
elif [ ! -d /run/systemd/system ]; then
   DISABLE_MULLVAD_REPO=1
elif [ -e /sbin/init ] && [ ! -L /sbin/init ]; then
   DISABLE_MULLVAD_REPO=1
fi

if [ -f /etc/apt/sources.list.d/extrepo_mullvad.sources ] && [ "$DISABLE_MULLVAD_REPO" = 1 ]; then
sed -i -e '$aEnabled: no' -e '/^Enabled/Id' /etc/apt/sources.list.d/extrepo_mullvad.sources
apt-get update
echo "--------------------------------"
echo "Note: Mullvad repo disabled (sysvinit detected) - use MX Package Installer for install/upgrade only."
echo "--------------------------------"
fi

echo "--------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"

}

do_postuninstall() {

# MXPI already removed mullvad-browser; defensive purge only (no autoremove/autopurge - policy).
apt-get -y purge mullvad-browser 2>/dev/null

rm -f /usr/share/xfce4/helpers/mullvad-browser.desktop  2>/dev/null

# Only tear down the shared mullvad apt repo/keyring if mullvad-vpn isn't also installed through it.
if ! dpkg -l mullvad-vpn 2>/dev/null | grep -sq '^ii'; then
rm -f /etc/apt/keyrings/*mullvad*               2>/dev/null
rm -f /etc/apt/trusted.gpg.d/*mullvad*          2>/dev/null
rm -f /usr/share/keyrings/*mullvad*             2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.list    2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.sources 2>/dev/null
rm -f /var/lib/extrepo/keys/*mullvad*.asc       2>/dev/null

apt-get update
fi

echo "--------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"

}

case "${1:-}" in
    preinstall)     do_preinstall ;;
    postinstall)    do_postinstall ;;
    postuninstall)  do_postuninstall ;;
    *) usage; exit 1 ;;
esac

#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: install/upgrade/uninstall logic for the "Mullvad VPN" entry
# (mullvad-vpn.pm). Called by the .pm with a stage name; not meant to be
# run manually.

usage() {
    cat <<'EOF'
mullvad-vpn.sh - install/upgrade/uninstall helper for Mullvad VPN (MX Package Installer)

Usage: mullvad-vpn.sh {preinstall|postinstall|postuninstall}

Called by MX Package Installer's "Mullvad VPN" entry (mullvad-vpn.pm).
Not intended to be run directly.
EOF
}

do_preinstall() {

# remove existing repo and keyring for mullvad
rm -f /etc/apt/keyrings/*mullvad*               2>/dev/null
rm -f /etc/apt/trusted.gpg.d/*mullvad*          2>/dev/null
rm -f /usr/share/keyrings/*mullvad*             2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.list    2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.sources 2>/dev/null
rm -f /var/lib/extrepo/keys/mullvad.asc         2>/dev/null

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

echo "Downloading Mullvad VPN for Linux 64bit"
DEB=$(apt-get  download --print-uris  mullvad-vpn | head -1 | cut -d' ' -f2)
[ -z "$DEB" ] || [ -n "${DEB##*deb}" ] && echo "Mullvad VPN deb package not availabel" && exit 1

echo "---------------------------------------------------------"
echo "Installing Mullvad VPN"
echo "---------------------------------------------------------"

# Always convert to sysVinit regardless of current boot state.

apt-get --reinstall  --download-only  install mullvad-vpn
[ ! -f "/var/cache/apt/archives/${DEB}" ] && echo "Mullvad VPN deb package not availabel" && exit 1
DEB="/var/cache/apt/archives/${DEB}"

# SIGTERM for some reason causes the app to crash sometimes and SIGINT works as expected.
pkill -2 -x "mullvad-gui" || true
sleep 0.5
pkill -9 -x "mullvad-gui" || true

# Patched into maintainer scripts before dpkg --unpack; idempotent via grep guard.
INITHELPER=/usr/share/mx-packageinstaller-pkglist/mullvad_sysvinit_helper.sh
patch_mullvad_shim() {
for F in /var/lib/dpkg/info/mullvad-vpn.p*; do
   [ -f "$F" ] || continue
   grep -qF "source $INITHELPER" "$F" 2>/dev/null && continue
   # Insert shebang+source ahead of line 1 rather than replace it. Renames
   # any stale inline "function systemctl" so the sourced one wins.
   sed -i -e '1i#!/bin/bash' -e "1itest -r $INITHELPER && source $INITHELPER" -e 's/^function systemctl {/function systemctl_old {/' "$F"
done
}

if ls /var/lib/dpkg/info/mullvad-vpn.p* >/dev/null 2>&1; then
   patch_mullvad_shim
fi

dpkg --ignore-depends=mullvad-vpn  --unpack $DEB

# Patch again for the freshly-unpacked pristine scripts.
patch_mullvad_shim

echo '
for INIT in /etc/init.d/mullvad-*; do
    [ -e "$INIT" ] || continue
    update-rc.d -f "${INIT##*/}" remove 2>/dev/null || true
    rm -f "$INIT"
done
' >> /var/lib/dpkg/info/mullvad-vpn.postrm

# convert mullvad-daemon.service to sysV-init
if [ -f /etc/init.d/mullvad-daemon ]; then
	service mullvad-daemon stop || true
	rm /etc/init.d/mullvad-daemon
fi

for SRV in /usr/lib/systemd/system/mullvad-*.service; do
[ -f $SRV ] || continue
INIT=${SRV%.service}; INIT=${INIT##*/};
sysd2v.sh $SRV > /etc/init.d/$INIT
chmod +x /etc/init.d/$INIT
[ "$INIT" == "mullvad-daemon" ] || continue

# sysd2v mis-quotes Environment= as one token; re-quote around the value.
sed -i '
    /^"MULLVAD_RESOURCE_DIR=/{s/^"MULLVAD_RESOURCE_DIR=\(.*\)"$/MULLVAD_RESOURCE_DIR="\1"/}
    s/mullvad-daemon.service/mullvad-daemon/g
    s/-sysd2v.pid/.pid/
    /X-Start-Before:/d
    /X-Stop-After:/d
    /Should-Start:/s/NetworkManager/$network &/
    /Should-Stop:/s/NetworkManager/$network &/
    ' /etc/init.d/$INIT

# Split tunneling: under sysvinit /sys/fs/cgroup is mounted by elogind, which
# starts in parallel with the daemon; if the daemon wins, its net_cls mount
# fails. Mount net_cls under /run instead, and start after elogind.
sed -i '
    /^# Should-Start:/s/$/ elogind/
    /^# Should-Stop:/s/$/ elogind/
    /^MULLVAD_RESOURCE_DIR=/a TALPID_NET_CLS_MOUNT_DIR="/run/mullvad-net_cls"
    ' /etc/init.d/$INIT

done

dpkg --configure mullvad-vpn
echo " "
apt-get install -yf

echo "--------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"

}

do_postinstall() {

# Disable repo unless sysvinit is unreachable on this system - otherwise a
# later plain "apt upgrade" could reinstall mullvad-vpn unpatched.
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

}

do_postuninstall() {

# MXPI already removed mullvad-vpn; defensive purge only (no autoremove/autopurge - policy).
apt-get -y purge mullvad-vpn 2>/dev/null

# Safety-net sweep for any leftover sysvinit/systemd artifacts.
for INIT in /etc/init.d/mullvad-*; do
    [ -e "$INIT" ] || continue
    update-rc.d -f "${INIT##*/}" remove 2>/dev/null
    rm -f "$INIT"
done
find /etc/systemd/system -mindepth 2 -maxdepth 2 -iname 'mullvad-*' -delete 2>/dev/null
find /etc/systemd/system -mindepth 1 -maxdepth 1 -type d -name '*.wants' -empty -delete 2>/dev/null
# Leftover from an old, now-removed /opt/MullvadVPN symlink workaround.
[ -L /opt/MullvadVPN ] && rm -f /opt/MullvadVPN

# Redundant - upstream's postrm already handles this on purge.
for f in /home/*/.config/autostart/mullvad-vpn.desktop /root/.config/autostart/mullvad-vpn.desktop; do
   [ -L "$f" ] && rm -f "$f"
done

# Only tear down the shared repo/keyring if mullvad-browser isn't also using it.
if ! dpkg -l mullvad-browser 2>/dev/null | grep -sq '^ii'; then
rm -f /etc/apt/keyrings/*mullvad*               2>/dev/null
rm -f /etc/apt/trusted.gpg.d/*mullvad*          2>/dev/null
rm -f /usr/share/keyrings/*mullvad*             2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.list    2>/dev/null
rm -f /etc/apt/sources.list.d/*mullvad*.sources 2>/dev/null
rm -f /var/lib/extrepo/keys/*mullvad*.asc       2>/dev/null

apt-get update
fi

echo "---------------------------------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "---------------------------------------------------------"

}

case "${1:-}" in
    preinstall)     do_preinstall ;;
    postinstall)    do_postinstall ;;
    postuninstall)  do_postuninstall ;;
    *) usage; exit 1 ;;
esac

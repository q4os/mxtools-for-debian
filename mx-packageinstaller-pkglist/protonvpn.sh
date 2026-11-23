#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: shared repo/daemon bootstrap and cleanup logic for the "Proton VPN
# CLI"/"Proton VPN GUI" entries. Called by either .pm with a stage name; not
# meant to be run manually.

usage() {
    cat <<'EOF'
protonvpn.sh - install/upgrade/uninstall helper for Proton VPN (MX Package Installer)

Usage: protonvpn.sh preinstall {cli|gui}
       protonvpn.sh postinstall
       protonvpn.sh postuninstall {cli|gui}

Called by MX Package Installer's "Proton VPN CLI"/"Proton VPN GUI" entries
(protonvpn-cli.pm/protonvpn-gui.pm). Not intended to be run directly.
EOF
}

INITHELPER=/usr/share/mx-packageinstaller-pkglist/proton-vpn-daemon_sysvinit_helper.sh
REPO_BASE=https://repo.protonvpn.com/debian
REPO_SRC=/etc/apt/sources.list.d/protonvpn-stable.sources

# Repo disabled when sysvinit can be booted: a plain "apt upgrade" would
# install proton-vpn-daemon without our systemctl shim helper, and its
# maintainer scripts fail under sysvinit. Upgrade via MXPI instead.

# Edit $REPO_SRC: remove every Enabled: line (also commented ones), tidy
# blanks and empty lines; "off" adds "Enabled: no" as the first line of
# each stanza that has fields.
proton_repo_edit() {
[ -f "$REPO_SRC" ] || return 0
local t
t=$(awk -v off="$1" '
   function flush(  i) {
      if (n) { if (off == "off" && f) print "Enabled: no"
               for (i = 1; i <= n; i++) print b[i] }
      n = 0; f = 0 }
   tolower($0) ~ /^[# \t]*enabled[ \t]*:/ { next }
   { sub(/[ \t]+$/, ""); gsub(/[ \t]+/, " ") }
   $0 == "" { if (n) { flush(); gap = 1 }; next }
   { if (gap && seen) print ""; gap = 0; seen = 1
     b[++n] = $0; if ($0 !~ /^#/) f = 1 }
   END { flush() }' "$REPO_SRC") || return 0
[ "$t" = "$(cat "$REPO_SRC")" ] || printf '%s\n' "$t" > "$REPO_SRC"
}

proton_repo_enable() {
proton_repo_edit on
}

# apt's own view, in any sources file, also with an empty cache.
# An apt error (exit 100, e.g. a malformed sources file) counts as active.
proton_repo_active() {
local uris
uris=$(LC_ALL=C apt-get update --print-uris 2>/dev/null) || return 0
printf '%s\n' "$uris" | grep -qi 'repo\.protonvpn\.com/debian/dists/'
}

proton_repo_disable() {
if [ -e /usr/lib/sysvinit/init ] && [ -e /usr/lib/systemd/systemd ]; then
   :
elif [ ! -d /run/systemd/system ]; then
   :
elif [ -e /sbin/init ] && [ ! -L /sbin/init ]; then
   :
else
   return 0
fi
proton_repo_active || return 0
proton_repo_edit off
apt-get update >/dev/null 2>&1
echo "--------------------------------"
if ! LC_ALL=C apt-get update --print-uris >/dev/null 2>&1; then
   echo "WARNING: apt can't read its sources lists - Proton VPN repo state unknown."
   echo "Check with: sudo apt-get update"
elif proton_repo_active; then
   echo "WARNING: Proton VPN repo is still enabled (sysvinit detected) in:"
   grep -rlis 'repo\.protonvpn\.com' /etc/apt/sources.list /etc/apt/sources.list.d/ \
      | grep -vxF "$REPO_SRC" | sed 's/^/   /'
   echo "Disable it there - a plain apt upgrade can break proton-vpn-daemon under sysvinit."
else
   echo "Note: Proton VPN repo disabled (sysvinit detected) - use MX Package Installer for install/upgrade only."
fi
echo "--------------------------------"
}

# Short, user-facing refusal when apt would remove packages; the package
# names go on one wrapped line at the end, for helpers/forum posts.
# $1: what is being installed, $2: newline-separated packages apt would remove.
refuse_removals() {
echo
echo "ERROR: $1 can't be installed on this system."
echo
if dpkg -l elogind 2>/dev/null | grep -sq '^ii'; then
   echo "$1 depends on the 'systemd' package (as an installed package, not as"
   echo "the running init system). On this sysvinit-only MX installation that"
   echo "package conflicts with elogind: installing it would remove important"
   echo "system packages, such as the login screen (lightdm) and MX Package"
   echo "Installer itself."
else
   echo "Installing it would remove other installed packages."
fi
echo
echo "Nothing was changed."
echo
echo "(Would be removed: $(echo $2))" | fmt -w 76
}

do_preinstall() {
SCOPE="${1:-}"

export DEBIAN_FRONTEND=noninteractive

# Early exit before downloading anything: Proton hard-depends on systemd
# (proton-vpn-daemon, python3-proton-vpn-api-core). Both come from Debian's
# own repo, so this needs no Proton repo yet. On a sysvinit-only MX install
# elogind declares Conflicts: systemd, and apt would swap elogind for systemd,
# removing libpam-elogind, lightdm, polkitd, pkexec, synaptic, gparted, ...
# (confirmed against the MX-25.3 sysvinit-only ISO's dpkg status).
if ! dpkg -l systemd 2>/dev/null | grep -sq '^ii'; then
   REMV=$(apt-get -s --no-install-recommends install systemd python3-systemd 2>/dev/null | awk '$1 == "Remv" { print $2 }')
   if [ -n "$REMV" ]; then
      refuse_removals "Proton VPN" "$REMV"
      exit 1
   fi
fi

TMP_DIR=$(mktemp -d /tmp/mxpi-protonvpn-installer.XXXXXXXXXX)
tidy_up() { rm -r /tmp/mxpi-protonvpn-installer.* 2>/dev/null ; proton_repo_disable ; }
trap tidy_up EXIT
# apt's sandboxed download runs as _apt; needs write access to this dir.
chown _apt "$TMP_DIR"
pushd $TMP_DIR >/dev/null

# Chicken-and-egg: fetch the repo's Packages index directly to find the release .deb.
curl -sL "$REPO_BASE/dists/stable/main/binary-all/Packages" -o Packages
RELPATH=$(grep -E "^Filename: .*/protonvpn-stable-release.*\.deb" Packages | cut -d' ' -f2 | sort -rV | head -n1)
if [ -z "$RELPATH" ]; then
   echo "ERROR: could not determine protonvpn-stable-release package from $REPO_BASE/dists/stable/main/binary-all/Packages"
   popd >/dev/null
   exit 1
fi
REL_URL="$REPO_BASE/$RELPATH"
echo " "
echo "Downloading ... ${RELPATH##*/}"
echo "curl -RLJO $REL_URL"
echo " "
curl -RLJO "$REL_URL"
sleep 0.3
echo
echo
RELDEB=${RELPATH##*/}
if [ ! -r "$RELDEB" ]; then
   echo "Error downloading $RELDEB"
   popd >/dev/null
   exit 1
fi

REL_WAS_INSTALLED=
dpkg -l 'protonvpn-*-release' 2>/dev/null | grep -sq '^ii' && REL_WAS_INSTALLED=1

proton_repo_enable
apt-get install -y "./$RELDEB"
apt-get update

# Safety net after the early systemd check above: simulate the full install
# now that the Proton repo is known, and refuse if apt would remove anything
# (catches other/future dependency conflicts too).
case "$SCOPE" in
   cli) FRONTEND="proton-vpn-cli" ;;
   *)   FRONTEND="proton-vpn-gnome-desktop proton-vpn-cli" ;;
esac
REMV=$(apt-get -s --no-install-recommends install proton-vpn-daemon $FRONTEND 2>/dev/null | awk '$1 == "Remv" { print $2 }')
if [ -n "$REMV" ]; then
   refuse_removals "Proton VPN" "$REMV"
   # Undo the repo setup, unless it was already there before.
   if [ -z "$REL_WAS_INSTALLED" ]; then
      apt-get -y purge 'protonvpn-*-release' >/dev/null 2>&1
      apt-get update >/dev/null 2>&1
   fi
   popd >/dev/null
   exit 1
fi

# Pre-install proton-vpn-daemon's dependencies only, filtered out of the Inst list.
DEP=$(apt-get -s --no-install-recommends install proton-vpn-daemon 2>/dev/null | awk '$1 == "Inst" && $2 != "proton-vpn-daemon" { print $2 }')
if [ -n "$DEP" ]; then
   apt-get install -y --mark-auto $DEP
fi

# Download only - patched before dpkg unpacks/configures it below.
apt-get download proton-vpn-daemon
DEB=$(ls proton-vpn-daemon_*.deb 2>/dev/null | head -1)
if [ -z "$DEB" ]; then
   echo "ERROR: could not download proton-vpn-daemon"
   popd >/dev/null
   exit 1
fi

patch_protonvpn_shim() {
for F in /var/lib/dpkg/info/proton-vpn-daemon.p*; do
   [ -f "$F" ] || continue
   grep -qF "source $INITHELPER" "$F" 2>/dev/null && continue
   # Insert shebang+source ahead of line 1 rather than replace it.
   sed -i -e '1i#!/bin/bash' -e "1itest -r $INITHELPER && source $INITHELPER" "$F"
done
}

if ls /var/lib/dpkg/info/proton-vpn-daemon.p* >/dev/null 2>&1; then
   patch_protonvpn_shim
fi

dpkg --unpack "$DEB"

# Patch again for the freshly-unpacked pristine scripts.
patch_protonvpn_shim

for SRV in /usr/lib/systemd/system/me.proton.vpn.*.service /usr/lib/systemd/system/proton-vpn-*.service; do
   [ -f "$SRV" ] || continue
   INIT=${SRV%.service}; INIT=${INIT##*/}
   sysd2v.sh "$SRV" | sed "s/${INIT}-sysd2v.pid/${INIT}.pid/" > /etc/init.d/$INIT
   chmod 755 /etc/init.d/$INIT
done

dpkg --configure proton-vpn-daemon
apt-get install -yf

# Mark auto so removing the CLI/GUI later cleans up the daemon too.
apt-mark auto proton-vpn-daemon

# Install the actual frontend here (not via <install_package_names>) so MXPI
# doesn't pop up a second confirmation mid-flow.
case "$SCOPE" in
   cli) apt-get install -y proton-vpn-cli ;;
   gui) apt-get install -y proton-vpn-gnome-desktop proton-vpn-cli ;;
esac

# Split tunneling is a separate entry - see protonvpn-splittunnel.pm/.sh.
echo "...$(gettext -d apt -s ' Done')!"

}

do_postinstall() {

# Repo already disabled at the end of preinstall if needed; no-op otherwise.
proton_repo_disable

}

do_postuninstall() {
SCOPE="${1:-}"

case "$SCOPE" in
   cli) apt-get -y purge proton-vpn-cli 2>/dev/null ;;
   gui) apt-get -y purge proton-vpn-gnome-desktop proton-vpn-gtk-app 2>/dev/null ;;
esac

# Daemon/repo teardown only once neither frontend is still installed.
if ! dpkg -l proton-vpn-cli 2>/dev/null | grep -sq '^ii' \
   && ! dpkg -l proton-vpn-gnome-desktop 2>/dev/null | grep -sq '^ii'; then

apt-get -y purge proton-vpn-daemon 2>/dev/null

for INIT in /etc/init.d/me.proton.vpn.* /etc/init.d/proton-vpn-*; do
    [ -e "$INIT" ] || continue
    update-rc.d -f "${INIT##*/}" remove 2>/dev/null
    rm -f "$INIT"
done
find /etc/systemd/system -mindepth 2 -maxdepth 2 -iname 'me.proton.vpn.*' -delete 2>/dev/null
find /etc/systemd/system -mindepth 2 -maxdepth 2 -iname 'proton-vpn-*' -delete 2>/dev/null
find /etc/systemd/system -mindepth 1 -maxdepth 1 -type d -name '*.wants' -empty -delete 2>/dev/null

apt-get -y purge 'protonvpn-*-release' 2>/dev/null
apt-get update

fi

# User-enabled "start on login" autostart entry.
for f in /home/*/.config/autostart/proton.vpn.app.gtk.desktop /root/.config/autostart/proton.vpn.app.gtk.desktop; do
   [ -e "$f" ] || [ -L "$f" ] && rm -f "$f"
done

echo "...$(gettext -d apt -s ' Done')!"

}

case "${1:-}" in
    preinstall)     do_preinstall "${2:-}" ;;
    postinstall)    do_postinstall ;;
    postuninstall)  do_postuninstall "${2:-}" ;;
    *) usage; exit 1 ;;
esac

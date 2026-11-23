#!/bin/bash
# Part of MX Package Installer's mx-packageinstaller-pkglist.
# Purpose: install/upgrade/uninstall logic for the "Windscribe VPN" entry
# (windscribe-vpn.pm). Called by the .pm with a stage name; not meant to be
# run manually.

usage() {
    cat <<'EOF'
windscribe-vpn.sh - install/upgrade/uninstall helper for Windscribe VPN (MX Package Installer)

Usage: windscribe-vpn.sh {preinstall|postinstall|postuninstall}

Called by MX Package Installer's "Windscribe VPN" entry (windscribe-vpn.pm).
Not intended to be run directly.
EOF
}

INITHELPER=/usr/share/mx-packageinstaller-pkglist/windscribe_sysvinit_helper.sh

do_preinstall() {

export DEBIAN_FRONTEND=noninteractive

TMP_DIR=$(mktemp -d /tmp/mxpi-windscribe-installer.XXXXXXXXXX)
tidy_up() { rm -r /tmp/mxpi-windscribe-installer.* 2>/dev/null ; }
trap tidy_up EXIT
chmod +xr $TMP_DIR
pushd $TMP_DIR >/dev/null

command -v jq >/dev/null 2>&1 || apt-get install -y jq

API_URL='https://api.windscribe.com/CheckUpdate?platform=linux_deb_x64&beta=0'
API_RESPONSE=$(curl -s "$API_URL" -H 'Authorization: Bearer 1234')
DEB_URL=$(echo "$API_RESPONSE" | jq -r '.data.update_url // empty')
SHA256=$(echo "$API_RESPONSE" | jq -r '.data.sha256 // empty')
DEB=${DEB_URL##*/}
if [ -z "$DEB_URL" ]; then
   echo "Error: could not determine Windscribe download URL from $API_URL"
   popd >/dev/null
   exit 1
fi
# Reuse an already-verified cached copy from apt's cache dir if present;
# only ever trusted by checksum match, never filename/mtime.
CACHE_DIR=/var/cache/apt/archives
CACHED_DEB="$CACHE_DIR/$DEB"
if [ -n "$SHA256" ] && [ -f "$CACHED_DEB" ] && [ "$(sha256sum "$CACHED_DEB" | cut -d' ' -f1)" = "$SHA256" ]; then
   echo "Found $DEB already cached and checksum-verified in $CACHE_DIR - skipping download."
   DEB="$CACHED_DEB"
else
   echo " "
   echo "Downloading ... $DEB"
   echo "curl -RLJO $DEB_URL"
   echo " "
   curl -RLJO $DEB_URL
   # Let curl's progress-meter output settle before printing more.
   sleep 0.3
   echo
   echo
   DEB=${DEB_URL##*/}
   if [ ! -r "$DEB" ]; then
      echo "Error downloading $DEB"
      popd >/dev/null
      exit 1
   fi

   # Windscribe publishes no GPG signature for the .deb; SHA256 from the
   # CheckUpdate API is the strongest integrity check available.
   if [ -n "$SHA256" ]; then
      echo "Verifying checksum (SHA256) of the downloaded package..."
      ACTUAL_SHA256=$(sha256sum "$DEB" | cut -d' ' -f1)
      if [ "$ACTUAL_SHA256" != "$SHA256" ]; then
         echo "WARNING: SHA256 mismatch for $DEB - refusing to install"
         echo "  expected: $SHA256"
         echo "  actual:   $ACTUAL_SHA256"
         popd >/dev/null
         exit 1
      fi
      echo "SHA256 verified OK ($ACTUAL_SHA256)"
      mkdir -p "$CACHE_DIR"
      cp "$DEB" "$CACHED_DEB" 2>/dev/null || true
   else
      echo "Warning: CheckUpdate API did not return a sha256 - skipping integrity check"
   fi
fi

# Patch on-disk maintainer scripts before dpkg --unpack (its old-prerm step
# reads them first). Idempotent via grep guard.
patch_windscribe_shim() {
for F in /var/lib/dpkg/info/windscribe.p*; do
   [ -f "$F" ] || continue
   grep -qF "source $INITHELPER" "$F" 2>/dev/null && continue
   # Insert shebang+source ahead of line 1 rather than replace it.
   sed -i -e '1i#!/bin/bash' -e "1itest -r $INITHELPER && source $INITHELPER" "$F" 2>/dev/null
done
}

if ls /var/lib/dpkg/info/windscribe.p* >/dev/null 2>&1; then
   patch_windscribe_shim
fi

# Simulated install against the .deb directly, so dependencies stay current
# rather than a hardcoded list going stale (a local path must start with "/" or "./").
case "$DEB" in
   /*) DEB_PATH="$DEB" ;;
   *)  DEB_PATH="./$DEB" ;;
esac
DEP=$(apt-get -s --no-install-recommends install "$DEB_PATH" 2>/dev/null | awk '$1 == "Inst" && $2 != "windscribe" { print $2 }')
if [ -n "$DEP" ]; then
   apt-get install -y --mark-auto $DEP
fi

# Upstream's postinst creates the "windscribe" group without -r; pre-create
# it as a system group so postinst's own check just no-ops.
getent group windscribe >/dev/null 2>&1 || groupadd -r windscribe

dpkg --ignore-depends=windscribe --unpack $DEB

# Patch again for the freshly-unpacked pristine scripts.
patch_windscribe_shim

# windscribe ships no postrm; append sysvinit-script cleanup to prerm,
# guarded so a re-run doesn't duplicate it.
grep -qF '/etc/init.d/windscribe-helper' /var/lib/dpkg/info/windscribe.prerm 2>/dev/null || {
   echo "# remove sysVinit script" >> /var/lib/dpkg/info/windscribe.prerm
   echo "test -e /etc/init.d/windscribe-helper && rm /etc/init.d/windscribe-helper" >> /var/lib/dpkg/info/windscribe.prerm
}

for SRV in /usr/lib/systemd/system/windscribe-*.service; do
   [ -f "$SRV" ] || continue
   INIT=${SRV%.service}; INIT=${INIT##*/}
   sysd2v.sh "$SRV" | sed "s/${INIT}-sysd2v.pid/${INIT}.pid/" > /etc/init.d/$INIT
   chmod 755 /etc/init.d/$INIT
done

dpkg --configure windscribe
apt-get -y install -f
echo "...$(gettext -d apt -s ' Done')!"

}

do_postinstall() {
    :
}

do_postuninstall() {

# MXPI already removed windscribe; defensive purge only (no autoremove/autopurge - policy).
apt-get -y purge windscribe 2>/dev/null

test -e /etc/init.d/windscribe-helper && rm /etc/init.d/windscribe-helper

# Windscribe creates a per-user autostart symlink at runtime (not on
# install); upstream's prerm removes the target but not the symlink itself.
for f in /home/*/.config/autostart/windscribe.desktop /root/.config/autostart/windscribe.desktop; do
   [ -L "$f" ] && rm -f "$f"
done

echo "...$(gettext -d apt -s ' Done')!"

}

case "${1:-}" in
    preinstall)     do_preinstall ;;
    postinstall)    do_postinstall ;;
    postuninstall)  do_postuninstall ;;
    *) usage; exit 1 ;;
esac

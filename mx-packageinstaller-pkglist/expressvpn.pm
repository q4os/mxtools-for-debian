<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Network
</category>

<name>
ExpressVPN
</name>

<description>
   <am>ExpressVPN and sysVinit scripts</am>
   <ar>ExpressVPN and sysVinit scripts</ar>
   <bg>ExpressVPN and sysVinit scripts</bg>
   <bn>ExpressVPN and sysVinit scripts</bn>
   <ca>Scripts per ExpressVPN i sysVinit</ca>
   <cs>ExpressVPN and sysVinit scripts</cs>
   <da>ExpressVPN and sysVinit scripts</da>
   <de>ExpressVPN aud sysVinit Skripte</de>
   <el>Σενάρια του ExpressVPN client και sysVinit</el>
   <en>ExpressVPN and sysVinit scripts</en>
   <es>ExpressVPN y sysVinit scripts</es>
   <et>ExpressVPN and sysVinit scripts</et>
   <eu>ExpressVPN and sysVinit scripts</eu>
   <fa>ExpressVPN and sysVinit scripts</fa>
   <fil_PH>ExpressVPN and sysVinit scripts</fil_PH>
   <fi>ExpressVPN ja sysVinit ohjelmakoodisyötteet</fi>
   <fr>Scripts pour ExpressVPN et sysVinit</fr>
   <he_IL>ExpressVPN and sysVinit scripts</he_IL>
   <hi>एक्सप्रेस वीपीएन व sysVinit स्क्रिप्ट</hi>
   <hr>ExpressVPN and sysVinit scripts</hr>
   <hu>ExpressVPN and sysVinit scripts</hu>
   <id>ExpressVPN and sysVinit scripts</id>
   <is>ExpressVPN and sysVinit scripts</is>
   <it>ExpressVPN e script sysVinit</it>
   <ja>ExpressVPN と sysVinit スクリプト</ja>
   <kk>ExpressVPN and sysVinit scripts</kk>
   <ko>ExpressVPN and sysVinit scripts</ko>
   <lt>ExpressVPN and sysVinit scripts</lt>
   <mk>ExpressVPN and sysVinit scripts</mk>
   <mr>ExpressVPN and sysVinit scripts</mr>
   <nb>ExpressVPN- og sysVinit-skript</nb>
   <nl>ExpressVPN en sysVinit scripts</nl>
   <pl>Skrypty ExpressVPN i sysVinit</pl>
   <pt_BR>Scripts (funções ou instruções escritas em texto dos programas) do ExpressVPN e sysVinit</pt_BR>
   <pt>Scripts ExpressVPN e sysVinit</pt>
   <ro>ExpressVPN and sysVinit scripts</ro>
   <ru>ExpressVPN и cкрипты sysVinit</ru>
   <sk>ExpressVPN and sysVinit scripts</sk>
   <sl>ExpressVPN in sysVinit skripti</sl>
   <sq>programthe  për ExpressVPN dhe sysVinit</sq>
   <sr>ExpressVPN and sysVinit scripts</sr>
   <sv>ExpressVPN och sysVinit skript</sv>
   <tr>ExpressVPN and sysVinit betikleri</tr>
   <uk>ExpressVPN and sysVinit scripts</uk>
   <vi>ExpressVPN and sysVinit scripts</vi>
   <zh_CN>ExpressVPN and sysVinit scripts</zh_CN>
   <zh_TW>ExpressVPN and sysVinit scripts</zh_TW>
</description>

<installable>
32,64,armhf
</installable>

<screenshot>none</screenshot>

<preinstall>
<![CDATA[

ARCH="$(dpkg --print-architecture)"

# ExpressVPN Release signing key  0xAFF2A1415F6A3A38
SKEY="0xAFF2A1415F6A3A38"
LKEY="1D0B09AD6C93FEE93FDDBD9DAFF2A1415F6A3A38"

# prepare temp dir
DIR=$(mktemp -d /tmp/tmpdir-expressvpn-keyring.XXXXXXXXXXXXX)
TKG=$DIR/keyring.gpg
GPG_HOME=$DIR
chmod 700 $DIR
# prepare tidy-up
tidy_up() {
    rm -r /tmp/tmpdir-expressvpn-keyring.* 2>/dev/null
}
trap tidy_up EXIT

# get deb and sig
DEB=$(curl -RJL -s 'https://www.expressvpn.com/latest#linux' |
      grep -m1 -oE  "https://[[:alnum:]._/-]+/expressvpn_[0-9._-]+_${ARCH}.deb" |
      head -1)
SIG="${DEB}.asc"

pushd $DIR >/dev/null
echo "--------------------------------------"
echo "get ExpressVPN deb-package ${DEB##*/}"
echo "--------------------------------------"
echo " "
echo " "
curl -RLJO "$DEB"
echo "--------------------------------------"
echo "get ExpressVPN signature ${SIG##*/}"
echo "--------------------------------------"
echo " "
curl -RLJO "$SIG"
DEB="${DEB##*/}"
SIG="${SIG##*/}"
if ! [ -f "$DEB" ]; then
   echo "Error downloading deb-package: $DEB"
   echo exit
   exit 1
fi
if ! [ -f "$SIG" ]; then
   echo "Error downloading signature $SIG"
   echo exit
   exit 1
fi

echo "--------------------------------------"
echo "get ExpressVPN Release signing key: $SKEY "
echo "--------------------------------------"
echo " "
found="false"
OPT="--homedir=$GPG_HOME --no-default-keyring --keyring=gnupg-ring:$TKG"
IMP="--import-options import-clean,self-sigs-only"
for ksrv in keyserver.ubuntu.com pgpkeys.eu; do
touch $TKG
gpg -q $OPT --keyserver $ksrv $IMP --recv-keys $LKEY  1>/dev/null 2>&1
# keyring sanity check
if gpg -q $OPT --keyid-format 0xlong --list-keys $LKEY 1>/dev/null 2>&1; then
found="true"
break
fi
rm $TKG
done
if [ "$found" = "false" ]; then
   echo "Error downloading signing key $SKEY ( $LKEY )"
   echo exit
   exit 1
fi

# list-keys
gpg $OPT --keyid-format 0xlong --list-keys $LKEY

echo "--------------------------------------"
echo "verify ExpressVPN package signature ..."
echo "--------------------------------------"
if ! gpgv --ignore-time-conflict --keyring=gnupg-ring:$TKG $SIG $DEB  2>&1; then
echo "Fatal: signature not verified - exit"
exit 1
fi

echo "--------------------------------------"
echo "install ExpressVPN deb package"
echo "--------------------------------------"

export SUDO_USER=$(logname)
PAT='(which|command[[:space:]]+-v)[[:space:]]+((/usr)?/bin/)?systemctl'
RPL='test -d /run/systemd/system'
sed -i -E "s=$PAT=$RPL=" /var/lib/dpkg/info/expressvpn.{post,pre}* 2>/dev/null
dpkg --unpack  $DIR/$DEB
sed -i -E "s=$PAT=$RPL=" /var/lib/dpkg/info/expressvpn.{post,pre}* 2>/dev/null
dpkg --configure  expressvpn
apt-get -yf install
]]>
</preinstall>

<install_package_names>

</install_package_names>

<postinstall>
<![CDATA[
SLST=/etc/apt/sources.list.d/expressvpn.list
if [ -e $SLST ]; then
     mv $SLST $SLST.by-mxpi.disabled
fi
echo "...$(gettext -d apt -s ' Done')"'!'
]]>

</postinstall>

<uninstall_package_names>
expressvpn
</uninstall_package_names>

<preuninstall>
<![CDATA[
echo " "
#echo "pre-uninstall ..."
if [ ! -d /run/systemd/system ]; then
# fix post/pre-rm for SysVinit
PAT='(which|command[[:space:]]+-v)[[:space:]]+((/usr)?/bin/)?systemctl'
RPL='test -d /run/systemd/system'
sed -i -E "s=$PAT=$RPL=" /var/lib/dpkg/info/expressvpn.{post,pre}* 2>/dev/null
fi

]]>

</preuninstall>

<postuninstall>
<![CDATA[
#echo "post-uninstall ..."
HELPER=(
.mozilla/native-messaging-hosts/com.expressvpn.helper.json
.config/BraveSoftware/Brave-Browser/NativeMessagingHosts/com.expressvpn.helper.json
.config/chromium/NativeMessagingHosts/com.expressvpn.helper.json
.config/google-chrome/NativeMessagingHosts/com.expressvpn.helper.json
)
USER=$(logname)
case "x$(dpkg-query -f '${db:Status-Abbrev}' -W expressvpn 2>/dev/null)" in
xr*)
apt-get -y remove --purge expressvpn
update-rc.d expressvpn remove 2>/dev/null
rm -f /usr/share/bash-completion/completions/expressvpn 2>/dev/null
rm -f /etc/bash_completion.d/expressvpn 2>/dev/null
rm -f /etc/defaults/expressvpn 2>/dev/null
rm -f /etc/init.d/expressvpn 2>/dev/null
rm -f /etc/apt/sources.list.d/expressvpn* 2>/dev/null
rm -f /usr/share/keyrings/expressvpn* 2>/dev/null
rm -rf /usr/lib/expressvpn 2>/dev/null
rm -rf /var/lib/expressvpn 2>/dev/null
rm -rf /var/log/expressvpn 2>/dev/null
rm -rf /var/run/expressvpn 2>/dev/null
for helper in "${HELPER[@]}"; do
rm /home/$USER/$helper 2>/dev/null
rm /root/$helper 2>/dev/null
done
;;
esac
echo "...$(gettext -d apt -s ' Done')"'!'
]]>

</postuninstall>

</app>

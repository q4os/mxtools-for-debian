<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Network
</category>

<name>
AirVPN
</name>

<description>
   <am>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</am>
   <ar>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</ar>
   <bg>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</bg>
   <bn>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</bn>
   <ca>AirVPN - Eddie - Entorn gràfic per OpenVPN amb característiques amigables</ca>
   <cs>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</cs>
   <da>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</da>
   <de>AirVPN - Eddie - OpenVPN GUI mit zusätzlichen nutzerfreundlichen Möglichkeiten</de>
   <el>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</el>
   <en>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</en>
   <es>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</es>
   <et>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</et>
   <eu>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</eu>
   <fa>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</fa>
   <fil_PH>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</fil_PH>
   <fi>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</fi>
   <fr>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</fr>
   <he_IL>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</he_IL>
   <hi>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</hi>
   <hr>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</hr>
   <hu>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</hu>
   <id>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</id>
   <is>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</is>
   <it>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</it>
   <ja_JP>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</ja_JP>
   <ja>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</ja>
   <kk>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</kk>
   <ko>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</ko>
   <lt>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</lt>
   <mk>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</mk>
   <mr>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</mr>
   <nb>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</nb>
   <nl>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</nl>
   <pl>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</pl>
   <pt_BR>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</pt_BR>
   <pt>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</pt>
   <ro>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</ro>
   <ru>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</ru>
   <sk>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</sk>
   <sl>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</sl>
   <sq>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</sq>
   <sr>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</sr>
   <sv>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</sv>
   <tr>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</tr>
   <uk>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</uk>
   <vi>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</vi>
   <zh_CN>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</zh_CN>
   <zh_TW>AirVPN - Eddie - OpenVPN GUI with additional user-friendly features</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>none</screenshot>

<preinstall>
# install airvpn
# prepare temp dir and files
DIR=$(mktemp -d /tmp/tempdir-airvpn-keyring.XXXXXXXXXXXXX)
chmod 700 $DIR
ASC=$DIR/airvpn.asc
GPG=$DIR/airvpn.gpg
KBX=$DIR/airvpn.kbx
AKR=/etc/apt/trusted.gpg.d/airvpn-archive-keyring.gpg
LST=/etc/apt/sources.list.d/airvpn-stable.list
SRC="deb http://eddie.website/repository/apt stable main"
INR=$DIR/InRelease

# prepare tidy-up
tidy_up() { rm -r /tmp/tempdir-airvpn-keyring.* 2>/dev/null; }
trap tidy_up EXIT

echo "Get AirVPN repository keyring"
curl -sq -o $ASC -RLJ https://eddie.website/repository/keys/eddie_maintainer_gpg.key
[ -f "$ASC" ] || { echo "Fatal: Failed to fetch AirVPN repository signing key"; exit 1; }
gpg --no-default-keyring --homedir=$DIR --keyring $KBX --import  $ASC 2>/dev/null

printf "Check Repository InRelease signature\n\n"
curl -sq -RLJ -o $INR http://eddie.website/repository/apt/dists/stable/InRelease
[ -f "$INR" ] || { echo "Fatal: Failed to fetch AirVPN InRelease"; exit 2; }

# E2S to handle ampersand redirects
E2S="$(printf '2\x3e\x261')"
CMD="env LC_ALL=C gpg --verify  $INR $E2S | sed -n 's/^gpg:.*using.*key //p' | grep -ioE '[A-F0-9]{40}'"
INS="$(eval $CMD)"
gpg --no-default-keyring --homedir=$DIR --keyring $KBX --output $GPG --export $INS 2>/dev/null
[ -f "$GPG" ] || { echo "Fatal: Repo signing key error "; exit 3; }
printf "Setup Repository Archive Key: \n"
CMD="env LC_ALL=C gpg --no-default-keyring --homedir=$DIR --keyring $KBX --list-keys $INS $E2S"
printf "$(eval $CMD)\n\n"

cp $GPG $AKR

if ! sed  -re '/^[[:space:]]*#/d; s/#.*//; /^[[:space:]]*deb[[:space:]]+/!d' /etc/apt/sources.list{,.d/*.list} \
| grep -sqE "eddie.website/repository/apt[[:space:]]+stable[[:space:]]+main" ; then
echo "Setup AirVPN repository apt source list"
echo "$SRC" | tee $LST
fi
printf "Refresh package list ...\n\n"
apt-get update

</preinstall>

<install_package_names>
eddie-ui
</install_package_names>

<postinstall>

</postinstall>

<uninstall_package_names>
eddie-ui
</uninstall_package_names>

<postuninstall>
# purge AirVPN-client
if dpkg-query -f '${db:Status-Abbrev}' -W eddie-ui  2>/dev/null 1>/dev/null; then
echo "Postuninstall: Purge AirVPN"
apt-get -y remove --purge eddie-ui
fi
rm -f /etc/apt/sources.list.d/airvpn-stable.list 2>/dev/null
rm -f /etc/apt/trusted.gpg.d/airvpn-archive-keyring.gpg 2>/dev/null
</postuninstall>

</app>

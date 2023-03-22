<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
Brave
</name>

<description>
   <am>Latest Brave browser</am>
   <ar>Latest Brave browser</ar>
   <bg>Latest Brave browser</bg>
   <bn>Latest Brave browser</bn>
   <ca>Darrer navegador Brave</ca>
   <cs>Latest Brave browser</cs>
   <da>Latest Brave browser</da>
   <de>Brave Browser (neuste Version)</de>
   <el>Latest Brave browser</el>
   <en>Latest Brave browser</en>
   <es>Latest Brave browser</es>
   <et>Latest Brave browser</et>
   <eu>Latest Brave browser</eu>
   <fa>Latest Brave browser</fa>
   <fil_PH>Latest Brave browser</fil_PH>
   <fi>Latest Brave browser</fi>
   <fr>Latest Brave browser</fr>
   <he_IL>Latest Brave browser</he_IL>
   <hi>Latest Brave browser</hi>
   <hr>Latest Brave browser</hr>
   <hu>Latest Brave browser</hu>
   <id>Latest Brave browser</id>
   <is>Latest Brave browser</is>
   <it>Latest Brave browser</it>
   <ja_JP>Latest Brave browser</ja_JP>
   <ja>Latest Brave browser</ja>
   <kk>Latest Brave browser</kk>
   <ko>Latest Brave browser</ko>
   <lt>Latest Brave browser</lt>
   <mk>Latest Brave browser</mk>
   <mr>Latest Brave browser</mr>
   <nb>Latest Brave browser</nb>
   <nl>Latest Brave browser</nl>
   <pl>Latest Brave browser</pl>
   <pt_BR>Latest Brave browser</pt_BR>
   <pt>Latest Brave browser</pt>
   <ro>Latest Brave browser</ro>
   <ru>Latest Brave browser</ru>
   <sk>Latest Brave browser</sk>
   <sl>Latest Brave browser</sl>
   <sq>Latest Brave browser</sq>
   <sr>Latest Brave browser</sr>
   <sv>Latest Brave browser</sv>
   <tr>Latest Brave browser</tr>
   <uk>Latest Brave browser</uk>
   <vi>Latest Brave browser</vi>
   <zh_CN>Latest Brave browser</zh_CN>
   <zh_TW>Latest Brave browser</zh_TW>
</description>

<installable>
64
</installable>

<screenshot> </screenshot>

<preinstall>
URL="https://brave-browser-apt-release.s3.brave.com"
ASC="${URL}/brave-core.asc"
TKR="/etc/apt/trusted.gpg.d/brave-browser-release.gpg"
SRC="/etc/apt/sources.list.d/brave-browser-release.list"
VER="$(sed -n  's/^VERSION_CODENAME=//p' /etc/os-release)"
case $(cat /etc/debian_version) in
8*) VER=jessie   ;;
9*) VER=stretch  ;;
10*) VER=buster   ;;
11*) VER=bullseye ;;
*) VER=bullseye ;;
esac

DEB="deb [arch=amd64] ${URL}/ $VER main"

curl -s $ASC | ( apt-key --keyring $TKR add - 2>/dev/null )
echo "$DEB" | tee $SRC
apt-get update

</preinstall>

<install_package_names>
brave-browser
</install_package_names>

<postinstall>
# fix sandbox

SYSCTL=/etc/sysctl.d/10-securized-yama-scope.conf
echo "kernel.yama.ptrace_scope = 1" > $SYSCTL
if [ -f $SYSCTL ]; then  sysctl --load $SYSCTL; fi

</postinstall>

<uninstall_package_names>
brave-browser
brave-keyring
</uninstall_package_names>

<postuninstall>

apt-get -y remove --purge brave-browser brave-browser
SRC=/etc/apt/sources.list.d/brave-browser-release.list
TKR="/etc/apt/trusted.gpg.d/brave-browser-release.gpg"
rm -f $SRC 2>/dev/null
rm -f $TKR 2>/dev/null
apt-get update
</postuninstall>

</app>

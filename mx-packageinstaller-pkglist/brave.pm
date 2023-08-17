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
   <el>Το τελευταίο πρόγραμμα περιήγησης Brave</el>
   <en>Latest Brave browser</en>
   <es>Último navegador Brave</es>
   <et>Latest Brave browser</et>
   <eu>Latest Brave browser</eu>
   <fa>Latest Brave browser</fa>
   <fil_PH>Latest Brave browser</fil_PH>
   <fi>Viimeisin Brave-selain</fi>
   <fr>Dernière version du navigateur Brave</fr>
   <he_IL>Latest Brave browser</he_IL>
   <hi>नवीनतम ब्रेव ब्राउज़र</hi>
   <hr>Latest Brave browser</hr>
   <hu>Latest Brave browser</hu>
   <id>Latest Brave browser</id>
   <is>Latest Brave browser</is>
   <it>Ultimo browser Brave</it>
   <ja>最新の Brave ブラウザ</ja>
   <kk>Latest Brave browser</kk>
   <ko>Latest Brave browser</ko>
   <lt>Latest Brave browser</lt>
   <mk>Latest Brave browser</mk>
   <mr>Latest Brave browser</mr>
   <nb>Nettleseren Brave, seneste utgivelse</nb>
   <nl>Meest recente Brave browser</nl>
   <pl>Najnowsza przeglądarka Brave</pl>
   <pt_BR>Brave - Versão mais recente do navegador de internet</pt_BR>
   <pt>Navegador web Brave, última versão</pt>
   <ro>Latest Brave browser</ro>
   <ru>Последняя версия браузера Brave</ru>
   <sk>Latest Brave browser</sk>
   <sl>Zadnja različica Brave brskalnika</sl>
   <sq>Shfletuesi më i ri Brave</sq>
   <sr>Latest Brave browser</sr>
   <sv>Senaste Brave Webbläsare</sv>
   <tr>En son Brave tarayıcı</tr>
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

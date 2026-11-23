<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install the NordVPN CLI client with dual-init (systemd/sysvinit) support. -->
<app>

<category>
Network
</category>

<name>
NordVPN CLI
</name>

<description>
   <am>NordVPN CLI and sysVinit scripts</am>
   <ar>NordVPN CLI and sysVinit scripts</ar>
   <bg>NordVPN CLI and sysVinit scripts</bg>
   <bn>NordVPN CLI and sysVinit scripts</bn>
   <ca>Scripts CLI NordVPN i sysVinit</ca>
   <cs>NordVPN CLI and sysVinit scripts</cs>
   <da>NordVPN CLI and sysVinit scripts</da>
   <de>NordVPN CLI und sysVinit Scripte</de>
   <el>Σενάρια του NordVPN CLI και sysVinit</el>
   <en>NordVPN CLI and sysVinit scripts</en>
   <es>CLI NordVPN y scripts sysVinit</es>
   <et>NordVPN CLI and sysVinit scripts</et>
   <eu>NordVPN CLI and sysVinit scripts</eu>
   <fa>NordVPN CLI and sysVinit scripts</fa>
   <fil_PH>NordVPN CLI and sysVinit scripts</fil_PH>
   <fi>NordVPN CLI ja sysVinit-skriptit</fi>
   <fr>Scripts pour NordVPN CLI et sysVinit</fr>
   <he_IL>NordVPN CLI and sysVinit scripts</he_IL>
   <hi>NordVPN CLI व sysVinit स्क्रिप्ट</hi>
   <hr>NordVPN CLI and sysVinit scripts</hr>
   <hu>NordVPN CLI and sysVinit scripts</hu>
   <id>NordVPN CLI and sysVinit scripts</id>
   <is>NordVPN CLI and sysVinit scripts</is>
   <it>NordVPN CLI e script sysVinit</it>
   <ja>NordVPN CLI と sysVinit スクリプト</ja>
   <kk>NordVPN CLI and sysVinit scripts</kk>
   <ko>NordVPN CLI and sysVinit scripts</ko>
   <lt>NordVPN CLI and sysVinit scripts</lt>
   <mk>NordVPN CLI and sysVinit scripts</mk>
   <mr>NordVPN CLI and sysVinit scripts</mr>
   <nb>NordVPN-CLI og sysVinit-skript</nb>
   <nl>NordVPN CLI en sysVinit scripts</nl>
   <pl>NordVPN CLI and sysVinit scripts</pl>
   <pt_BR>Scripts para sysVinit e CLI NordVPN</pt_BR>
   <pt>Scripts para sysVinit e CLI NordVPN</pt>
   <ro>NordVPN CLI and sysVinit scripts</ro>
   <ru>NordVPN CLI и скрипты SysVinit</ru>
   <sk>NordVPN CLI and sysVinit scripts</sk>
   <sl>NordVPN CLI in sysVinit skripte</sl>
   <sq>CLI NordVPN dhe programthe për sysVinit</sq>
   <sr>NordVPN CLI and sysVinit scripts</sr>
   <sv>NordVPN CLI och sysVinit skript</sv>
   <tr>NordVPN CLI ve sysVinit betikleri</tr>
   <uk>NordVPN CLI and sysVinit scripts</uk>
   <vi>NordVPN CLI and sysVinit scripts</vi>
   <zh_CN>NordVPN CLI and sysVinit scripts</zh_CN>
   <zh_TW>NordVPN CLI and sysVinit scripts</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/nordvpn.sh"
[ -f "$SH" ] || { echo "ERROR: NordVPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" preinstall
]]>
</preinstall>

<install_package_names>
nordvpn
xterm
</install_package_names>

<postinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/nordvpn.sh"
[ -f "$SH" ] || { echo "ERROR: NordVPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postinstall
]]>
</postinstall>

<uninstall_package_names>
nordvpn
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/nordvpn.sh"
[ -f "$SH" ] || { echo "ERROR: NordVPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall full
]]>
</postuninstall>

</app>

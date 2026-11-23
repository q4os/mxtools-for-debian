<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install the NordVPN GUI client with dual-init (systemd/sysvinit) support. -->
<app>

<category>
Network
</category>

<name>
NordVPN GUI
</name>

<description>
   <am>NordVPN GUI App and sysVinit scripts</am>
   <ar>NordVPN GUI App and sysVinit scripts</ar>
   <bg>NordVPN GUI App and sysVinit scripts</bg>
   <bn>NordVPN GUI App and sysVinit scripts</bn>
   <ca>Scripts GUI App NordVPN i sysVinit</ca>
   <cs>NordVPN GUI App and sysVinit scripts</cs>
   <da>NordVPN GUI App and sysVinit scripts</da>
   <de>NordVPN GUI App und sysVinit Scripte</de>
   <el>Σενάρια του NordVPN GUI App και sysVinit</el>
   <en>NordVPN GUI App and sysVinit scripts</en>
   <es>GUI App NordVPN y scripts sysVinit</es>
   <et>NordVPN GUI App and sysVinit scripts</et>
   <eu>NordVPN GUI App and sysVinit scripts</eu>
   <fa>NordVPN GUI App and sysVinit scripts</fa>
   <fil_PH>NordVPN GUI App and sysVinit scripts</fil_PH>
   <fi>NordVPN GUI-sovellus ja sysVinit-skriptit</fi>
   <fr>Scripts pour NordVPN GUI App et sysVinit</fr>
   <he_IL>NordVPN GUI App and sysVinit scripts</he_IL>
   <hi>NordVPN GUI ऐप व sysVinit स्क्रिप्ट</hi>
   <hr>NordVPN GUI App and sysVinit scripts</hr>
   <hu>NordVPN GUI App and sysVinit scripts</hu>
   <id>NordVPN GUI App and sysVinit scripts</id>
   <is>NordVPN GUI App and sysVinit scripts</is>
   <it>NordVPN GUI App e script sysVinit</it>
   <ja>NordVPN GUI アプリと sysVinit スクリプト</ja>
   <kk>NordVPN GUI App and sysVinit scripts</kk>
   <ko>NordVPN GUI App and sysVinit scripts</ko>
   <lt>NordVPN GUI App and sysVinit scripts</lt>
   <mk>NordVPN GUI App and sysVinit scripts</mk>
   <mr>NordVPN GUI App and sysVinit scripts</mr>
   <nb>NordVPN-GUI-app og sysVinit-skript</nb>
   <nl>NordVPN GUI app en sysVinit scripts</nl>
   <pl>NordVPN GUI App and sysVinit scripts</pl>
   <pt_BR>Scripts para sysVinit e GUI App NordVPN</pt_BR>
   <pt>Scripts para sysVinit e GUI App NordVPN</pt>
   <ro>NordVPN GUI App and sysVinit scripts</ro>
   <ru>NordVPN GUI-приложение и скрипты SysVinit</ru>
   <sk>NordVPN GUI App and sysVinit scripts</sk>
   <sl>NordVPN GUI aplikacija in sysVinit skripte</sl>
   <sq>GUI Aplikacioni NordVPN dhe programthe për sysVinit</sq>
   <sr>NordVPN GUI App and sysVinit scripts</sr>
   <sv>NordVPN GUI-app och sysVinit skript</sv>
   <tr>NordVPN GUI uygulaması ve sysVinit betikleri</tr>
   <uk>NordVPN GUI App and sysVinit scripts</uk>
   <vi>NordVPN GUI App and sysVinit scripts</vi>
   <zh_CN>NordVPN GUI App and sysVinit scripts</zh_CN>
   <zh_TW>NordVPN GUI App and sysVinit scripts</zh_TW>
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
nordvpn-gui
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
nordvpn-gui
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/nordvpn.sh"
[ -f "$SH" ] || { echo "ERROR: NordVPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall gui
]]>
</postuninstall>

</app>

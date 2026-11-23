<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install Mullvad VPN with dual-init (systemd/sysvinit) support. -->
<app>

<category>
Network
</category>

<name>
Mullvad VPN
</name>

<description>
   <am>Mullvad VPN and sysVinit scripts</am>
   <ar>سكربتات Mullvad VPN و sysVinit</ar>
   <bg>Mullvad VPN и sysVinit скриптове</bg>
   <bn>Mullvad VPN এবং sysVinit স্ক্রিপ্ট</bn>
   <ca>Scripts Mullvad VPN i sysVinit</ca>
   <cs>Mullvad VPN a sysVinit skripty</cs>
   <da>Mullvad VPN og sysVinit-scripts</da>
   <de>Mullvad VPN und sysVinit Scripte</de>
   <el>Σενάρια του Mullvad VPN και sysVinit</el>
   <en>Mullvad VPN and sysVinit scripts</en>
   <es>Mullvad VPN y scripts sysVinit</es>
   <et>Mullvad VPN ja sysVinit skriptid</et>
   <eu>Mullvad VPN and sysVinit scripts</eu>
   <fa>اسکریپت‌های Mullvad VPN و sysVinit</fa>
   <fil_PH>Mullvad VPN at sysVinit scripts</fil_PH>
   <fi>Mullvad VPN ja sysVinit script-ohjelmakoodisyötteet</fi>
   <fr>Scripts pour Mullvad VPN et sysVinit</fr>
   <he_IL>Mullvad VPN וסקריפטים של sysVinit</he_IL>
   <hi>Mullvad वीपीएन व sysVinit स्क्रिप्ट</hi>
   <hr>Mullvad VPN i sysVinit skripte</hr>
   <hu>Mullvad VPN és sysVinit szkriptek</hu>
   <id>Mullvad VPN dan skrip sysVinit</id>
   <is>Mullvad VPN and sysVinit scripts</is>
   <it>Mullvad VPN e script sysVinit</it>
   <ja>Mullvad VPN と sysVinit スクリプト</ja>
   <kk>Mullvad VPN and sysVinit scripts</kk>
   <ko>Mullvad VPN 및 sysVinit 스크립트</ko>
   <lt>Mullvad VPN ir sysVinit scenarijai</lt>
   <mk>Mullvad VPN and sysVinit scripts</mk>
   <mr>Mullvad VPN and sysVinit scripts</mr>
   <nb>Mullvad VPN- og sysVinit-skript</nb>
   <nl>Mullvad VPN en sysVinit scripts</nl>
   <pl>Skrypty Mullvad VPN i sysVinit</pl>
   <pt_BR>Scripts para sysVinit e Mullvad VPN</pt_BR>
   <pt>Scripts para sysVinit e VPN Mullvad</pt>
   <ro>Mullvad VPN și scripturi sysVinit</ro>
   <ru>VPN и SysVinit - скрипты от Mullvad</ru>
   <sk>Mullvad VPN a skripty sysVinit</sk>
   <sl>Mullvad VPN in syVinit skripte</sl>
   <sq>programthe për Mullvad VPN dhe sysVinit</sq>
   <sr>Mullvad VPN и sysVinit скрипте</sr>
   <sv>Mullvad VPN och sysVinit skript</sv>
   <tr>Mullvad VPN ve sysVinit betikleri</tr>
   <uk>Mullvad VPN та скрипти sysVinit</uk>
   <vi>Mullvad VPN và các tập lệnh sysVinit</vi>
   <zh_CN>Mullvad VPN 和 sysVinit 脚本</zh_CN>
   <zh_TW>Mullvad VPN 和 sysVinit 腳本</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/mullvad-vpn.sh"
[ -f "$SH" ] || { echo "ERROR: Mullvad VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" preinstall
]]>
</preinstall>

<install_package_names>
</install_package_names>

<postinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/mullvad-vpn.sh"
[ -f "$SH" ] || { echo "ERROR: Mullvad VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postinstall
]]>
</postinstall>

<uninstall_package_names>
mullvad-vpn
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/mullvad-vpn.sh"
[ -f "$SH" ] || { echo "ERROR: Mullvad VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall
]]>
</postuninstall>

</app>

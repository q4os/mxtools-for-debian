<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install Windscribe VPN with dual-init (systemd/sysvinit) support. -->
<app>

<category>
Network
</category>

<name>
Windscribe VPN
</name>

<description>
   <am>Windscribe VPN and sysVinit scripts</am>
   <ar>سكربتات Windscribe VPN و sysVinit</ar>
   <bg>Windscribe VPN и sysVinit скриптове</bg>
   <bn>Windscribe VPN এবং sysVinit স্ক্রিপ্ট</bn>
   <ca>Scripts per Windscribe VPN i sysVinit</ca>
   <cs>Windscribe VPN a sysVinit skripty</cs>
   <da>Windscribe VPN og sysVinit-scripts</da>
   <de>Windscribe VPN und sysVinit Skripte</de>
   <el>σενάρια του Windscribe VPN client και sysVinit</el>
   <en>Windscribe VPN and sysVinit scripts</en>
   <es>Windscribe VPN y sysVinit scripts</es>
   <et>Windscribe VPN ja sysVinit skriptid</et>
   <eu>Windscribe VPN and sysVinit scripts</eu>
   <fa>اسکریپت‌های Windscribe VPN و sysVinit</fa>
   <fil_PH>Windscribe VPN at sysVinit scripts</fil_PH>
   <fi>Windscribe VPN ja sysVinit-skriptit</fi>
   <fr>Scripts pour Windscribe VPN et sysVinit</fr>
   <he_IL>Windscribe VPN וסקריפטים של sysVinit</he_IL>
   <hi>Windscribe VPN और sysVinit स्क्रिप्ट</hi>
   <hr>Windscribe VPN i sysVinit skripte</hr>
   <hu>Windscribe VPN és sysVinit szkriptek</hu>
   <id>Windscribe VPN dan skrip sysVinit</id>
   <is>Windscribe VPN and sysVinit scripts</is>
   <it>Windscribe VPN e script sysVinit</it>
   <ja>Windscribe VPN と sysVinit スクリプト</ja>
   <kk>Windscribe VPN and sysVinit scripts</kk>
   <ko>Windscribe VPN 및 sysVinit 스크립트</ko>
   <lt>Windscribe VPN ir sysVinit scenarijai</lt>
   <mk>Windscribe VPN and sysVinit scripts</mk>
   <mr>Windscribe VPN and sysVinit scripts</mr>
   <nb>Windscribe VPN og sysVinit-skript</nb>
   <nl>Windscribe VPN en sysVinit scripts</nl>
   <pl>Skrypty Windscribe VPN i sysVinit</pl>
   <pt_BR>Scripts para sysVinit e Windscribe VPN</pt_BR>
   <pt>Scripts Windscribe VPN e sysVinit</pt>
   <ro>Windscribe VPN și scripturi sysVinit</ro>
   <ru>Windscribe VPN и cкрипты sysVinit</ru>
   <sk>Windscribe VPN a skripty sysVinit</sk>
   <sl>Windscribe VPN in sysVinit skripti</sl>
   <sq>Windscribe VPN and sysVinit scripts</sq>
   <sr>Windscribe VPN и sysVinit скрипте</sr>
   <sv>Windscribe VPN och sysVinit skript</sv>
   <tr>Windscribe VPN ve sysVinit betikleri</tr>
   <uk>Windscribe VPN та скрипти sysVinit</uk>
   <vi>Windscribe VPN và các tập lệnh sysVinit</vi>
   <zh_CN>Windscribe VPN 和 sysVinit 脚本</zh_CN>
   <zh_TW>Windscribe VPN 和 sysVinit 腳本</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/windscribe-vpn.sh"
[ -f "$SH" ] || { echo "ERROR: Windscribe VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" preinstall
]]>
</preinstall>

<install_package_names>

</install_package_names>

<postinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/windscribe-vpn.sh"
[ -f "$SH" ] || { echo "ERROR: Windscribe VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postinstall
]]>
</postinstall>

<uninstall_package_names>
windscribe
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/windscribe-vpn.sh"
[ -f "$SH" ] || { echo "ERROR: Windscribe VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall
]]>
</postuninstall>

</app>

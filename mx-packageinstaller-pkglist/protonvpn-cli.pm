<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install the Proton VPN CLI with dual-init (systemd/sysvinit) support. -->
<app>

<category>
Network
</category>

<name>
Proton VPN CLI
</name>

<description>
   <am>Proton VPN CLI and sysVinit scripts</am>
   <ar>سكربتات Proton VPN CLI و sysVinit</ar>
   <bg>Proton VPN CLI и sysVinit скриптове</bg>
   <bn>Proton VPN CLI এবং sysVinit স্ক্রিপ্ট</bn>
   <ca>Scripts per Proton VPN CLI i sysVinit</ca>
   <cs>Proton VPN CLI a sysVinit skripty</cs>
   <da>Proton VPN CLI og sysVinit-scripts</da>
   <de>Proton VPN CLI und sysVinit Skripte</de>
   <el>σενάρια του Proton VPN CLI και sysVinit</el>
   <en>Proton VPN CLI and sysVinit scripts</en>
   <es>Proton VPN CLI y sysVinit scripts</es>
   <et>Proton VPN CLI ja sysVinit skriptid</et>
   <eu>Proton VPN CLI and sysVinit scripts</eu>
   <fa>اسکریپت‌های Proton VPN CLI و sysVinit</fa>
   <fil_PH>Proton VPN CLI at sysVinit scripts</fil_PH>
   <fi>Proton VPN CLI ja sysVinit-skriptit</fi>
   <fr>Scripts pour Proton VPN CLI et sysVinit</fr>
   <he_IL>Proton VPN CLI וסקריפטים של sysVinit</he_IL>
   <hi>Proton VPN CLI और sysVinit स्क्रिप्ट</hi>
   <hr>Proton VPN CLI i sysVinit skripte</hr>
   <hu>Proton VPN CLI és sysVinit szkriptek</hu>
   <id>Proton VPN CLI dan skrip sysVinit</id>
   <is>Proton VPN CLI and sysVinit scripts</is>
   <it>Proton VPN CLI e script sysVinit</it>
   <ja>Proton VPN CLI と sysVinit スクリプト</ja>
   <kk>Proton VPN CLI and sysVinit scripts</kk>
   <ko>Proton VPN CLI 및 sysVinit 스크립트</ko>
   <lt>Proton VPN CLI ir sysVinit scenarijai</lt>
   <mk>Proton VPN CLI and sysVinit scripts</mk>
   <mr>Proton VPN CLI and sysVinit scripts</mr>
   <nb>Proton VPN CLI og sysVinit-skript</nb>
   <nl>Proton VPN CLI en sysVinit scripts</nl>
   <pl>Skrypty Proton VPN CLI i sysVinit</pl>
   <pt_BR>Scripts para sysVinit e Proton VPN CLI</pt_BR>
   <pt>Scripts Proton VPN CLI e sysVinit</pt>
   <ro>Proton VPN CLI și scripturi sysVinit</ro>
   <ru>Proton VPN CLI и cкрипты sysVinit</ru>
   <sk>Proton VPN CLI a skripty sysVinit</sk>
   <sl>Proton VPN CLI in sysVinit skripti</sl>
   <sq>Proton VPN CLI and sysVinit scripts</sq>
   <sr>Proton VPN CLI и sysVinit скрипте</sr>
   <sv>Proton VPN CLI och sysVinit skript</sv>
   <tr>Proton VPN CLI ve sysVinit betikleri</tr>
   <uk>Proton VPN CLI та скрипти sysVinit</uk>
   <vi>Proton VPN CLI và các tập lệnh sysVinit</vi>
   <zh_CN>Proton VPN CLI 和 sysVinit 脚本</zh_CN>
   <zh_TW>Proton VPN CLI 和 sysVinit 腳本</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/protonvpn.sh"
[ -f "$SH" ] || { echo "ERROR: Proton VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" preinstall cli
]]>
</preinstall>

<install_package_names>
</install_package_names>

<postinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/protonvpn.sh"
[ -f "$SH" ] || { echo "ERROR: Proton VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postinstall
]]>
</postinstall>

<uninstall_package_names>
proton-vpn-cli
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/protonvpn.sh"
[ -f "$SH" ] || { echo "ERROR: Proton VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall cli
]]>
</postuninstall>

</app>

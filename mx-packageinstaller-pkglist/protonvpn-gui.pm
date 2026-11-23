<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install Proton VPN GUI with dual-init (systemd/sysvinit) support. -->
<app>

<category>
Network
</category>

<name>
Proton VPN GUI
</name>

<description>
   <am>Proton VPN GUI App and sysVinit scripts</am>
   <ar>سكربتات Proton VPN GUI App و sysVinit</ar>
   <bg>Proton VPN GUI App и sysVinit скриптове</bg>
   <bn>Proton VPN GUI App এবং sysVinit স্ক্রিপ্ট</bn>
   <ca>Scripts per Proton VPN GUI App i sysVinit</ca>
   <cs>Proton VPN GUI App a sysVinit skripty</cs>
   <da>Proton VPN GUI App og sysVinit-scripts</da>
   <de>Proton VPN GUI App und sysVinit Skripte</de>
   <el>σενάρια του Proton VPN GUI App και sysVinit</el>
   <en>Proton VPN GUI App and sysVinit scripts</en>
   <es>Proton VPN GUI App y sysVinit scripts</es>
   <et>Proton VPN GUI App ja sysVinit skriptid</et>
   <eu>Proton VPN GUI App and sysVinit scripts</eu>
   <fa>اسکریپت‌های Proton VPN GUI App و sysVinit</fa>
   <fil_PH>Proton VPN GUI App at sysVinit scripts</fil_PH>
   <fi>Proton VPN GUI App ja sysVinit-skriptit</fi>
   <fr>Scripts pour Proton VPN GUI App et sysVinit</fr>
   <he_IL>Proton VPN GUI App וסקריפטים של sysVinit</he_IL>
   <hi>Proton VPN GUI App और sysVinit स्क्रिप्ट</hi>
   <hr>Proton VPN GUI App i sysVinit skripte</hr>
   <hu>Proton VPN GUI App és sysVinit szkriptek</hu>
   <id>Proton VPN GUI App dan skrip sysVinit</id>
   <is>Proton VPN GUI App and sysVinit scripts</is>
   <it>Proton VPN GUI App e script sysVinit</it>
   <ja>Proton VPN GUI App と sysVinit スクリプト</ja>
   <kk>Proton VPN GUI App and sysVinit scripts</kk>
   <ko>Proton VPN GUI App 및 sysVinit 스크립트</ko>
   <lt>Proton VPN GUI App ir sysVinit scenarijai</lt>
   <mk>Proton VPN GUI App and sysVinit scripts</mk>
   <mr>Proton VPN GUI App and sysVinit scripts</mr>
   <nb>Proton VPN GUI App og sysVinit-skript</nb>
   <nl>Proton VPN GUI App en sysVinit scripts</nl>
   <pl>Skrypty Proton VPN GUI App i sysVinit</pl>
   <pt_BR>Scripts para sysVinit e Proton VPN GUI App</pt_BR>
   <pt>Scripts Proton VPN GUI App e sysVinit</pt>
   <ro>Proton VPN GUI App și scripturi sysVinit</ro>
   <ru>Proton VPN GUI App и cкрипты sysVinit</ru>
   <sk>Proton VPN GUI App a skripty sysVinit</sk>
   <sl>Proton VPN GUI App in sysVinit skripti</sl>
   <sq>Proton VPN GUI App and sysVinit scripts</sq>
   <sr>Proton VPN GUI App и sysVinit скрипте</sr>
   <sv>Proton VPN GUI App och sysVinit skript</sv>
   <tr>Proton VPN GUI App ve sysVinit betikleri</tr>
   <uk>Proton VPN GUI App та скрипти sysVinit</uk>
   <vi>Proton VPN GUI App và các tập lệnh sysVinit</vi>
   <zh_CN>Proton VPN GUI App 和 sysVinit 脚本</zh_CN>
   <zh_TW>Proton VPN GUI App 和 sysVinit 腳本</zh_TW>
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
"$SH" preinstall gui
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
proton-vpn-gnome-desktop
proton-vpn-gtk-app
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/protonvpn.sh"
[ -f "$SH" ] || { echo "ERROR: Proton VPN installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall gui
]]>
</postuninstall>

</app>

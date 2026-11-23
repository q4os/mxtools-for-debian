<?xml version="1.0" encoding="UTF-8"?>
<!-- Part of MX Package Installer's mx-packageinstaller-pkglist.
     Purpose: install Mullvad Browser via the shared Mullvad apt repo. -->
<app>

<category>
Browser
</category>

<name>
Mullvad browser
</name>

<description>
   <am>Latest Mullvad browser</am>
   <ar>Latest Mullvad browser</ar>
   <bg>Latest Mullvad browser</bg>
   <bn>Latest Mullvad browser</bn>
   <ca>Darrer navegador Mullvad</ca>
   <cs>Latest Mullvad browser</cs>
   <da>Seneste Mullvad-browser</da>
   <de>Aktueller Mullvad-Browser</de>
   <el>Τελευταίο πρόγραμμα περιήγησης Mullvad</el>
   <en>Latest Mullvad browser</en>
   <es>El último Mullvad (estable)</es>
   <et>Latest Mullvad browser</et>
   <eu>Latest Mullvad browser</eu>
   <fa>Latest Mullvad browser</fa>
   <fil_PH>Latest Mullvad browser</fil_PH>
   <fi>Latest Mullvad browser</fi>
   <fr>La dernière version de Mullvad</fr>
   <he_IL>Latest Mullvad browser</he_IL>
   <hi>Latest Mullvad browser</hi>
   <hr>Latest Mullvad browser</hr>
   <hu>Latest Mullvad browser</hu>
   <id>Latest Mullvad browser</id>
   <is>Latest Mullvad browser</is>
   <it>Ultima versione del browser Mullvad</it>
   <ja>Latest Mullvad browser</ja>
   <kk>Latest Mullvad browser</kk>
   <ko>Latest Mullvad browser</ko>
   <lt>Latest Mullvad browser</lt>
   <mk>Latest Mullvad browser</mk>
   <mr>Latest Mullvad browser</mr>
   <nb>Latest Mullvad browser</nb>
   <nl>Meest recente Mullvad browser</nl>
   <pl>najnowsza przeglądarka Mullvad</pl>
   <pt_BR>Versão mais recente do navegador web Mullvad</pt_BR>
   <pt>Versão mais recente do navegador web Mullvad</pt>
   <ro>Latest Mullvad browser</ro>
   <ru>Браузер Mullvad</ru>
   <sk>Posledný Mullvad prehliadač</sk>
   <sl>Zadnja različica googlovega Mullvad brskalnika</sl>
   <sq>Latest Mullvad browser</sq>
   <sr>Latest Mullvad browser</sr>
   <sv>SenasteMullvad webbläsare</sv>
   <tr>Latest Mullvad browser</tr>
   <uk>Крайня стабільна версія браузера Mullvad</uk>
   <vi>Latest Mullvad browser</vi>
   <zh_CN>Latest Mullvad browser</zh_CN>
   <zh_TW>Latest Mullvad browser</zh_TW>
</description>

<installable>
 64
</installable>

<screenshot></screenshot>

<preinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/mullvad-browser.sh"
[ -f "$SH" ] || { echo "ERROR: Mullvad browser installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" preinstall
]]>
</preinstall>

<install_package_names>
mullvad-browser
</install_package_names>

<postinstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/mullvad-browser.sh"
[ -f "$SH" ] || { echo "ERROR: Mullvad browser installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postinstall
]]>
</postinstall>

<preuninstall>

</preuninstall>

<uninstall_package_names>
mullvad-browser
</uninstall_package_names>

<postuninstall>
<![CDATA[
SH="/usr/share/mx-packageinstaller-pkglist/mullvad-browser.sh"
[ -f "$SH" ] || { echo "ERROR: Mullvad browser installer helper script not found: $SH"; exit 1; }
[ -x "$SH" ] || chmod +x "$SH"
"$SH" postuninstall
]]>
</postuninstall>
</app>

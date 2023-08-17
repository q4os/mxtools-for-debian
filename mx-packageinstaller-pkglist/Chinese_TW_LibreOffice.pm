<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Chinese_TW_LO
</name>

<description>
   <am>Chinese_traditional localisation of LibreOffice</am>
   <ar>Chinese_traditional localisation of LibreOffice</ar>
   <bg>Chinese_traditional localisation of LibreOffice</bg>
   <bn>Chinese_traditional localisation of LibreOffice</bn>
   <ca>Localització de LibreOffice en Xinès tradicional</ca>
   <cs>Chinese_traditional localisation of LibreOffice</cs>
   <da>Kinesisk (traditionel) oversættelse af LibreOffice</da>
   <de>Chinesische (traditionell) Lokalisierung von LibreOffice</de>
   <el>LibreOffice για παραδοσιακά Κινεζικά</el>
   <en>Chinese_traditional localisation of LibreOffice</en>
   <es>Localización Chino tradicional de LibreOffice</es>
   <et>Chinese_traditional localisation of LibreOffice</et>
   <eu>Chinese_traditional localisation of LibreOffice</eu>
   <fa>Chinese_traditional localisation of LibreOffice</fa>
   <fil_PH>Chinese_traditional localisation of LibreOffice</fil_PH>
   <fi>Perinteisen kiinankielen kotoistus LibreOffice:lle</fi>
   <fr>Localisation chinois-traditionnel pour LibreOffice</fr>
   <he_IL>Chinese_traditional localisation of LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस का पारंपरिक चीनी संस्करण</hi>
   <hr>Chinese_traditional localisation of LibreOffice</hr>
   <hu>Chinese_traditional localisation of LibreOffice</hu>
   <id>Chinese_traditional localisation of LibreOffice</id>
   <is>Chinese_traditional localisation of LibreOffice</is>
   <it>Localizzazione in Cinese_tradizionale di LibreOffice </it>
   <ja> LibreOffice の繁体字中国語パック</ja>
   <kk>Chinese_traditional localisation of LibreOffice</kk>
   <ko>Chinese_traditional localisation of LibreOffice</ko>
   <lt>Chinese_traditional localisation of LibreOffice</lt>
   <mk>Chinese_traditional localisation of LibreOffice</mk>
   <mr>Chinese_traditional localisation of LibreOffice</mr>
   <nb>Kinesisk (tradisjonell) lokaltilpassing av LibreOffice</nb>
   <nl>Traditioneel Chinese lokalisatie van LibreOffice</nl>
   <pl>Chiński tradycyjny lokalizacja LibreOffice</pl>
   <pt_BR>Chinês tradicional Localização para o LibreOffice</pt_BR>
   <pt>Chinês_tradicional Localização para LibreOffice</pt>
   <ro>Chinese_traditional localisation of LibreOffice</ro>
   <ru>Китайская традиционная локализация LibreOffice</ru>
   <sk>Chinese_traditional localisation of LibreOffice</sk>
   <sl>Kitajske (tradicionalno) krajevne nastavitve za LibreOffice</sl>
   <sq>Përkthimi i LibreOffice-it në kinezçe tradicionale</sq>
   <sr>Chinese_traditional localisation of LibreOffice</sr>
   <sv>Kinesisk_traditionell lokalisering av LibreOffice</sv>
   <tr>LibreOffice Geleneksel_Çince yerelleştirmesi</tr>
   <uk>Chinese_traditional локалізація LibreOffice</uk>
   <vi>Chinese_traditional localisation of LibreOffice</vi>
   <zh_CN>Chinese_traditional localisation of LibreOffice</zh_CN>
   <zh_TW>Chinese_traditional localisation of LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-zh-tw
libreoffice-help-zh-tw
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-zh-tw
libreoffice-help-zh-tw
</uninstall_package_names>
</app>

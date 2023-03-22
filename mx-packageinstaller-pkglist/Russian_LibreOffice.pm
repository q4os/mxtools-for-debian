<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Russian_Libreoffice
</name>

<description>
   <am>Russian localisation of LibreOffice</am>
   <ar>Russian localisation of LibreOffice</ar>
   <bg>Russian localisation of LibreOffice</bg>
   <bn>Russian localisation of LibreOffice</bn>
   <ca>Localització de LibreOffice en Rus</ca>
   <cs>Russian localisation of LibreOffice</cs>
   <da>Russisk oversættelse af LibreOffice</da>
   <de>Russische Lokalisierung von LibreOffice</de>
   <el>LibreOffice στα ρωσικά</el>
   <en>Russian localisation of LibreOffice</en>
   <es>Localización Rusa de LibreOffice</es>
   <et>Russian localisation of LibreOffice</et>
   <eu>Russian localisation of LibreOffice</eu>
   <fa>Russian localisation of LibreOffice</fa>
   <fil_PH>Russian localisation of LibreOffice</fil_PH>
   <fi>Russian localisation of LibreOffice</fi>
   <fr>Localisation russe pour LibreOffice</fr>
   <he_IL>Russian localisation of LibreOffice</he_IL>
   <hi>Russian localisation of LibreOffice</hi>
   <hr>Russian localisation of LibreOffice</hr>
   <hu>Russian localisation of LibreOffice</hu>
   <id>Russian localisation of LibreOffice</id>
   <is>Russian localisation of LibreOffice</is>
   <it>Localizzazione russa di LibreOffice</it>
   <ja_JP>Russian localisation of LibreOffice</ja_JP>
   <ja>Russian localisation of LibreOffice</ja>
   <kk>Russian localisation of LibreOffice</kk>
   <ko>Russian localisation of LibreOffice</ko>
   <lt>Russian localisation of LibreOffice</lt>
   <mk>Russian localisation of LibreOffice</mk>
   <mr>Russian localisation of LibreOffice</mr>
   <nb>Russian localisation of LibreOffice</nb>
   <nl>Russische lokalisatie van LibreOffice</nl>
   <pl>Rosyjska lokalizacja LibreOffice</pl>
   <pt_BR>Russo Localização para LibreOffice</pt_BR>
   <pt>Russo Localização para LibreOffice</pt>
   <ro>Russian localisation of LibreOffice</ro>
   <ru>Русский перевод для LibreOffice</ru>
   <sk>Russian localisation of LibreOffice</sk>
   <sl>Russian localisation of LibreOffice</sl>
   <sq>Russian localisation of LibreOffice</sq>
   <sr>Russian localisation of LibreOffice</sr>
   <sv>Rysk  lokalisering av LibreOffice</sv>
   <tr>LibreOffice Rusça yerelleştirmesi</tr>
   <uk>Russian локалізація LibreOffice</uk>
   <vi>Russian localisation of LibreOffice</vi>
   <zh_CN>Russian localisation of LibreOffice</zh_CN>
   <zh_TW>Russian localisation of LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ru
libreoffice-help-ru
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ru
libreoffice-help-ru
</uninstall_package_names>
</app>

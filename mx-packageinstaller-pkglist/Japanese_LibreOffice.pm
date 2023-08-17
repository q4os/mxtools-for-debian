<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Japanses_Libreoffice
</name>

<description>
   <am>Japanese localisation of LibreOffice</am>
   <ar>Japanese localisation of LibreOffice</ar>
   <bg>Japanese localisation of LibreOffice</bg>
   <bn>Japanese localisation of LibreOffice</bn>
   <ca>Localització en japonès de LibreOffice</ca>
   <cs>Japanese localisation of LibreOffice</cs>
   <da>Japansk oversættelse af LibreOffice</da>
   <de>Japanische Lokalisierung von LibreOffice</de>
   <el>LibreOffice στα Ιαπωνικά</el>
   <en>Japanese localisation of LibreOffice</en>
   <es>Localización Japonés de LibreOffice</es>
   <et>Japanese localisation of LibreOffice</et>
   <eu>Japanese localisation of LibreOffice</eu>
   <fa>Japanese localisation of LibreOffice</fa>
   <fil_PH>Japanese localisation of LibreOffice</fil_PH>
   <fi>Japanilainen kotoistus LibreOffice:lle</fi>
   <fr>Localisation japonaise pour LibreOffice</fr>
   <he_IL>Japanese localisation of LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस का जापानी संस्करण</hi>
   <hr>Japanese localisation of LibreOffice</hr>
   <hu>Japanese localisation of LibreOffice</hu>
   <id>Japanese localisation of LibreOffice</id>
   <is>Japanese localisation of LibreOffice</is>
   <it>Localizzazione giapponese di LibreOffice </it>
   <ja>LibreOffice の日本語パック</ja>
   <kk>Japanese localisation of LibreOffice</kk>
   <ko>Japanese localisation of LibreOffice</ko>
   <lt>Japanese localisation of LibreOffice</lt>
   <mk>Japanese localisation of LibreOffice</mk>
   <mr>Japanese localisation of LibreOffice</mr>
   <nb>Japansk lokaltilpassing av LibreOffice</nb>
   <nl>Japanse lokalisatie van LibreOffice</nl>
   <pl>Japońska lokalizacja LibreOffice</pl>
   <pt_BR>Japonês Localização para o LibreOffice</pt_BR>
   <pt>Japonês Localização para LibreOffice</pt>
   <ro>Japanese localisation of LibreOffice</ro>
   <ru>Японский перевод для LibreOffice</ru>
   <sk>Japanese localisation of LibreOffice</sk>
   <sl>Japonske krajevne nastavitve za LibreOffice</sl>
   <sq>Përkthimi i LibreOffice-it në japonisht</sq>
   <sr>Japanese localisation of LibreOffice</sr>
   <sv>Japansk lokalisering av LibreOffice</sv>
   <tr>LibreOffice Japonca yerelleştirmesi</tr>
   <uk>Japanese локалізація LibreOffice</uk>
   <vi>Japanese localisation of LibreOffice</vi>
   <zh_CN>Japanese localisation of LibreOffice</zh_CN>
   <zh_TW>Japanese localisation of LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ja
libreoffice-help-ja
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ja
libreoffice-help-ja
</uninstall_package_names>
</app>

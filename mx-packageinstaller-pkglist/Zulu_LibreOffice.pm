<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Zulu_Libreoffice
</name>

<description>
   <am>Zulu Language Meta-Package for LibreOffice</am>
   <ar>Zulu Language Meta-Package for LibreOffice</ar>
   <bg>Zulu Language Meta-Package for LibreOffice</bg>
   <bn>Zulu Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Zulu per LibreOffice</ca>
   <cs>Zulu Language Meta-Package for LibreOffice</cs>
   <da>Zulu Language Meta-Package for LibreOffice</da>
   <de>Zulu Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Zulu</el>
   <en>Zulu Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma zulú para LibreOffice</es>
   <et>Zulu Language Meta-Package for LibreOffice</et>
   <eu>Zulu Language Meta-Package for LibreOffice</eu>
   <fa>Zulu Language Meta-Package for LibreOffice</fa>
   <fil_PH>Zulu Language Meta-Package for LibreOffice</fil_PH>
   <fi>Zulu Language Meta-Package for LibreOffice</fi>
   <fr>Meta-Paquet de langue Zoulou pour LibreOffice</fr>
   <he_IL>Zulu Language Meta-Package for LibreOffice</he_IL>
   <hi>Zulu Language Meta-Package for LibreOffice</hi>
   <hr>Zulu Language Meta-Package for LibreOffice</hr>
   <hu>Zulu Language Meta-Package for LibreOffice</hu>
   <id>Zulu Language Meta-Package for LibreOffice</id>
   <is>Zulu Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua zulu per LibreOffice</it>
   <ja_JP>Zulu Language Meta-Package for LibreOffice</ja_JP>
   <ja>Zulu Language Meta-Package for LibreOffice</ja>
   <kk>Zulu Language Meta-Package for LibreOffice</kk>
   <ko>Zulu Language Meta-Package for LibreOffice</ko>
   <lt>Zulu Language Meta-Package for LibreOffice</lt>
   <mk>Zulu Language Meta-Package for LibreOffice</mk>
   <mr>Zulu Language Meta-Package for LibreOffice</mr>
   <nb>Zulu Language Meta-Package for LibreOffice</nb>
   <nl>Zulu Language Meta-Package for LibreOffice</nl>
   <pl>Zulu Language Meta-Package for LibreOffice</pl>
   <pt_BR>Zulu Language Meta-Package for LibreOffice</pt_BR>
   <pt>Zulu Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Zulu Language Meta-Package for LibreOffice</ro>
   <ru>Zulu Language Meta-Package for LibreOffice</ru>
   <sk>Zulu Language Meta-Package for LibreOffice</sk>
   <sl>Zulujski jezikovni meta-paket za LibreOffice</sl>
   <sq>Zulu Language Meta-Package for LibreOffice</sq>
   <sr>Zulu Language Meta-Package for LibreOffice</sr>
   <sv>Zulu Språk Meta-Paket för LibreOffice</sv>
   <tr>Zulu Language Meta-Package for LibreOffice</tr>
   <uk>Zulu Language Meta-Package for LibreOffice</uk>
   <vi>Zulu Language Meta-Package for LibreOffice</vi>
   <zh_CN>Zulu Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Zulu Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-zu
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-zu
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Gaelic_Libreoffice
</name>

<description>
   <am>Gaelic Language Meta-Package for LibreOffice</am>
   <ar>Gaelic Language Meta-Package for LibreOffice</ar>
   <bg>Gaelic Language Meta-Package for LibreOffice</bg>
   <bn>Gaelic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Gaèlic per LibreOffice</ca>
   <cs>Gaelic Language Meta-Package for LibreOffice</cs>
   <da>Gaelic Language Meta-Package for LibreOffice</da>
   <de>Gälisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Gaelic</el>
   <en>Gaelic Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma gaélico para LibreOffice</es>
   <et>Gaelic Language Meta-Package for LibreOffice</et>
   <eu>Gaelic Language Meta-Package for LibreOffice</eu>
   <fa>Gaelic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Gaelic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Gaelic Language Meta-Package for LibreOffice</fi>
   <fr>Gaelic Language Meta-Package for LibreOffice</fr>
   <he_IL>Gaelic Language Meta-Package for LibreOffice</he_IL>
   <hi>Gaelic Language Meta-Package for LibreOffice</hi>
   <hr>Gaelic Language Meta-Package for LibreOffice</hr>
   <hu>Gaelic Language Meta-Package for LibreOffice</hu>
   <id>Gaelic Language Meta-Package for LibreOffice</id>
   <is>Gaelic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua gaelica per LibreOffice</it>
   <ja_JP>Gaelic Language Meta-Package for LibreOffice</ja_JP>
   <ja>Gaelic Language Meta-Package for LibreOffice</ja>
   <kk>Gaelic Language Meta-Package for LibreOffice</kk>
   <ko>Gaelic Language Meta-Package for LibreOffice</ko>
   <lt>Gaelic Language Meta-Package for LibreOffice</lt>
   <mk>Gaelic Language Meta-Package for LibreOffice</mk>
   <mr>Gaelic Language Meta-Package for LibreOffice</mr>
   <nb>Gaelic Language Meta-Package for LibreOffice</nb>
   <nl>Gaelic Language Meta-Package for LibreOffice</nl>
   <pl>Gaelic Language Meta-Package for LibreOffice</pl>
   <pt_BR>Gaelic Language Meta-Package for LibreOffice</pt_BR>
   <pt>Gaélico Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Gaelic Language Meta-Package for LibreOffice</ro>
   <ru>Gaelic Language Meta-Package for LibreOffice</ru>
   <sk>Gaelic Language Meta-Package for LibreOffice</sk>
   <sl>Galski jezikovni meta-paket za LibreOffice</sl>
   <sq>Gaelic Language Meta-Package for LibreOffice</sq>
   <sr>Gaelic Language Meta-Package for LibreOffice</sr>
   <sv>Gaeliskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Gaelic Language Meta-Package for LibreOffice</tr>
   <uk>Gaelic Language Meta-Package for LibreOffice</uk>
   <vi>Gaelic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Gaelic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Gaelic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ga
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ga
</uninstall_package_names>
</app>

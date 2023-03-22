<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Turkish_Libreoffice
</name>

<description>
   <am>Turkish Language Meta-Package for LibreOffice</am>
   <ar>Turkish Language Meta-Package for LibreOffice</ar>
   <bg>Turkish Language Meta-Package for LibreOffice</bg>
   <bn>Turkish Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Turc per LibreOffice</ca>
   <cs>Turkish Language Meta-Package for LibreOffice</cs>
   <da>Turkish Language Meta-Package for LibreOffice</da>
   <de>Türkisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα τούρκικά</el>
   <en>Turkish Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma turco para LibreOffice</es>
   <et>Turkish Language Meta-Package for LibreOffice</et>
   <eu>Turkish Language Meta-Package for LibreOffice</eu>
   <fa>Turkish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Turkish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Turkish Language Meta-Package for LibreOffice</fi>
   <fr>Turkish Language Meta-Package for LibreOffice</fr>
   <he_IL>Turkish Language Meta-Package for LibreOffice</he_IL>
   <hi>Turkish Language Meta-Package for LibreOffice</hi>
   <hr>Turkish Language Meta-Package for LibreOffice</hr>
   <hu>Turkish Language Meta-Package for LibreOffice</hu>
   <id>Turkish Language Meta-Package for LibreOffice</id>
   <is>Turkish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua turca per LibreOffice</it>
   <ja_JP>Turkish Language Meta-Package for LibreOffice</ja_JP>
   <ja>Turkish Language Meta-Package for LibreOffice</ja>
   <kk>Turkish Language Meta-Package for LibreOffice</kk>
   <ko>Turkish Language Meta-Package for LibreOffice</ko>
   <lt>Turkish Language Meta-Package for LibreOffice</lt>
   <mk>Turkish Language Meta-Package for LibreOffice</mk>
   <mr>Turkish Language Meta-Package for LibreOffice</mr>
   <nb>Turkish Language Meta-Package for LibreOffice</nb>
   <nl>Turkish Language Meta-Package for LibreOffice</nl>
   <pl>Turkish Language Meta-Package for LibreOffice</pl>
   <pt_BR>Turkish Language Meta-Package for LibreOffice</pt_BR>
   <pt>Turco Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Turkish Language Meta-Package for LibreOffice</ro>
   <ru>Turkish Language Meta-Package for LibreOffice</ru>
   <sk>Turkish Language Meta-Package for LibreOffice</sk>
   <sl>Turškijezikovni meta-paket za LibreOffice</sl>
   <sq>Turkish Language Meta-Package for LibreOffice</sq>
   <sr>Turkish Language Meta-Package for LibreOffice</sr>
   <sv>Turkiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Turkish Language Meta-Package for LibreOffice</tr>
   <uk>Turkish Language Meta-Package for LibreOffice</uk>
   <vi>Turkish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Turkish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Turkish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-tr
libreoffice-help-tr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-tr
libreoffice-help-tr
</uninstall_package_names>
</app>

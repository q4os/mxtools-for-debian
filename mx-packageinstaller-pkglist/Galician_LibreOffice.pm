<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Galician_Libreoffice
</name>

<description>
   <am>Galician Language Meta-Package for LibreOffice</am>
   <ar>Galician Language Meta-Package for LibreOffice</ar>
   <bg>Galician Language Meta-Package for LibreOffice</bg>
   <bn>Galician Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Gallec per LibreOffice</ca>
   <cs>Galician Language Meta-Package for LibreOffice</cs>
   <da>Galician Language Meta-Package for LibreOffice</da>
   <de>Galizisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Galician</el>
   <en>Galician Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma gallego para LibreOffice</es>
   <et>Galician Language Meta-Package for LibreOffice</et>
   <eu>Galician Language Meta-Package for LibreOffice</eu>
   <fa>Galician Language Meta-Package for LibreOffice</fa>
   <fil_PH>Galician Language Meta-Package for LibreOffice</fil_PH>
   <fi>Galician Language Meta-Package for LibreOffice</fi>
   <fr>Galician Language Meta-Package for LibreOffice</fr>
   <he_IL>Galician Language Meta-Package for LibreOffice</he_IL>
   <hi>Galician Language Meta-Package for LibreOffice</hi>
   <hr>Galician Language Meta-Package for LibreOffice</hr>
   <hu>Galician Language Meta-Package for LibreOffice</hu>
   <id>Galician Language Meta-Package for LibreOffice</id>
   <is>Galician Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua galiziana per LibreOffice</it>
   <ja_JP>Galician Language Meta-Package for LibreOffice</ja_JP>
   <ja>Galician Language Meta-Package for LibreOffice</ja>
   <kk>Galician Language Meta-Package for LibreOffice</kk>
   <ko>Galician Language Meta-Package for LibreOffice</ko>
   <lt>Galician Language Meta-Package for LibreOffice</lt>
   <mk>Galician Language Meta-Package for LibreOffice</mk>
   <mr>Galician Language Meta-Package for LibreOffice</mr>
   <nb>Galician Language Meta-Package for LibreOffice</nb>
   <nl>Galician Language Meta-Package for LibreOffice</nl>
   <pl>Galician Language Meta-Package for LibreOffice</pl>
   <pt_BR>Galician Language Meta-Package for LibreOffice</pt_BR>
   <pt>Galego Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Galician Language Meta-Package for LibreOffice</ro>
   <ru>Galician Language Meta-Package for LibreOffice</ru>
   <sk>Galician Language Meta-Package for LibreOffice</sk>
   <sl>Galicijski jezikovni meta-paket za LibreOffice</sl>
   <sq>Galician Language Meta-Package for LibreOffice</sq>
   <sr>Galician Language Meta-Package for LibreOffice</sr>
   <sv>Galiciskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Galician Language Meta-Package for LibreOffice</tr>
   <uk>Galician Language Meta-Package for LibreOffice</uk>
   <vi>Galician Language Meta-Package for LibreOffice</vi>
   <zh_CN>Galician Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Galician Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-gl
libreoffice-help-gl
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-gl
libreoffice-help-gl
</uninstall_package_names>
</app>

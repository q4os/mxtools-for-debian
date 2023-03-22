<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Estonian_Libreoffice
</name>

<description>
   <am>Estonian Language Meta-Package for LibreOffice</am>
   <ar>Estonian Language Meta-Package for LibreOffice</ar>
   <bg>Estonian Language Meta-Package for LibreOffice</bg>
   <bn>Estonian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Estonià per LibreOffice</ca>
   <cs>Estonian Language Meta-Package for LibreOffice</cs>
   <da>Estonian Language Meta-Package for LibreOffice</da>
   <de>Estnisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Εσθονικά</el>
   <en>Estonian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma estonio para LibreOffice</es>
   <et>Estonian Language Meta-Package for LibreOffice</et>
   <eu>Estonian Language Meta-Package for LibreOffice</eu>
   <fa>Estonian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Estonian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Estonian Language Meta-Package for LibreOffice</fi>
   <fr>Estonian Language Meta-Package for LibreOffice</fr>
   <he_IL>Estonian Language Meta-Package for LibreOffice</he_IL>
   <hi>Estonian Language Meta-Package for LibreOffice</hi>
   <hr>Estonian Language Meta-Package for LibreOffice</hr>
   <hu>Estonian Language Meta-Package for LibreOffice</hu>
   <id>Estonian Language Meta-Package for LibreOffice</id>
   <is>Estonian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua estone per LibreOffice</it>
   <ja_JP>Estonian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Estonian Language Meta-Package for LibreOffice</ja>
   <kk>Estonian Language Meta-Package for LibreOffice</kk>
   <ko>Estonian Language Meta-Package for LibreOffice</ko>
   <lt>Estonian Language Meta-Package for LibreOffice</lt>
   <mk>Estonian Language Meta-Package for LibreOffice</mk>
   <mr>Estonian Language Meta-Package for LibreOffice</mr>
   <nb>Estonian Language Meta-Package for LibreOffice</nb>
   <nl>Estonian Language Meta-Package for LibreOffice</nl>
   <pl>Estonian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Estonian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Estónio Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Estonian Language Meta-Package for LibreOffice</ro>
   <ru>Estonian Language Meta-Package for LibreOffice</ru>
   <sk>Estonian Language Meta-Package for LibreOffice</sk>
   <sl>Estonski jezikovni meta-paket za LibreOffice</sl>
   <sq>Estonian Language Meta-Package for LibreOffice</sq>
   <sr>Estonian Language Meta-Package for LibreOffice</sr>
   <sv>Estniskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Estonian Language Meta-Package for LibreOffice</tr>
   <uk>Estonian Language Meta-Package for LibreOffice</uk>
   <vi>Estonian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Estonian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Estonian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-et
libreoffice-help-et
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-et
libreoffice-help-et
</uninstall_package_names>
</app>

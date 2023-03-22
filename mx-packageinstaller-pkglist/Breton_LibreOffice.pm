<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Breton_Libreoffice
</name>

<description>
   <am>Breton Language Meta-Package for LibreOffice</am>
   <ar>Breton Language Meta-Package for LibreOffice</ar>
   <bg>Breton Language Meta-Package for LibreOffice</bg>
   <bn>Breton Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Bretó per LibreOffice</ca>
   <cs>Breton Language Meta-Package for LibreOffice</cs>
   <da>Breton Language Meta-Package for LibreOffice</da>
   <de>Bretonisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Breton</el>
   <en>Breton Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma bretón para LibreOffice</es>
   <et>Breton Language Meta-Package for LibreOffice</et>
   <eu>Breton Language Meta-Package for LibreOffice</eu>
   <fa>Breton Language Meta-Package for LibreOffice</fa>
   <fil_PH>Breton Language Meta-Package for LibreOffice</fil_PH>
   <fi>Breton Language Meta-Package for LibreOffice</fi>
   <fr>Breton Language Meta-Package for LibreOffice</fr>
   <he_IL>Breton Language Meta-Package for LibreOffice</he_IL>
   <hi>Breton Language Meta-Package for LibreOffice</hi>
   <hr>Breton Language Meta-Package for LibreOffice</hr>
   <hu>Breton Language Meta-Package for LibreOffice</hu>
   <id>Breton Language Meta-Package for LibreOffice</id>
   <is>Breton Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua bretone per LibreOffice</it>
   <ja_JP>Breton Language Meta-Package for LibreOffice</ja_JP>
   <ja>Breton Language Meta-Package for LibreOffice</ja>
   <kk>Breton Language Meta-Package for LibreOffice</kk>
   <ko>Breton Language Meta-Package for LibreOffice</ko>
   <lt>Breton Language Meta-Package for LibreOffice</lt>
   <mk>Breton Language Meta-Package for LibreOffice</mk>
   <mr>Breton Language Meta-Package for LibreOffice</mr>
   <nb>Breton Language Meta-Package for LibreOffice</nb>
   <nl>Breton Language Meta-Package for LibreOffice</nl>
   <pl>Breton Language Meta-Package for LibreOffice</pl>
   <pt_BR>Breton Language Meta-Package for LibreOffice</pt_BR>
   <pt>Bretão Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Breton Language Meta-Package for LibreOffice</ro>
   <ru>Breton Language Meta-Package for LibreOffice</ru>
   <sk>Breton Language Meta-Package for LibreOffice</sk>
   <sl>Bretonski jezikovni meta-paket za LibreOffice</sl>
   <sq>Breton Language Meta-Package for LibreOffice</sq>
   <sr>Breton Language Meta-Package for LibreOffice</sr>
   <sv>Bretonska Språk Meta-Paket för LibreOffice</sv>
   <tr>Breton Language Meta-Package for LibreOffice</tr>
   <uk>Breton Language Meta-Package for LibreOffice</uk>
   <vi>Breton Language Meta-Package for LibreOffice</vi>
   <zh_CN>Breton Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Breton Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-br
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-br
</uninstall_package_names>
</app>

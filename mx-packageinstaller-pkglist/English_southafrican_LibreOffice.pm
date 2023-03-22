<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
English_southafrican_Libreoffice
</name>

<description>
   <am>English_southafrican Language Meta-Package for LibreOffice</am>
   <ar>English_southafrican Language Meta-Package for LibreOffice</ar>
   <bg>English_southafrican Language Meta-Package for LibreOffice</bg>
   <bn>English_southafrican Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'Angés Sudafricà per LibreOffice</ca>
   <cs>English_southafrican Language Meta-Package for LibreOffice</cs>
   <da>English_southafrican Language Meta-Package for LibreOffice</da>
   <de>Südafrikanisches Englisch Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα αγγλικά-Νότια Αφρικής</el>
   <en>English_southafrican Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma inglés_southafricano para LibreOffice</es>
   <et>English_southafrican Language Meta-Package for LibreOffice</et>
   <eu>English_southafrican Language Meta-Package for LibreOffice</eu>
   <fa>English_southafrican Language Meta-Package for LibreOffice</fa>
   <fil_PH>English_southafrican Language Meta-Package for LibreOffice</fil_PH>
   <fi>English_southafrican Language Meta-Package for LibreOffice</fi>
   <fr>English_southafrican Language Meta-Package for LibreOffice</fr>
   <he_IL>English_southafrican Language Meta-Package for LibreOffice</he_IL>
   <hi>English_southafrican Language Meta-Package for LibreOffice</hi>
   <hr>English_southafrican Language Meta-Package for LibreOffice</hr>
   <hu>English_southafrican Language Meta-Package for LibreOffice</hu>
   <id>English_southafrican Language Meta-Package for LibreOffice</id>
   <is>English_southafrican Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua anglo-sudafricana per LibreOffice</it>
   <ja_JP>English_southafrican Language Meta-Package for LibreOffice</ja_JP>
   <ja>English_southafrican Language Meta-Package for LibreOffice</ja>
   <kk>English_southafrican Language Meta-Package for LibreOffice</kk>
   <ko>English_southafrican Language Meta-Package for LibreOffice</ko>
   <lt>English_southafrican Language Meta-Package for LibreOffice</lt>
   <mk>English_southafrican Language Meta-Package for LibreOffice</mk>
   <mr>English_southafrican Language Meta-Package for LibreOffice</mr>
   <nb>English_southafrican Language Meta-Package for LibreOffice</nb>
   <nl>English_southafrican Language Meta-Package for LibreOffice</nl>
   <pl>English_southafrican Language Meta-Package for LibreOffice</pl>
   <pt_BR>English_southafrican Language Meta-Package for LibreOffice</pt_BR>
   <pt>Inglês SA Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>English_southafrican Language Meta-Package for LibreOffice</ro>
   <ru>English_southafrican Language Meta-Package for LibreOffice</ru>
   <sk>English_southafrican Language Meta-Package for LibreOffice</sk>
   <sl>Angleški južnoafriški jezikovni meta-paket za LibreOffice</sl>
   <sq>English_southafrican Language Meta-Package for LibreOffice</sq>
   <sr>English_southafrican Language Meta-Package for LibreOffice</sr>
   <sv>Engelska_sydafrikansk Språk Meta-Paket för LibreOffice</sv>
   <tr>English_southafrican Language Meta-Package for LibreOffice</tr>
   <uk>English_southafrican Language Meta-Package for LibreOffice</uk>
   <vi>English_southafrican Language Meta-Package for LibreOffice</vi>
   <zh_CN>English_southafrican Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>English_southafrican Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-en-za
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-en-za
</uninstall_package_names>
</app>

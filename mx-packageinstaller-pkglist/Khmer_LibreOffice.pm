<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Khmer_Libreoffice
</name>

<description>
   <am>Khmer Language Meta-Package for LibreOffice</am>
   <ar>Khmer Language Meta-Package for LibreOffice</ar>
   <bg>Khmer Language Meta-Package for LibreOffice</bg>
   <bn>Khmer Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Khmer per LibreOffice</ca>
   <cs>Khmer Language Meta-Package for LibreOffice</cs>
   <da>Khmer Language Meta-Package for LibreOffice</da>
   <de>Khmer Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Χμερ</el>
   <en>Khmer Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma khmer para LibreOffice</es>
   <et>Khmer Language Meta-Package for LibreOffice</et>
   <eu>Khmer Language Meta-Package for LibreOffice</eu>
   <fa>Khmer Language Meta-Package for LibreOffice</fa>
   <fil_PH>Khmer Language Meta-Package for LibreOffice</fil_PH>
   <fi>Khmer Language Meta-Package for LibreOffice</fi>
   <fr>Khmer Language Meta-Package for LibreOffice</fr>
   <he_IL>Khmer Language Meta-Package for LibreOffice</he_IL>
   <hi>Khmer Language Meta-Package for LibreOffice</hi>
   <hr>Khmer Language Meta-Package for LibreOffice</hr>
   <hu>Khmer Language Meta-Package for LibreOffice</hu>
   <id>Khmer Language Meta-Package for LibreOffice</id>
   <is>Khmer Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua khmer per LibreOffice</it>
   <ja_JP>Khmer Language Meta-Package for LibreOffice</ja_JP>
   <ja>Khmer Language Meta-Package for LibreOffice</ja>
   <kk>Khmer Language Meta-Package for LibreOffice</kk>
   <ko>Khmer Language Meta-Package for LibreOffice</ko>
   <lt>Khmer Language Meta-Package for LibreOffice</lt>
   <mk>Khmer Language Meta-Package for LibreOffice</mk>
   <mr>Khmer Language Meta-Package for LibreOffice</mr>
   <nb>Khmer Language Meta-Package for LibreOffice</nb>
   <nl>Khmer Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Khmer Language Meta-Package for LibreOffice</pl>
   <pt_BR>Khmer Language Meta-Package for LibreOffice</pt_BR>
   <pt>Khmer Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Khmer Language Meta-Package for LibreOffice</ro>
   <ru>Khmer Language Meta-Package for LibreOffice</ru>
   <sk>Khmer Language Meta-Package for LibreOffice</sk>
   <sl>Kmerski jezikovni meta-paket za LibreOffice</sl>
   <sq>Khmer Language Meta-Package for LibreOffice</sq>
   <sr>Khmer Language Meta-Package for LibreOffice</sr>
   <sv>Khmer Språk Meta-Paket för LibreOffice</sv>
   <tr>Khmer Language Meta-Package for LibreOffice</tr>
   <uk>Khmer Language Meta-Package for LibreOffice</uk>
   <vi>Khmer Language Meta-Package for LibreOffice</vi>
   <zh_CN>Khmer Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Khmer Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-km
libreoffice-help-km
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-km
libreoffice-help-km
</uninstall_package_names>
</app>

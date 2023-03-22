<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Slovenian_Libreoffice
</name>

<description>
   <am>Slovenian Language Meta-Package for LibreOffice</am>
   <ar>Slovenian Language Meta-Package for LibreOffice</ar>
   <bg>Slovenian Language Meta-Package for LibreOffice</bg>
   <bn>Slovenian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Eslovè per LibreOffice</ca>
   <cs>Slovenian Language Meta-Package for LibreOffice</cs>
   <da>Slovenian Language Meta-Package for LibreOffice</da>
   <de>Slowenisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Slovenian</el>
   <en>Slovenian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma esloveno para LibreOffice</es>
   <et>Slovenian Language Meta-Package for LibreOffice</et>
   <eu>Slovenian Language Meta-Package for LibreOffice</eu>
   <fa>Slovenian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Slovenian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Slovenian Language Meta-Package for LibreOffice</fi>
   <fr>Slovenian Language Meta-Package for LibreOffice</fr>
   <he_IL>Slovenian Language Meta-Package for LibreOffice</he_IL>
   <hi>Slovenian Language Meta-Package for LibreOffice</hi>
   <hr>Slovenian Language Meta-Package for LibreOffice</hr>
   <hu>Slovenian Language Meta-Package for LibreOffice</hu>
   <id>Slovenian Language Meta-Package for LibreOffice</id>
   <is>Slovenian Language Meta-Package for LibreOffice</is>
   <it>Slovenian Language Meta-Package for LibreOffice</it>
   <ja_JP>Slovenian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Slovenian Language Meta-Package for LibreOffice</ja>
   <kk>Slovenian Language Meta-Package for LibreOffice</kk>
   <ko>Slovenian Language Meta-Package for LibreOffice</ko>
   <lt>Slovenian Language Meta-Package for LibreOffice</lt>
   <mk>Slovenian Language Meta-Package for LibreOffice</mk>
   <mr>Slovenian Language Meta-Package for LibreOffice</mr>
   <nb>Slovenian Language Meta-Package for LibreOffice</nb>
   <nl>Slovenian Language Meta-Package for LibreOffice</nl>
   <pl>Slovenian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Slovenian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Esloveno Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Slovenian Language Meta-Package for LibreOffice</ro>
   <ru>Slovenian Language Meta-Package for LibreOffice</ru>
   <sk>Slovenian Language Meta-Package for LibreOffice</sk>
   <sl>Slovenski jezikovni meta-paket za LibreOffice</sl>
   <sq>Slovenian Language Meta-Package for LibreOffice</sq>
   <sr>Slovenian Language Meta-Package for LibreOffice</sr>
   <sv>Sloveniskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Slovenian Language Meta-Package for LibreOffice</tr>
   <uk>Slovenian Language Meta-Package for LibreOffice</uk>
   <vi>Slovenian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Slovenian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Slovenian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-sl
libreoffice-help-sl
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-sl
libreoffice-help-sl
</uninstall_package_names>
</app>

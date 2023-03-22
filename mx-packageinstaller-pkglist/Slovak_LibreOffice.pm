<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Slovak_Libreoffice
</name>

<description>
   <am>Slovak Language Meta-Package for LibreOffice</am>
   <ar>Slovak Language Meta-Package for LibreOffice</ar>
   <bg>Slovak Language Meta-Package for LibreOffice</bg>
   <bn>Slovak Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Eslovac per LibreOffice</ca>
   <cs>Slovak Language Meta-Package for LibreOffice</cs>
   <da>Slovak Language Meta-Package for LibreOffice</da>
   <de>Slowakisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Slovak</el>
   <en>Slovak Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma eslovaco para LibreOffice</es>
   <et>Slovak Language Meta-Package for LibreOffice</et>
   <eu>Slovak Language Meta-Package for LibreOffice</eu>
   <fa>Slovak Language Meta-Package for LibreOffice</fa>
   <fil_PH>Slovak Language Meta-Package for LibreOffice</fil_PH>
   <fi>Slovak Language Meta-Package for LibreOffice</fi>
   <fr>Slovak Language Meta-Package for LibreOffice</fr>
   <he_IL>Slovak Language Meta-Package for LibreOffice</he_IL>
   <hi>Slovak Language Meta-Package for LibreOffice</hi>
   <hr>Slovak Language Meta-Package for LibreOffice</hr>
   <hu>Slovak Language Meta-Package for LibreOffice</hu>
   <id>Slovak Language Meta-Package for LibreOffice</id>
   <is>Slovak Language Meta-Package for LibreOffice</is>
   <it>Slovak Language Meta-Package for LibreOffice</it>
   <ja_JP>Slovak Language Meta-Package for LibreOffice</ja_JP>
   <ja>Slovak Language Meta-Package for LibreOffice</ja>
   <kk>Slovak Language Meta-Package for LibreOffice</kk>
   <ko>Slovak Language Meta-Package for LibreOffice</ko>
   <lt>Slovak Language Meta-Package for LibreOffice</lt>
   <mk>Slovak Language Meta-Package for LibreOffice</mk>
   <mr>Slovak Language Meta-Package for LibreOffice</mr>
   <nb>Slovak Language Meta-Package for LibreOffice</nb>
   <nl>Slovak Language Meta-Package for LibreOffice</nl>
   <pl>Slovak Language Meta-Package for LibreOffice</pl>
   <pt_BR>Slovak Language Meta-Package for LibreOffice</pt_BR>
   <pt>ESlovaco Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Slovak Language Meta-Package for LibreOffice</ro>
   <ru>Slovak Language Meta-Package for LibreOffice</ru>
   <sk>Slovak Language Meta-Package for LibreOffice</sk>
   <sl>Slovaški jezikovni meta-paket za LibreOffice</sl>
   <sq>Slovak Language Meta-Package for LibreOffice</sq>
   <sr>Slovak Language Meta-Package for LibreOffice</sr>
   <sv>Slovakiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Slovak Language Meta-Package for LibreOffice</tr>
   <uk>Slovak Language Meta-Package for LibreOffice</uk>
   <vi>Slovak Language Meta-Package for LibreOffice</vi>
   <zh_CN>Slovak Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Slovak Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-sk
libreoffice-help-sk
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-sk
libreoffice-help-sk
</uninstall_package_names>
</app>

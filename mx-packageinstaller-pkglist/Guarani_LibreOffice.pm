<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Guarani_Libreoffice
</name>

<description>
   <am>Guarani Language Meta-Package for LibreOffice</am>
   <ar>Guarani Language Meta-Package for LibreOffice</ar>
   <bg>Guarani Language Meta-Package for LibreOffice</bg>
   <bn>Guarani Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Guaraní per LibreOffice</ca>
   <cs>Guarani Language Meta-Package for LibreOffice</cs>
   <da>Guarani Language Meta-Package for LibreOffice</da>
   <de>Guarani Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Guarani</el>
   <en>Guarani Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma guaraní para LibreOffice</es>
   <et>Guarani Language Meta-Package for LibreOffice</et>
   <eu>Guarani Language Meta-Package for LibreOffice</eu>
   <fa>Guarani Language Meta-Package for LibreOffice</fa>
   <fil_PH>Guarani Language Meta-Package for LibreOffice</fil_PH>
   <fi>Guarani Language Meta-Package for LibreOffice</fi>
   <fr>Guarani Language Meta-Package for LibreOffice</fr>
   <he_IL>Guarani Language Meta-Package for LibreOffice</he_IL>
   <hi>Guarani Language Meta-Package for LibreOffice</hi>
   <hr>Guarani Language Meta-Package for LibreOffice</hr>
   <hu>Guarani Language Meta-Package for LibreOffice</hu>
   <id>Guarani Language Meta-Package for LibreOffice</id>
   <is>Guarani Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua guarani per LibreOffice</it>
   <ja_JP>Guarani Language Meta-Package for LibreOffice</ja_JP>
   <ja>Guarani Language Meta-Package for LibreOffice</ja>
   <kk>Guarani Language Meta-Package for LibreOffice</kk>
   <ko>Guarani Language Meta-Package for LibreOffice</ko>
   <lt>Guarani Language Meta-Package for LibreOffice</lt>
   <mk>Guarani Language Meta-Package for LibreOffice</mk>
   <mr>Guarani Language Meta-Package for LibreOffice</mr>
   <nb>Guarani Language Meta-Package for LibreOffice</nb>
   <nl>Guarani Language Meta-Package for LibreOffice</nl>
   <pl>Guarani Language Meta-Package for LibreOffice</pl>
   <pt_BR>Guarani Language Meta-Package for LibreOffice</pt_BR>
   <pt>Guarani Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Guarani Language Meta-Package for LibreOffice</ro>
   <ru>Guarani Language Meta-Package for LibreOffice</ru>
   <sk>Guarani Language Meta-Package for LibreOffice</sk>
   <sl>Guaranski jezikovni meta-paket za LibreOffice</sl>
   <sq>Guarani Language Meta-Package for LibreOffice</sq>
   <sr>Guarani Language Meta-Package for LibreOffice</sr>
   <sv>Guarani Språk Meta-Paket för LibreOffice</sv>
   <tr>Guarani Language Meta-Package for LibreOffice</tr>
   <uk>Guarani Language Meta-Package for LibreOffice</uk>
   <vi>Guarani Language Meta-Package for LibreOffice</vi>
   <zh_CN>Guarani Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Guarani Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-gug
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-gug
</uninstall_package_names>
</app>

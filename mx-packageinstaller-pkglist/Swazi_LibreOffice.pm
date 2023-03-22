<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Swazi_Libreoffice
</name>

<description>
   <am>Swazi Language Meta-Package for LibreOffice</am>
   <ar>Swazi Language Meta-Package for LibreOffice</ar>
   <bg>Swazi Language Meta-Package for LibreOffice</bg>
   <bn>Swazi Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Swazi per LibreOffice</ca>
   <cs>Swazi Language Meta-Package for LibreOffice</cs>
   <da>Swazi Language Meta-Package for LibreOffice</da>
   <de>Swazi Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Swazi</el>
   <en>Swazi Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma swazi para LibreOffice</es>
   <et>Swazi Language Meta-Package for LibreOffice</et>
   <eu>Swazi Language Meta-Package for LibreOffice</eu>
   <fa>Swazi Language Meta-Package for LibreOffice</fa>
   <fil_PH>Swazi Language Meta-Package for LibreOffice</fil_PH>
   <fi>Swazi Language Meta-Package for LibreOffice</fi>
   <fr>Swazi Language Meta-Package for LibreOffice</fr>
   <he_IL>Swazi Language Meta-Package for LibreOffice</he_IL>
   <hi>Swazi Language Meta-Package for LibreOffice</hi>
   <hr>Swazi Language Meta-Package for LibreOffice</hr>
   <hu>Swazi Language Meta-Package for LibreOffice</hu>
   <id>Swazi Language Meta-Package for LibreOffice</id>
   <is>Swazi Language Meta-Package for LibreOffice</is>
   <it>Swazi Language Meta-Package for LibreOffice</it>
   <ja_JP>Swazi Language Meta-Package for LibreOffice</ja_JP>
   <ja>Swazi Language Meta-Package for LibreOffice</ja>
   <kk>Swazi Language Meta-Package for LibreOffice</kk>
   <ko>Swazi Language Meta-Package for LibreOffice</ko>
   <lt>Swazi Language Meta-Package for LibreOffice</lt>
   <mk>Swazi Language Meta-Package for LibreOffice</mk>
   <mr>Swazi Language Meta-Package for LibreOffice</mr>
   <nb>Swazi Language Meta-Package for LibreOffice</nb>
   <nl>Swazi Language Meta-Package for LibreOffice</nl>
   <pl>Swazi Language Meta-Package for LibreOffice</pl>
   <pt_BR>Swazi Language Meta-Package for LibreOffice</pt_BR>
   <pt>Swazi Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Swazi Language Meta-Package for LibreOffice</ro>
   <ru>Swazi Language Meta-Package for LibreOffice</ru>
   <sk>Swazi Language Meta-Package for LibreOffice</sk>
   <sl>Svazijski jezikovni meta-paket za LibreOffice</sl>
   <sq>Swazi Language Meta-Package for LibreOffice</sq>
   <sr>Swazi Language Meta-Package for LibreOffice</sr>
   <sv>Swazi Språk Meta-Paket för LibreOffice</sv>
   <tr>Swazi Language Meta-Package for LibreOffice</tr>
   <uk>Swazi Language Meta-Package for LibreOffice</uk>
   <vi>Swazi Language Meta-Package for LibreOffice</vi>
   <zh_CN>Swazi Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Swazi Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ss
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ss
</uninstall_package_names>
</app>

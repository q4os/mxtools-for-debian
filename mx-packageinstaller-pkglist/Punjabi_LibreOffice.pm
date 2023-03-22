<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Punjabi_Libreoffice
</name>

<description>
   <am>Punjabi Language Meta-Package for LibreOffice</am>
   <ar>Punjabi Language Meta-Package for LibreOffice</ar>
   <bg>Punjabi Language Meta-Package for LibreOffice</bg>
   <bn>Punjabi Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Punjabi per LibreOffice</ca>
   <cs>Punjabi Language Meta-Package for LibreOffice</cs>
   <da>Punjabi Language Meta-Package for LibreOffice</da>
   <de>Punjabi Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Punjabi</el>
   <en>Punjabi Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma Punjabi para LibreOffice</es>
   <et>Punjabi Language Meta-Package for LibreOffice</et>
   <eu>Punjabi Language Meta-Package for LibreOffice</eu>
   <fa>Punjabi Language Meta-Package for LibreOffice</fa>
   <fil_PH>Punjabi Language Meta-Package for LibreOffice</fil_PH>
   <fi>Punjabi Language Meta-Package for LibreOffice</fi>
   <fr>Punjabi Language Meta-Package for LibreOffice</fr>
   <he_IL>Punjabi Language Meta-Package for LibreOffice</he_IL>
   <hi>Punjabi Language Meta-Package for LibreOffice</hi>
   <hr>Punjabi Language Meta-Package for LibreOffice</hr>
   <hu>Punjabi Language Meta-Package for LibreOffice</hu>
   <id>Punjabi Language Meta-Package for LibreOffice</id>
   <is>Punjabi Language Meta-Package for LibreOffice</is>
   <it>Punjabi Language Meta-Package for LibreOffice</it>
   <ja_JP>Punjabi Language Meta-Package for LibreOffice</ja_JP>
   <ja>Punjabi Language Meta-Package for LibreOffice</ja>
   <kk>Punjabi Language Meta-Package for LibreOffice</kk>
   <ko>Punjabi Language Meta-Package for LibreOffice</ko>
   <lt>Punjabi Language Meta-Package for LibreOffice</lt>
   <mk>Punjabi Language Meta-Package for LibreOffice</mk>
   <mr>Punjabi Language Meta-Package for LibreOffice</mr>
   <nb>Punjabi Language Meta-Package for LibreOffice</nb>
   <nl>Punjabi Language Meta-Package for LibreOffice</nl>
   <pl>Punjabi Language Meta-Package for LibreOffice</pl>
   <pt_BR>Punjabi Language Meta-Package for LibreOffice</pt_BR>
   <pt>Panjabi Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Punjabi Language Meta-Package for LibreOffice</ro>
   <ru>Punjabi Language Meta-Package for LibreOffice</ru>
   <sk>Punjabi Language Meta-Package for LibreOffice</sk>
   <sl>Pundžabski jezikovni meta-paket za LibreOffice</sl>
   <sq>Punjabi Language Meta-Package for LibreOffice</sq>
   <sr>Punjabi Language Meta-Package for LibreOffice</sr>
   <sv>Punjabi Språk Meta-Paket för LibreOffice</sv>
   <tr>Punjabi Language Meta-Package for LibreOffice</tr>
   <uk>Punjabi Language Meta-Package for LibreOffice</uk>
   <vi>Punjabi Language Meta-Package for LibreOffice</vi>
   <zh_CN>Punjabi Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Punjabi Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pa-in
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pa-in
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Telugu_Libreoffice
</name>

<description>
   <am>Telugu Language Meta-Package for LibreOffice</am>
   <ar>Telugu Language Meta-Package for LibreOffice</ar>
   <bg>Telugu Language Meta-Package for LibreOffice</bg>
   <bn>Telugu Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Telugu per LibreOffice</ca>
   <cs>Telugu Language Meta-Package for LibreOffice</cs>
   <da>Telugu Language Meta-Package for LibreOffice</da>
   <de>Telugu Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Telugu</el>
   <en>Telugu Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma Telugu para LibreOffice</es>
   <et>Telugu Language Meta-Package for LibreOffice</et>
   <eu>Telugu Language Meta-Package for LibreOffice</eu>
   <fa>Telugu Language Meta-Package for LibreOffice</fa>
   <fil_PH>Telugu Language Meta-Package for LibreOffice</fil_PH>
   <fi>Telugu Language Meta-Package for LibreOffice</fi>
   <fr>Telugu Language Meta-Package for LibreOffice</fr>
   <he_IL>Telugu Language Meta-Package for LibreOffice</he_IL>
   <hi>Telugu Language Meta-Package for LibreOffice</hi>
   <hr>Telugu Language Meta-Package for LibreOffice</hr>
   <hu>Telugu Language Meta-Package for LibreOffice</hu>
   <id>Telugu Language Meta-Package for LibreOffice</id>
   <is>Telugu Language Meta-Package for LibreOffice</is>
   <it>Telugu Language Meta-Package for LibreOffice</it>
   <ja_JP>Telugu Language Meta-Package for LibreOffice</ja_JP>
   <ja>Telugu Language Meta-Package for LibreOffice</ja>
   <kk>Telugu Language Meta-Package for LibreOffice</kk>
   <ko>Telugu Language Meta-Package for LibreOffice</ko>
   <lt>Telugu Language Meta-Package for LibreOffice</lt>
   <mk>Telugu Language Meta-Package for LibreOffice</mk>
   <mr>Telugu Language Meta-Package for LibreOffice</mr>
   <nb>Telugu Language Meta-Package for LibreOffice</nb>
   <nl>Telugu Language Meta-Package for LibreOffice</nl>
   <pl>Telugu Language Meta-Package for LibreOffice</pl>
   <pt_BR>Telugu Language Meta-Package for LibreOffice</pt_BR>
   <pt>Telego Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Telugu Language Meta-Package for LibreOffice</ro>
   <ru>Telugu Language Meta-Package for LibreOffice</ru>
   <sk>Telugu Language Meta-Package for LibreOffice</sk>
   <sl>Teluški jezikovni meta-paket za LibreOffice</sl>
   <sq>Telugu Language Meta-Package for LibreOffice</sq>
   <sr>Telugu Language Meta-Package for LibreOffice</sr>
   <sv>Telugu Språk Meta-Paket för LibreOffice</sv>
   <tr>Telugu Language Meta-Package for LibreOffice</tr>
   <uk>Telugu Language Meta-Package for LibreOffice</uk>
   <vi>Telugu Language Meta-Package for LibreOffice</vi>
   <zh_CN>Telugu Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Telugu Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-te
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-te
</uninstall_package_names>
</app>

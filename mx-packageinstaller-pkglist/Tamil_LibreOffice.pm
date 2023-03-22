<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Tamil_Libreoffice
</name>

<description>
   <am>Tamil Language Meta-Package for LibreOffice</am>
   <ar>Tamil Language Meta-Package for LibreOffice</ar>
   <bg>Tamil Language Meta-Package for LibreOffice</bg>
   <bn>Tamil Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Tàmil per LibreOffice</ca>
   <cs>Tamil Language Meta-Package for LibreOffice</cs>
   <da>Tamil Language Meta-Package for LibreOffice</da>
   <de>Tamilisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Tamil</el>
   <en>Tamil Language Meta-Package for LibreOffice</en>
   <es>Meta Paquete de Idioma Tamil para LibreOffice</es>
   <et>Tamil Language Meta-Package for LibreOffice</et>
   <eu>Tamil Language Meta-Package for LibreOffice</eu>
   <fa>Tamil Language Meta-Package for LibreOffice</fa>
   <fil_PH>Tamil Language Meta-Package for LibreOffice</fil_PH>
   <fi>Tamil Language Meta-Package for LibreOffice</fi>
   <fr>Tamil Language Meta-Package for LibreOffice</fr>
   <he_IL>Tamil Language Meta-Package for LibreOffice</he_IL>
   <hi>Tamil Language Meta-Package for LibreOffice</hi>
   <hr>Tamil Language Meta-Package for LibreOffice</hr>
   <hu>Tamil Language Meta-Package for LibreOffice</hu>
   <id>Tamil Language Meta-Package for LibreOffice</id>
   <is>Tamil Language Meta-Package for LibreOffice</is>
   <it>Tamil Language Meta-Package for LibreOffice</it>
   <ja_JP>Tamil Language Meta-Package for LibreOffice</ja_JP>
   <ja>Tamil Language Meta-Package for LibreOffice</ja>
   <kk>Tamil Language Meta-Package for LibreOffice</kk>
   <ko>Tamil Language Meta-Package for LibreOffice</ko>
   <lt>Tamil Language Meta-Package for LibreOffice</lt>
   <mk>Tamil Language Meta-Package for LibreOffice</mk>
   <mr>Tamil Language Meta-Package for LibreOffice</mr>
   <nb>Tamil Language Meta-Package for LibreOffice</nb>
   <nl>Tamil Language Meta-Package for LibreOffice</nl>
   <pl>Tamil Language Meta-Package for LibreOffice</pl>
   <pt_BR>Tamil Language Meta-Package for LibreOffice</pt_BR>
   <pt>Tamil Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Tamil Language Meta-Package for LibreOffice</ro>
   <ru>Tamil Language Meta-Package for LibreOffice</ru>
   <sk>Tamil Language Meta-Package for LibreOffice</sk>
   <sl>Tamilski jezikovni meta-paket za LibreOffice</sl>
   <sq>Tamil Language Meta-Package for LibreOffice</sq>
   <sr>Tamil Language Meta-Package for LibreOffice</sr>
   <sv>Tamil Språk Meta-Paket för LibreOffice</sv>
   <tr>Tamil Language Meta-Package for LibreOffice</tr>
   <uk>Tamil Language Meta-Package for LibreOffice</uk>
   <vi>Tamil Language Meta-Package for LibreOffice</vi>
   <zh_CN>Tamil Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Tamil Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ta
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ta
</uninstall_package_names>
</app>

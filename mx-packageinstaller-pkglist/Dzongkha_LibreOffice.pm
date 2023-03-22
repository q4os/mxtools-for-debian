<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Dzongkha_Libreoffice
</name>

<description>
   <am>Dzongkha Language Meta-Package for LibreOffice</am>
   <ar>Dzongkha Language Meta-Package for LibreOffice</ar>
   <bg>Dzongkha Language Meta-Package for LibreOffice</bg>
   <bn>Dzongkha Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Dzongkha per LibreOffice</ca>
   <cs>Dzongkha Language Meta-Package for LibreOffice</cs>
   <da>Dzongkha Language Meta-Package for LibreOffice</da>
   <de>Dzongkha Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Dzongkha</el>
   <en>Dzongkha Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de Dzongkha Language para LibreOffice</es>
   <et>Dzongkha Language Meta-Package for LibreOffice</et>
   <eu>Dzongkha Language Meta-Package for LibreOffice</eu>
   <fa>Dzongkha Language Meta-Package for LibreOffice</fa>
   <fil_PH>Dzongkha Language Meta-Package for LibreOffice</fil_PH>
   <fi>Dzongkha Language Meta-Package for LibreOffice</fi>
   <fr>Dzongkha Language Meta-Package for LibreOffice</fr>
   <he_IL>Dzongkha Language Meta-Package for LibreOffice</he_IL>
   <hi>Dzongkha Language Meta-Package for LibreOffice</hi>
   <hr>Dzongkha Language Meta-Package for LibreOffice</hr>
   <hu>Dzongkha Language Meta-Package for LibreOffice</hu>
   <id>Dzongkha Language Meta-Package for LibreOffice</id>
   <is>Dzongkha Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua dzongkha per LibreOffice</it>
   <ja_JP>Dzongkha Language Meta-Package for LibreOffice</ja_JP>
   <ja>Dzongkha Language Meta-Package for LibreOffice</ja>
   <kk>Dzongkha Language Meta-Package for LibreOffice</kk>
   <ko>Dzongkha Language Meta-Package for LibreOffice</ko>
   <lt>Dzongkha Language Meta-Package for LibreOffice</lt>
   <mk>Dzongkha Language Meta-Package for LibreOffice</mk>
   <mr>Dzongkha Language Meta-Package for LibreOffice</mr>
   <nb>Dzongkha Language Meta-Package for LibreOffice</nb>
   <nl>Dzongkha Language Meta-Package for LibreOffice</nl>
   <pl>Dzongkha Language Meta-Package for LibreOffice</pl>
   <pt_BR>Dzongkha Language Meta-Package for LibreOffice</pt_BR>
   <pt>Dzonga Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Dzongkha Language Meta-Package for LibreOffice</ro>
   <ru>Dzongkha Language Meta-Package for LibreOffice</ru>
   <sk>Dzongkha Language Meta-Package for LibreOffice</sk>
   <sl>Dzongha jezikovni meta-paket za LibreOffice</sl>
   <sq>Dzongkha Language Meta-Package for LibreOffice</sq>
   <sr>Dzongkha Language Meta-Package for LibreOffice</sr>
   <sv>Dzongkha Språk Meta-Paket för LibreOffice</sv>
   <tr>Dzongkha Language Meta-Package for LibreOffice</tr>
   <uk>Dzongkha Language Meta-Package for LibreOffice</uk>
   <vi>Dzongkha Language Meta-Package for LibreOffice</vi>
   <zh_CN>Dzongkha Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Dzongkha Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-dz
libreoffice-help-dz
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-dz
libreoffice-help-dz
</uninstall_package_names>
</app>

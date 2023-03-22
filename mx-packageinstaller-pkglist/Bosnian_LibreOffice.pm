<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Bosnian_Libreoffice
</name>

<description>
   <am>Bosnian Language Meta-Package for LibreOffice</am>
   <ar>Bosnian Language Meta-Package for LibreOffice</ar>
   <bg>Bosnian Language Meta-Package for LibreOffice</bg>
   <bn>Bosnian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Bosnià per LibreOffice</ca>
   <cs>Bosnian Language Meta-Package for LibreOffice</cs>
   <da>Bosnian Language Meta-Package for LibreOffice</da>
   <de>Bosnisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Bosnian</el>
   <en>Bosnian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma bosnio para LibreOffice</es>
   <et>Bosnian Language Meta-Package for LibreOffice</et>
   <eu>Bosnian Language Meta-Package for LibreOffice</eu>
   <fa>Bosnian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Bosnian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Bosnian Language Meta-Package for LibreOffice</fi>
   <fr>Bosnian Language Meta-Package for LibreOffice</fr>
   <he_IL>Bosnian Language Meta-Package for LibreOffice</he_IL>
   <hi>Bosnian Language Meta-Package for LibreOffice</hi>
   <hr>Bosnian Language Meta-Package for LibreOffice</hr>
   <hu>Bosnian Language Meta-Package for LibreOffice</hu>
   <id>Bosnian Language Meta-Package for LibreOffice</id>
   <is>Bosnian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua bosniaca per LibreOffice</it>
   <ja_JP>Bosnian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Bosnian Language Meta-Package for LibreOffice</ja>
   <kk>Bosnian Language Meta-Package for LibreOffice</kk>
   <ko>Bosnian Language Meta-Package for LibreOffice</ko>
   <lt>Bosnian Language Meta-Package for LibreOffice</lt>
   <mk>Bosnian Language Meta-Package for LibreOffice</mk>
   <mr>Bosnian Language Meta-Package for LibreOffice</mr>
   <nb>Bosnian Language Meta-Package for LibreOffice</nb>
   <nl>Bosnian Language Meta-Package for LibreOffice</nl>
   <pl>Bosnian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Bosnian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Bósnio Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Bosnian Language Meta-Package for LibreOffice</ro>
   <ru>Bosnian Language Meta-Package for LibreOffice</ru>
   <sk>Bosnian Language Meta-Package for LibreOffice</sk>
   <sl>Bosanski jezikovni meta-paket za LibreOffice</sl>
   <sq>Bosnian Language Meta-Package for LibreOffice</sq>
   <sr>Bosnian Language Meta-Package for LibreOffice</sr>
   <sv>Bosniskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Bosnian Language Meta-Package for LibreOffice</tr>
   <uk>Bosnian Language Meta-Package for LibreOffice</uk>
   <vi>Bosnian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Bosnian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Bosnian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-bs
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-bs
</uninstall_package_names>
</app>

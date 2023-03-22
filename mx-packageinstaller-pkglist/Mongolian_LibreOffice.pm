<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Mongolian_Libreoffice
</name>

<description>
   <am>Mongolian Language Meta-Package for LibreOffice</am>
   <ar>Mongolian Language Meta-Package for LibreOffice</ar>
   <bg>Mongolian Language Meta-Package for LibreOffice</bg>
   <bn>Mongolian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Mongol per LibreOffice</ca>
   <cs>Mongolian Language Meta-Package for LibreOffice</cs>
   <da>Mongolian Language Meta-Package for LibreOffice</da>
   <de>Mongolisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Mongolian</el>
   <en>Mongolian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma mongol para LibreOffice</es>
   <et>Mongolian Language Meta-Package for LibreOffice</et>
   <eu>Mongolian Language Meta-Package for LibreOffice</eu>
   <fa>Mongolian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Mongolian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Mongolian Language Meta-Package for LibreOffice</fi>
   <fr>Mongolian Language Meta-Package for LibreOffice</fr>
   <he_IL>Mongolian Language Meta-Package for LibreOffice</he_IL>
   <hi>Mongolian Language Meta-Package for LibreOffice</hi>
   <hr>Mongolian Language Meta-Package for LibreOffice</hr>
   <hu>Mongolian Language Meta-Package for LibreOffice</hu>
   <id>Mongolian Language Meta-Package for LibreOffice</id>
   <is>Mongolian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua mongola per LibreOffice</it>
   <ja_JP>Mongolian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Mongolian Language Meta-Package for LibreOffice</ja>
   <kk>Mongolian Language Meta-Package for LibreOffice</kk>
   <ko>Mongolian Language Meta-Package for LibreOffice</ko>
   <lt>Mongolian Language Meta-Package for LibreOffice</lt>
   <mk>Mongolian Language Meta-Package for LibreOffice</mk>
   <mr>Mongolian Language Meta-Package for LibreOffice</mr>
   <nb>Mongolian Language Meta-Package for LibreOffice</nb>
   <nl>Mongools Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Mongolian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Mongolian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Mongol Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Mongolian Language Meta-Package for LibreOffice</ro>
   <ru>Mongolian Language Meta-Package for LibreOffice</ru>
   <sk>Mongolian Language Meta-Package for LibreOffice</sk>
   <sl>Mongolski jezikovni meta-paket za LibreOffice</sl>
   <sq>Mongolian Language Meta-Package for LibreOffice</sq>
   <sr>Mongolian Language Meta-Package for LibreOffice</sr>
   <sv>Mongoliskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Mongolian Language Meta-Package for LibreOffice</tr>
   <uk>Mongolian Language Meta-Package for LibreOffice</uk>
   <vi>Mongolian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Mongolian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Mongolian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-mn
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-mn
</uninstall_package_names>
</app>

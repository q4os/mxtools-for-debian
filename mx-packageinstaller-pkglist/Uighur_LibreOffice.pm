<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Uighur_Libreoffice
</name>

<description>
   <am>Uighur Language Meta-Package for LibreOffice</am>
   <ar>Uighur Language Meta-Package for LibreOffice</ar>
   <bg>Uighur Language Meta-Package for LibreOffice</bg>
   <bn>Uighur Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Uigur per LibreOffice</ca>
   <cs>Uighur Language Meta-Package for LibreOffice</cs>
   <da>Uighur Language Meta-Package for LibreOffice</da>
   <de>Uighurisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Uighur</el>
   <en>Uighur Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete idioma uighur para LibreOffice</es>
   <et>Uighur Language Meta-Package for LibreOffice</et>
   <eu>Uighur Language Meta-Package for LibreOffice</eu>
   <fa>Uighur Language Meta-Package for LibreOffice</fa>
   <fil_PH>Uighur Language Meta-Package for LibreOffice</fil_PH>
   <fi>Uighur Language Meta-Package for LibreOffice</fi>
   <fr>Uighur Language Meta-Package for LibreOffice</fr>
   <he_IL>Uighur Language Meta-Package for LibreOffice</he_IL>
   <hi>Uighur Language Meta-Package for LibreOffice</hi>
   <hr>Uighur Language Meta-Package for LibreOffice</hr>
   <hu>Uighur Language Meta-Package for LibreOffice</hu>
   <id>Uighur Language Meta-Package for LibreOffice</id>
   <is>Uighur Language Meta-Package for LibreOffice</is>
   <it>Uighur Language Meta-Package for LibreOffice</it>
   <ja_JP>Uighur Language Meta-Package for LibreOffice</ja_JP>
   <ja>Uighur Language Meta-Package for LibreOffice</ja>
   <kk>Uighur Language Meta-Package for LibreOffice</kk>
   <ko>Uighur Language Meta-Package for LibreOffice</ko>
   <lt>Uighur Language Meta-Package for LibreOffice</lt>
   <mk>Uighur Language Meta-Package for LibreOffice</mk>
   <mr>Uighur Language Meta-Package for LibreOffice</mr>
   <nb>Uighur Language Meta-Package for LibreOffice</nb>
   <nl>Uighur Language Meta-Package for LibreOffice</nl>
   <pl>Uighur Language Meta-Package for LibreOffice</pl>
   <pt_BR>Uighur Language Meta-Package for LibreOffice</pt_BR>
   <pt>Uigur Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Uighur Language Meta-Package for LibreOffice</ro>
   <ru>Uighur Language Meta-Package for LibreOffice</ru>
   <sk>Uighur Language Meta-Package for LibreOffice</sk>
   <sl>Ujgurski jezikovni meta-paket za LibreOffice</sl>
   <sq>Uighur Language Meta-Package for LibreOffice</sq>
   <sr>Uighur Language Meta-Package for LibreOffice</sr>
   <sv>Uighur Språk Meta-Paket för LibreOffice</sv>
   <tr>Uighur Language Meta-Package for LibreOffice</tr>
   <uk>Uighur Language Meta-Package for LibreOffice</uk>
   <vi>Uighur Language Meta-Package for LibreOffice</vi>
   <zh_CN>Uighur Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Uighur Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ug
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ug
</uninstall_package_names>
</app>

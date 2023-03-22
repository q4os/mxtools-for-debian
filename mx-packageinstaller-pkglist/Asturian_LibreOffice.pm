<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Asturian_Libreoffice
</name>

<description>
   <am>Asturian Language Meta-Package for LibreOffice</am>
   <ar>Asturian Language Meta-Package for LibreOffice</ar>
   <bg>Asturian Language Meta-Package for LibreOffice</bg>
   <bn>Asturian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Asturià per LibreOffice</ca>
   <cs>Asturian Language Meta-Package for LibreOffice</cs>
   <da>Asturian Language Meta-Package for LibreOffice</da>
   <de>Asturisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Asturian</el>
   <en>Asturian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de lengua asturiana para LibreOffice</es>
   <et>Asturian Language Meta-Package for LibreOffice</et>
   <eu>Asturian Language Meta-Package for LibreOffice</eu>
   <fa>Asturian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Asturian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Asturian Language Meta-Package for LibreOffice</fi>
   <fr>Asturian Language Meta-Package for LibreOffice</fr>
   <he_IL>Asturian Language Meta-Package for LibreOffice</he_IL>
   <hi>Asturian Language Meta-Package for LibreOffice</hi>
   <hr>Asturian Language Meta-Package for LibreOffice</hr>
   <hu>Asturian Language Meta-Package for LibreOffice</hu>
   <id>Asturian Language Meta-Package for LibreOffice</id>
   <is>Asturian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua asturiana per LibreOffice</it>
   <ja_JP>Asturian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Asturian Language Meta-Package for LibreOffice</ja>
   <kk>Asturian Language Meta-Package for LibreOffice</kk>
   <ko>Asturian Language Meta-Package for LibreOffice</ko>
   <lt>Asturian Language Meta-Package for LibreOffice</lt>
   <mk>Asturian Language Meta-Package for LibreOffice</mk>
   <mr>Asturian Language Meta-Package for LibreOffice</mr>
   <nb>Asturian Language Meta-Package for LibreOffice</nb>
   <nl>Asturiaans Taal Meta-Pakket voor LibreOffice​</nl>
   <pl>Asturian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Asturian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Asturiano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Asturian Language Meta-Package for LibreOffice</ro>
   <ru>Asturian Language Meta-Package for LibreOffice</ru>
   <sk>Asturian Language Meta-Package for LibreOffice</sk>
   <sl>Asturijanski jezikovni meta-paket za LibreOffice</sl>
   <sq>Asturian Language Meta-Package for LibreOffice</sq>
   <sr>Asturian Language Meta-Package for LibreOffice</sr>
   <sv>Asturiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Asturian Language Meta-Package for LibreOffice</tr>
   <uk>Asturian Language Meta-Package for LibreOffice</uk>
   <vi>Asturian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Asturian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Asturian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ast
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ast
</uninstall_package_names>
</app>

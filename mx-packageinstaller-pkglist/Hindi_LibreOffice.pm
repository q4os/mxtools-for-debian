<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Hindi_Libreoffice
</name>

<description>
   <am>Hindi Language Meta-Package for LibreOffice</am>
   <ar>Hindi Language Meta-Package for LibreOffice</ar>
   <bg>Hindi Language Meta-Package for LibreOffice</bg>
   <bn>Hindi Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Hindi per LibreOffice</ca>
   <cs>Hindi Language Meta-Package for LibreOffice</cs>
   <da>Hindi Language Meta-Package for LibreOffice</da>
   <de>Hindi Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Χίντι</el>
   <en>Hindi Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma hindi para LibreOffice</es>
   <et>Hindi Language Meta-Package for LibreOffice</et>
   <eu>Hindi Language Meta-Package for LibreOffice</eu>
   <fa>Hindi Language Meta-Package for LibreOffice</fa>
   <fil_PH>Hindi Language Meta-Package for LibreOffice</fil_PH>
   <fi>Hindi Language Meta-Package for LibreOffice</fi>
   <fr>Hindi Language Meta-Package for LibreOffice</fr>
   <he_IL>Hindi Language Meta-Package for LibreOffice</he_IL>
   <hi>Hindi Language Meta-Package for LibreOffice</hi>
   <hr>Hindi Language Meta-Package for LibreOffice</hr>
   <hu>Hindi Language Meta-Package for LibreOffice</hu>
   <id>Hindi Language Meta-Package for LibreOffice</id>
   <is>Hindi Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua hindi per LibreOffice</it>
   <ja_JP>Hindi Language Meta-Package for LibreOffice</ja_JP>
   <ja>Hindi Language Meta-Package for LibreOffice</ja>
   <kk>Hindi Language Meta-Package for LibreOffice</kk>
   <ko>Hindi Language Meta-Package for LibreOffice</ko>
   <lt>Hindi Language Meta-Package for LibreOffice</lt>
   <mk>Hindi Language Meta-Package for LibreOffice</mk>
   <mr>Hindi Language Meta-Package for LibreOffice</mr>
   <nb>Hindi Language Meta-Package for LibreOffice</nb>
   <nl>Hindi Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Hindi Language Meta-Package for LibreOffice</pl>
   <pt_BR>Hindi Language Meta-Package for LibreOffice</pt_BR>
   <pt>Hindi Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Hindi Language Meta-Package for LibreOffice</ro>
   <ru>Hindi Language Meta-Package for LibreOffice</ru>
   <sk>Hindi Language Meta-Package for LibreOffice</sk>
   <sl>Hindujski jezikovni meta-paket za LibreOffice</sl>
   <sq>Hindi Language Meta-Package for LibreOffice</sq>
   <sr>Hindi Language Meta-Package for LibreOffice</sr>
   <sv>Hindi Språk Meta-Paket för LibreOffice</sv>
   <tr>Hindi Language Meta-Package for LibreOffice</tr>
   <uk>Hindi Language Meta-Package for LibreOffice</uk>
   <vi>Hindi Language Meta-Package for LibreOffice</vi>
   <zh_CN>Hindi Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Hindi Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-hi
libreoffice-help-hi
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-hi
libreoffice-help-hi
</uninstall_package_names>
</app>

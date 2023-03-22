<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Gujarati_Libreoffice
</name>

<description>
   <am>Gujarati Language Meta-Package for LibreOffice</am>
   <ar>Gujarati Language Meta-Package for LibreOffice</ar>
   <bg>Gujarati Language Meta-Package for LibreOffice</bg>
   <bn>Gujarati Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Gujarati per LibreOffice</ca>
   <cs>Gujarati Language Meta-Package for LibreOffice</cs>
   <da>Gujarati Language Meta-Package for LibreOffice</da>
   <de>Gujarati Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Gujarati</el>
   <en>Gujarati Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma Guyaratí para LibreOffice</es>
   <et>Gujarati Language Meta-Package for LibreOffice</et>
   <eu>Gujarati Language Meta-Package for LibreOffice</eu>
   <fa>Gujarati Language Meta-Package for LibreOffice</fa>
   <fil_PH>Gujarati Language Meta-Package for LibreOffice</fil_PH>
   <fi>Gujarati Language Meta-Package for LibreOffice</fi>
   <fr>Gujarati Language Meta-Package for LibreOffice</fr>
   <he_IL>Gujarati Language Meta-Package for LibreOffice</he_IL>
   <hi>Gujarati Language Meta-Package for LibreOffice</hi>
   <hr>Gujarati Language Meta-Package for LibreOffice</hr>
   <hu>Gujarati Language Meta-Package for LibreOffice</hu>
   <id>Gujarati Language Meta-Package for LibreOffice</id>
   <is>Gujarati Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua gujarati per LibreOffice</it>
   <ja_JP>Gujarati Language Meta-Package for LibreOffice</ja_JP>
   <ja>Gujarati Language Meta-Package for LibreOffice</ja>
   <kk>Gujarati Language Meta-Package for LibreOffice</kk>
   <ko>Gujarati Language Meta-Package for LibreOffice</ko>
   <lt>Gujarati Language Meta-Package for LibreOffice</lt>
   <mk>Gujarati Language Meta-Package for LibreOffice</mk>
   <mr>Gujarati Language Meta-Package for LibreOffice</mr>
   <nb>Gujarati Language Meta-Package for LibreOffice</nb>
   <nl>Gujarati Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Gujarati Language Meta-Package for LibreOffice</pl>
   <pt_BR>Gujarati Language Meta-Package for LibreOffice</pt_BR>
   <pt>Gujarati Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Gujarati Language Meta-Package for LibreOffice</ro>
   <ru>Gujarati Language Meta-Package for LibreOffice</ru>
   <sk>Gujarati Language Meta-Package for LibreOffice</sk>
   <sl>Gujaratski jezikovni meta-paket za LibreOffice</sl>
   <sq>Gujarati Language Meta-Package for LibreOffice</sq>
   <sr>Gujarati Language Meta-Package for LibreOffice</sr>
   <sv>Gujarati Språk Meta-Paket för LibreOffice</sv>
   <tr>Gujarati Language Meta-Package for LibreOffice</tr>
   <uk>Gujarati Language Meta-Package for LibreOffice</uk>
   <vi>Gujarati Language Meta-Package for LibreOffice</vi>
   <zh_CN>Gujarati Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Gujarati Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-gu
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-gu
</uninstall_package_names>
</app>

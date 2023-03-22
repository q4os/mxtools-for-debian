<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Nepalese_Libreoffice
</name>

<description>
   <am>Nepalese Language Meta-Package for LibreOffice</am>
   <ar>Nepalese Language Meta-Package for LibreOffice</ar>
   <bg>Nepalese Language Meta-Package for LibreOffice</bg>
   <bn>Nepalese Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Nepalès per LibreOffice</ca>
   <cs>Nepalese Language Meta-Package for LibreOffice</cs>
   <da>Nepalese Language Meta-Package for LibreOffice</da>
   <de>Nepalesisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Nepalese</el>
   <en>Nepalese Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma nepalí para LibreOffice</es>
   <et>Nepalese Language Meta-Package for LibreOffice</et>
   <eu>Nepalese Language Meta-Package for LibreOffice</eu>
   <fa>Nepalese Language Meta-Package for LibreOffice</fa>
   <fil_PH>Nepalese Language Meta-Package for LibreOffice</fil_PH>
   <fi>Nepalese Language Meta-Package for LibreOffice</fi>
   <fr>Nepalese Language Meta-Package for LibreOffice</fr>
   <he_IL>Nepalese Language Meta-Package for LibreOffice</he_IL>
   <hi>Nepalese Language Meta-Package for LibreOffice</hi>
   <hr>Nepalese Language Meta-Package for LibreOffice</hr>
   <hu>Nepalese Language Meta-Package for LibreOffice</hu>
   <id>Nepalese Language Meta-Package for LibreOffice</id>
   <is>Nepalese Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua nepalese per LibreOffice</it>
   <ja_JP>Nepalese Language Meta-Package for LibreOffice</ja_JP>
   <ja>Nepalese Language Meta-Package for LibreOffice</ja>
   <kk>Nepalese Language Meta-Package for LibreOffice</kk>
   <ko>Nepalese Language Meta-Package for LibreOffice</ko>
   <lt>Nepalese Language Meta-Package for LibreOffice</lt>
   <mk>Nepalese Language Meta-Package for LibreOffice</mk>
   <mr>Nepalese Language Meta-Package for LibreOffice</mr>
   <nb>Nepalese Language Meta-Package for LibreOffice</nb>
   <nl>Nepalees Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Nepalese Language Meta-Package for LibreOffice</pl>
   <pt_BR>Nepalese Language Meta-Package for LibreOffice</pt_BR>
   <pt>Nepalês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Nepalese Language Meta-Package for LibreOffice</ro>
   <ru>Nepalese Language Meta-Package for LibreOffice</ru>
   <sk>Nepalese Language Meta-Package for LibreOffice</sk>
   <sl>Nepalski jezikovni meta-paket za LibreOffice</sl>
   <sq>Nepalese Language Meta-Package for LibreOffice</sq>
   <sr>Nepalese Language Meta-Package for LibreOffice</sr>
   <sv>Nepalesiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Nepalese Language Meta-Package for LibreOffice</tr>
   <uk>Nepalese Language Meta-Package for LibreOffice</uk>
   <vi>Nepalese Language Meta-Package for LibreOffice</vi>
   <zh_CN>Nepalese Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Nepalese Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ne
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ne
</uninstall_package_names>
</app>

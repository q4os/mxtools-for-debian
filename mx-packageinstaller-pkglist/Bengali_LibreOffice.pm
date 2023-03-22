<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Bengali_Libreoffice
</name>

<description>
   <am>Bengali Language Meta-Package for LibreOffice</am>
   <ar>Bengali Language Meta-Package for LibreOffice</ar>
   <bg>Bengali Language Meta-Package for LibreOffice</bg>
   <bn>Bengali Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Bengalí per LibreOffice</ca>
   <cs>Bengali Language Meta-Package for LibreOffice</cs>
   <da>Bengali Language Meta-Package for LibreOffice</da>
   <de>Bengalisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Bengali</el>
   <en>Bengali Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma bengalí para LibreOffice</es>
   <et>Bengali Language Meta-Package for LibreOffice</et>
   <eu>Bengali Language Meta-Package for LibreOffice</eu>
   <fa>Bengali Language Meta-Package for LibreOffice</fa>
   <fil_PH>Bengali Language Meta-Package for LibreOffice</fil_PH>
   <fi>Bengali Language Meta-Package for LibreOffice</fi>
   <fr>Bengali Language Meta-Package for LibreOffice</fr>
   <he_IL>Bengali Language Meta-Package for LibreOffice</he_IL>
   <hi>Bengali Language Meta-Package for LibreOffice</hi>
   <hr>Bengali Language Meta-Package for LibreOffice</hr>
   <hu>Bengali Language Meta-Package for LibreOffice</hu>
   <id>Bengali Language Meta-Package for LibreOffice</id>
   <is>Bengali Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua bengalese per LibreOffice</it>
   <ja_JP>Bengali Language Meta-Package for LibreOffice</ja_JP>
   <ja>Bengali Language Meta-Package for LibreOffice</ja>
   <kk>Bengali Language Meta-Package for LibreOffice</kk>
   <ko>Bengali Language Meta-Package for LibreOffice</ko>
   <lt>Bengali Language Meta-Package for LibreOffice</lt>
   <mk>Bengali Language Meta-Package for LibreOffice</mk>
   <mr>Bengali Language Meta-Package for LibreOffice</mr>
   <nb>Bengali Language Meta-Package for LibreOffice</nb>
   <nl>Bengali Language Meta-Package for LibreOffice</nl>
   <pl>Bengali Language Meta-Package for LibreOffice</pl>
   <pt_BR>Bengali Language Meta-Package for LibreOffice</pt_BR>
   <pt>Bengali Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Bengali Language Meta-Package for LibreOffice</ro>
   <ru>Bengali Language Meta-Package for LibreOffice</ru>
   <sk>Bengali Language Meta-Package for LibreOffice</sk>
   <sl>Bengalski jezikovni meta-paket za LibreOffice</sl>
   <sq>Bengali Language Meta-Package for LibreOffice</sq>
   <sr>Bengali Language Meta-Package for LibreOffice</sr>
   <sv>Bengali Språk Meta-Paket för LibreOffice</sv>
   <tr>Bengali Language Meta-Package for LibreOffice</tr>
   <uk>Bengali Language Meta-Package for LibreOffice</uk>
   <vi>Bengali Language Meta-Package for LibreOffice</vi>
   <zh_CN>Bengali Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Bengali Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-bn
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-bn
</uninstall_package_names>
</app>

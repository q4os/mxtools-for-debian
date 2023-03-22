<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Indic_Libreoffice
</name>

<description>
   <am>Indic Language Meta-Package for LibreOffice</am>
   <ar>Indic Language Meta-Package for LibreOffice</ar>
   <bg>Indic Language Meta-Package for LibreOffice</bg>
   <bn>Indic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Índic per LibreOffice</ca>
   <cs>Indic Language Meta-Package for LibreOffice</cs>
   <da>Indic Language Meta-Package for LibreOffice</da>
   <de>Indisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Indic</el>
   <en>Indic Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Indic para LibreOffice</es>
   <et>Indic Language Meta-Package for LibreOffice</et>
   <eu>Indic Language Meta-Package for LibreOffice</eu>
   <fa>Indic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Indic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Indic Language Meta-Package for LibreOffice</fi>
   <fr>Indic Language Meta-Package for LibreOffice</fr>
   <he_IL>Indic Language Meta-Package for LibreOffice</he_IL>
   <hi>Indic Language Meta-Package for LibreOffice</hi>
   <hr>Indic Language Meta-Package for LibreOffice</hr>
   <hu>Indic Language Meta-Package for LibreOffice</hu>
   <id>Indic Language Meta-Package for LibreOffice</id>
   <is>Indic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua indica per LibreOffice</it>
   <ja_JP>Indic Language Meta-Package for LibreOffice</ja_JP>
   <ja>Indic Language Meta-Package for LibreOffice</ja>
   <kk>Indic Language Meta-Package for LibreOffice</kk>
   <ko>Indic Language Meta-Package for LibreOffice</ko>
   <lt>Indic Language Meta-Package for LibreOffice</lt>
   <mk>Indic Language Meta-Package for LibreOffice</mk>
   <mr>Indic Language Meta-Package for LibreOffice</mr>
   <nb>Indic Language Meta-Package for LibreOffice</nb>
   <nl>Indic Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Indic Language Meta-Package for LibreOffice</pl>
   <pt_BR>Indic Language Meta-Package for LibreOffice</pt_BR>
   <pt>Indic Language Meta-Package for LibreOffice</pt>
   <ro>Indic Language Meta-Package for LibreOffice</ro>
   <ru>Indic Language Meta-Package for LibreOffice</ru>
   <sk>Indic Language Meta-Package for LibreOffice</sk>
   <sl>Indikski jezikovni meta-paket za LibreOffice</sl>
   <sq>Indic Language Meta-Package for LibreOffice</sq>
   <sr>Indic Language Meta-Package for LibreOffice</sr>
   <sv>Indiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Indic Language Meta-Package for LibreOffice</tr>
   <uk>Indic Language Meta-Package for LibreOffice</uk>
   <vi>Indic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Indic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Indic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-in
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-in
</uninstall_package_names>
</app>

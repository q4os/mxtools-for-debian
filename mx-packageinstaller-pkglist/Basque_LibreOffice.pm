<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Basque_Libreoffice
</name>

<description>
   <am>Basque Language Meta-Package for LibreOffice</am>
   <ar>Basque Language Meta-Package for LibreOffice</ar>
   <bg>Basque Language Meta-Package for LibreOffice</bg>
   <bn>Basque Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Èuscar per LibreOffice</ca>
   <cs>Basque Language Meta-Package for LibreOffice</cs>
   <da>Basque Language Meta-Package for LibreOffice</da>
   <de>Baskisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Basque</el>
   <en>Basque Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete para LibreOffice en euskera</es>
   <et>Basque Language Meta-Package for LibreOffice</et>
   <eu>Basque Language Meta-Package for LibreOffice</eu>
   <fa>Basque Language Meta-Package for LibreOffice</fa>
   <fil_PH>Basque Language Meta-Package for LibreOffice</fil_PH>
   <fi>Basque Language Meta-Package for LibreOffice</fi>
   <fr>Basque Language Meta-Package for LibreOffice</fr>
   <he_IL>Basque Language Meta-Package for LibreOffice</he_IL>
   <hi>Basque Language Meta-Package for LibreOffice</hi>
   <hr>Basque Language Meta-Package for LibreOffice</hr>
   <hu>Basque Language Meta-Package for LibreOffice</hu>
   <id>Basque Language Meta-Package for LibreOffice</id>
   <is>Basque Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua basca per LibreOffice</it>
   <ja_JP>Basque Language Meta-Package for LibreOffice</ja_JP>
   <ja>Basque Language Meta-Package for LibreOffice</ja>
   <kk>Basque Language Meta-Package for LibreOffice</kk>
   <ko>Basque Language Meta-Package for LibreOffice</ko>
   <lt>Basque Language Meta-Package for LibreOffice</lt>
   <mk>Basque Language Meta-Package for LibreOffice</mk>
   <mr>Basque Language Meta-Package for LibreOffice</mr>
   <nb>Basque Language Meta-Package for LibreOffice</nb>
   <nl>Baskisch Taal Meta-Pakket voor LibreOffice​</nl>
   <pl>Basque Language Meta-Package for LibreOffice</pl>
   <pt_BR>Basque Language Meta-Package for LibreOffice</pt_BR>
   <pt>Basco Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Basque Language Meta-Package for LibreOffice</ro>
   <ru>Basque Language Meta-Package for LibreOffice</ru>
   <sk>Basque Language Meta-Package for LibreOffice</sk>
   <sl>Baskovski jezikovni meta-paket za LibreOffice</sl>
   <sq>Basque Language Meta-Package for LibreOffice</sq>
   <sr>Basque Language Meta-Package for LibreOffice</sr>
   <sv>Baskiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Basque Language Meta-Package for LibreOffice</tr>
   <uk>Basque Language Meta-Package for LibreOffice</uk>
   <vi>Basque Language Meta-Package for LibreOffice</vi>
   <zh_CN>Basque Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Basque Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-eu
libreoffice-help-eu
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-eu
libreoffice-help-eu
</uninstall_package_names>
</app>

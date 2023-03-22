<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Esperanto_Libreoffice
</name>

<description>
   <am>Esperanto Language Meta-Package for LibreOffice</am>
   <ar>Esperanto Language Meta-Package for LibreOffice</ar>
   <bg>Esperanto Language Meta-Package for LibreOffice</bg>
   <bn>Esperanto Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Esperanto per Libreoffice</ca>
   <cs>Esperanto Language Meta-Package for LibreOffice</cs>
   <da>Esperanto Language Meta-Package for LibreOffice</da>
   <de>Esperanto Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Εσπεράντο</el>
   <en>Esperanto Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma Esperanto para LibreOffice</es>
   <et>Esperanto Language Meta-Package for LibreOffice</et>
   <eu>Esperanto Language Meta-Package for LibreOffice</eu>
   <fa>Esperanto Language Meta-Package for LibreOffice</fa>
   <fil_PH>Esperanto Language Meta-Package for LibreOffice</fil_PH>
   <fi>Esperanto Language Meta-Package for LibreOffice</fi>
   <fr>Esperanto Language Meta-Package for LibreOffice</fr>
   <he_IL>Esperanto Language Meta-Package for LibreOffice</he_IL>
   <hi>Esperanto Language Meta-Package for LibreOffice</hi>
   <hr>Esperanto Language Meta-Package for LibreOffice</hr>
   <hu>Esperanto Language Meta-Package for LibreOffice</hu>
   <id>Esperanto Language Meta-Package for LibreOffice</id>
   <is>Esperanto Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua esperanto per LibreOffice</it>
   <ja_JP>Esperanto Language Meta-Package for LibreOffice</ja_JP>
   <ja>Esperanto Language Meta-Package for LibreOffice</ja>
   <kk>Esperanto Language Meta-Package for LibreOffice</kk>
   <ko>Esperanto Language Meta-Package for LibreOffice</ko>
   <lt>Esperanto Language Meta-Package for LibreOffice</lt>
   <mk>Esperanto Language Meta-Package for LibreOffice</mk>
   <mr>Esperanto Language Meta-Package for LibreOffice</mr>
   <nb>Esperanto Language Meta-Package for LibreOffice</nb>
   <nl>Esperanto Language Meta-Package for LibreOffice</nl>
   <pl>Esperanto Language Meta-Package for LibreOffice</pl>
   <pt_BR>Esperanto Language Meta-Package for LibreOffice</pt_BR>
   <pt>Esperanto Language Meta-Package for LibreOffice</pt>
   <ro>Esperanto Language Meta-Package for LibreOffice</ro>
   <ru>Esperanto Language Meta-Package for LibreOffice</ru>
   <sk>Esperanto Language Meta-Package for LibreOffice</sk>
   <sl>Esperanto jezikovni meta-paket za LibreOffice</sl>
   <sq>Esperanto Language Meta-Package for LibreOffice</sq>
   <sr>Esperanto Language Meta-Package for LibreOffice</sr>
   <sv>Esperanto Språk Meta-Paket för LibreOffice</sv>
   <tr>Esperanto Language Meta-Package for LibreOffice</tr>
   <uk>Esperanto Language Meta-Package for LibreOffice</uk>
   <vi>Esperanto Language Meta-Package for LibreOffice</vi>
   <zh_CN>Esperanto Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Esperanto Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-eo
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-eo
</uninstall_package_names>
</app>

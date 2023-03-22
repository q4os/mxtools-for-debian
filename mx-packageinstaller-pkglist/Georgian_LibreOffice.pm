<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Georgian_Libreoffice
</name>

<description>
   <am>Georgian Language Meta-Package for LibreOffice</am>
   <ar>Georgian Language Meta-Package for LibreOffice</ar>
   <bg>Georgian Language Meta-Package for LibreOffice</bg>
   <bn>Georgian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Georgià per LibreOffice</ca>
   <cs>Georgian Language Meta-Package for LibreOffice</cs>
   <da>Georgian Language Meta-Package for LibreOffice</da>
   <de>Georgisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Georgian</el>
   <en>Georgian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma georgiano para LibreOffice</es>
   <et>Georgian Language Meta-Package for LibreOffice</et>
   <eu>Georgian Language Meta-Package for LibreOffice</eu>
   <fa>Georgian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Georgian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Georgian Language Meta-Package for LibreOffice</fi>
   <fr>Georgian Language Meta-Package for LibreOffice</fr>
   <he_IL>Georgian Language Meta-Package for LibreOffice</he_IL>
   <hi>Georgian Language Meta-Package for LibreOffice</hi>
   <hr>Georgian Language Meta-Package for LibreOffice</hr>
   <hu>Georgian Language Meta-Package for LibreOffice</hu>
   <id>Georgian Language Meta-Package for LibreOffice</id>
   <is>Georgian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua georgiana per LibreOffice</it>
   <ja_JP>Georgian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Georgian Language Meta-Package for LibreOffice</ja>
   <kk>Georgian Language Meta-Package for LibreOffice</kk>
   <ko>Georgian Language Meta-Package for LibreOffice</ko>
   <lt>Georgian Language Meta-Package for LibreOffice</lt>
   <mk>Georgian Language Meta-Package for LibreOffice</mk>
   <mr>Georgian Language Meta-Package for LibreOffice</mr>
   <nb>Georgian Language Meta-Package for LibreOffice</nb>
   <nl>Georgian Language Meta-Package for LibreOffice</nl>
   <pl>Georgian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Georgian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Georgiano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Georgian Language Meta-Package for LibreOffice</ro>
   <ru>Georgian Language Meta-Package for LibreOffice</ru>
   <sk>Georgian Language Meta-Package for LibreOffice</sk>
   <sl>Gruzijski jezikovni meta-paket za LibreOffice</sl>
   <sq>Georgian Language Meta-Package for LibreOffice</sq>
   <sr>Georgian Language Meta-Package for LibreOffice</sr>
   <sv>Georgiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Georgian Language Meta-Package for LibreOffice</tr>
   <uk>Georgian Language Meta-Package for LibreOffice</uk>
   <vi>Georgian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Georgian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Georgian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ka
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ka
</uninstall_package_names>
</app>

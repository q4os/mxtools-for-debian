<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Tswana_Libreoffice
</name>

<description>
   <am>Tswana Language Meta-Package for LibreOffice</am>
   <ar>Tswana Language Meta-Package for LibreOffice</ar>
   <bg>Tswana Language Meta-Package for LibreOffice</bg>
   <bn>Tswana Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Tswana per LibreOffice</ca>
   <cs>Tswana Language Meta-Package for LibreOffice</cs>
   <da>Tswana Language Meta-Package for LibreOffice</da>
   <de>Tswana Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Tswana</el>
   <en>Tswana Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Tswana para LibreOffice</es>
   <et>Tswana Language Meta-Package for LibreOffice</et>
   <eu>Tswana Language Meta-Package for LibreOffice</eu>
   <fa>Tswana Language Meta-Package for LibreOffice</fa>
   <fil_PH>Tswana Language Meta-Package for LibreOffice</fil_PH>
   <fi>Tswana Language Meta-Package for LibreOffice</fi>
   <fr>Tswana Language Meta-Package for LibreOffice</fr>
   <he_IL>Tswana Language Meta-Package for LibreOffice</he_IL>
   <hi>Tswana Language Meta-Package for LibreOffice</hi>
   <hr>Tswana Language Meta-Package for LibreOffice</hr>
   <hu>Tswana Language Meta-Package for LibreOffice</hu>
   <id>Tswana Language Meta-Package for LibreOffice</id>
   <is>Tswana Language Meta-Package for LibreOffice</is>
   <it>Tswana Language Meta-Package for LibreOffice</it>
   <ja_JP>Tswana Language Meta-Package for LibreOffice</ja_JP>
   <ja>Tswana Language Meta-Package for LibreOffice</ja>
   <kk>Tswana Language Meta-Package for LibreOffice</kk>
   <ko>Tswana Language Meta-Package for LibreOffice</ko>
   <lt>Tswana Language Meta-Package for LibreOffice</lt>
   <mk>Tswana Language Meta-Package for LibreOffice</mk>
   <mr>Tswana Language Meta-Package for LibreOffice</mr>
   <nb>Tswana Language Meta-Package for LibreOffice</nb>
   <nl>Tswana Language Meta-Package for LibreOffice</nl>
   <pl>Tswana Language Meta-Package for LibreOffice</pl>
   <pt_BR>Tswana Language Meta-Package for LibreOffice</pt_BR>
   <pt>Tswana Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Tswana Language Meta-Package for LibreOffice</ro>
   <ru>Tswana Language Meta-Package for LibreOffice</ru>
   <sk>Tswana Language Meta-Package for LibreOffice</sk>
   <sl>Tswanski jezikovni meta-paket za LibreOffice</sl>
   <sq>Tswana Language Meta-Package for LibreOffice</sq>
   <sr>Tswana Language Meta-Package for LibreOffice</sr>
   <sv>Tswana Språk Meta-Paket för LibreOffice</sv>
   <tr>Tswana Language Meta-Package for LibreOffice</tr>
   <uk>Tswana Language Meta-Package for LibreOffice</uk>
   <vi>Tswana Language Meta-Package for LibreOffice</vi>
   <zh_CN>Tswana Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Tswana Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-tn
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-tn
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Vietnamese_Libreoffice
</name>

<description>
   <am>Vietnamese Language Meta-Package for LibreOffice</am>
   <ar>Vietnamese Language Meta-Package for LibreOffice</ar>
   <bg>Vietnamese Language Meta-Package for LibreOffice</bg>
   <bn>Vietnamese Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Vietnamita per LibreOffice</ca>
   <cs>Vietnamese Language Meta-Package for LibreOffice</cs>
   <da>Vietnamese Language Meta-Package for LibreOffice</da>
   <de>Vietnamesisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα βιετναμέζικα</el>
   <en>Vietnamese Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma vietnamita para LibreOffice</es>
   <et>Vietnamese Language Meta-Package for LibreOffice</et>
   <eu>Vietnamese Language Meta-Package for LibreOffice</eu>
   <fa>Vietnamese Language Meta-Package for LibreOffice</fa>
   <fil_PH>Vietnamese Language Meta-Package for LibreOffice</fil_PH>
   <fi>Vietnamese Language Meta-Package for LibreOffice</fi>
   <fr>Vietnamese Language Meta-Package for LibreOffice</fr>
   <he_IL>Vietnamese Language Meta-Package for LibreOffice</he_IL>
   <hi>Vietnamese Language Meta-Package for LibreOffice</hi>
   <hr>Vietnamese Language Meta-Package for LibreOffice</hr>
   <hu>Vietnamese Language Meta-Package for LibreOffice</hu>
   <id>Vietnamese Language Meta-Package for LibreOffice</id>
   <is>Vietnamese Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua vietnamita per LibreOffice</it>
   <ja_JP>Vietnamese Language Meta-Package for LibreOffice</ja_JP>
   <ja>Vietnamese Language Meta-Package for LibreOffice</ja>
   <kk>Vietnamese Language Meta-Package for LibreOffice</kk>
   <ko>Vietnamese Language Meta-Package for LibreOffice</ko>
   <lt>Vietnamese Language Meta-Package for LibreOffice</lt>
   <mk>Vietnamese Language Meta-Package for LibreOffice</mk>
   <mr>Vietnamese Language Meta-Package for LibreOffice</mr>
   <nb>Vietnamese Language Meta-Package for LibreOffice</nb>
   <nl>Vietnamese Language Meta-Package for LibreOffice</nl>
   <pl>Vietnamese Language Meta-Package for LibreOffice</pl>
   <pt_BR>Vietnamese Language Meta-Package for LibreOffice</pt_BR>
   <pt>Vietnamita Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Vietnamese Language Meta-Package for LibreOffice</ro>
   <ru>Vietnamese Language Meta-Package for LibreOffice</ru>
   <sk>Vietnamese Language Meta-Package for LibreOffice</sk>
   <sl>Vietnamski jezikovni meta-paket za LibreOffice</sl>
   <sq>Vietnamese Language Meta-Package for LibreOffice</sq>
   <sr>Vietnamese Language Meta-Package for LibreOffice</sr>
   <sv>Vietnamesiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Vietnamese Language Meta-Package for LibreOffice</tr>
   <uk>Vietnamese Language Meta-Package for LibreOffice</uk>
   <vi>Vietnamese Language Meta-Package for LibreOffice</vi>
   <zh_CN>Vietnamese Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Vietnamese Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-vi
libreoffice-help-vi
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-vi
libreoffice-help-vi
</uninstall_package_names>
</app>

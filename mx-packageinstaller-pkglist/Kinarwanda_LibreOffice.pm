<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Kinarwanda_Libreoffice
</name>

<description>
   <am>Kinarwanda Language Meta-Package for LibreOffice</am>
   <ar>Kinarwanda Language Meta-Package for LibreOffice</ar>
   <bg>Kinarwanda Language Meta-Package for LibreOffice</bg>
   <bn>Kinarwanda Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Kinarwanda per LibreOffice</ca>
   <cs>Kinarwanda Language Meta-Package for LibreOffice</cs>
   <da>Kinarwanda Language Meta-Package for LibreOffice</da>
   <de>Kinarwanda Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Kinarwanda</el>
   <en>Kinarwanda Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma kinarwanda para LibreOffice</es>
   <et>Kinarwanda Language Meta-Package for LibreOffice</et>
   <eu>Kinarwanda Language Meta-Package for LibreOffice</eu>
   <fa>Kinarwanda Language Meta-Package for LibreOffice</fa>
   <fil_PH>Kinarwanda Language Meta-Package for LibreOffice</fil_PH>
   <fi>Kinarwanda Language Meta-Package for LibreOffice</fi>
   <fr>Kinarwanda Language Meta-Package for LibreOffice</fr>
   <he_IL>Kinarwanda Language Meta-Package for LibreOffice</he_IL>
   <hi>Kinarwanda Language Meta-Package for LibreOffice</hi>
   <hr>Kinarwanda Language Meta-Package for LibreOffice</hr>
   <hu>Kinarwanda Language Meta-Package for LibreOffice</hu>
   <id>Kinarwanda Language Meta-Package for LibreOffice</id>
   <is>Kinarwanda Language Meta-Package for LibreOffice</is>
   <it>Kinarwanda Language Meta-Package for LibreOffice</it>
   <ja_JP>Kinarwanda Language Meta-Package for LibreOffice</ja_JP>
   <ja>Kinarwanda Language Meta-Package for LibreOffice</ja>
   <kk>Kinarwanda Language Meta-Package for LibreOffice</kk>
   <ko>Kinarwanda Language Meta-Package for LibreOffice</ko>
   <lt>Kinarwanda Language Meta-Package for LibreOffice</lt>
   <mk>Kinarwanda Language Meta-Package for LibreOffice</mk>
   <mr>Kinarwanda Language Meta-Package for LibreOffice</mr>
   <nb>Kinarwanda Language Meta-Package for LibreOffice</nb>
   <nl>Kinarwanda Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Kinarwanda Language Meta-Package for LibreOffice</pl>
   <pt_BR>Kinarwanda Language Meta-Package for LibreOffice</pt_BR>
   <pt>Kinarwanda Language Meta-Package for LibreOffice</pt>
   <ro>Kinarwanda Language Meta-Package for LibreOffice</ro>
   <ru>Kinarwanda Language Meta-Package for LibreOffice</ru>
   <sk>Kinarwanda Language Meta-Package for LibreOffice</sk>
   <sl>Kinarvandski jezikovni meta-paket za LibreOffice</sl>
   <sq>Kinarwanda Language Meta-Package for LibreOffice</sq>
   <sr>Kinarwanda Language Meta-Package for LibreOffice</sr>
   <sv>Kinarwanda Språk Meta-Paket för LibreOffice</sv>
   <tr>Kinarwanda Language Meta-Package for LibreOffice</tr>
   <uk>Kinarwanda Language Meta-Package for LibreOffice</uk>
   <vi>Kinarwanda Language Meta-Package for LibreOffice</vi>
   <zh_CN>Kinarwanda Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Kinarwanda Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-rw
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-rw
</uninstall_package_names>
</app>

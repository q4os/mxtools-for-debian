<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Farsi_Libreoffice
</name>

<description>
   <am>Farsi Language Meta-Package for LibreOffice</am>
   <ar>Farsi Language Meta-Package for LibreOffice</ar>
   <bg>Farsi Language Meta-Package for LibreOffice</bg>
   <bn>Farsi Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Farsi per LibreOffice</ca>
   <cs>Farsi Language Meta-Package for LibreOffice</cs>
   <da>Farsi Language Meta-Package for LibreOffice</da>
   <de>Farsi Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Φάρσι</el>
   <en>Farsi Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma farsi para LibreOffice</es>
   <et>Farsi Language Meta-Package for LibreOffice</et>
   <eu>Farsi Language Meta-Package for LibreOffice</eu>
   <fa>Farsi Language Meta-Package for LibreOffice</fa>
   <fil_PH>Farsi Language Meta-Package for LibreOffice</fil_PH>
   <fi>Farsi Language Meta-Package for LibreOffice</fi>
   <fr>Farsi Language Meta-Package for LibreOffice</fr>
   <he_IL>Farsi Language Meta-Package for LibreOffice</he_IL>
   <hi>Farsi Language Meta-Package for LibreOffice</hi>
   <hr>Farsi Language Meta-Package for LibreOffice</hr>
   <hu>Farsi Language Meta-Package for LibreOffice</hu>
   <id>Farsi Language Meta-Package for LibreOffice</id>
   <is>Farsi Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua persiana per LibreOffice</it>
   <ja_JP>Farsi Language Meta-Package for LibreOffice</ja_JP>
   <ja>Farsi Language Meta-Package for LibreOffice</ja>
   <kk>Farsi Language Meta-Package for LibreOffice</kk>
   <ko>Farsi Language Meta-Package for LibreOffice</ko>
   <lt>Farsi Language Meta-Package for LibreOffice</lt>
   <mk>Farsi Language Meta-Package for LibreOffice</mk>
   <mr>Farsi Language Meta-Package for LibreOffice</mr>
   <nb>Farsi Language Meta-Package for LibreOffice</nb>
   <nl>Farsi Language Meta-Package for LibreOffice</nl>
   <pl>Farsi Language Meta-Package for LibreOffice</pl>
   <pt_BR>Farsi Language Meta-Package for LibreOffice</pt_BR>
   <pt>Persa Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Farsi Language Meta-Package for LibreOffice</ro>
   <ru>Farsi Language Meta-Package for LibreOffice</ru>
   <sk>Farsi Language Meta-Package for LibreOffice</sk>
   <sl>Farsi jezikovni meta-paket za LibreOffice</sl>
   <sq>Farsi Language Meta-Package for LibreOffice</sq>
   <sr>Farsi Language Meta-Package for LibreOffice</sr>
   <sv>Farsi Språk Meta-Paket för LibreOffice</sv>
   <tr>Farsi Language Meta-Package for LibreOffice</tr>
   <uk>Farsi Language Meta-Package for LibreOffice</uk>
   <vi>Farsi Language Meta-Package for LibreOffice</vi>
   <zh_CN>Farsi Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Farsi Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-fa
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-fa
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Danish_Libreoffice
</name>

<description>
   <am>Danish Language Meta-Package for LibreOffice</am>
   <ar>Danish Language Meta-Package for LibreOffice</ar>
   <bg>Danish Language Meta-Package for LibreOffice</bg>
   <bn>Danish Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Danès per LibreOffice</ca>
   <cs>Danish Language Meta-Package for LibreOffice</cs>
   <da>Danish Language Meta-Package for LibreOffice</da>
   <de>Dänisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα δανικά</el>
   <en>Danish Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma danés para LibreOffice</es>
   <et>Danish Language Meta-Package for LibreOffice</et>
   <eu>Danish Language Meta-Package for LibreOffice</eu>
   <fa>Danish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Danish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Danish Language Meta-Package for LibreOffice</fi>
   <fr>Danish Language Meta-Package for LibreOffice</fr>
   <he_IL>Danish Language Meta-Package for LibreOffice</he_IL>
   <hi>Danish Language Meta-Package for LibreOffice</hi>
   <hr>Danish Language Meta-Package for LibreOffice</hr>
   <hu>Danish Language Meta-Package for LibreOffice</hu>
   <id>Danish Language Meta-Package for LibreOffice</id>
   <is>Danish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua danese per LibreOffice</it>
   <ja_JP>Danish Language Meta-Package for LibreOffice</ja_JP>
   <ja>Danish Language Meta-Package for LibreOffice</ja>
   <kk>Danish Language Meta-Package for LibreOffice</kk>
   <ko>Danish Language Meta-Package for LibreOffice</ko>
   <lt>Danish Language Meta-Package for LibreOffice</lt>
   <mk>Danish Language Meta-Package for LibreOffice</mk>
   <mr>Danish Language Meta-Package for LibreOffice</mr>
   <nb>Danish Language Meta-Package for LibreOffice</nb>
   <nl>Danish Language Meta-Package for LibreOffice</nl>
   <pl>Danish Language Meta-Package for LibreOffice</pl>
   <pt_BR>Danish Language Meta-Package for LibreOffice</pt_BR>
   <pt>Dinamarquês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Danish Language Meta-Package for LibreOffice</ro>
   <ru>Danish Language Meta-Package for LibreOffice</ru>
   <sk>Danish Language Meta-Package for LibreOffice</sk>
   <sl>Danski jezikovni meta-paket za LibreOffice</sl>
   <sq>Danish Language Meta-Package for LibreOffice</sq>
   <sr>Danish Language Meta-Package for LibreOffice</sr>
   <sv>Danskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Danish Language Meta-Package for LibreOffice</tr>
   <uk>Danish Language Meta-Package for LibreOffice</uk>
   <vi>Danish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Danish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Danish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-da
libreoffice-help-da
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-da
libreoffice-help-da
</uninstall_package_names>
</app>

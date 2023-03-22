<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Bulgarian_Libreoffice
</name>

<description>
   <am>Bulgarian Language Meta-Package for LibreOffice</am>
   <ar>Bulgarian Language Meta-Package for LibreOffice</ar>
   <bg>Bulgarian Language Meta-Package for LibreOffice</bg>
   <bn>Bulgarian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Búlgar per LibreOffice</ca>
   <cs>Bulgarian Language Meta-Package for LibreOffice</cs>
   <da>Bulgarian Language Meta-Package for LibreOffice</da>
   <de>Bulgarisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα βουλγαρικά</el>
   <en>Bulgarian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma búlgaro para LibreOffice</es>
   <et>Bulgarian Language Meta-Package for LibreOffice</et>
   <eu>Bulgarian Language Meta-Package for LibreOffice</eu>
   <fa>Bulgarian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Bulgarian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Bulgarian Language Meta-Package for LibreOffice</fi>
   <fr>Bulgarian Language Meta-Package for LibreOffice</fr>
   <he_IL>Bulgarian Language Meta-Package for LibreOffice</he_IL>
   <hi>Bulgarian Language Meta-Package for LibreOffice</hi>
   <hr>Bulgarian Language Meta-Package for LibreOffice</hr>
   <hu>Bulgarian Language Meta-Package for LibreOffice</hu>
   <id>Bulgarian Language Meta-Package for LibreOffice</id>
   <is>Bulgarian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua bulgara per LibreOffice</it>
   <ja_JP>Bulgarian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Bulgarian Language Meta-Package for LibreOffice</ja>
   <kk>Bulgarian Language Meta-Package for LibreOffice</kk>
   <ko>Bulgarian Language Meta-Package for LibreOffice</ko>
   <lt>Bulgarian Language Meta-Package for LibreOffice</lt>
   <mk>Bulgarian Language Meta-Package for LibreOffice</mk>
   <mr>Bulgarian Language Meta-Package for LibreOffice</mr>
   <nb>Bulgarian Language Meta-Package for LibreOffice</nb>
   <nl>Bulgarian Language Meta-Package for LibreOffice</nl>
   <pl>Bulgarian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Bulgarian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Búlgaro Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Bulgarian Language Meta-Package for LibreOffice</ro>
   <ru>Bulgarian Language Meta-Package for LibreOffice</ru>
   <sk>Bulgarian Language Meta-Package for LibreOffice</sk>
   <sl>Bolgarski jezikovni meta-paket za LibreOffice</sl>
   <sq>Bulgarian Language Meta-Package for LibreOffice</sq>
   <sr>Bulgarian Language Meta-Package for LibreOffice</sr>
   <sv>Bulgariskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Bulgarian Language Meta-Package for LibreOffice</tr>
   <uk>Bulgarian Language Meta-Package for LibreOffice</uk>
   <vi>Bulgarian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Bulgarian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Bulgarian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-bg
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-bg
</uninstall_package_names>
</app>

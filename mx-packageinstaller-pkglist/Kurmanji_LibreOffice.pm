<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Kurmanji_Libreoffice
</name>

<description>
   <am>Kurmanji Language Meta-Package for LibreOffice</am>
   <ar>Kurmanji Language Meta-Package for LibreOffice</ar>
   <bg>Kurmanji Language Meta-Package for LibreOffice</bg>
   <bn>Kurmanji Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Kurmanji per LibreOffice</ca>
   <cs>Kurmanji Language Meta-Package for LibreOffice</cs>
   <da>Kurmanji Language Meta-Package for LibreOffice</da>
   <de>Kurmanji Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Kurmanji</el>
   <en>Kurmanji Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma kurmanji para LibreOffice</es>
   <et>Kurmanji Language Meta-Package for LibreOffice</et>
   <eu>Kurmanji Language Meta-Package for LibreOffice</eu>
   <fa>Kurmanji Language Meta-Package for LibreOffice</fa>
   <fil_PH>Kurmanji Language Meta-Package for LibreOffice</fil_PH>
   <fi>Kurmanji Language Meta-Package for LibreOffice</fi>
   <fr>Kurmanji Language Meta-Package for LibreOffice</fr>
   <he_IL>Kurmanji Language Meta-Package for LibreOffice</he_IL>
   <hi>Kurmanji Language Meta-Package for LibreOffice</hi>
   <hr>Kurmanji Language Meta-Package for LibreOffice</hr>
   <hu>Kurmanji Language Meta-Package for LibreOffice</hu>
   <id>Kurmanji Language Meta-Package for LibreOffice</id>
   <is>Kurmanji Language Meta-Package for LibreOffice</is>
   <it>Kurmanji Language Meta-Package for LibreOffice</it>
   <ja_JP>Kurmanji Language Meta-Package for LibreOffice</ja_JP>
   <ja>Kurmanji Language Meta-Package for LibreOffice</ja>
   <kk>Kurmanji Language Meta-Package for LibreOffice</kk>
   <ko>Kurmanji Language Meta-Package for LibreOffice</ko>
   <lt>Kurmanji Language Meta-Package for LibreOffice</lt>
   <mk>Kurmanji Language Meta-Package for LibreOffice</mk>
   <mr>Kurmanji Language Meta-Package for LibreOffice</mr>
   <nb>Kurmanji Language Meta-Package for LibreOffice</nb>
   <nl>Kurmanji Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Kurmanji Language Meta-Package for LibreOffice</pl>
   <pt_BR>Kurmanji Language Meta-Package for LibreOffice</pt_BR>
   <pt>Curmanji Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Kurmanji Language Meta-Package for LibreOffice</ro>
   <ru>Kurmanji Language Meta-Package for LibreOffice</ru>
   <sk>Kurmanji Language Meta-Package for LibreOffice</sk>
   <sl>Kurmanjski jezikovni meta-paket za LibreOffice</sl>
   <sq>Kurmanji Language Meta-Package for LibreOffice</sq>
   <sr>Kurmanji Language Meta-Package for LibreOffice</sr>
   <sv>Kurmanji Språk Meta-Paket för LibreOffice</sv>
   <tr>Kurmanji Language Meta-Package for LibreOffice</tr>
   <uk>Kurmanji Language Meta-Package for LibreOffice</uk>
   <vi>Kurmanji Language Meta-Package for LibreOffice</vi>
   <zh_CN>Kurmanji Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Kurmanji Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-kmr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-kmr
</uninstall_package_names>
</app>

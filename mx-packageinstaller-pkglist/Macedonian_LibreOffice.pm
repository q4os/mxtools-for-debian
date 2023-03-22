<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Macedonian_Libreoffice
</name>

<description>
   <am>Macedonian Language Meta-Package for LibreOffice</am>
   <ar>Macedonian Language Meta-Package for LibreOffice</ar>
   <bg>Macedonian Language Meta-Package for LibreOffice</bg>
   <bn>Macedonian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Macedoni per LibreOffice</ca>
   <cs>Macedonian Language Meta-Package for LibreOffice</cs>
   <da>Macedonian Language Meta-Package for LibreOffice</da>
   <de>Mazedonisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα μακεδονικά</el>
   <en>Macedonian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma macedonio para LibreOffice</es>
   <et>Macedonian Language Meta-Package for LibreOffice</et>
   <eu>Macedonian Language Meta-Package for LibreOffice</eu>
   <fa>Macedonian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Macedonian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Macedonian Language Meta-Package for LibreOffice</fi>
   <fr>Macedonian Language Meta-Package for LibreOffice</fr>
   <he_IL>Macedonian Language Meta-Package for LibreOffice</he_IL>
   <hi>Macedonian Language Meta-Package for LibreOffice</hi>
   <hr>Macedonian Language Meta-Package for LibreOffice</hr>
   <hu>Macedonian Language Meta-Package for LibreOffice</hu>
   <id>Macedonian Language Meta-Package for LibreOffice</id>
   <is>Macedonian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua macedone per LibreOffice</it>
   <ja_JP>Macedonian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Macedonian Language Meta-Package for LibreOffice</ja>
   <kk>Macedonian Language Meta-Package for LibreOffice</kk>
   <ko>Macedonian Language Meta-Package for LibreOffice</ko>
   <lt>Macedonian Language Meta-Package for LibreOffice</lt>
   <mk>Macedonian Language Meta-Package for LibreOffice</mk>
   <mr>Macedonian Language Meta-Package for LibreOffice</mr>
   <nb>Macedonian Language Meta-Package for LibreOffice</nb>
   <nl>Macedonisch Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Macedonian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Macedonian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Macedónio Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Macedonian Language Meta-Package for LibreOffice</ro>
   <ru>Macedonian Language Meta-Package for LibreOffice</ru>
   <sk>Macedonian Language Meta-Package for LibreOffice</sk>
   <sl>Makedonski jezikovni meta-paket za LibreOffice</sl>
   <sq>Macedonian Language Meta-Package for LibreOffice</sq>
   <sr>Macedonian Language Meta-Package for LibreOffice</sr>
   <sv>Macedoniskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Macedonian Language Meta-Package for LibreOffice</tr>
   <uk>Macedonian Language Meta-Package for LibreOffice</uk>
   <vi>Macedonian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Macedonian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Macedonian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-mk
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-mk
</uninstall_package_names>
</app>

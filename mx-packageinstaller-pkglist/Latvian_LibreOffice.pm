<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Latvian_Libreoffice
</name>

<description>
   <am>Latvian Language Meta-Package for LibreOffice</am>
   <ar>Latvian Language Meta-Package for LibreOffice</ar>
   <bg>Latvian Language Meta-Package for LibreOffice</bg>
   <bn>Latvian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Letó per LibreOffice</ca>
   <cs>Latvian Language Meta-Package for LibreOffice</cs>
   <da>Latvian Language Meta-Package for LibreOffice</da>
   <de>Lettisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Latvian</el>
   <en>Latvian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma letón para LibreOffice</es>
   <et>Latvian Language Meta-Package for LibreOffice</et>
   <eu>Latvian Language Meta-Package for LibreOffice</eu>
   <fa>Latvian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Latvian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Latvian Language Meta-Package for LibreOffice</fi>
   <fr>Latvian Language Meta-Package for LibreOffice</fr>
   <he_IL>Latvian Language Meta-Package for LibreOffice</he_IL>
   <hi>Latvian Language Meta-Package for LibreOffice</hi>
   <hr>Latvian Language Meta-Package for LibreOffice</hr>
   <hu>Latvian Language Meta-Package for LibreOffice</hu>
   <id>Latvian Language Meta-Package for LibreOffice</id>
   <is>Latvian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua lettone per LibreOffice</it>
   <ja_JP>Latvian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Latvian Language Meta-Package for LibreOffice</ja>
   <kk>Latvian Language Meta-Package for LibreOffice</kk>
   <ko>Latvian Language Meta-Package for LibreOffice</ko>
   <lt>Latvian Language Meta-Package for LibreOffice</lt>
   <mk>Latvian Language Meta-Package for LibreOffice</mk>
   <mr>Latvian Language Meta-Package for LibreOffice</mr>
   <nb>Latvian Language Meta-Package for LibreOffice</nb>
   <nl>Lets Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Latvian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Latvian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Letão Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Latvian Language Meta-Package for LibreOffice</ro>
   <ru>Latvian Language Meta-Package for LibreOffice</ru>
   <sk>Latvian Language Meta-Package for LibreOffice</sk>
   <sl>Latvijski jezikovni meta-paket za LibreOffice</sl>
   <sq>Latvian Language Meta-Package for LibreOffice</sq>
   <sr>Latvian Language Meta-Package for LibreOffice</sr>
   <sv>Lettländskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Latvian Language Meta-Package for LibreOffice</tr>
   <uk>Latvian Language Meta-Package for LibreOffice</uk>
   <vi>Latvian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Latvian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Latvian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-lv
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-lv
</uninstall_package_names>
</app>

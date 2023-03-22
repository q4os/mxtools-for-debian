<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Arabic_Libreoffice
</name>

<description>
   <am>Arabic Language Meta-Package for LibreOffice</am>
   <ar>Arabic Language Meta-Package for LibreOffice</ar>
   <bg>Arabic Language Meta-Package for LibreOffice</bg>
   <bn>Arabic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Àrab per LibreOffice</ca>
   <cs>Arabic Language Meta-Package for LibreOffice</cs>
   <da>Arabic Language Meta-Package for LibreOffice</da>
   <de>Arabisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα αραβικα</el>
   <en>Arabic Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma árabe para LibreOffice</es>
   <et>Arabic Language Meta-Package for LibreOffice</et>
   <eu>Arabic Language Meta-Package for LibreOffice</eu>
   <fa>Arabic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Arabic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Arabic Language Meta-Package for LibreOffice</fi>
   <fr>Arabic Language Meta-Package for LibreOffice</fr>
   <he_IL>Arabic Language Meta-Package for LibreOffice</he_IL>
   <hi>Arabic Language Meta-Package for LibreOffice</hi>
   <hr>Arabic Language Meta-Package for LibreOffice</hr>
   <hu>Arabic Language Meta-Package for LibreOffice</hu>
   <id>Arabic Language Meta-Package for LibreOffice</id>
   <is>Arabic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua araba per LibreOffice</it>
   <ja_JP>Arabic Language Meta-Package for LibreOffice</ja_JP>
   <ja>Arabic Language Meta-Package for LibreOffice</ja>
   <kk>Arabic Language Meta-Package for LibreOffice</kk>
   <ko>Arabic Language Meta-Package for LibreOffice</ko>
   <lt>Arabic Language Meta-Package for LibreOffice</lt>
   <mk>Arabic Language Meta-Package for LibreOffice</mk>
   <mr>Arabic Language Meta-Package for LibreOffice</mr>
   <nb>Arabic Language Meta-Package for LibreOffice</nb>
   <nl>Arabische Taal Meta-Pakket voor LibreOffice​</nl>
   <pl>Arabic Language Meta-Package for LibreOffice</pl>
   <pt_BR>Arabic Language Meta-Package for LibreOffice</pt_BR>
   <pt>Árabe Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Arabic Language Meta-Package for LibreOffice</ro>
   <ru>Arabic Language Meta-Package for LibreOffice</ru>
   <sk>Arabic Language Meta-Package for LibreOffice</sk>
   <sl>Arabski jezikovni meta-paket za LibreOffice</sl>
   <sq>Arabic Language Meta-Package for LibreOffice</sq>
   <sr>Arabic Language Meta-Package for LibreOffice</sr>
   <sv>Arabiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Arabic Language Meta-Package for LibreOffice</tr>
   <uk>Arabic Language Meta-Package for LibreOffice</uk>
   <vi>Arabic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Arabic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Arabic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ar
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ar
</uninstall_package_names>
</app>

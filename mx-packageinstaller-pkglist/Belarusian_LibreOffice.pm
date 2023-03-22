<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Belarusian_Libreoffice
</name>

<description>
   <am>Belarusian Language Meta-Package for LibreOffice</am>
   <ar>Belarusian Language Meta-Package for LibreOffice</ar>
   <bg>Belarusian Language Meta-Package for LibreOffice</bg>
   <bn>Belarusian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Bielorús per LibreOffice</ca>
   <cs>Belarusian Language Meta-Package for LibreOffice</cs>
   <da>Belarusian Language Meta-Package for LibreOffice</da>
   <de>Weißrussisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Belarusian</el>
   <en>Belarusian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma bielorruso para LibreOffice</es>
   <et>Belarusian Language Meta-Package for LibreOffice</et>
   <eu>Belarusian Language Meta-Package for LibreOffice</eu>
   <fa>Belarusian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Belarusian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Belarusian Language Meta-Package for LibreOffice</fi>
   <fr>Belarusian Language Meta-Package for LibreOffice</fr>
   <he_IL>Belarusian Language Meta-Package for LibreOffice</he_IL>
   <hi>Belarusian Language Meta-Package for LibreOffice</hi>
   <hr>Belarusian Language Meta-Package for LibreOffice</hr>
   <hu>Belarusian Language Meta-Package for LibreOffice</hu>
   <id>Belarusian Language Meta-Package for LibreOffice</id>
   <is>Belarusian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua bielorussa per LibreOffice</it>
   <ja_JP>Belarusian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Belarusian Language Meta-Package for LibreOffice</ja>
   <kk>Belarusian Language Meta-Package for LibreOffice</kk>
   <ko>Belarusian Language Meta-Package for LibreOffice</ko>
   <lt>Belarusian Language Meta-Package for LibreOffice</lt>
   <mk>Belarusian Language Meta-Package for LibreOffice</mk>
   <mr>Belarusian Language Meta-Package for LibreOffice</mr>
   <nb>Belarusian Language Meta-Package for LibreOffice</nb>
   <nl>Russisch Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Belarusian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Belarusian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Bielorrusso Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Belarusian Language Meta-Package for LibreOffice</ro>
   <ru>Belarusian Language Meta-Package for LibreOffice</ru>
   <sk>Belarusian Language Meta-Package for LibreOffice</sk>
   <sl>Beloruski jezikovni meta-paket za LibreOffice</sl>
   <sq>Belarusian Language Meta-Package for LibreOffice</sq>
   <sr>Belarusian Language Meta-Package for LibreOffice</sr>
   <sv>Belarusiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Belarusian Language Meta-Package for LibreOffice</tr>
   <uk>Belarusian Language Meta-Package for LibreOffice</uk>
   <vi>Belarusian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Belarusian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Belarusian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-be
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-be
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
South African_Libreoffice
</name>

<description>
   <am>South African Language Meta-Package for LibreOffice</am>
   <ar>South African Language Meta-Package for LibreOffice</ar>
   <bg>South African Language Meta-Package for LibreOffice</bg>
   <bn>South African Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Sudafricà per LibreOffice</ca>
   <cs>South African Language Meta-Package for LibreOffice</cs>
   <da>South African Language Meta-Package for LibreOffice</da>
   <de>Südafrikanisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Νοτιοαφρικανικά</el>
   <en>South African Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma sudafricano para LibreOffice</es>
   <et>South African Language Meta-Package for LibreOffice</et>
   <eu>South African Language Meta-Package for LibreOffice</eu>
   <fa>South African Language Meta-Package for LibreOffice</fa>
   <fil_PH>South African Language Meta-Package for LibreOffice</fil_PH>
   <fi>South African Language Meta-Package for LibreOffice</fi>
   <fr>South African Language Meta-Package for LibreOffice</fr>
   <he_IL>South African Language Meta-Package for LibreOffice</he_IL>
   <hi>South African Language Meta-Package for LibreOffice</hi>
   <hr>South African Language Meta-Package for LibreOffice</hr>
   <hu>South African Language Meta-Package for LibreOffice</hu>
   <id>South African Language Meta-Package for LibreOffice</id>
   <is>South African Language Meta-Package for LibreOffice</is>
   <it>South African Language Meta-Package for LibreOffice</it>
   <ja_JP>South African Language Meta-Package for LibreOffice</ja_JP>
   <ja>South African Language Meta-Package for LibreOffice</ja>
   <kk>South African Language Meta-Package for LibreOffice</kk>
   <ko>South African Language Meta-Package for LibreOffice</ko>
   <lt>South African Language Meta-Package for LibreOffice</lt>
   <mk>South African Language Meta-Package for LibreOffice</mk>
   <mr>South African Language Meta-Package for LibreOffice</mr>
   <nb>South African Language Meta-Package for LibreOffice</nb>
   <nl>South African Language Meta-Package for LibreOffice</nl>
   <pl>South African Language Meta-Package for LibreOffice</pl>
   <pt_BR>South African Language Meta-Package for LibreOffice</pt_BR>
   <pt>Sul Africano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>South African Language Meta-Package for LibreOffice</ro>
   <ru>South African Language Meta-Package for LibreOffice</ru>
   <sk>South African Language Meta-Package for LibreOffice</sk>
   <sl>Južnoafriški jezikovni meta-paket za LibreOffice</sl>
   <sq>South African Language Meta-Package for LibreOffice</sq>
   <sr>South African Language Meta-Package for LibreOffice</sr>
   <sv>Sydafrikanskt Språk Meta-Paket för LibreOffice</sv>
   <tr>South African Language Meta-Package for LibreOffice</tr>
   <uk>South African Language Meta-Package for LibreOffice</uk>
   <vi>South African Language Meta-Package for LibreOffice</vi>
   <zh_CN>South African Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>South African Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-za
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-za
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Ndebele_Libreoffice
</name>

<description>
   <am>Ndebele Language Meta-Package for LibreOffice</am>
   <ar>Ndebele Language Meta-Package for LibreOffice</ar>
   <bg>Ndebele Language Meta-Package for LibreOffice</bg>
   <bn>Ndebele Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Ndebele per LibreOffice</ca>
   <cs>Ndebele Language Meta-Package for LibreOffice</cs>
   <da>Ndebele Language Meta-Package for LibreOffice</da>
   <de>Ndebele Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Ndebele</el>
   <en>Ndebele Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Ndebele para LibreOffice</es>
   <et>Ndebele Language Meta-Package for LibreOffice</et>
   <eu>Ndebele Language Meta-Package for LibreOffice</eu>
   <fa>Ndebele Language Meta-Package for LibreOffice</fa>
   <fil_PH>Ndebele Language Meta-Package for LibreOffice</fil_PH>
   <fi>Ndebele Language Meta-Package for LibreOffice</fi>
   <fr>Ndebele Language Meta-Package for LibreOffice</fr>
   <he_IL>Ndebele Language Meta-Package for LibreOffice</he_IL>
   <hi>Ndebele Language Meta-Package for LibreOffice</hi>
   <hr>Ndebele Language Meta-Package for LibreOffice</hr>
   <hu>Ndebele Language Meta-Package for LibreOffice</hu>
   <id>Ndebele Language Meta-Package for LibreOffice</id>
   <is>Ndebele Language Meta-Package for LibreOffice</is>
   <it>Ndebele Language Meta-Package for LibreOffice</it>
   <ja_JP>Ndebele Language Meta-Package for LibreOffice</ja_JP>
   <ja>Ndebele Language Meta-Package for LibreOffice</ja>
   <kk>Ndebele Language Meta-Package for LibreOffice</kk>
   <ko>Ndebele Language Meta-Package for LibreOffice</ko>
   <lt>Ndebele Language Meta-Package for LibreOffice</lt>
   <mk>Ndebele Language Meta-Package for LibreOffice</mk>
   <mr>Ndebele Language Meta-Package for LibreOffice</mr>
   <nb>Ndebele Language Meta-Package for LibreOffice</nb>
   <nl>Ndebele Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Ndebele Language Meta-Package for LibreOffice</pl>
   <pt_BR>Ndebele Language Meta-Package for LibreOffice</pt_BR>
   <pt>Ndebele Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Ndebele Language Meta-Package for LibreOffice</ro>
   <ru>Ndebele Language Meta-Package for LibreOffice</ru>
   <sk>Ndebele Language Meta-Package for LibreOffice</sk>
   <sl>Ndebelski jezikovni meta-paket za LibreOffice</sl>
   <sq>Ndebele Language Meta-Package for LibreOffice</sq>
   <sr>Ndebele Language Meta-Package for LibreOffice</sr>
   <sv>Ndebele Språk Meta-Paket för LibreOffice</sv>
   <tr>Ndebele Language Meta-Package for LibreOffice</tr>
   <uk>Ndebele Language Meta-Package for LibreOffice</uk>
   <vi>Ndebele Language Meta-Package for LibreOffice</vi>
   <zh_CN>Ndebele Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Ndebele Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-nr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-nr
</uninstall_package_names>
</app>

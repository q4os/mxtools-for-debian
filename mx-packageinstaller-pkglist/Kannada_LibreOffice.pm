<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Kannada_Libreoffice
</name>

<description>
   <am>Kannada Language Meta-Package for LibreOffice</am>
   <ar>Kannada Language Meta-Package for LibreOffice</ar>
   <bg>Kannada Language Meta-Package for LibreOffice</bg>
   <bn>Kannada Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Kannada per LibreOffice</ca>
   <cs>Kannada Language Meta-Package for LibreOffice</cs>
   <da>Kannada Language Meta-Package for LibreOffice</da>
   <de>Kannada Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Kannada</el>
   <en>Kannada Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Kannada Language para LibreOffice</es>
   <et>Kannada Language Meta-Package for LibreOffice</et>
   <eu>Kannada Language Meta-Package for LibreOffice</eu>
   <fa>Kannada Language Meta-Package for LibreOffice</fa>
   <fil_PH>Kannada Language Meta-Package for LibreOffice</fil_PH>
   <fi>Kannada Language Meta-Package for LibreOffice</fi>
   <fr>Kannada Language Meta-Package for LibreOffice</fr>
   <he_IL>Kannada Language Meta-Package for LibreOffice</he_IL>
   <hi>Kannada Language Meta-Package for LibreOffice</hi>
   <hr>Kannada Language Meta-Package for LibreOffice</hr>
   <hu>Kannada Language Meta-Package for LibreOffice</hu>
   <id>Kannada Language Meta-Package for LibreOffice</id>
   <is>Kannada Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua kannada per LibreOffice</it>
   <ja_JP>Kannada Language Meta-Package for LibreOffice</ja_JP>
   <ja>Kannada Language Meta-Package for LibreOffice</ja>
   <kk>Kannada Language Meta-Package for LibreOffice</kk>
   <ko>Kannada Language Meta-Package for LibreOffice</ko>
   <lt>Kannada Language Meta-Package for LibreOffice</lt>
   <mk>Kannada Language Meta-Package for LibreOffice</mk>
   <mr>Kannada Language Meta-Package for LibreOffice</mr>
   <nb>Kannada Language Meta-Package for LibreOffice</nb>
   <nl>Kannada Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Kannada Language Meta-Package for LibreOffice</pl>
   <pt_BR>Kannada Language Meta-Package for LibreOffice</pt_BR>
   <pt>Canarim Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Kannada Language Meta-Package for LibreOffice</ro>
   <ru>Kannada Language Meta-Package for LibreOffice</ru>
   <sk>Kannada Language Meta-Package for LibreOffice</sk>
   <sl>Kannadski jezikovni meta-paket za LibreOffice</sl>
   <sq>Kannada Language Meta-Package for LibreOffice</sq>
   <sr>Kannada Language Meta-Package for LibreOffice</sr>
   <sv>Kannada Språk Meta-Paket för LibreOffice</sv>
   <tr>Kannada Language Meta-Package for LibreOffice</tr>
   <uk>Kannada Language Meta-Package for LibreOffice</uk>
   <vi>Kannada Language Meta-Package for LibreOffice</vi>
   <zh_CN>Kannada Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Kannada Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-kn
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-kn
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Kazakh_Libreoffice
</name>

<description>
   <am>Kazakh Language Meta-Package for LibreOffice</am>
   <ar>Kazakh Language Meta-Package for LibreOffice</ar>
   <bg>Kazakh Language Meta-Package for LibreOffice</bg>
   <bn>Kazakh Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Kazakh per LibreOffice</ca>
   <cs>Kazakh Language Meta-Package for LibreOffice</cs>
   <da>Kazakh Language Meta-Package for LibreOffice</da>
   <de>Kasachisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Καζακχ</el>
   <en>Kazakh Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma kazajo para LibreOffice</es>
   <et>Kazakh Language Meta-Package for LibreOffice</et>
   <eu>Kazakh Language Meta-Package for LibreOffice</eu>
   <fa>Kazakh Language Meta-Package for LibreOffice</fa>
   <fil_PH>Kazakh Language Meta-Package for LibreOffice</fil_PH>
   <fi>Kazakh Language Meta-Package for LibreOffice</fi>
   <fr>Kazakh Language Meta-Package for LibreOffice</fr>
   <he_IL>Kazakh Language Meta-Package for LibreOffice</he_IL>
   <hi>Kazakh Language Meta-Package for LibreOffice</hi>
   <hr>Kazakh Language Meta-Package for LibreOffice</hr>
   <hu>Kazakh Language Meta-Package for LibreOffice</hu>
   <id>Kazakh Language Meta-Package for LibreOffice</id>
   <is>Kazakh Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua kazaka per LibreOffice</it>
   <ja_JP>Kazakh Language Meta-Package for LibreOffice</ja_JP>
   <ja>Kazakh Language Meta-Package for LibreOffice</ja>
   <kk>Kazakh Language Meta-Package for LibreOffice</kk>
   <ko>Kazakh Language Meta-Package for LibreOffice</ko>
   <lt>Kazakh Language Meta-Package for LibreOffice</lt>
   <mk>Kazakh Language Meta-Package for LibreOffice</mk>
   <mr>Kazakh Language Meta-Package for LibreOffice</mr>
   <nb>Kazakh Language Meta-Package for LibreOffice</nb>
   <nl>Kazakh Language Meta-Package for LibreOffice</nl>
   <pl>Kazakh Language Meta-Package for LibreOffice</pl>
   <pt_BR>Kazakh Language Meta-Package for LibreOffice</pt_BR>
   <pt>Cazaque Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Kazakh Language Meta-Package for LibreOffice</ro>
   <ru>Kazakh Language Meta-Package for LibreOffice</ru>
   <sk>Kazakh Language Meta-Package for LibreOffice</sk>
   <sl>Kazahksi jezikovni meta-paket za LibreOffice</sl>
   <sq>Kazakh Language Meta-Package for LibreOffice</sq>
   <sr>Kazakh Language Meta-Package for LibreOffice</sr>
   <sv>Kazakiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Kazakh Language Meta-Package for LibreOffice</tr>
   <uk>Kazakh Language Meta-Package for LibreOffice</uk>
   <vi>Kazakh Language Meta-Package for LibreOffice</vi>
   <zh_CN>Kazakh Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Kazakh Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-kk
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-kk
</uninstall_package_names>
</app>

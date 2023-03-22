<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Hebrew_Libreoffice
</name>

<description>
   <am>Hebrew Language Meta-Package for LibreOffice</am>
   <ar>Hebrew Language Meta-Package for LibreOffice</ar>
   <bg>Hebrew Language Meta-Package for LibreOffice</bg>
   <bn>Hebrew Language Meta-Package for LibreOffice</bn>
   <ca>Metpaquet d'idioma Hebreu per LibreOffice</ca>
   <cs>Hebrew Language Meta-Package for LibreOffice</cs>
   <da>Hebrew Language Meta-Package for LibreOffice</da>
   <de>Hebräisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Εβραϊκά</el>
   <en>Hebrew Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma hebreo para LibreOffice</es>
   <et>Hebrew Language Meta-Package for LibreOffice</et>
   <eu>Hebrew Language Meta-Package for LibreOffice</eu>
   <fa>Hebrew Language Meta-Package for LibreOffice</fa>
   <fil_PH>Hebrew Language Meta-Package for LibreOffice</fil_PH>
   <fi>Hebrew Language Meta-Package for LibreOffice</fi>
   <fr>Hebrew Language Meta-Package for LibreOffice</fr>
   <he_IL>Hebrew Language Meta-Package for LibreOffice</he_IL>
   <hi>Hebrew Language Meta-Package for LibreOffice</hi>
   <hr>Hebrew Language Meta-Package for LibreOffice</hr>
   <hu>Hebrew Language Meta-Package for LibreOffice</hu>
   <id>Hebrew Language Meta-Package for LibreOffice</id>
   <is>Hebrew Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua ebraica per LibreOffice</it>
   <ja_JP>Hebrew Language Meta-Package for LibreOffice</ja_JP>
   <ja>Hebrew Language Meta-Package for LibreOffice</ja>
   <kk>Hebrew Language Meta-Package for LibreOffice</kk>
   <ko>Hebrew Language Meta-Package for LibreOffice</ko>
   <lt>Hebrew Language Meta-Package for LibreOffice</lt>
   <mk>Hebrew Language Meta-Package for LibreOffice</mk>
   <mr>Hebrew Language Meta-Package for LibreOffice</mr>
   <nb>Hebrew Language Meta-Package for LibreOffice</nb>
   <nl>Hebreews Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Hebrew Language Meta-Package for LibreOffice</pl>
   <pt_BR>Hebrew Language Meta-Package for LibreOffice</pt_BR>
   <pt>Hebreu Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Hebrew Language Meta-Package for LibreOffice</ro>
   <ru>Hebrew Language Meta-Package for LibreOffice</ru>
   <sk>Hebrew Language Meta-Package for LibreOffice</sk>
   <sl>Hebrejski jezikovni meta-paket za LibreOffice</sl>
   <sq>Hebrew Language Meta-Package for LibreOffice</sq>
   <sr>Hebrew Language Meta-Package for LibreOffice</sr>
   <sv>Hebreiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Hebrew Language Meta-Package for LibreOffice</tr>
   <uk>Hebrew Language Meta-Package for LibreOffice</uk>
   <vi>Hebrew Language Meta-Package for LibreOffice</vi>
   <zh_CN>Hebrew Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Hebrew Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-he
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-he
</uninstall_package_names>
</app>

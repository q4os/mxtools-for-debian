<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Spanish_Libreoffice
</name>

<description>
   <am>Spanish Language Meta-Package for LibreOffice</am>
   <ar>Spanish Language Meta-Package for LibreOffice</ar>
   <bg>Spanish Language Meta-Package for LibreOffice</bg>
   <bn>Spanish Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua castellana per LibreOffice</ca>
   <cs>Spanish Language Meta-Package for LibreOffice</cs>
   <da>Spansk sprog-metapakke til LibreOffice</da>
   <de>Spanisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα ισπανικά</el>
   <en>Spanish Language Meta-Package for LibreOffice</en>
   <es>Meta paquete de idioma español para LibreOffice</es>
   <et>Spanish Language Meta-Package for LibreOffice</et>
   <eu>Spanish Language Meta-Package for LibreOffice</eu>
   <fa>Spanish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Spanish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Spanish Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue espagnole pour LibreOffice</fr>
   <he_IL>Spanish Language Meta-Package for LibreOffice</he_IL>
   <hi>Spanish Language Meta-Package for LibreOffice</hi>
   <hr>Spanish Language Meta-Package for LibreOffice</hr>
   <hu>Spanish Language Meta-Package for LibreOffice</hu>
   <id>Spanish Language Meta-Package for LibreOffice</id>
   <is>Spanish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua spagnola per LibreOffice</it>
   <ja_JP>Spanish Language Meta-Package for LibreOffice</ja_JP>
   <ja>Spanish Language Meta-Package for LibreOffice</ja>
   <kk>Spanish Language Meta-Package for LibreOffice</kk>
   <ko>Spanish Language Meta-Package for LibreOffice</ko>
   <lt>Spanish Language Meta-Package for LibreOffice</lt>
   <mk>Spanish Language Meta-Package for LibreOffice</mk>
   <mr>Spanish Language Meta-Package for LibreOffice</mr>
   <nb>Spanish Language Meta-Package for LibreOffice</nb>
   <nl>Spaanse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Hiszpański metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Castelhano Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Castelhano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Spanish Language Meta-Package for LibreOffice</ro>
   <ru>Spanish Language Meta-Package for LibreOffice</ru>
   <sk>Spanish Language Meta-Package for LibreOffice</sk>
   <sl>Spanish Language Meta-Package for LibreOffice</sl>
   <sq>Spanish Language Meta-Package for LibreOffice</sq>
   <sr>Spanish Language Meta-Package for LibreOffice</sr>
   <sv>Spanskt Språk-Meta-Paket för LibreOffice</sv>
   <tr>Spanish Language Meta-Package for LibreOffice</tr>
   <uk>Spanish Language Meta-Package for LibreOffice</uk>
   <vi>Spanish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Spanish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Spanish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-es
libreoffice-help-es
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-es
libreoffice-help-es
</uninstall_package_names>
</app>

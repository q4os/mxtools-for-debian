<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Swedish_Libreoffice
</name>

<description>
   <am>Swedish Language Meta-Package for LibreOffice</am>
   <ar>Swedish Language Meta-Package for LibreOffice</ar>
   <bg>Swedish Language Meta-Package for LibreOffice</bg>
   <bn>Swedish Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua en Suec per a LibreOffice</ca>
   <cs>Swedish Language Meta-Package for LibreOffice</cs>
   <da>Svensk sprog-metapakke til LibreOffice</da>
   <de>Schwedisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα σουηδικά</el>
   <en>Swedish Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Sueco para LibreOffice</es>
   <et>Swedish Language Meta-Package for LibreOffice</et>
   <eu>Swedish Language Meta-Package for LibreOffice</eu>
   <fa>Swedish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Swedish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Swedish Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue suédoise pour LibreOffice</fr>
   <he_IL>Swedish Language Meta-Package for LibreOffice</he_IL>
   <hi>Swedish Language Meta-Package for LibreOffice</hi>
   <hr>Swedish Language Meta-Package for LibreOffice</hr>
   <hu>Swedish Language Meta-Package for LibreOffice</hu>
   <id>Swedish Language Meta-Package for LibreOffice</id>
   <is>Swedish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua svedese per LibreOffice</it>
   <ja_JP>Swedish Language Meta-Package for LibreOffice</ja_JP>
   <ja>Swedish Language Meta-Package for LibreOffice</ja>
   <kk>Swedish Language Meta-Package for LibreOffice</kk>
   <ko>Swedish Language Meta-Package for LibreOffice</ko>
   <lt>Swedish Language Meta-Package for LibreOffice</lt>
   <mk>Swedish Language Meta-Package for LibreOffice</mk>
   <mr>Swedish Language Meta-Package for LibreOffice</mr>
   <nb>Swedish Language Meta-Package for LibreOffice</nb>
   <nl>Zweedse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Szwedzki metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Sueco Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Sueco Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Swedish Language Meta-Package for LibreOffice</ro>
   <ru>Swedish Language Meta-Package for LibreOffice</ru>
   <sk>Swedish Language Meta-Package for LibreOffice</sk>
   <sl>Swedish Language Meta-Package for LibreOffice</sl>
   <sq>Swedish Language Meta-Package for LibreOffice</sq>
   <sr>Swedish Language Meta-Package for LibreOffice</sr>
   <sv>Svenskt Språk-Meta-Paket för LibreOffice</sv>
   <tr>Swedish Language Meta-Package for LibreOffice</tr>
   <uk>Swedish Language Meta-Package for LibreOffice</uk>
   <vi>Swedish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Swedish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Swedish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-sv
libreoffice-help-sv
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-sv
libreoffice-help-sv
</uninstall_package_names>
</app>

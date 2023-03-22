<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
French_Libreoffice
</name>

<description>
   <am>French Language Meta-Package for LibreOffice</am>
   <ar>French Language Meta-Package for LibreOffice</ar>
   <bg>French Language Meta-Package for LibreOffice</bg>
   <bn>French Language Meta-Package for LibreOffice</bn>
   <ca>Meta paquet per LibreOffice en llengua francesa</ca>
   <cs>French Language Meta-Package for LibreOffice</cs>
   <da>Fransk sprog-metapakke til LibreOffice</da>
   <de>Französisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στη γαλλική γλώσσα</el>
   <en>French Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Francés para LibreOffice</es>
   <et>French Language Meta-Package for LibreOffice</et>
   <eu>French Language Meta-Package for LibreOffice</eu>
   <fa>French Language Meta-Package for LibreOffice</fa>
   <fil_PH>French Language Meta-Package for LibreOffice</fil_PH>
   <fi>French Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue française pour LibreOffice</fr>
   <he_IL>French Language Meta-Package for LibreOffice</he_IL>
   <hi>French Language Meta-Package for LibreOffice</hi>
   <hr>French Language Meta-Package for LibreOffice</hr>
   <hu>French Language Meta-Package for LibreOffice</hu>
   <id>French Language Meta-Package for LibreOffice</id>
   <is>French Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua francese per LibreOffice</it>
   <ja_JP>French Language Meta-Package for LibreOffice</ja_JP>
   <ja>French Language Meta-Package for LibreOffice</ja>
   <kk>French Language Meta-Package for LibreOffice</kk>
   <ko>French Language Meta-Package for LibreOffice</ko>
   <lt>French Language Meta-Package for LibreOffice</lt>
   <mk>French Language Meta-Package for LibreOffice</mk>
   <mr>French Language Meta-Package for LibreOffice</mr>
   <nb>French Language Meta-Package for LibreOffice</nb>
   <nl>Franse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Francuski metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Francês Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Francês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>French Language Meta-Package for LibreOffice</ro>
   <ru>French Language Meta-Package for LibreOffice</ru>
   <sk>French Language Meta-Package for LibreOffice</sk>
   <sl>French Language Meta-Package for LibreOffice</sl>
   <sq>French Language Meta-Package for LibreOffice</sq>
   <sr>French Language Meta-Package for LibreOffice</sr>
   <sv>Franskt Språk Meta-Paket för LibreOffice</sv>
   <tr>French Language Meta-Package for LibreOffice</tr>
   <uk>French Language Meta-Package for LibreOffice</uk>
   <vi>French Language Meta-Package for LibreOffice</vi>
   <zh_CN>French Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>French Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-fr
libreoffice-help-fr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-fr
libreoffice-help-fr
</uninstall_package_names>
</app>

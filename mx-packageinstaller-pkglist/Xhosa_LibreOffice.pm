<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Xhosa_Libreoffice
</name>

<description>
   <am>Xhosa Language Meta-Package for LibreOffice</am>
   <ar>Xhosa Language Meta-Package for LibreOffice</ar>
   <bg>Xhosa Language Meta-Package for LibreOffice</bg>
   <bn>Xhosa Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Xhosa per LibreOffice</ca>
   <cs>Xhosa Language Meta-Package for LibreOffice</cs>
   <da>Xhosa Language Meta-Package for LibreOffice</da>
   <de>Xhosa Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Xhosa</el>
   <en>Xhosa Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete idioma xhosa para LibreOffice</es>
   <et>Xhosa Language Meta-Package for LibreOffice</et>
   <eu>Xhosa Language Meta-Package for LibreOffice</eu>
   <fa>Xhosa Language Meta-Package for LibreOffice</fa>
   <fil_PH>Xhosa Language Meta-Package for LibreOffice</fil_PH>
   <fi>Xhosa Language Meta-Package for LibreOffice</fi>
   <fr>Meta-Paquet de langue Xhosa pour LibreOffice</fr>
   <he_IL>Xhosa Language Meta-Package for LibreOffice</he_IL>
   <hi>Xhosa Language Meta-Package for LibreOffice</hi>
   <hr>Xhosa Language Meta-Package for LibreOffice</hr>
   <hu>Xhosa Language Meta-Package for LibreOffice</hu>
   <id>Xhosa Language Meta-Package for LibreOffice</id>
   <is>Xhosa Language Meta-Package for LibreOffice</is>
   <it>Xhosa Language Meta-Package for LibreOffice</it>
   <ja_JP>Xhosa Language Meta-Package for LibreOffice</ja_JP>
   <ja>Xhosa Language Meta-Package for LibreOffice</ja>
   <kk>Xhosa Language Meta-Package for LibreOffice</kk>
   <ko>Xhosa Language Meta-Package for LibreOffice</ko>
   <lt>Xhosa Language Meta-Package for LibreOffice</lt>
   <mk>Xhosa Language Meta-Package for LibreOffice</mk>
   <mr>Xhosa Language Meta-Package for LibreOffice</mr>
   <nb>Xhosa Language Meta-Package for LibreOffice</nb>
   <nl>Xhosa Language Meta-Package for LibreOffice</nl>
   <pl>Xhosa Language Meta-Package for LibreOffice</pl>
   <pt_BR>Xhosa Language Meta-Package for LibreOffice</pt_BR>
   <pt>Xosa Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Xhosa Language Meta-Package for LibreOffice</ro>
   <ru>Xhosa Language Meta-Package for LibreOffice</ru>
   <sk>Xhosa Language Meta-Package for LibreOffice</sk>
   <sl>Khoški jezikovni meta-paket za LibreOffice</sl>
   <sq>Xhosa Language Meta-Package for LibreOffice</sq>
   <sr>Xhosa Language Meta-Package for LibreOffice</sr>
   <sv>Xhosa Språk Meta-Paket för LibreOffice</sv>
   <tr>Xhosa Language Meta-Package for LibreOffice</tr>
   <uk>Xhosa Language Meta-Package for LibreOffice</uk>
   <vi>Xhosa Language Meta-Package for LibreOffice</vi>
   <zh_CN>Xhosa Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Xhosa Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-xh
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-xh
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Icelandic_Libreoffice
</name>

<description>
   <am>Icelandic Language Meta-Package for LibreOffice</am>
   <ar>Icelandic Language Meta-Package for LibreOffice</ar>
   <bg>Icelandic Language Meta-Package for LibreOffice</bg>
   <bn>Icelandic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Islandès per LibreOffice</ca>
   <cs>Icelandic Language Meta-Package for LibreOffice</cs>
   <da>Icelandic Language Meta-Package for LibreOffice</da>
   <de>Isländisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα ισλανδικά</el>
   <en>Icelandic Language Meta-Package for LibreOffice</en>
   <es>Meta Paquete de Idioma Islandés para LibreOffice</es>
   <et>Icelandic Language Meta-Package for LibreOffice</et>
   <eu>Icelandic Language Meta-Package for LibreOffice</eu>
   <fa>Icelandic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Icelandic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Icelandic Language Meta-Package for LibreOffice</fi>
   <fr>Icelandic Language Meta-Package for LibreOffice</fr>
   <he_IL>Icelandic Language Meta-Package for LibreOffice</he_IL>
   <hi>Icelandic Language Meta-Package for LibreOffice</hi>
   <hr>Icelandic Language Meta-Package for LibreOffice</hr>
   <hu>Icelandic Language Meta-Package for LibreOffice</hu>
   <id>Icelandic Language Meta-Package for LibreOffice</id>
   <is>Icelandic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua islandese per LibreOffice</it>
   <ja_JP>Icelandic Language Meta-Package for LibreOffice</ja_JP>
   <ja>Icelandic Language Meta-Package for LibreOffice</ja>
   <kk>Icelandic Language Meta-Package for LibreOffice</kk>
   <ko>Icelandic Language Meta-Package for LibreOffice</ko>
   <lt>Icelandic Language Meta-Package for LibreOffice</lt>
   <mk>Icelandic Language Meta-Package for LibreOffice</mk>
   <mr>Icelandic Language Meta-Package for LibreOffice</mr>
   <nb>Icelandic Language Meta-Package for LibreOffice</nb>
   <nl>IJslandse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Icelandic Language Meta-Package for LibreOffice</pl>
   <pt_BR>Icelandic Language Meta-Package for LibreOffice</pt_BR>
   <pt>Islandês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Icelandic Language Meta-Package for LibreOffice</ro>
   <ru>Icelandic Language Meta-Package for LibreOffice</ru>
   <sk>Icelandic Language Meta-Package for LibreOffice</sk>
   <sl>Islandski jezikovni meta-paket za LibreOffice</sl>
   <sq>Icelandic Language Meta-Package for LibreOffice</sq>
   <sr>Icelandic Language Meta-Package for LibreOffice</sr>
   <sv>Isländskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Icelandic Language Meta-Package for LibreOffice</tr>
   <uk>Icelandic Language Meta-Package for LibreOffice</uk>
   <vi>Icelandic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Icelandic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Icelandic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-is
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-is
</uninstall_package_names>
</app>

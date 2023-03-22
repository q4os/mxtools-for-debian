<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Finnish_Libreoffice
</name>

<description>
   <am>Finnish Language Meta-Package for LibreOffice</am>
   <ar>Finnish Language Meta-Package for LibreOffice</ar>
   <bg>Finnish Language Meta-Package for LibreOffice</bg>
   <bn>Finnish Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Finès per LibreOffice</ca>
   <cs>Finnish Language Meta-Package for LibreOffice</cs>
   <da>Finnish Language Meta-Package for LibreOffice</da>
   <de>Finnisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα φινλανδικά</el>
   <en>Finnish Language Meta-Package for LibreOffice</en>
   <es>Meta Paquete de idioma finlandés para LibreOffice</es>
   <et>Finnish Language Meta-Package for LibreOffice</et>
   <eu>Finnish Language Meta-Package for LibreOffice</eu>
   <fa>Finnish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Finnish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Finnish Language Meta-Package for LibreOffice</fi>
   <fr>Finnish Language Meta-Package for LibreOffice</fr>
   <he_IL>Finnish Language Meta-Package for LibreOffice</he_IL>
   <hi>Finnish Language Meta-Package for LibreOffice</hi>
   <hr>Finnish Language Meta-Package for LibreOffice</hr>
   <hu>Finnish Language Meta-Package for LibreOffice</hu>
   <id>Finnish Language Meta-Package for LibreOffice</id>
   <is>Finnish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua finlandese per LibreOffice</it>
   <ja_JP>Finnish Language Meta-Package for LibreOffice</ja_JP>
   <ja>Finnish Language Meta-Package for LibreOffice</ja>
   <kk>Finnish Language Meta-Package for LibreOffice</kk>
   <ko>Finnish Language Meta-Package for LibreOffice</ko>
   <lt>Finnish Language Meta-Package for LibreOffice</lt>
   <mk>Finnish Language Meta-Package for LibreOffice</mk>
   <mr>Finnish Language Meta-Package for LibreOffice</mr>
   <nb>Finnish Language Meta-Package for LibreOffice</nb>
   <nl>Finnish Language Meta-Package for LibreOffice</nl>
   <pl>Finnish Language Meta-Package for LibreOffice</pl>
   <pt_BR>Finnish Language Meta-Package for LibreOffice</pt_BR>
   <pt>Finlandês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Finnish Language Meta-Package for LibreOffice</ro>
   <ru>Finnish Language Meta-Package for LibreOffice</ru>
   <sk>Finnish Language Meta-Package for LibreOffice</sk>
   <sl>Finska jezikovni meta-paket za LibreOffice</sl>
   <sq>Finnish Language Meta-Package for LibreOffice</sq>
   <sr>Finnish Language Meta-Package for LibreOffice</sr>
   <sv>Finskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Finnish Language Meta-Package for LibreOffice</tr>
   <uk>Finnish Language Meta-Package for LibreOffice</uk>
   <vi>Finnish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Finnish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Finnish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-fi
libreoffice-help-fi
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-fi
libreoffice-help-fi
</uninstall_package_names>
</app>

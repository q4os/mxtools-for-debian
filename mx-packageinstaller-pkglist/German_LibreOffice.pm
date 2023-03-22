<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
German_Libreoffice
</name>

<description>
   <am>German Language Meta-Package for LibreOffice</am>
   <ar>German Language Meta-Package for LibreOffice</ar>
   <bg>German Language Meta-Package for LibreOffice</bg>
   <bn>German Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet d'alemany per LibreOffice</ca>
   <cs>German Language Meta-Package for LibreOffice</cs>
   <da>Tysk sprog-metapakke til LibreOffice</da>
   <de>Deutsches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα γερμανικά</el>
   <en>German Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Alemán para LibreOffice</es>
   <et>German Language Meta-Package for LibreOffice</et>
   <eu>German Language Meta-Package for LibreOffice</eu>
   <fa>German Language Meta-Package for LibreOffice</fa>
   <fil_PH>German Language Meta-Package for LibreOffice</fil_PH>
   <fi>German Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue allemande pour LibreOffice</fr>
   <he_IL>German Language Meta-Package for LibreOffice</he_IL>
   <hi>German Language Meta-Package for LibreOffice</hi>
   <hr>German Language Meta-Package for LibreOffice</hr>
   <hu>German Language Meta-Package for LibreOffice</hu>
   <id>German Language Meta-Package for LibreOffice</id>
   <is>German Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua tedesca per LibreOffice</it>
   <ja_JP>German Language Meta-Package for LibreOffice</ja_JP>
   <ja>German Language Meta-Package for LibreOffice</ja>
   <kk>German Language Meta-Package for LibreOffice</kk>
   <ko>German Language Meta-Package for LibreOffice</ko>
   <lt>German Language Meta-Package for LibreOffice</lt>
   <mk>German Language Meta-Package for LibreOffice</mk>
   <mr>German Language Meta-Package for LibreOffice</mr>
   <nb>German Language Meta-Package for LibreOffice</nb>
   <nl>Duitse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Niemiecki metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Alemão Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Alemão Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>German Language Meta-Package for LibreOffice</ro>
   <ru>German Language Meta-Package for LibreOffice</ru>
   <sk>German Language Meta-Package for LibreOffice</sk>
   <sl>German Language Meta-Package for LibreOffice</sl>
   <sq>German Language Meta-Package for LibreOffice</sq>
   <sr>German Language Meta-Package for LibreOffice</sr>
   <sv>Tyskt Språk Meta-Paket för LibreOffice</sv>
   <tr>German Language Meta-Package for LibreOffice</tr>
   <uk>German Language Meta-Package for LibreOffice</uk>
   <vi>German Language Meta-Package for LibreOffice</vi>
   <zh_CN>German Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>German Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-de
libreoffice-help-de
"
lo-installer.sh  $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-de
libreoffice-help-de
</uninstall_package_names>
</app>

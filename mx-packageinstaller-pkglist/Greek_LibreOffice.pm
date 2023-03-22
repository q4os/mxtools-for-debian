<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Greek_Libreoffice
</name>

<description>
   <am>Greek Language Meta-Package for LibreOffice</am>
   <ar>Greek Language Meta-Package for LibreOffice</ar>
   <bg>Greek Language Meta-Package for LibreOffice</bg>
   <bn>Greek Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua grega per LibreOffice</ca>
   <cs>Greek Language Meta-Package for LibreOffice</cs>
   <da>Græsk sprog-metapakke til LibreOffice</da>
   <de>Griechisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα ελληνικά</el>
   <en>Greek Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Griego para LibreOffice</es>
   <et>Greek Language Meta-Package for LibreOffice</et>
   <eu>Greek Language Meta-Package for LibreOffice</eu>
   <fa>Greek Language Meta-Package for LibreOffice</fa>
   <fil_PH>Greek Language Meta-Package for LibreOffice</fil_PH>
   <fi>Greek Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue grecque pour LibreOffice</fr>
   <he_IL>Greek Language Meta-Package for LibreOffice</he_IL>
   <hi>Greek Language Meta-Package for LibreOffice</hi>
   <hr>Greek Language Meta-Package for LibreOffice</hr>
   <hu>Greek Language Meta-Package for LibreOffice</hu>
   <id>Greek Language Meta-Package for LibreOffice</id>
   <is>Greek Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua greca per LibreOffice</it>
   <ja_JP>Greek Language Meta-Package for LibreOffice</ja_JP>
   <ja>Greek Language Meta-Package for LibreOffice</ja>
   <kk>Greek Language Meta-Package for LibreOffice</kk>
   <ko>Greek Language Meta-Package for LibreOffice</ko>
   <lt>Greek Language Meta-Package for LibreOffice</lt>
   <mk>Greek Language Meta-Package for LibreOffice</mk>
   <mr>Greek Language Meta-Package for LibreOffice</mr>
   <nb>Greek Language Meta-Package for LibreOffice</nb>
   <nl>Griekse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Grecki metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Grego Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Grego Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Greek Language Meta-Package for LibreOffice</ro>
   <ru>Greek Language Meta-Package for LibreOffice</ru>
   <sk>Greek Language Meta-Package for LibreOffice</sk>
   <sl>Grški jezikovni meta-paket za LibreOffice</sl>
   <sq>Greek Language Meta-Package for LibreOffice</sq>
   <sr>Greek Language Meta-Package for LibreOffice</sr>
   <sv>Grekiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Greek Language Meta-Package for LibreOffice</tr>
   <uk>Greek Language Meta-Package for LibreOffice</uk>
   <vi>Greek Language Meta-Package for LibreOffice</vi>
   <zh_CN>Greek Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Greek Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-el
libreoffice-help-el
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-el
libreoffice-help-el
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Norwegian_Libreoffice
</name>

<description>
   <am>Norwegian Language Meta-Package for LibreOffice</am>
   <ar>Norwegian Language Meta-Package for LibreOffice</ar>
   <bg>Norwegian Language Meta-Package for LibreOffice</bg>
   <bn>Norwegian Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua per a LibreOffice</ca>
   <cs>Norwegian Language Meta-Package for LibreOffice</cs>
   <da>Norsk sprog-metapakke til LibreOffice</da>
   <de>Norwegisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Νορβηγικά</el>
   <en>Norwegian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Noruego para LibreOffice</es>
   <et>Norwegian Language Meta-Package for LibreOffice</et>
   <eu>Norwegian Language Meta-Package for LibreOffice</eu>
   <fa>Norwegian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Norwegian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Norwegian Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue norvégienne pour LibreOffice</fr>
   <he_IL>Norwegian Language Meta-Package for LibreOffice</he_IL>
   <hi>Norwegian Language Meta-Package for LibreOffice</hi>
   <hr>Norwegian Language Meta-Package for LibreOffice</hr>
   <hu>Norwegian Language Meta-Package for LibreOffice</hu>
   <id>Norwegian Language Meta-Package for LibreOffice</id>
   <is>Norwegian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua norvegese per LibreOffice</it>
   <ja_JP>Norwegian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Norwegian Language Meta-Package for LibreOffice</ja>
   <kk>Norwegian Language Meta-Package for LibreOffice</kk>
   <ko>Norwegian Language Meta-Package for LibreOffice</ko>
   <lt>Norwegian Language Meta-Package for LibreOffice</lt>
   <mk>Norwegian Language Meta-Package for LibreOffice</mk>
   <mr>Norwegian Language Meta-Package for LibreOffice</mr>
   <nb>Norwegian Language Meta-Package for LibreOffice</nb>
   <nl>Noorse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Norweski metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Norueguês Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Norueguês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Norwegian Language Meta-Package for LibreOffice</ro>
   <ru>Norwegian Language Meta-Package for LibreOffice</ru>
   <sk>Norwegian Language Meta-Package for LibreOffice</sk>
   <sl>Norwegian Language Meta-Package for LibreOffice</sl>
   <sq>Norwegian Language Meta-Package for LibreOffice</sq>
   <sr>Norwegian Language Meta-Package for LibreOffice</sr>
   <sv>Norskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Norwegian Language Meta-Package for LibreOffice</tr>
   <uk>Norwegian Language Meta-Package for LibreOffice</uk>
   <vi>Norwegian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Norwegian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Norwegian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-nb
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-nb
</uninstall_package_names>
</app>

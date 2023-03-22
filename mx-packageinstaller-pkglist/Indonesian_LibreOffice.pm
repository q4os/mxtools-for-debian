<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Indonesian_Libreoffice
</name>

<description>
   <am>Indonesian Language Meta-Package for LibreOffice</am>
   <ar>Indonesian Language Meta-Package for LibreOffice</ar>
   <bg>Indonesian Language Meta-Package for LibreOffice</bg>
   <bn>Indonesian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Indonesi per LibreOffice</ca>
   <cs>Indonesian Language Meta-Package for LibreOffice</cs>
   <da>Indonesian Language Meta-Package for LibreOffice</da>
   <de>Indonesisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα ινδονησιακά</el>
   <en>Indonesian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma indonesio para LibreOffice</es>
   <et>Indonesian Language Meta-Package for LibreOffice</et>
   <eu>Indonesian Language Meta-Package for LibreOffice</eu>
   <fa>Indonesian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Indonesian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Indonesian Language Meta-Package for LibreOffice</fi>
   <fr>Indonesian Language Meta-Package for LibreOffice</fr>
   <he_IL>Indonesian Language Meta-Package for LibreOffice</he_IL>
   <hi>Indonesian Language Meta-Package for LibreOffice</hi>
   <hr>Indonesian Language Meta-Package for LibreOffice</hr>
   <hu>Indonesian Language Meta-Package for LibreOffice</hu>
   <id>Indonesian Language Meta-Package for LibreOffice</id>
   <is>Indonesian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua indonesiana per LibreOffice</it>
   <ja_JP>Indonesian Language Meta-Package for LibreOffice</ja_JP>
   <ja>Indonesian Language Meta-Package for LibreOffice</ja>
   <kk>Indonesian Language Meta-Package for LibreOffice</kk>
   <ko>Indonesian Language Meta-Package for LibreOffice</ko>
   <lt>Indonesian Language Meta-Package for LibreOffice</lt>
   <mk>Indonesian Language Meta-Package for LibreOffice</mk>
   <mr>Indonesian Language Meta-Package for LibreOffice</mr>
   <nb>Indonesian Language Meta-Package for LibreOffice</nb>
   <nl>Indonesisch Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Indonesian Language Meta-Package for LibreOffice</pl>
   <pt_BR>Indonesian Language Meta-Package for LibreOffice</pt_BR>
   <pt>Indonésio Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Indonesian Language Meta-Package for LibreOffice</ro>
   <ru>Indonesian Language Meta-Package for LibreOffice</ru>
   <sk>Indonesian Language Meta-Package for LibreOffice</sk>
   <sl>Indonezijski jezikovni meta-paket za LibreOffice</sl>
   <sq>Indonesian Language Meta-Package for LibreOffice</sq>
   <sr>Indonesian Language Meta-Package for LibreOffice</sr>
   <sv>Indonesiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>Indonesian Language Meta-Package for LibreOffice</tr>
   <uk>Indonesian Language Meta-Package for LibreOffice</uk>
   <vi>Indonesian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Indonesian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Indonesian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-id
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-id
</uninstall_package_names>
</app>

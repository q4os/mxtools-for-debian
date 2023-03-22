<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Afrikaans_Libreoffice
</name>

<description>
   <am>Afrikaans Language Meta-Package for LibreOffice</am>
   <ar>Afrikaans Language Meta-Package for LibreOffice</ar>
   <bg>Afrikaans Language Meta-Package for LibreOffice</bg>
   <bn>Afrikaans Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Afrikaans per LibreOffice</ca>
   <cs>Afrikaans Language Meta-Package for LibreOffice</cs>
   <da>Afrikaans Language Meta-Package for LibreOffice</da>
   <de>Afrikaans Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Afrikaans</el>
   <en>Afrikaans Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma afrikáans para LibreOffice</es>
   <et>Afrikaans Language Meta-Package for LibreOffice</et>
   <eu>Afrikaans Language Meta-Package for LibreOffice</eu>
   <fa>Afrikaans Language Meta-Package for LibreOffice</fa>
   <fil_PH>Afrikaans Language Meta-Package for LibreOffice</fil_PH>
   <fi>Afrikaans Language Meta-Package for LibreOffice</fi>
   <fr>Afrikaans Language Meta-Package for LibreOffice</fr>
   <he_IL>Afrikaans Language Meta-Package for LibreOffice</he_IL>
   <hi>Afrikaans Language Meta-Package for LibreOffice</hi>
   <hr>Afrikaans Language Meta-Package for LibreOffice</hr>
   <hu>Afrikaans Language Meta-Package for LibreOffice</hu>
   <id>Afrikaans Language Meta-Package for LibreOffice</id>
   <is>Afrikaans Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua afrikaans per LibreOffice</it>
   <ja_JP>Afrikaans Language Meta-Package for LibreOffice</ja_JP>
   <ja>Afrikaans Language Meta-Package for LibreOffice</ja>
   <kk>Afrikaans Language Meta-Package for LibreOffice</kk>
   <ko>Afrikaans Language Meta-Package for LibreOffice</ko>
   <lt>Afrikaans Language Meta-Package for LibreOffice</lt>
   <mk>Afrikaans Language Meta-Package for LibreOffice</mk>
   <mr>Afrikaans Language Meta-Package for LibreOffice</mr>
   <nb>Afrikaans Language Meta-Package for LibreOffice</nb>
   <nl>Afrikaans Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Afrikaans Language Meta-Package for LibreOffice</pl>
   <pt_BR>Afrikaans Language Meta-Package for LibreOffice</pt_BR>
   <pt>Africander Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Afrikaans Language Meta-Package for LibreOffice</ro>
   <ru>Afrikaans Language Meta-Package for LibreOffice</ru>
   <sk>Afrikaans Language Meta-Package for LibreOffice</sk>
   <sl>Afrikaan jezikovni meta-paket za LibreOffice</sl>
   <sq>Afrikaans Language Meta-Package for LibreOffice</sq>
   <sr>Afrikaans Language Meta-Package for LibreOffice</sr>
   <sv>Afrikaans Språk Meta-Paket för LibreOffice</sv>
   <tr>Afrikaans Language Meta-Package for LibreOffice</tr>
   <uk>Afrikaans Language Meta-Package for LibreOffice</uk>
   <vi>Afrikaans Language Meta-Package for LibreOffice</vi>
   <zh_CN>Afrikaans Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Afrikaans Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-af
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-af
</uninstall_package_names>
</app>

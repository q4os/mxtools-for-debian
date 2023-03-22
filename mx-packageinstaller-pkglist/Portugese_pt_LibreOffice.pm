<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Portugese(PT) Libreoffice
</name>

<description>
   <am>Portugese(PT) Language Meta-Package for LibreOffice</am>
   <ar>Portugese(PT) Language Meta-Package for LibreOffice</ar>
   <bg>Portugese(PT) Language Meta-Package for LibreOffice</bg>
   <bn>Portugese(PT) Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet de llengua Portuguesa(PT) per LibreOffice</ca>
   <cs>Portugese(PT) Language Meta-Package for LibreOffice</cs>
   <da>Portugisisk (portugal) sprog-metapakke til LibreOffice</da>
   <de>Portugiesisches (PT) Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα πορτογαλικά</el>
   <en>Portugese(PT) Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Portugués (PT) para LibreOffice</es>
   <et>Portugese(PT) Language Meta-Package for LibreOffice</et>
   <eu>Portugese(PT) Language Meta-Package for LibreOffice</eu>
   <fa>Portugese(PT) Language Meta-Package for LibreOffice</fa>
   <fil_PH>Portugese(PT) Language Meta-Package for LibreOffice</fil_PH>
   <fi>Portugese(PT) Language Meta-Package for LibreOffice</fi>
   <fr>Meta-Paquet de langue Portuguais(PT) pour LibreOffice</fr>
   <he_IL>Portugese(PT) Language Meta-Package for LibreOffice</he_IL>
   <hi>Portugese(PT) Language Meta-Package for LibreOffice</hi>
   <hr>Portugese(PT) Language Meta-Package for LibreOffice</hr>
   <hu>Portugese(PT) Language Meta-Package for LibreOffice</hu>
   <id>Portugese(PT) Language Meta-Package for LibreOffice</id>
   <is>Portugese(PT) Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua portoghese(PT) per LibreOffice</it>
   <ja_JP>Portugese(PT) Language Meta-Package for LibreOffice</ja_JP>
   <ja>Portugese(PT) Language Meta-Package for LibreOffice</ja>
   <kk>Portugese(PT) Language Meta-Package for LibreOffice</kk>
   <ko>Portugese(PT) Language Meta-Package for LibreOffice</ko>
   <lt>Portugese(PT) Language Meta-Package for LibreOffice</lt>
   <mk>Portugese(PT) Language Meta-Package for LibreOffice</mk>
   <mr>Portugese(PT) Language Meta-Package for LibreOffice</mr>
   <nb>Portugese(PT) Language Meta-Package for LibreOffice</nb>
   <nl>Portugese(PT) Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Portugalski (PT) metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Português(PT) Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Português(PT) Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Portugese(PT) Language Meta-Package for LibreOffice</ro>
   <ru>Portugese(PT) Language Meta-Package for LibreOffice</ru>
   <sk>Portugese(PT) Language Meta-Package for LibreOffice</sk>
   <sl>Portugese(PT) Language Meta-Package for LibreOffice</sl>
   <sq>Portugese(PT) Language Meta-Package for LibreOffice</sq>
   <sr>Portugese(PT) Language Meta-Package for LibreOffice</sr>
   <sv>Portugisiskt(PT) Språk Meta-Paket för LibreOffice</sv>
   <tr>Portugese(PT) Language Meta-Package for LibreOffice</tr>
   <uk>Portugese(PT) Language Meta-Package for LibreOffice</uk>
   <vi>Portugese(PT) Language Meta-Package for LibreOffice</vi>
   <zh_CN>Portugese(PT) Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Portugese(PT) Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pt
libreoffice-help-pt
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pt
libreoffice-help-pt
</uninstall_package_names>
</app>

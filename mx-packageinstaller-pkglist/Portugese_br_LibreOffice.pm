<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Portugese(BR) Libreoffice
</name>

<description>
   <am>Portugese(BR) Language Meta-Package for LibreOffice</am>
   <ar>Portugese(BR) Language Meta-Package for LibreOffice</ar>
   <bg>Portugese(BR) Language Meta-Package for LibreOffice</bg>
   <bn>Portugese(BR) Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua per LibreOffice en Portuguès (BR)</ca>
   <cs>Portugese(BR) Language Meta-Package for LibreOffice</cs>
   <da>Portugisisk (brasilien) sprog-metapakke til LibreOffice</da>
   <de>Portugiesisches (BR) Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα πορτογαλικά (Βραζιλίας)</el>
   <en>Portugese(BR) Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Portugués (BR) para LibreOffice</es>
   <et>Portugese(BR) Language Meta-Package for LibreOffice</et>
   <eu>Portugese(BR) Language Meta-Package for LibreOffice</eu>
   <fa>Portugese(BR) Language Meta-Package for LibreOffice</fa>
   <fil_PH>Portugese(BR) Language Meta-Package for LibreOffice</fil_PH>
   <fi>Portugese(BR) Language Meta-Package for LibreOffice</fi>
   <fr>Méta-paquet langue portugaise (BR) pour LibreOffice</fr>
   <he_IL>Portugese(BR) Language Meta-Package for LibreOffice</he_IL>
   <hi>Portugese(BR) Language Meta-Package for LibreOffice</hi>
   <hr>Portugese(BR) Language Meta-Package for LibreOffice</hr>
   <hu>Portugese(BR) Language Meta-Package for LibreOffice</hu>
   <id>Portugese(BR) Language Meta-Package for LibreOffice</id>
   <is>Portugese(BR) Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua portoghese(BR) per LibreOffice</it>
   <ja_JP>Portugese(BR) Language Meta-Package for LibreOffice</ja_JP>
   <ja>Portugese(BR) Language Meta-Package for LibreOffice</ja>
   <kk>Portugese(BR) Language Meta-Package for LibreOffice</kk>
   <ko>Portugese(BR) Language Meta-Package for LibreOffice</ko>
   <lt>Portugese(BR) Language Meta-Package for LibreOffice</lt>
   <mk>Portugese(BR) Language Meta-Package for LibreOffice</mk>
   <mr>Portugese(BR) Language Meta-Package for LibreOffice</mr>
   <nb>Portugese(BR) Language Meta-Package for LibreOffice</nb>
   <nl>Portugese(BR) Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Portugalski (BR) metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Português(BR) Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Português(BR) Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Portugese(BR) Language Meta-Package for LibreOffice</ro>
   <ru>Portugese(BR) Language Meta-Package for LibreOffice</ru>
   <sk>Portugese(BR) Language Meta-Package for LibreOffice</sk>
   <sl>Portugese(BR) Language Meta-Package for LibreOffice</sl>
   <sq>Portugese(BR) Language Meta-Package for LibreOffice</sq>
   <sr>Portugese(BR) Language Meta-Package for LibreOffice</sr>
   <sv>Portugisiskt(BR) Språk Meta-Paket för LibreOffice</sv>
   <tr>Portugese(BR) Language Meta-Package for LibreOffice</tr>
   <uk>Portugese(BR) Language Meta-Package for LibreOffice</uk>
   <vi>Portugese(BR) Language Meta-Package for LibreOffice</vi>
   <zh_CN>Portugese(BR) Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Portugese(BR) Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pt-br
libreoffice-help-pt-br
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pt-br
libreoffice-help-pt-br
</uninstall_package_names>
</app>

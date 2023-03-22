<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Hungarian_Libreoffice
</name>

<description>
   <am>Hungarian LibreOffice Language Meta-Package</am>
   <ar>Hungarian LibreOffice Language Meta-Package</ar>
   <bg>Hungarian LibreOffice Language Meta-Package</bg>
   <bn>Hungarian LibreOffice Language Meta-Package</bn>
   <ca>Meta-paquet per LibreOffice en Hongarès</ca>
   <cs>Hungarian LibreOffice Language Meta-Package</cs>
   <da>Ungarsk LibreOffice sprog-metapakke</da>
   <de>Ungarisches LibreOffice Meta-Paket</de>
   <el>LibreOffice στην ουγγρική γλώσσα</el>
   <en>Hungarian LibreOffice Language Meta-Package</en>
   <es>Meta-Paquete de Idioma Húngaro LibreOffice</es>
   <et>Hungarian LibreOffice Language Meta-Package</et>
   <eu>Hungarian LibreOffice Language Meta-Package</eu>
   <fa>Hungarian LibreOffice Language Meta-Package</fa>
   <fil_PH>Hungarian LibreOffice Language Meta-Package</fil_PH>
   <fi>Hungarian LibreOffice Language Meta-Package</fi>
   <fr>Méta-paquet langue hongroise pour LibreOffice</fr>
   <he_IL>Hungarian LibreOffice Language Meta-Package</he_IL>
   <hi>Hungarian LibreOffice Language Meta-Package</hi>
   <hr>Hungarian LibreOffice Language Meta-Package</hr>
   <hu>Hungarian LibreOffice Language Meta-Package</hu>
   <id>Hungarian LibreOffice Language Meta-Package</id>
   <is>Hungarian LibreOffice Language Meta-Package</is>
   <it>Meta-pacchetto della lingua ungherese per LibreOffice</it>
   <ja_JP>Hungarian LibreOffice Language Meta-Package</ja_JP>
   <ja>Hungarian LibreOffice Language Meta-Package</ja>
   <kk>Hungarian LibreOffice Language Meta-Package</kk>
   <ko>Hungarian LibreOffice Language Meta-Package</ko>
   <lt>Hungarian LibreOffice Language Meta-Package</lt>
   <mk>Hungarian LibreOffice Language Meta-Package</mk>
   <mr>Hungarian LibreOffice Language Meta-Package</mr>
   <nb>Hungarian LibreOffice Language Meta-Package</nb>
   <nl>Hongaarse LibreOffice Taal Meta-Pakket</nl>
   <pl>Węgierski metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Húngaro Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Húngaro Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Hungarian LibreOffice Language Meta-Package</ro>
   <ru>Hungarian LibreOffice Language Meta-Package</ru>
   <sk>Hungarian LibreOffice Language Meta-Package</sk>
   <sl>Hungarian LibreOffice Language Meta-Package</sl>
   <sq>Hungarian LibreOffice Language Meta-Package</sq>
   <sr>Hungarian LibreOffice Language Meta-Package</sr>
   <sv>Ungerskt LibreOffice Språk Meta-Paket </sv>
   <tr>Hungarian LibreOffice Language Meta-Package</tr>
   <uk>Hungarian LibreOffice Language Meta-Package</uk>
   <vi>Hungarian LibreOffice Language Meta-Package</vi>
   <zh_CN>Hungarian LibreOffice Language Meta-Package</zh_CN>
   <zh_TW>Hungarian LibreOffice Language Meta-Package</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-hu
libreoffice-help-hu
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-hu
libreoffice-help-hu
</uninstall_package_names>
</app>

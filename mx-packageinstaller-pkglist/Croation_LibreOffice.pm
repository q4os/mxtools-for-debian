<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Croatian_Libreoffice
</name>

<description>
   <am>Croatian LibreOffice Language Meta-Package</am>
   <ar>Croatian LibreOffice Language Meta-Package</ar>
   <bg>Croatian LibreOffice Language Meta-Package</bg>
   <bn>Croatian LibreOffice Language Meta-Package</bn>
   <ca>Localització de LibreOffice en Croata</ca>
   <cs>Croatian LibreOffice Language Meta-Package</cs>
   <da>Kroatisk LibreOffice sprog-metapakke</da>
   <de>Kroatisches LibreOffice Meta-Paket</de>
   <el>LibreOffice για Κροατική γλώσσα</el>
   <en>Croatian LibreOffice Language Meta-Package</en>
   <es>Meta paquete de lenguaje Croata para LibreOffice</es>
   <et>Croatian LibreOffice Language Meta-Package</et>
   <eu>Croatian LibreOffice Language Meta-Package</eu>
   <fa>Croatian LibreOffice Language Meta-Package</fa>
   <fil_PH>Croatian LibreOffice Language Meta-Package</fil_PH>
   <fi>Croatian LibreOffice Language Meta-Package</fi>
   <fr>Méta-Paquet langue croate pour LibreOffice</fr>
   <he_IL>Croatian LibreOffice Language Meta-Package</he_IL>
   <hi>Croatian LibreOffice Language Meta-Package</hi>
   <hr>Croatian LibreOffice Language Meta-Package</hr>
   <hu>Croatian LibreOffice Language Meta-Package</hu>
   <id>Croatian LibreOffice Language Meta-Package</id>
   <is>Croatian LibreOffice Language Meta-Package</is>
   <it>Meta-pacchetto della lingua croata per LibreOffice</it>
   <ja_JP>Croatian LibreOffice Language Meta-Package</ja_JP>
   <ja>Croatian LibreOffice Language Meta-Package</ja>
   <kk>Croatian LibreOffice Language Meta-Package</kk>
   <ko>Croatian LibreOffice Language Meta-Package</ko>
   <lt>Croatian LibreOffice Language Meta-Package</lt>
   <mk>Croatian LibreOffice Language Meta-Package</mk>
   <mr>Croatian LibreOffice Language Meta-Package</mr>
   <nb>Croatian LibreOffice Language Meta-Package</nb>
   <nl>Kroatische LibreOffice Taal Meta-Pakket</nl>
   <pl>Chorwacki metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Croata Meta-Pacote de Idioma para LibreOffice</pt_BR>
   <pt>Croata Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Croatian LibreOffice Language Meta-Package</ro>
   <ru>Croatian LibreOffice Language Meta-Package</ru>
   <sk>Croatian LibreOffice Language Meta-Package</sk>
   <sl>Croatian LibreOffice Language Meta-Package</sl>
   <sq>Croatian LibreOffice Language Meta-Package</sq>
   <sr>Croatian LibreOffice Language Meta-Package</sr>
   <sv>Kroatisk LibreOffice Språk Meta-Paket</sv>
   <tr>Croatian LibreOffice Language Meta-Package</tr>
   <uk>Croatian LibreOffice Language Meta-Package</uk>
   <vi>Croatian LibreOffice Language Meta-Package</vi>
   <zh_CN>Croatian LibreOffice Language Meta-Package</zh_CN>
   <zh_TW>Croatian LibreOffice Language Meta-Package</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-hr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-hr
</uninstall_package_names>
</app>

<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Southern_sotho_Libreoffice
</name>

<description>
   <am>Southern_sotho Language Meta-Package for LibreOffice</am>
   <ar>Southern_sotho Language Meta-Package for LibreOffice</ar>
   <bg>Southern_sotho Language Meta-Package for LibreOffice</bg>
   <bn>Southern_sotho Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet de Southern_sotho per LibreOffice</ca>
   <cs>Southern_sotho Language Meta-Package for LibreOffice</cs>
   <da>Southern_sotho Language Meta-Package for LibreOffice</da>
   <de>Süd-Sotho Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Southern Sotho</el>
   <en>Southern_sotho Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idiomas Southern_sotho para LibreOffice</es>
   <et>Southern_sotho Language Meta-Package for LibreOffice</et>
   <eu>Southern_sotho Language Meta-Package for LibreOffice</eu>
   <fa>Southern_sotho Language Meta-Package for LibreOffice</fa>
   <fil_PH>Southern_sotho Language Meta-Package for LibreOffice</fil_PH>
   <fi>Southern_sotho Language Meta-Package for LibreOffice</fi>
   <fr>Southern_sotho Language Meta-Package for LibreOffice</fr>
   <he_IL>Southern_sotho Language Meta-Package for LibreOffice</he_IL>
   <hi>Southern_sotho Language Meta-Package for LibreOffice</hi>
   <hr>Southern_sotho Language Meta-Package for LibreOffice</hr>
   <hu>Southern_sotho Language Meta-Package for LibreOffice</hu>
   <id>Southern_sotho Language Meta-Package for LibreOffice</id>
   <is>Southern_sotho Language Meta-Package for LibreOffice</is>
   <it>Southern_sotho Language Meta-Package for LibreOffice</it>
   <ja_JP>Southern_sotho Language Meta-Package for LibreOffice</ja_JP>
   <ja>Southern_sotho Language Meta-Package for LibreOffice</ja>
   <kk>Southern_sotho Language Meta-Package for LibreOffice</kk>
   <ko>Southern_sotho Language Meta-Package for LibreOffice</ko>
   <lt>Southern_sotho Language Meta-Package for LibreOffice</lt>
   <mk>Southern_sotho Language Meta-Package for LibreOffice</mk>
   <mr>Southern_sotho Language Meta-Package for LibreOffice</mr>
   <nb>Southern_sotho Language Meta-Package for LibreOffice</nb>
   <nl>Southern_sotho Language Meta-Package for LibreOffice</nl>
   <pl>Southern_sotho Language Meta-Package for LibreOffice</pl>
   <pt_BR>Southern_sotho Language Meta-Package for LibreOffice</pt_BR>
   <pt>Sotho meridional Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Southern_sotho Language Meta-Package for LibreOffice</ro>
   <ru>Southern_sotho Language Meta-Package for LibreOffice</ru>
   <sk>Southern_sotho Language Meta-Package for LibreOffice</sk>
   <sl>Južno-sotski jezikovni meta-paket za LibreOffice</sl>
   <sq>Southern_sotho Language Meta-Package for LibreOffice</sq>
   <sr>Southern_sotho Language Meta-Package for LibreOffice</sr>
   <sv>Southern_sotho Språk Meta-Paket för LibreOffice</sv>
   <tr>Southern_sotho Language Meta-Package for LibreOffice</tr>
   <uk>Southern_sotho Language Meta-Package for LibreOffice</uk>
   <vi>Southern_sotho Language Meta-Package for LibreOffice</vi>
   <zh_CN>Southern_sotho Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Southern_sotho Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-st
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-st
</uninstall_package_names>
</app>

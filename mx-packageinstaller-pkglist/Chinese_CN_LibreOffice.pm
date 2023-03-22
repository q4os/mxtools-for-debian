<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Chinese_CN_LO
</name>

<description>
   <am>Chinese_simplified localisation of LibreOffice</am>
   <ar>Chinese_simplified localisation of LibreOffice</ar>
   <bg>Chinese_simplified localisation of LibreOffice</bg>
   <bn>Chinese_simplified localisation of LibreOffice</bn>
   <ca>Localització de LibreOffice en xinès simplificat</ca>
   <cs>Chinese_simplified localisation of LibreOffice</cs>
   <da>Kinesisk (forenklet) oversættelse af LibreOffice</da>
   <de>Chinesisch (vereinfacht) Lokalisierung von LibreOffice</de>
   <el>LibreOffice για κινέζικα</el>
   <en>Chinese_simplified localisation of LibreOffice</en>
   <es>Localización Chino simplificado de LibreOffice</es>
   <et>Chinese_simplified localisation of LibreOffice</et>
   <eu>Chinese_simplified localisation of LibreOffice</eu>
   <fa>Chinese_simplified localisation of LibreOffice</fa>
   <fil_PH>Chinese_simplified localisation of LibreOffice</fil_PH>
   <fi>Chinese_simplified localisation of LibreOffice</fi>
   <fr>Localisation chinois-simplifié pour LibreOffice</fr>
   <he_IL>Chinese_simplified localisation of LibreOffice</he_IL>
   <hi>Chinese_simplified localisation of LibreOffice</hi>
   <hr>Chinese_simplified localisation of LibreOffice</hr>
   <hu>Chinese_simplified localisation of LibreOffice</hu>
   <id>Chinese_simplified localisation of LibreOffice</id>
   <is>Chinese_simplified localisation of LibreOffice</is>
   <it>Localizzazione in Cinese_semplificato di LibreOffice</it>
   <ja_JP>Chinese_simplified localisation of LibreOffice</ja_JP>
   <ja>Chinese_simplified localisation of LibreOffice</ja>
   <kk>Chinese_simplified localisation of LibreOffice</kk>
   <ko>Chinese_simplified localisation of LibreOffice</ko>
   <lt>Chinese_simplified localisation of LibreOffice</lt>
   <mk>Chinese_simplified localisation of LibreOffice</mk>
   <mr>Chinese_simplified localisation of LibreOffice</mr>
   <nb>Chinese_simplified localisation of LibreOffice</nb>
   <nl>Vereenvoudigd Chinese lokalisatie van LibreOffice</nl>
   <pl>Chiński uproszczony lokalizacja LibreOffice</pl>
   <pt_BR>Chinês_simplificado Localização para LibreOffice</pt_BR>
   <pt>Chinês_simplificado Localização para LibreOffice</pt>
   <ro>Chinese_simplified localisation of LibreOffice</ro>
   <ru>Chinese_simplified localisation of LibreOffice</ru>
   <sk>Chinese_simplified localisation of LibreOffice</sk>
   <sl>Chinese_simplified localisation of LibreOffice</sl>
   <sq>Chinese_simplified localisation of LibreOffice</sq>
   <sr>Chinese_simplified localisation of LibreOffice</sr>
   <sv>Kinesisk_förenklad lokalisering av LibreOffice</sv>
   <tr>Chinese_simplified localisation of LibreOffice</tr>
   <uk>Chinese_simplified локалізація LibreOffice</uk>
   <vi>Chinese_simplified localisation of LibreOffice</vi>
   <zh_CN>Chinese_simplified localisation of LibreOffice</zh_CN>
   <zh_TW>Chinese_simplified localisation of LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-zh-cn
libreoffice-help-zh-cn
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-zh-cn
libreoffice-help-zh-cn
</uninstall_package_names>
</app>

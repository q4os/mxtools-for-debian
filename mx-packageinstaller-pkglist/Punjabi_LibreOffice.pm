<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Punjabi_Libreoffice
</name>

<description>
   <am>Punjabi Language Meta-Package for LibreOffice</am>
   <ar>Punjabi Language Meta-Package for LibreOffice</ar>
   <bg>Punjabi Language Meta-Package for LibreOffice</bg>
   <bn>Punjabi Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Punjabi per LibreOffice</ca>
   <cs>Punjabi Language Meta-Package for LibreOffice</cs>
   <da>Punjabi Language Meta-Package for LibreOffice</da>
   <de>Punjabi Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Punjabi</el>
   <en>Punjabi Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma Punjabi para LibreOffice</es>
   <et>Punjabi Language Meta-Package for LibreOffice</et>
   <eu>Punjabi Language Meta-Package for LibreOffice</eu>
   <fa>Punjabi Language Meta-Package for LibreOffice</fa>
   <fil_PH>Punjabi Language Meta-Package for LibreOffice</fil_PH>
   <fi>Punjabinkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue punjabi pour LibreOffice</fr>
   <he_IL>Punjabi Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु पंजाबी भाषा मेटा-पैकेज</hi>
   <hr>Punjabi Language Meta-Package for LibreOffice</hr>
   <hu>Punjabi Language Meta-Package for LibreOffice</hu>
   <id>Punjabi Language Meta-Package for LibreOffice</id>
   <is>Punjabi Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua punjabi per LibreOffice</it>
   <ja>LibreOffice用パンジャブ語メタパッケージ</ja>
   <kk>Punjabi Language Meta-Package for LibreOffice</kk>
   <ko>Punjabi Language Meta-Package for LibreOffice</ko>
   <lt>Punjabi Language Meta-Package for LibreOffice</lt>
   <mk>Punjabi Language Meta-Package for LibreOffice</mk>
   <mr>Punjabi Language Meta-Package for LibreOffice</mr>
   <nb>Punjabi språkpakke for LibreOffice</nb>
   <nl>Punjabi Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka Punjabi dla LibreOffice</pl>
   <pt_BR>Punjabi Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Panjabi Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Punjabi Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет языка панджаби для LibreOffice</ru>
   <sk>Punjabi Language Meta-Package for LibreOffice</sk>
   <sl>Pundžabski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në panxhabi për LibreOffice-in</sq>
   <sr>Punjabi Language Meta-Package for LibreOffice</sr>
   <sv>Punjabi Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Pencap Dili Üst-Paketi</tr>
   <uk>Punjabi Language Meta-Package for LibreOffice</uk>
   <vi>Punjabi Language Meta-Package for LibreOffice</vi>
   <zh_CN>Punjabi Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Punjabi Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pa-in
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pa-in
</uninstall_package_names>
</app>

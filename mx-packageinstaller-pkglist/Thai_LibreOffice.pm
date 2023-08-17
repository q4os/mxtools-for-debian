<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Thai_Libreoffice
</name>

<description>
   <am>Thai Language Meta-Package for LibreOffice</am>
   <ar>Thai Language Meta-Package for LibreOffice</ar>
   <bg>Thai Language Meta-Package for LibreOffice</bg>
   <bn>Thai Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Thai per LibreOffice</ca>
   <cs>Thai Language Meta-Package for LibreOffice</cs>
   <da>Thai Language Meta-Package for LibreOffice</da>
   <de>Thai Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Ταϊλανδέζικα</el>
   <en>Thai Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma tailandés para LibreOffice</es>
   <et>Thai Language Meta-Package for LibreOffice</et>
   <eu>Thai Language Meta-Package for LibreOffice</eu>
   <fa>Thai Language Meta-Package for LibreOffice</fa>
   <fil_PH>Thai Language Meta-Package for LibreOffice</fil_PH>
   <fi>Thainkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue thaïlandaise pour LibreOffice</fr>
   <he_IL>Thai Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु थाई भाषा मेटा-पैकेज</hi>
   <hr>Thai Language Meta-Package for LibreOffice</hr>
   <hu>Thai Language Meta-Package for LibreOffice</hu>
   <id>Thai Language Meta-Package for LibreOffice</id>
   <is>Thai Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua tailandese per LibreOffice</it>
   <ja>LibreOffice 用のタイ語メタパッケージ</ja>
   <kk>Thai Language Meta-Package for LibreOffice</kk>
   <ko>Thai Language Meta-Package for LibreOffice</ko>
   <lt>Thai Language Meta-Package for LibreOffice</lt>
   <mk>Thai Language Meta-Package for LibreOffice</mk>
   <mr>Thai Language Meta-Package for LibreOffice</mr>
   <nb>Thai språkpakke for LibreOffice</nb>
   <nl>Thais Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Tajski meta-pakiet językowy dla LibreOffice</pl>
   <pt_BR>Tailandês Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Tailandês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Thai Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет тайского языка для LibreOffice</ru>
   <sk>Thai Language Meta-Package for LibreOffice</sk>
   <sl>Tajski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në tajlandisht për LibreOffice-in</sq>
   <sr>Thai Language Meta-Package for LibreOffice</sr>
   <sv>Thai Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Tayca Dili Üst-Paketi</tr>
   <uk>Thai Language Meta-Package for LibreOffice</uk>
   <vi>Thai Language Meta-Package for LibreOffice</vi>
   <zh_CN>Thai Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Thai Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-th
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-th
</uninstall_package_names>
</app>

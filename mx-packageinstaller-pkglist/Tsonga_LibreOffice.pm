<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Tsonga_Libreoffice
</name>

<description>
   <am>Tsonga Language Meta-Package for LibreOffice</am>
   <ar>Tsonga Language Meta-Package for LibreOffice</ar>
   <bg>Tsonga Language Meta-Package for LibreOffice</bg>
   <bn>Tsonga Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Tsonga per LibreOffice</ca>
   <cs>Tsonga Language Meta-Package for LibreOffice</cs>
   <da>Tsonga Language Meta-Package for LibreOffice</da>
   <de>Tsonga Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Tsonga</el>
   <en>Tsonga Language Meta-Package for LibreOffice</en>
   <es>Meta-Package idioma songa para LibreOffice</es>
   <et>Tsonga Language Meta-Package for LibreOffice</et>
   <eu>Tsonga Language Meta-Package for LibreOffice</eu>
   <fa>Tsonga Language Meta-Package for LibreOffice</fa>
   <fil_PH>Tsonga Language Meta-Package for LibreOffice</fil_PH>
   <fi>Tsongankielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue Tsonga pour LibreOffice</fr>
   <he_IL>Tsonga Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु सोंगा भाषा मेटा-पैकेज</hi>
   <hr>Tsonga Language Meta-Package for LibreOffice</hr>
   <hu>Tsonga Language Meta-Package for LibreOffice</hu>
   <id>Tsonga Language Meta-Package for LibreOffice</id>
   <is>Tsonga Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua tsonga per LibreOffice</it>
   <ja>LibreOffice 用のツォンガ語メタパッケージ</ja>
   <kk>Tsonga Language Meta-Package for LibreOffice</kk>
   <ko>Tsonga Language Meta-Package for LibreOffice</ko>
   <lt>Tsonga Language Meta-Package for LibreOffice</lt>
   <mk>Tsonga Language Meta-Package for LibreOffice</mk>
   <mr>Tsonga Language Meta-Package for LibreOffice</mr>
   <nb>Tsonga språkpakke for LibreOffice</nb>
   <nl>Tsonga Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka Tsonga dla LibreOffice</pl>
   <pt_BR>Tsonga Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Tsonga Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Tsonga Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет языка тсонга для LibreOffice</ru>
   <sk>Tsonga Language Meta-Package for LibreOffice</sk>
   <sl>Tsongški jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në conga për LibreOffice-in</sq>
   <sr>Tsonga Language Meta-Package for LibreOffice</sr>
   <sv>Tsonga Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Tsonga Dili Üst-Paketi</tr>
   <uk>Tsonga Language Meta-Package for LibreOffice</uk>
   <vi>Tsonga Language Meta-Package for LibreOffice</vi>
   <zh_CN>Tsonga Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Tsonga Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ts
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ts
</uninstall_package_names>
</app>

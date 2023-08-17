<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Marathi_Libreoffice
</name>

<description>
   <am>Marathi Language Meta-Package for LibreOffice</am>
   <ar>Marathi Language Meta-Package for LibreOffice</ar>
   <bg>Marathi Language Meta-Package for LibreOffice</bg>
   <bn>Marathi Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Marathi per LibreOffice</ca>
   <cs>Marathi Language Meta-Package for LibreOffice</cs>
   <da>Marathi Language Meta-Package for LibreOffice</da>
   <de>Marathi Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Marathi</el>
   <en>Marathi Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma maratí para LibreOffice</es>
   <et>Marathi Language Meta-Package for LibreOffice</et>
   <eu>Marathi Language Meta-Package for LibreOffice</eu>
   <fa>Marathi Language Meta-Package for LibreOffice</fa>
   <fil_PH>Marathi Language Meta-Package for LibreOffice</fil_PH>
   <fi> Marathinkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue marathi pour LibreOffice</fr>
   <he_IL>Marathi Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मराठी भाषा मेटा-पैकेज</hi>
   <hr>Marathi Language Meta-Package for LibreOffice</hr>
   <hu>Marathi Language Meta-Package for LibreOffice</hu>
   <id>Marathi Language Meta-Package for LibreOffice</id>
   <is>Marathi Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua marathi per LibreOffice</it>
   <ja>LibreOffice 用のマラーティー語メタパッケージ</ja>
   <kk>Marathi Language Meta-Package for LibreOffice</kk>
   <ko>Marathi Language Meta-Package for LibreOffice</ko>
   <lt>Marathi Language Meta-Package for LibreOffice</lt>
   <mk>Marathi Language Meta-Package for LibreOffice</mk>
   <mr>Marathi Language Meta-Package for LibreOffice</mr>
   <nb>Marathi språkpakke for LibreOffice</nb>
   <nl>Marathi Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka Marathi dla LibreOffice</pl>
   <pt_BR>Marata Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Marata Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Marathi Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет языка маратхи для LibreOffice</ru>
   <sk>Marathi Language Meta-Package for LibreOffice</sk>
   <sl>Maratski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në marati për LibreOffice-in</sq>
   <sr>Marathi Language Meta-Package for LibreOffice</sr>
   <sv>Marathi Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Marathi Dili Üst-Paketi</tr>
   <uk>Marathi Language Meta-Package for LibreOffice</uk>
   <vi>Marathi Language Meta-Package for LibreOffice</vi>
   <zh_CN>Marathi Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Marathi Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-mr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-mr
</uninstall_package_names>
</app>
